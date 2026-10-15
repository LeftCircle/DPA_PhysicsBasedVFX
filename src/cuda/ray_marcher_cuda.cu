#include "ray_marcher_cuda.h"
#include <nanovdb/math/SampleFromVoxels.h>


namespace lux{
namespace {

using v3f = nanovdb::math::Vec3f;


constexpr int kThreadsPerBlock = 128;
constexpr int kMaxBlocksPerGrid = 65535;

__device__ __forceinline__ float3 add3(float3 a, float3 b)
{
    return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

__device__ __forceinline__ float3 multiply3(float3 a, float scale)
{
    return make_float3(a.x * scale, a.y * scale, a.z * scale);
}

__device__ __forceinline__ float3 multiply3(float3 a, float3 b)
{
    return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}

__device__ __forceinline__ float mix(float a, float b, float t)
{
    return a + (b - a) * t;
}

struct CudaRay {
    float3 origin;
    float3 direction;

    __device__ __forceinline__ float3 at(float distance) const {
        return make_float3(
            origin.x + distance * direction.x,
            origin.y + distance * direction.y,
            origin.z + distance * direction.z
        );
    }
};

/*
Used with 
auto densityAcc = volume.density->getAccessor();
auto densitySampler = nanovdb::math::createSampler<1>(densityAcc);
<1> does trilinear interp
*/
template <typename GridT, typename SamplerT>
__device__ typename GridT::ValueType sampleTrilinearWorld(
    const GridT* grid,
    SamplerT& sampler,
    float3 world
){
    // Keep fractional index coordinates for the interpolation weights.
    nanovdb::Vec3f ijkF = grid->worldToIndexF(nanovdb::Vec3f(world.x, world.y, world.z));

    // Floor returns the lower voxel coordinate and changes ijkF
    // to its fractional offset within that voxel.
    nanovdb::Coord ijk =
        nanovdb::math::Floor<nanovdb::Coord>(ijkF);

    typename GridT::ValueType values[2][2][2];
    sampler.stencil(ijk, values); // Fetch the 8 voxels
    return sampler.sample(ijkF, values); // interpolate them based on fractional part
}

__device__ float3 cameraRayDirection(
    int x,
    int y,
    const GPUCamera& camera,
    const RaymarchParameters& parameters
){
    const float u = (2.0f * x / parameters.width - 1.0f) * camera.horizontal_tan_fov;
    const float v = (2.0f * y / parameters.height - 1.0f) * camera.vertical_tan_fov;

    float3 direction = make_float3(
        u * camera.right.x + v * camera.up.x + camera.view.x,
        u * camera.right.y + v * camera.up.y + camera.view.y,
        u * camera.right.z + v * camera.up.z + camera.view.z
    );

    const float inverse_length = rsqrtf(
        direction.x * direction.x +
        direction.y * direction.y +
        direction.z * direction.z
    );
    return multiply3(direction, inverse_length);
}

__device__ bool clipRayToAxis(
    float origin,
    float direction,
    float lower,
    float upper,
    float& t_near,
    float& t_far
){
    if (fabsf(direction) < 1.0e-8f) {
        return origin >= lower && origin <= upper;
    }

    const float inverse_direction = 1.0f / direction;
    float t0 = (lower - origin) * inverse_direction;
    float t1 = (upper - origin) * inverse_direction;
    if (t0 > t1) {
        const float temp = t0;
        t0 = t1;
        t1 = temp;
    }
    t_near = fmaxf(t_near, t0);
    t_far = fminf(t_far, t1);
    return t_near <= t_far;
}

__device__ bool clipRayToVolume(
    const ShadowMapLayout& bounds,
    float3 origin,
    float3 direction,
    float near_limit,
    float far_limit,
    float& t_near,
    float& t_far
){
    if (bounds.dimensions.x <= 0 || bounds.dimensions.y <= 0 ||
        bounds.dimensions.z <= 0 || bounds.voxel_size <= 0.0f){
        return false;
    }

    const float3 upper = make_float3(
        bounds.origin.x + (bounds.dimensions.x - 1) * bounds.voxel_size,
        bounds.origin.y + (bounds.dimensions.y - 1) * bounds.voxel_size,
        bounds.origin.z + (bounds.dimensions.z - 1) * bounds.voxel_size
    );
    t_near = near_limit;
    t_far = far_limit;
    return clipRayToAxis(origin.x, direction.x, bounds.origin.x, upper.x, t_near, t_far) &&
           clipRayToAxis(origin.y, direction.y, bounds.origin.y, upper.y, t_near, t_far) &&
           clipRayToAxis(origin.z, direction.z, bounds.origin.z, upper.z, t_near, t_far);
}

__device__ float denseMapValue(
    const float* values,
    int3 dimensions,
    int x,
    int y,
    int z
){
    const std::size_t index = (static_cast<std::size_t>(z) * dimensions.y + y) * dimensions.x + x;
    return values[index];
}

__device__ float sampleShadowTransmittance(
    const GPUVolume& volume,
    const GPUPointLight& light,
    float3 world)
{
    if (!light.shadow_log_transmittance ||
        volume.shadow_map.dimensions.x <= 0 ||
        volume.shadow_map.dimensions.y <= 0 ||
        volume.shadow_map.dimensions.z <= 0 ||
        volume.shadow_map.voxel_size <= 0.0f) {
        return 1.0f;
    }

    const float inverse_voxel_size = 1.0f / volume.shadow_map.voxel_size;
    const float3 p = make_float3(
        (world.x - volume.shadow_map.origin.x) * inverse_voxel_size,
        (world.y - volume.shadow_map.origin.y) * inverse_voxel_size,
        (world.z - volume.shadow_map.origin.z) * inverse_voxel_size);

    const int3 dims = volume.shadow_map.dimensions;
    if (p.x < 0.0f || p.y < 0.0f || p.z < 0.0f ||
        p.x > dims.x - 1 || p.y > dims.y - 1 || p.z > dims.z - 1) {
        return 1.0f;
    }

    const int x0 = static_cast<int>(floorf(p.x));
    const int y0 = static_cast<int>(floorf(p.y));
    const int z0 = static_cast<int>(floorf(p.z));
    const int x1 = x0 + 1 < dims.x ? x0 + 1 : dims.x - 1;
    const int y1 = y0 + 1 < dims.y ? y0 + 1 : dims.y - 1;
    const int z1 = z0 + 1 < dims.z ? z0 + 1 : dims.z - 1;
    const float fx = p.x - x0;
    const float fy = p.y - y0;
    const float fz = p.z - z0;

    const float c00 = mix(
        denseMapValue(light.shadow_log_transmittance, dims, x0, y0, z0),
        denseMapValue(light.shadow_log_transmittance, dims, x1, y0, z0),
        fx);
    const float c10 = mix(
        denseMapValue(light.shadow_log_transmittance, dims, x0, y1, z0),
        denseMapValue(light.shadow_log_transmittance, dims, x1, y1, z0),
        fx);
    const float c01 = mix(
        denseMapValue(light.shadow_log_transmittance, dims, x0, y0, z1),
        denseMapValue(light.shadow_log_transmittance, dims, x1, y0, z1),
        fx);
    const float c11 = mix(
        denseMapValue(light.shadow_log_transmittance, dims, x0, y1, z1),
        denseMapValue(light.shadow_log_transmittance, dims, x1, y1, z1),
        fx);

    const float log_transmittance =
        mix(mix(c00, c10, fy), mix(c01, c11, fy), fz);
    return expf(fminf(log_transmittance, 0.0f));
}

__device__ float3 sampleLight(
    const GPUVolume& volume,
    float3 world)
{
    float3 result = make_float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < volume.light_count; ++i) {
        const GPUPointLight& light = volume.lights[i];
        const float visibility = sampleShadowTransmittance(volume, light, world);

        const float3 to_light = make_float3(
            light.position.x - world.x,
            light.position.y - world.y,
            light.position.z - world.z);
        const float distance_squared =
            to_light.x * to_light.x +
            to_light.y * to_light.y +
            to_light.z * to_light.z;
        const float attenuation =
            light.intensity * visibility / fmaxf(distance_squared, 1.0e-4f);

        result = add3(result, multiply3(light.color, attenuation));
    }
    return result;
}

__device__ float3 integratePixel(
    const GPUVolume& volume,
    const GPUCamera& camera,
    const RaymarchParameters& parameters,
    int x,
    int y,
    float& alpha_out
){
    const float3 direction = cameraRayDirection(x, y, camera, parameters);
    float t_near = 0.0f;
    float t_far = 0.0f;
    if (!clipRayToVolume(
            volume.shadow_map,
            camera.eye,
            direction,
            parameters.near_distance,
            parameters.far_distance,
            t_near,
            t_far)) {
        alpha_out = 0.0f;
        return make_float3(0.0f, 0.0f, 0.0f);
    }

    const float voxel_size = static_cast<float>(volume.density->voxelSize()[0]);
    const float step_size = fminf(parameters.max_step, fmaxf(parameters.min_step, voxel_size));

    float3 radiance = make_float3(0.0f, 0.0f, 0.0f);
    float transmittance = 1.0f;
    if (step_size <= 0.0f) {
        alpha_out = 0.0f;
        return radiance;
    }

    auto density_accessor = volume.density->getAccessor();
    auto density_sampler = nanovdb::math::createSampler<1>(density_accessor);
    auto color_accessor = volume.color->getAccessor();
    auto color_sampler = nanovdb::math::createSampler<1>(color_accessor);

    for (float t = t_near; t < t_far && transmittance > 1.0e-3f; t += step_size) {
        const float3 world = make_float3(
            camera.eye.x + t * direction.x,
            camera.eye.y + t * direction.y,
            camera.eye.z + t * direction.z);
        const float density = sampleTrilinearWorld(volume.density, density_sampler, world);
        if (density >= 0.0f) continue;

        const nanovdb::Vec3f sampled_color = sampleTrilinearWorld(volume.color, color_sampler, world);
        const float3 albedo = make_float3( sampled_color[0], sampled_color[1], sampled_color[2]);
        const float3 illumination = sampleLight(volume, world);

        const float next_transmittance = expf(step_size * parameters.extinction * density);
        const float sample_alpha = 1.0f - next_transmittance;
        radiance = add3(
            radiance,
            multiply3(
                multiply3(albedo, illumination),
                transmittance * sample_alpha));
        transmittance *= next_transmittance;
    }

    alpha_out = 1.0f - transmittance;
    return radiance;
}

__global__ void raymarchKernel(
    GPUVolume volume,
    float4* output,
    GPUCamera camera,
    RaymarchParameters parameters)
{
    const int pixel = blockIdx.x * blockDim.x + threadIdx.x;
    const int pixel_count = parameters.width * parameters.height;
    if (pixel >= pixel_count) return;

    if (!volume.density || !volume.color ||
        (volume.light_count > 0 && !volume.lights)) {
        output[pixel] = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    const int x = pixel % parameters.width;
    const int y = pixel / parameters.width;
    float alpha = 0.0f;
    const float3 radiance =
        integratePixel(volume, camera, parameters, x, y, alpha);
    output[pixel] = make_float4(radiance.x, radiance.y, radiance.z, alpha);
}

__device__ void integrateShadowVoxel(
    const nanovdb::NanoGrid<float>* density_grid,
    float* output,
    std::size_t index,
    int3 dimensions,
    float3 origin,
    float voxel_size,
    float3 light_position,
    float step_size,
    float extinction)
{
    const int x = static_cast<int>(index % dimensions.x);
    const int y = static_cast<int>((index / dimensions.x) % dimensions.y);
    const int z = static_cast<int>(
        index / (static_cast<std::size_t>(dimensions.x) * dimensions.y));

    const float3 start = make_float3(
        origin.x + x * voxel_size,
        origin.y + y * voxel_size,
        origin.z + z * voxel_size);
    const float3 to_light = make_float3(
        light_position.x - start.x,
        light_position.y - start.y,
        light_position.z - start.z);
    const float distance = sqrtf(
        to_light.x * to_light.x +
        to_light.y * to_light.y +
        to_light.z * to_light.z);
    if (distance <= 1.0e-8f) {
        output[index] = 0.0f;
        return;
    }

    const float inverse_distance = 1.0f / distance;
    const float3 direction = multiply3(to_light, inverse_distance);
    auto density_accessor = density_grid->getAccessor();
    auto density_sampler = nanovdb::math::createSampler<1>(density_accessor);

    float log_transmittance = 0.0f;
    for (float s = 0.0f; s < distance;) {
        const float ds = fminf(step_size, distance - s);
        const float3 sample_position = make_float3(
            start.x + (s + 0.5f * ds) * direction.x,
            start.y + (s + 0.5f * ds) * direction.y,
            start.z + (s + 0.5f * ds) * direction.z);
        const float density =
            sampleTrilinearWorld(density_grid, density_sampler, sample_position);
        log_transmittance += extinction * fminf(density, 0.0f) * ds;
        s += ds;
    }
    output[index] = fminf(log_transmittance, 0.0f);
}

__global__ void buildShadowMapKernel(
    const nanovdb::NanoGrid<float>* density_grid,
    float* output,
    ShadowMapLayout layout,
    float3 light_position,
    float step_size,
    float extinction,
    std::size_t sample_count)
{
    const std::size_t first =
        static_cast<std::size_t>(blockIdx.x) * blockDim.x + threadIdx.x;
    const std::size_t stride =
        static_cast<std::size_t>(blockDim.x) * gridDim.x;
    for (std::size_t index = first; index < sample_count; index += stride) {
        integrateShadowVoxel(
            density_grid,
            output,
            index,
            layout.dimensions,
            layout.origin,
            layout.voxel_size,
            light_position,
            step_size,
            extinction);
    }
}

} // namespace

cudaError_t launch_cuda_raymarch(
    const GPUVolume& volume,
    float4* output,
    const GPUCamera& camera,
    const RaymarchParameters& parameters,
    cudaStream_t stream)
{
    if (!output || parameters.width <= 0 || parameters.height <= 0 ||
        parameters.min_step <= 0.0f ||
        parameters.max_step < parameters.min_step ||
        parameters.far_distance <= parameters.near_distance ||
        parameters.extinction < 0.0f ||
        volume.light_count < 0) {
        return cudaErrorInvalidValue;
    }

    const long long pixel_count_wide =
        static_cast<long long>(parameters.width) * parameters.height;
    if (pixel_count_wide > 0x7fffffffLL) return cudaErrorInvalidValue;
    const int pixel_count = static_cast<int>(pixel_count_wide);
    const int blocks =
        (pixel_count + kThreadsPerBlock - 1) / kThreadsPerBlock;
    raymarchKernel<<<blocks, kThreadsPerBlock, 0, stream>>>(
        volume, output, camera, parameters);
    return cudaGetLastError();
}

cudaError_t launch_cuda_shadowmap(
    const nanovdb::NanoGrid<float>* density_grid,
    float* output_log_transmittance,
    const ShadowMapLayout& layout,
    float3 light_position,
    float step_size,
    float extinction,
    cudaStream_t stream)
{
    if (!density_grid || !output_log_transmittance ||
        layout.dimensions.x <= 0 ||
        layout.dimensions.y <= 0 ||
        layout.dimensions.z <= 0 ||
        layout.voxel_size <= 0.0f ||
        step_size <= 0.0f ||
        extinction < 0.0f) {
        return cudaErrorInvalidValue;
    }

    const std::size_t x = static_cast<std::size_t>(layout.dimensions.x);
    const std::size_t y = static_cast<std::size_t>(layout.dimensions.y);
    const std::size_t z = static_cast<std::size_t>(layout.dimensions.z);
    const std::size_t maximum = static_cast<std::size_t>(-1);
    if (y > maximum / x || z > maximum / (x * y)) {
        return cudaErrorInvalidValue;
    }
    const std::size_t sample_count = x * y * z;
    const std::size_t wanted_blocks =
        1 + (sample_count - 1) / kThreadsPerBlock;
    const int blocks = static_cast<int>(
        wanted_blocks < kMaxBlocksPerGrid ? wanted_blocks : kMaxBlocksPerGrid);

    buildShadowMapKernel<<<blocks, kThreadsPerBlock, 0, stream>>>(
        density_grid,
        output_log_transmittance,
        layout,
        light_position,
        step_size,
        extinction,
        sample_count);
    return cudaGetLastError();
}

} // namespace lux
