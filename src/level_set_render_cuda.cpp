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
    float color_vs = 0.05;
    int width = 1920;
    int height = 1080;
    float near_dist = 6;
    float far_dist = 15;
    float min_step = 0.01;
    float max_step = min_step * 5.0;
    float kappa = 3.0;
    printf("Stamping\n");
    auto bunn = bunny_info();

    auto dens_bounds = world_space_to_bounds({-limit, -limit, -limit}, {limit, limit, limit}, density_vs);
    auto col_bounds = world_space_to_bounds({-limit, -limit, -limit}, {limit, limit, limit}, color_vs);

    // Now let's stamp the density into nanovdb
    auto dens = bunn.density;
    auto dens_grid = create_float_grid(density_vs, 0.00001f);
    stamp_isf_to_grid(dens_grid, dens, dens_bounds);
    // auto density_handle = nanovdb::tools::createNanoGrid(*dens_grid);
    // auto* host_density_grid = density_handle.grid<float>();
    
    // auto device_density_handle = density_handle.deviceUpload();
    // const auto* device_density_grid = device_density_handle.deviceGrid<float>();
    
    // GPUVolume volume;
    // volume.density = 

    cudaStream_t stream{};
    CUDA_CHECK(cudaStreamCreate(&stream));

    //auto densityNano = nanovdb::tools::createNanoGrid(*dens_grid);
    //auto colorNano   = nanovdb::tools::createNanoGrid(*color_grid);

    // ----------------------------------------------------------------
    auto densityNano = nanovdb::tools::createNanoGrid(*dens_grid);

    const std::size_t densityBytes = static_cast<std::size_t>(densityNano.size());

    void* densityBuffer = nullptr;
    CUDA_CHECK(cudaMalloc(&densityBuffer, densityBytes));
    CUDA_CHECK(cudaMemcpyAsync(
        densityBuffer,
        densityNano.data(),
        densityBytes,
        cudaMemcpyHostToDevice,
        stream));

    GPUVolume volume{};
    volume.density = reinterpret_cast<const nanovdb::NanoGrid<float>*>(densityBuffer);
    volume.color = nullptr;
    volume.shadow_maps = nullptr;
    volume.shadow_map_count = 0;
    // -----------------------------------------------

    //GPUVolume volume{};
    // volume.density = densityDevice.deviceGrid<float>();
    // //volume.color = colorDevice.deviceGrid<nanovdb::Vec3f>();
    // volume.shadow_maps = nullptr;
    // volume.shadow_map_count = 0;
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

    CUDA_CHECK(launch_cuda_raymarch(
        volume, devicePixels, width, height, gpuCam,
        near_dist, far_dist, min_step, max_step, kappa, stream));

    CUDA_CHECK(cudaStreamSynchronize(stream));
    CUDA_CHECK(cudaMemcpy(
        pixels.data(), devicePixels,
        pixels.size() * sizeof(float4),
        cudaMemcpyDeviceToHost)
    );

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

    CUDA_CHECK(cudaFree(devicePixels));
    CUDA_CHECK(cudaStreamDestroy(stream));
}

} // end namespace lux