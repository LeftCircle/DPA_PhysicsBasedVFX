#include "ray_marcher_cuda.h"
#include <nanovdb/math/SampleFromVoxels.h>


namespace lux{


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
    float3 world)
{
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

__global__ void raymarch_kernel(
    GPUVolume render_data,
    float4* output,
    int width,
    int height,
    GPUCamera cam,
    float near_dist,
    float far_dist,
    float min_step,
    float max_step,
    float kappa
){
    const int pixel = blockIdx.x * blockDim.x + threadIdx.x;
    const int pixel_count = width * height;
    if (pixel >= pixel_count) return;

    if (!render_data.density) {
        output[pixel] = make_float4(0.f, 0.f, 0.f, 0.f);
        return;
    }

    const int x = pixel % width;
    const int y = pixel / width;

    // Same pixel-to-ray mapping as RayMarcher::get_ray.
    const float u = (2.f * x / width - 1.f) * cam.horizontal_tan_fov;
    const float v = (2.f * y / height - 1.f) * cam.vertical_tan_fov;

    float3 dir = make_float3(
        u * cam.right.x + v * cam.up.x + cam.view.x,
        u * cam.right.y + v * cam.up.y + cam.view.y,
        u * cam.right.z + v * cam.up.z + cam.view.z
    );

    const float inv_len = rsqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    dir.x *= inv_len;
    dir.y *= inv_len;
    dir.z *= inv_len;

    auto densityAcc = render_data.density->getAccessor();

    const bool has_color = render_data.color != nullptr;
    //nanovdb::NanoGrid<nanovdb::Vec3f>::AccessorType colorAcc;
    //if (has_color) colorAcc = render_data.color->getAccessor();

    float3 radiance = make_float3(0.f, 0.f, 0.f);
    float transmittance = 1.f;

    const float voxel_step = static_cast<float>(render_data.density->voxelSize()[0]);
    const float step = fminf(max_step, fmaxf(min_step, voxel_step));

    if (step <= 0.f) {
        output[pixel] = make_float4(0.f, 0.f, 0.f, 0.f);
        return;
    }

    auto dens_acc = render_data.density->getAccessor();
    auto dens_sampler = nanovdb::math::createSampler<1>(dens_acc);
    // TODO -> same sampler for color

    for (float t = near_dist;
         t < far_dist && transmittance > 1e-3f;
         t += step)
    {
        const float3 world = make_float3(
            cam.eye.x + t * dir.x,
            cam.eye.y + t * dir.y,
            cam.eye.z + t * dir.z
        );

        const float density = sampleTrilinearWorld(render_data.density, dens_sampler, world);

        if (density < 0.f) {
            float3 color = make_float3(1.f, 1.f, 1.f);

            // keeping color as constant for now
            // if (has_color) {
            //     const nanovdb::Vec3f c = sampleTrilinearWorld(volume.color, colorAcc, world);
            //     color = make_float3(c.x(), c.y(), c.z());
            // }

            if (kappa == 0.f) {
                radiance.x += color.x;
                radiance.y += color.y;
                radiance.z += color.z;
                transmittance = 0.f;
                break;
            }

            const float nextT = expf(step * kappa * density);
            const float alpha = 1.f - nextT;
            radiance.x += transmittance * alpha * color.x;
            radiance.y += transmittance * alpha * color.y;
            radiance.z += transmittance * alpha * color.z;
            transmittance *= nextT;
        }
    }

    output[pixel] = make_float4(
        radiance.x, radiance.y, radiance.z, 1.f - transmittance);
}

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
){
    if (width <= 0 || height <= 0) return cudaErrorInvalidValue;

    constexpr int threads = 128;
    const int pixel_count = width * height;
    const int blocks = (pixel_count + threads - 1) / threads;

    raymarch_kernel<<<blocks, threads, 0, stream>>>(
        render_data, output, width, height, cam,
        near_dist, far_dist, min_step, max_step, kappa);

    return cudaGetLastError();
}

// -------------------------------------------------------------
// A cuda demo
// -------------------------------------------------------------
__global__ void thresholdKernel(
    const nanovdb::NanoGrid<float>* grid,
    float* output,
    nanovdb::Coord first,
    int count,
    float threshold)
{
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= count) return;

    // This example samples a line in NanoVDB index coordinates.
    const nanovdb::Coord ijk(first.x() + i, first.y(), first.z());

    auto accessor = grid->getAccessor();
    const float density = accessor.getValue(ijk);

    output[i] = density >= threshold ? density : 0.0f;
}

cudaError_t launchThreshold(
    const nanovdb::NanoGrid<float>* deviceGrid,
    float* deviceOutput,
    nanovdb::Coord first,
    int count,
    float threshold,
    cudaStream_t stream)
{
    constexpr int threads = 128;
    const int blocks = (count + threads - 1) / threads;

    thresholdKernel<<<blocks, threads, 0, stream>>>(
        deviceGrid, deviceOutput, first, count, threshold);

    return cudaGetLastError();
}

} // end namespace lux
