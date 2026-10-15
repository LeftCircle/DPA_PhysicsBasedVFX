#pragma once


//#include <nanovdb/tools/CreateNanoGrid.h> // converter from OpenVDB to NanoVDB (includes NanoVDB.h and GridManager.h)
#include <cstddef>
#include <cuda_runtime.h>
#include <nanovdb/NanoVDB.h>
#include <nanovdb/cuda/Buffer.h>
#include <nanovdb/cuda/HandleStorage.h>

#include "Camera.h"
#include "ray_marcher.h"

namespace lux{


struct GPUPointLight {
    float3 position;
    float3 color;
    float intensity;
    const float* shadow_log_transmittance;
};

struct ShadowMapLayout {
    int3 dimensions;
    float3 origin;
    float voxel_size;
};

struct GPUVolume {
    const nanovdb::NanoGrid<float>* density;
    const nanovdb::NanoGrid<nanovdb::Vec3f>* color;
    const GPUPointLight* lights;
    int light_count;
    ShadowMapLayout shadow_map;
};

struct GPUCamera {
    float3 eye;
    float3 right;
    float3 up;
    float3 view;
    float horizontal_tan_fov;
    float vertical_tan_fov;
};

struct RaymarchParameters {
    int width;
    int height;
    float near_distance;
    float far_distance;
    float min_step;
    float max_step;
    float extinction;
};

cudaError_t launch_cuda_raymarch(
    const GPUVolume& volume,
    float4* output,
    const GPUCamera& camera,
    const RaymarchParameters& parameters,
    cudaStream_t stream
);

cudaError_t launch_cuda_shadowmap(
    const nanovdb::NanoGrid<float>* density_grid,
    float* output_log_transmittance,
    const ShadowMapLayout& layout,
    float3 light_position,
    float step_size,
    float extinction,
    cudaStream_t stream
);

} // end namespace lux