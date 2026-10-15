#include "level_set_render_cuda.h"

#include "openvdb_helpers.h"

#include <nanovdb/tools/CreateNanoGrid.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace lux {
namespace {

void checkCuda(cudaError_t result, const char* operation)
{
    if (result != cudaSuccess) {
        throw std::runtime_error(
            std::string(operation) + ": " + cudaGetErrorString(result));
    }
}

class CudaStream {
public:
    CudaStream()
    {
        checkCuda(cudaStreamCreate(&stream_), "create CUDA stream");
    }

    ~CudaStream()
    {
        if (stream_) cudaStreamDestroy(stream_);
    }

    CudaStream(const CudaStream&) = delete;
    CudaStream& operator=(const CudaStream&) = delete;

    cudaStream_t get() const noexcept { return stream_; }

private:
    cudaStream_t stream_ = nullptr;
};

template<typename T>
class CudaBuffer {
public:
    CudaBuffer() = default;

    explicit CudaBuffer(std::size_t count)
        : count_(count)
    {
        if (count_ != 0) {
            if (count_ > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
                throw std::overflow_error("CUDA buffer size overflow");
            }
            checkCuda(
                cudaMalloc(reinterpret_cast<void**>(&data_), count_ * sizeof(T)),
                "allocate CUDA buffer");
        }
    }

    ~CudaBuffer()
    {
        if (data_) cudaFree(data_);
    }

    CudaBuffer(const CudaBuffer&) = delete;
    CudaBuffer& operator=(const CudaBuffer&) = delete;

    CudaBuffer(CudaBuffer&& other) noexcept
        : data_(std::exchange(other.data_, nullptr))
        , count_(std::exchange(other.count_, 0))
    {
    }

    CudaBuffer& operator=(CudaBuffer&& other) noexcept
    {
        if (this == &other) return *this;
        if (data_) cudaFree(data_);
        data_ = std::exchange(other.data_, nullptr);
        count_ = std::exchange(other.count_, 0);
        return *this;
    }

    T* get() noexcept { return data_; }
    const T* get() const noexcept { return data_; }
    std::size_t size() const noexcept { return count_; }

private:
    T* data_ = nullptr;
    std::size_t count_ = 0;
};

template<typename NanoGridHandle>
CudaBuffer<std::byte> uploadNanoGrid(
    const NanoGridHandle& host_grid,
    cudaStream_t stream)
{
    const std::size_t bytes = static_cast<std::size_t>(host_grid.size());
    CudaBuffer<std::byte> device_grid(bytes);
    checkCuda(
        cudaMemcpyAsync(
            device_grid.get(),
            host_grid.data(),
            bytes,
            cudaMemcpyHostToDevice,
            stream),
        "upload NanoVDB grid");
    return device_grid;
}

openvdb::CoordBBox centeredBounds(float half_extent, float voxel_size)
{
    const Vector lower(-half_extent, -half_extent, -half_extent);
    const Vector upper(half_extent, half_extent, half_extent);
    return world_space_to_bounds(lower, upper, voxel_size);
}

int checkedDimension(int minimum, int maximum)
{
    const long long dimension =
        static_cast<long long>(maximum) - minimum + 1;
    if (dimension <= 0 || dimension > std::numeric_limits<int>::max()) {
        throw std::runtime_error("invalid shadow-map dimensions");
    }
    return static_cast<int>(dimension);
}

ShadowMapLayout makeShadowMapLayout(
    const openvdb::FloatGrid& density_grid,
    float voxel_size)
{
    const openvdb::CoordBBox active = density_grid.evalActiveVoxelBoundingBox();
    if (active.empty()) {
        throw std::runtime_error(
            "density grid has no active voxels; cannot build shadow maps");
    }

    const openvdb::Vec3d a = density_grid.indexToWorld(active.min());
    const openvdb::Vec3d b = density_grid.indexToWorld(active.max());
    const double padding = 2.0 * voxel_size;

    const int min_x = static_cast<int>(
        std::floor((std::min(a.x(), b.x()) - padding) / voxel_size));
    const int min_y = static_cast<int>(
        std::floor((std::min(a.y(), b.y()) - padding) / voxel_size));
    const int min_z = static_cast<int>(
        std::floor((std::min(a.z(), b.z()) - padding) / voxel_size));
    const int max_x = static_cast<int>(
        std::ceil((std::max(a.x(), b.x()) + padding) / voxel_size));
    const int max_y = static_cast<int>(
        std::ceil((std::max(a.y(), b.y()) + padding) / voxel_size));
    const int max_z = static_cast<int>(
        std::ceil((std::max(a.z(), b.z()) + padding) / voxel_size));

    ShadowMapLayout layout{};
    layout.dimensions = make_int3(
        checkedDimension(min_x, max_x),
        checkedDimension(min_y, max_y),
        checkedDimension(min_z, max_z));
    layout.origin = make_float3(
        min_x * voxel_size,
        min_y * voxel_size,
        min_z * voxel_size);
    layout.voxel_size = voxel_size;
    return layout;
}

std::size_t shadowMapSampleCount(const ShadowMapLayout& layout)
{
    const std::size_t x = static_cast<std::size_t>(layout.dimensions.x);
    const std::size_t y = static_cast<std::size_t>(layout.dimensions.y);
    const std::size_t z = static_cast<std::size_t>(layout.dimensions.z);
    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    if (x == 0 || y == 0 || z == 0 || y > maximum / x ||
        z > maximum / (x * y)) {
        throw std::overflow_error("shadow-map size overflow");
    }
    return x * y * z;
}

GPUCamera makeCamera()
{
    Camera camera;
    const Vector eye(0.0, 0.0, 10.0);
    const Vector view(0.0, 0.0, -1.0);
    camera.setEyeViewUp(eye, view, Vector(0.0, 1.0, 0.0));

    const Vector right = camera.right();
    const Vector up = camera.up();
    return GPUCamera{
        make_float3(eye.x(), eye.y(), eye.z()),
        make_float3(right.x(), right.y(), right.z()),
        make_float3(up.x(), up.y(), up.z()),
        make_float3(view.x(), view.y(), view.z()),
        camera.get_htanfov(),
        camera.get_vtanfov()};
}

std::vector<float4> renderPixels(
    const GPUVolume& volume,
    const GPUCamera& camera,
    const CudaRenderSettings& settings,
    cudaStream_t stream)
{
    const std::size_t pixel_count =
        static_cast<std::size_t>(settings.width) * settings.height;
    std::vector<float4> pixels(pixel_count);
    CudaBuffer<float4> device_pixels(pixel_count);

    const RaymarchParameters parameters{
        settings.width,
        settings.height,
        settings.near_distance,
        settings.far_distance,
        settings.min_step,
        settings.max_step,
        settings.extinction};
    checkCuda(
        launch_cuda_raymarch(
            volume, device_pixels.get(), camera, parameters, stream),
        "launch CUDA ray marcher");
    checkCuda(cudaStreamSynchronize(stream), "wait for CUDA render");
    checkCuda(
        cudaMemcpy(
            pixels.data(),
            device_pixels.get(),
            pixel_count * sizeof(float4),
            cudaMemcpyDeviceToHost),
        "copy rendered pixels to host");
    return pixels;
}

void writeImage(
    const std::string& path,
    const std::string& name,
    int width,
    int height,
    const std::vector<float4>& pixels)
{
    ImageData image(width, height, 4);
    for (int i = 0; i < width * height; ++i) {
        const int x = i % width;
        const int y = i / width;
        const ImageData::pixel pixel{
            pixels[i].x, pixels[i].y, pixels[i].z, pixels[i].w};
        image.set_pixel_values(x, y, pixel);
    }

    const std::string filename =
        path + name + "." +
        StringFuncs::get_zero_padded_number_string(0, 4) + ".exr";
    image.oiio_write_to(filename);
    std::cout << "Wrote to " << filename << std::endl;
}

void validateSettings(const CudaRenderSettings& settings)
{
    const long long pixel_count =
        static_cast<long long>(settings.width) * settings.height;
    if (settings.width <= 0 || settings.height <= 0 ||
        pixel_count > std::numeric_limits<int>::max() ||
        settings.volume_half_extent <= 0.0f ||
        settings.density_voxel_size <= 0.0f ||
        settings.color_voxel_size <= 0.0f ||
        settings.shadow_voxel_size <= 0.0f ||
        settings.shadow_step_size <= 0.0f ||
        settings.min_step <= 0.0f ||
        settings.max_step < settings.min_step ||
        settings.near_distance >= settings.far_distance ||
        settings.extinction < 0.0f ||
        settings.shadow_extinction < 0.0f) {
        throw std::invalid_argument("invalid CUDA render settings");
    }
}

} // namespace

void cuda_render_bunny(
    const std::string& path,
    const std::string& name,
    const std::vector<CudaPointLight>& lights,
    const CudaRenderSettings& settings)
{
    validateSettings(settings);
    if (lights.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("too many CUDA lights");
    }
    for (const CudaPointLight& light : lights) {
        if (!(light.intensity >= 0.0f)) {
            throw std::invalid_argument("CUDA light intensity must be nonnegative");
        }
    }

    auto bunny = bunny_info();
    auto density_bounds = centeredBounds(settings.volume_half_extent, settings.density_voxel_size);
    auto color_bounds = centeredBounds(settings.volume_half_extent, settings.color_voxel_size);

    auto density_grid = stamp_isf_to_grid(
        settings.density_voxel_size,
        1.0e-5f,
        bunny.density,
        density_bounds
    );
    auto color_field = make_constant<Color>(Color(
        settings.surface_color.x,
        settings.surface_color.y,
        settings.surface_color.z,
        0.0f
    ));
    auto color_grid = stamp_color_to_grid(
        color_field, color_bounds, settings.color_voxel_size
    );

    CudaStream stream;
    auto density_nano = nanovdb::tools::createNanoGrid(*density_grid);
    auto color_nano = nanovdb::tools::createNanoGrid(*color_grid);
    auto device_density = uploadNanoGrid(density_nano, stream.get());
    auto device_color = uploadNanoGrid(color_nano, stream.get());

    const ShadowMapLayout shadow_layout = makeShadowMapLayout(*density_grid, settings.shadow_voxel_size);
    const std::size_t shadow_count = shadowMapSampleCount(shadow_layout);

    std::vector<CudaBuffer<float>> shadow_maps;
    std::vector<GPUPointLight> gpu_lights;
    shadow_maps.reserve(lights.size());
    gpu_lights.reserve(lights.size());

    const auto* device_density_grid = reinterpret_cast<const nanovdb::NanoGrid<float>*>(device_density.get());
    for (const CudaPointLight& light : lights) {
        CudaBuffer<float> shadow_map(shadow_count);
        checkCuda(
            launch_cuda_shadowmap(
                device_density_grid,
                shadow_map.get(),
                shadow_layout,
                light.position,
                settings.shadow_step_size,
                settings.shadow_extinction,
                stream.get()),
            "launch CUDA shadow-map pass");

        gpu_lights.push_back(GPUPointLight{
            light.position,
            light.color,
            light.intensity,
            shadow_map.get()});
        shadow_maps.emplace_back(std::move(shadow_map));
    }

    CudaBuffer<GPUPointLight> device_lights(gpu_lights.size());
    if (!gpu_lights.empty()) {
        checkCuda(
            cudaMemcpyAsync(
                device_lights.get(),
                gpu_lights.data(),
                gpu_lights.size() * sizeof(GPUPointLight),
                cudaMemcpyHostToDevice,
                stream.get()),
            "upload CUDA lights");
    }

    const GPUVolume volume{
        device_density_grid,
        reinterpret_cast<const nanovdb::NanoGrid<nanovdb::Vec3f>*>(
            device_color.get()),
        device_lights.get(),
        static_cast<int>(gpu_lights.size()),
        shadow_layout};
    const GPUCamera camera = makeCamera();
    const std::vector<float4> pixels =
        renderPixels(volume, camera, settings, stream.get());
    writeImage(path, name, settings.width, settings.height, pixels);
}

void cuda_render_bunny(const std::string& path, const std::string& name)
{
    const std::vector<CudaPointLight> lights{
        CudaPointLight{
            make_float3(0.0f, 3.0f, 0.0f),
            make_float3(0.0f, 1.0f, 0.0f),
            100.0f}};
    cuda_render_bunny(path, name, lights, CudaRenderSettings{});
}

} // namespace lux
