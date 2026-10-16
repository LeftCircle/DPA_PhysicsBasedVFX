#include "level_set_render_cuda.h"
#include <nanovdb/tools/CreateNanoGrid.h>

#include <cuda_runtime.h>
#include <nanovdb/tools/CreateNanoGrid.h>

#include <stdexcept>
#include <vector>

namespace lux{


// -------------------------------------------------------------
// A cuda demo
// -------------------------------------------------------------


#define CUDA_CHECK(call)                                      \
    do {                                                      \
        cudaError_t err = (call);                             \
        if (err != cudaSuccess)                               \
            throw std::runtime_error(cudaGetErrorString(err));\
    } while (0)

// std::vector<float> thresholdLine(
//     const openvdb::FloatGrid& grid,
//     nanovdb::Coord first,
//     int count,
//     float threshold)
// {
//     // Convert the CPU OpenVDB grid to a NanoVDB grid in host memory.
//     auto hostHandle = nanovdb::tools::createNanoGrid(grid);
//     if (!hostHandle.grid<float>()){
//         throw std::runtime_error("NanoVDB conversion did not produce a float grid");
//     }
//     // Copy NanoVDB's buffer to device memory.
//     cudaStream_t stream{};
//     CUDA_CHECK(cudaStreamCreate(&stream));

//     auto deviceHandle = nanovdb::cuda::copyTo<nanovdb::cuda::Buffer<std::byte>>(hostHandle, stream);

//     const auto* deviceGrid = deviceHandle.deviceGrid<float>();
//     if (!deviceGrid){
//         throw std::runtime_error("Could not get the device NanoVDB grid");
//     }

//     float* deviceOutput = nullptr;
//     CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&deviceOutput), count * sizeof(float)));

//     // Actually launch the cuda calculation
//     CUDA_CHECK(launchThreshold(
//         deviceGrid, deviceOutput, first, count, threshold, stream
//     ));

//     std::vector<float> result(count);

//     // Wait for the kernel before copying results or releasing the grid handle.
//     CUDA_CHECK(cudaStreamSynchronize(stream));
//     CUDA_CHECK(
//         cudaMemcpy(
//             result.data(),
//             deviceOutput,
//             count * sizeof(float),
//             cudaMemcpyDeviceToHost
//         )
//     );

//     CUDA_CHECK(cudaFree(deviceOutput));
//     CUDA_CHECK(cudaStreamDestroy(stream));

//     return result;
// }

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
    // let's make the shadow map as big as the color map for now

    auto dims = sm_bounds.getEnd() - sm_bounds.getStart();
    const auto lo = sm_bounds.getStart();
    const auto hi = sm_bounds.getEnd(); // inclusive

    const int nx = hi[0] - lo[0] + 1;
    const int ny = hi[1] - lo[1] + 1;
    const int nz = hi[2] - lo[2] + 1;

    const float3 origin = make_float3(lo[0] * sm_vs, lo[1] * sm_vs, lo[2] * sm_vs);
    const int count = dims[0] * dims[1] * dims[2];
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
    for (int z = 0; z < dims[2]; ++z) {
        for (int y = 0; y < dims[1]; ++y) {
            for (int x = 0; x < dims[0]; ++x) {
                const int index = (z * dims[1] + y) * dims[0] + x;
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

    GPUCamera gpuCam{
        make_float3(eye.x(), eye.y(), eye.z()),
        make_float3(right.x(), right.y(), right.z()),
        make_float3(up.x(), up.y(), up.z()),
        make_float3(view.x(), view.y(), view.z()),
        cam.get_htanfov(),
        cam.get_vtanfov()
    };

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

    // CUDA_CHECK(cudaFree(devicePixels));
    // CUDA_CHECK(cudaFree(device_shadow_pointer_array));
    // CUDA_CHECK(cudaFree(densityBuffer));
    // CUDA_CHECK(cudaFree(colorBuffer));
    // CUDA_CHECK(cudaStreamDestroy(stream));
    // No more kernels will use device_shadow_handle.
    device_shadow_handle.buffer().destroy(stream);
    CUDA_CHECK(cudaStreamSynchronize(stream));

    CUDA_CHECK(cudaFree(device_shadow_pointer_array));
    CUDA_CHECK(cudaFree(densityBuffer));
    CUDA_CHECK(cudaFree(colorBuffer));
    CUDA_CHECK(cudaFree(devicePixels));
    CUDA_CHECK(cudaFree(device_light_colors));

    CUDA_CHECK(cudaStreamDestroy(stream));
}

} // end namespace lux