#pragma once


//#include <nanovdb/tools/CreateNanoGrid.h> // converter from OpenVDB to NanoVDB (includes NanoVDB.h and GridManager.h)
#include <cstddef>
#include <cuda_runtime.h>
#include <nanovdb/NanoVDB.h>

#include "Camera.h"
#include "ray_marcher.h"

namespace lux{


struct GPUVolume{
    const nanovdb::NanoGrid<float>* density;
    nanovdb::NanoGrid<nanovdb::Vec3f>* color;
    nanovdb::NanoGrid<float>** shadow_maps;
    int shadow_map_count = 0;
};

struct GPUCamera {
    float3 eye;
    float3 right;
    float3 up;
    float3 view;
    float horizontal_tan_fov;
    float vertical_tan_fov;
};

// void raymarch_cuda_interface(
//     float4* output,
//     int width,
//     int height,
//     lux::GPUVolume volume,
//     GPUCamera cam,
//     float near_dist,
//     float far_dist,
//     float min_step,
//     float max_step,
//     float kappa
// );

cudaError_t launch_cuda_raymarch(
    GPUVolume render_data,
    float4* output,
    int width,
    int height,
    GPUCamera cam,
    float near_dist,
    float far_dist,
    float min_step,
    float max_step,
    float kappa,
    cudaStream_t stream
);

// -------------------------------------------------------------
// A cuda demo
// -------------------------------------------------------------

cudaError_t launchThreshold(
    const nanovdb::NanoGrid<float>* deviceGrid,
    float* deviceOutput,
    nanovdb::Coord first,
    int count,
    float threshold,
    cudaStream_t stream);


} // end namespace lux
