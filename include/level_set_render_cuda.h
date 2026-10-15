#pragma once


#include <cuda_runtime.h>
#include <string>
#include <vector>

#include "level_set_renders.h"
#include "ray_marcher_cuda.h"

namespace lux {

struct CudaPointLight {
    float3 position;
    // RGB light color; each channel contributes independently to the volume.
    float3 color;
    // Scaled by inverse-square distance in the ray marcher. The renderer
    // builds one dense shadow map for each light.
    float intensity = 50.0f;
};

struct CudaRenderSettings {
    int width = 1920;
    int height = 1080;

    // The current bunny model fits in this centered stamping region.
    float volume_half_extent = 2.0f;
    float density_voxel_size = 0.025f;
    float color_voxel_size = 0.05f;

    float near_distance = 6.0f;
    float far_distance = 15.0f;
    float min_step = 0.0001f;
    float max_step = 0.05f;
    float extinction = 0.25f;

    // Shadow maps cover the active density bounds plus a small margin.
    float shadow_voxel_size = 0.025f;
    float shadow_step_size = 0.001f;
    float shadow_extinction = 0.5f;

    float3 surface_color{1.0f, 1.0f, 1.0f};
};

void cuda_render_bunny(
    const std::string& path,
    const std::string& name,
    const std::vector<CudaPointLight>& lights,
    const CudaRenderSettings& settings = CudaRenderSettings{});

// Compatibility overload: renders with one green point light.
void cuda_render_bunny(const std::string& path, const std::string& name);

} // namespace lux
