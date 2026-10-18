#include "level_set_render_cuda.h"
#include <nanovdb/tools/CreateNanoGrid.h>

#include <cuda_runtime.h>
#include <nanovdb/tools/CreateNanoGrid.h>
#include <nanovdb/cuda/DeviceBuffer.h>

#include <stdexcept>
#include <vector>

namespace lux{


// -------------------------------------------------------------
// A cuda demo
// -------------------------------------------------------------


// #define CUDA_CHECK(call)                                      \
//     do {                                                      \
//         cudaError_t err = (call);                             \
//         if (err != cudaSuccess)                               \
//             throw std::runtime_error(cudaGetErrorString(err));\
//     } while (0)


// -------------------------------------------------------------
// Launching cuda render code
// -------------------------------------------------------------

void cuda_render_bunny(const std::string& path, const std::string& name){
    float limit = 4.0;
    float density_vs = 0.025;
    float color_vs = 0.025;
    float sm_vs = 0.025;
    int width = 1920;
    int height = 1080;
    float near_dist = 6;
    float far_dist = 15;
    float min_step = 0.01;
    float max_step = min_step * 5.0;
    float kappa = 3.0;
    float shadow_kappa = 3.0;
    float shadow_step = 0.01;
    printf("Stamping\n");
    auto bunn = bunny_info();

    auto dens_bounds = world_space_to_bounds({-limit, -limit, -limit}, {limit, limit, limit}, density_vs);
    auto col_bounds = world_space_to_bounds({-limit, -limit, -limit}, {limit, limit, limit}, color_vs);
    auto sm_bounds = world_space_to_bounds({-limit, -limit, -limit}, {limit, limit, limit}, sm_vs);

    // Now let's stamp the density into nanovdb
    auto dens = bunn.density;
    auto dens_grid = stamp_isf_to_grid(density_vs, 0.00001f, dens, dens_bounds);
    bunn.color = make_constant<Color>(Color(1, 1, 0.5, 0));
    auto col_grid = stamp_color_to_grid(bunn.color, col_bounds, color_vs);


    // ----------------------------------------------------------------
    // Packing the data off to the GPU
    // ----------------------------------------------------------------
    cudaStream_t stream{};
    CUDA_CHECK(cudaStreamCreate(&stream));
    
    auto densityNano = nanovdb::tools::createNanoGrid(*dens_grid);
    auto colorNano = nanovdb::tools::createNanoGrid(*col_grid);

    const std::size_t densityBytes = static_cast<std::size_t>(densityNano.size());
    const std::size_t colorBytes = static_cast<std::size_t>(colorNano.size());
    void* densityBuffer = nullptr;
    CUDA_CHECK(cudaMalloc(&densityBuffer, densityBytes));
    CUDA_CHECK(cudaMemcpyAsync(
        densityBuffer,
        densityNano.data(),
        densityBytes,
        cudaMemcpyHostToDevice,
        stream)
    );
    
    void* colorBuffer = nullptr;
    CUDA_CHECK(cudaMalloc(&colorBuffer, colorBytes));
    CUDA_CHECK(cudaMemcpyAsync(
        colorBuffer,
        colorNano.data(),
        colorBytes,
        cudaMemcpyHostToDevice,
        stream)
    );
    printf("Density and color sent to gpu\n");

    // ----------------------------------------------------------------
    // Shadow map packing
    // ----------------------------------------------------------------

    const auto lo = sm_bounds.getStart();
    const auto hi = sm_bounds.getEnd(); // inclusive

    const int nx = hi[0] - lo[0] + 1;
    const int ny = hi[1] - lo[1] + 1;
    const int nz = hi[2] - lo[2] + 1;

    const float3 origin = make_float3(lo[0] * sm_vs, lo[1] * sm_vs, lo[2] * sm_vs);
    const int count = nx * ny * nz;
    const std::size_t shadowBytes = static_cast<std::size_t>(count * sizeof(float));
    float* shadowBuffer = nullptr;
    CUDA_CHECK(cudaMalloc(&shadowBuffer, shadowBytes));
    printf("Starting cuda shadow map\n");
    CUDA_CHECK(launch_cuda_shadowmap(
        reinterpret_cast<const nanovdb::NanoGrid<float>*>(densityBuffer),
        shadowBuffer,
        sm_bounds,
        {origin.x, origin.y, origin.z},
        sm_vs,
        {0, 3, 0},
        shadow_step,
        shadow_kappa,
        stream
    ));
    std::vector<float> host_shadow(count);
    CUDA_CHECK(cudaStreamSynchronize(stream));
    printf("Shadow map completed. Copying\n");

    CUDA_CHECK(cudaMemcpy(
        host_shadow.data(),
        shadowBuffer,
        shadowBytes,
        cudaMemcpyDeviceToHost
    ));
    CUDA_CHECK(cudaFree(shadowBuffer));
    
    auto shadow_grid = create_float_grid(sm_vs, 0.0f);
    auto accessor = shadow_grid->getAccessor();
    for (int z = 0; z < nz; ++z) {
        for (int y = 0; y < ny; ++y) {
            for (int x = 0; x < nx; ++x) {
                const int index = (z * ny + y) * nx + x;
                const float transmission = host_shadow[index];
                if (std::abs(transmission) > 1e-6f) {
                    accessor.setValue(
                        openvdb::Coord(x + lo[0], y + lo[1], z + lo[2]),
                        transmission
                    );
                }
            }
        }
    }


    auto shadow_nano = nanovdb::tools::createNanoGrid(*shadow_grid);
    auto device_shadow_handle = nanovdb::cuda::copyTo<nanovdb::cuda::Buffer<std::byte>>(shadow_nano, stream);

    const auto* device_shadow_grid = device_shadow_handle.deviceGrid<float>();
    const nanovdb::NanoGrid<float>* shadow_pointer_array[] = { device_shadow_grid };
    const nanovdb::NanoGrid<float>** device_shadow_pointer_array = nullptr;
    CUDA_CHECK(cudaMalloc(
        &device_shadow_pointer_array,
        sizeof(shadow_pointer_array)
    ));

    CUDA_CHECK(cudaMemcpy(
        device_shadow_pointer_array,
        shadow_pointer_array,
        sizeof(shadow_pointer_array),
        cudaMemcpyHostToDevice
    ));
    printf("Copy completed\n");

    // Now adding in the same nuber of lights as shadow maps
    // this should be done through a lights struct that has position and color but oh well
    const float3 light_colors[] = {make_float3(0.2f, 0.2f, 1.0f)}; 
    float3* device_light_colors = nullptr;
    CUDA_CHECK(cudaMalloc(
        &device_light_colors,
        sizeof(light_colors)
    ));
    CUDA_CHECK(cudaMemcpy(
        device_light_colors,
        light_colors,
        sizeof(light_colors),
        cudaMemcpyHostToDevice
    ));

    GPUVolume volume{};
    volume.density = reinterpret_cast<const nanovdb::NanoGrid<float>*>(densityBuffer);
    volume.color = reinterpret_cast<const nanovdb::NanoGrid<nanovdb::Vec3f>*>(colorBuffer);
    volume.shadow_maps = device_shadow_pointer_array;
    volume.light_colors = device_light_colors;
    volume.shadow_map_count = 1;

    
    // -----------------------------------------------
    // Getting the cam and image into the GPU
    // ----------------------------------------------------------------

    Camera cam;
    Vector eye = Vector(0, 0, 10);
    Vector view = Vector(0, 0, -1);
    cam.setEyeViewUp(eye, view, Vector(0, 1, 0));
    const auto right = cam.right();
    const auto up    = cam.up();

    GPUCamera gpuCam(cam);

    std::vector<float4> pixels(width * height);
    float4* devicePixels = nullptr;

    CUDA_CHECK(cudaMalloc(
        reinterpret_cast<void**>(&devicePixels),
        pixels.size() * sizeof(float4))
    );

    // ----------------------------------------------------------------
    // The actual render
    // ----------------------------------------------------------------

    CUDA_CHECK(launch_cuda_raymarch(
        volume, devicePixels, width, height, gpuCam,
        near_dist, far_dist, min_step, max_step, kappa, stream));

    CUDA_CHECK(cudaStreamSynchronize(stream));
    CUDA_CHECK(cudaMemcpy(
        pixels.data(), devicePixels,
        pixels.size() * sizeof(float4),
        cudaMemcpyDeviceToHost)
    );

    // ----------------------------------------------------------------
    // Saving out the final image
    // ----------------------------------------------------------------

    ImageData img(width, height, 4);
    for (int i = 0; i < width * height; ++i) {
        const int x = i % width;
        const int y = i / width;
        ImageData::pixel p = {pixels[i].x, pixels[i].y, pixels[i].z, pixels[i].w};
        img.set_pixel_values(x, y, p);
    }

    std::string frame_filename = path + name + "." + StringFuncs::get_zero_padded_number_string(0, 4) + ".exr";
    img.oiio_write_to(frame_filename);
    std::cout << "Wrote to " << frame_filename << std::endl;

    device_shadow_handle.buffer().destroy(stream);
    CUDA_CHECK(cudaStreamSynchronize(stream));

    CUDA_CHECK(cudaFree(device_shadow_pointer_array));
    CUDA_CHECK(cudaFree(densityBuffer));
    CUDA_CHECK(cudaFree(colorBuffer));
    CUDA_CHECK(cudaFree(devicePixels));
    CUDA_CHECK(cudaFree(device_light_colors));

    CUDA_CHECK(cudaStreamDestroy(stream));
}


void cuda_render_tests(const std::string& filename){
    // Just keep as default for now
    RenderSettings rs;

    // Defining the density and color
    auto bunn = bunny_info();
    auto dens = bunn.density;
    auto dens_grid = stamp_isf_to_grid(rs.get_density_vs(), 0.00001f, dens, rs.get_density_bounds());
    bunn.color = make_constant<Color>(Color(1, 1, 0.5, 0));
    auto col_grid = stamp_color_to_grid(bunn.color, rs.get_color_bounds(), rs.get_color_vs());
    Camera cam;
    Vector eye = Vector(0, 0, 10);
    Vector view = Vector(0, 0, -1);
    cam.setEyeViewUp(eye, view, Vector(0, 1, 0));
    std::vector<PointLight> lights = {
        PointLight({1, 0, 0, 0}, {-3, 0, 0}),
        PointLight({0, 1, 0, 0}, {3, 0, 0})
    };
    cuda_render_frame(rs, dens_grid, col_grid, lights, cam, filename);

}

void cuda_render_frame(
    const RenderSettings& rs,
    const GridPtr<float> dens_grid,
    const GridPtr<Color> col_grid,
    const std::vector<PointLight>& lights,
    const Camera& cam,
    const std::string& filename
){
    cudaStream_t stream{};
    CUDA_CHECK(cudaStreamCreate(&stream));

    // density and color packing
    auto density_handle = pack_grid_to_nano(dens_grid, stream);
    auto color_handle = pack_grid_to_nano(col_grid, stream);

    auto device_shadow_array = create_shadow_maps_and_send_to_gpu(
        density_handle.deviceGrid<float>(),
        lights, rs, stream
    );
    
    std::vector<float3> host_light_colors;
    host_light_colors.reserve(lights.size());

    for (const auto& light : lights) {
        host_light_colors.push_back(make_float3(
            light.color.red(),
            light.color.green(),
            light.color.blue()
        ));
    }
    float3* device_light_colors = send_array_to_gpu(host_light_colors.data(), host_light_colors.size());

    // GPU volume for ray marching
    GPUVolume volume{};
    volume.density = density_handle.deviceGrid<float>();
    volume.color = color_handle.deviceGrid<nanovdb::Vec3f>();
    volume.shadow_maps = device_shadow_array;
    volume.shadow_map_count = lights.size();
    volume.light_colors = device_light_colors;

    // -----------------------------------------------
    // Getting the cam and image into the GPU
    // ----------------------------------------------------------------
    GPUCamera gpuCam(cam);

    std::vector<float4> pixels(rs.width * rs.height);
    float4* devicePixels = nullptr;

    CUDA_CHECK(cudaMalloc(
        reinterpret_cast<void**>(&devicePixels),
        pixels.size() * sizeof(float4))
    );

    // ----------------------------------------------------------------
    // The actual render
    // ----------------------------------------------------------------

    CUDA_CHECK(launch_cuda_raymarch(
        volume, devicePixels, rs.width, rs.height, gpuCam,
        rs.near_dist, rs.far_dist, rs.min_step, rs.max_step, rs.kappa, stream));

    CUDA_CHECK(cudaStreamSynchronize(stream));
    CUDA_CHECK(cudaMemcpy(
        pixels.data(), devicePixels,
        pixels.size() * sizeof(float4),
        cudaMemcpyDeviceToHost)
    );

    // ----------------------------------------------------------------
    // Saving out the final image
    // ----------------------------------------------------------------

    ImageData img(rs.width, rs.height, 4);
    for (int i = 0; i < rs.width * rs.height; ++i) {
        const int x = i % rs.width;
        const int y = i / rs.width;
        ImageData::pixel p = {pixels[i].x, pixels[i].y, pixels[i].z, pixels[i].w};
        img.set_pixel_values(x, y, p);
    }
    //std::string frame_filename = path + name + "." + StringFuncs::get_zero_padded_number_string(0, 4) + ".exr";
    img.oiio_write_to(filename);
    std::cout << "Wrote to " << filename << std::endl;

    CUDA_CHECK(cudaFree(devicePixels));
    CUDA_CHECK(cudaFree(device_shadow_array));
    CUDA_CHECK(cudaFree(device_light_colors));
    CUDA_CHECK(cudaStreamDestroy(stream));
}

} // end namespace lux