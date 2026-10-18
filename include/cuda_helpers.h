#pragma once

#include <type_traits>

#include <nanovdb/tools/CreateNanoGrid.h>

#include <cuda_runtime.h>
#include <nanovdb/tools/CreateNanoGrid.h>
#include <nanovdb/cuda/DeviceBuffer.h>


namespace lux{

#define CUDA_CHECK(call)                                      \
    do {                                                      \
        cudaError_t err = (call);                             \
        if (err != cudaSuccess)                               \
            throw std::runtime_error(cudaGetErrorString(err));\
    } while (0)


struct NanoAndBuffer{
    nanovdb::GridHandle<nanovdb::HostBuffer> nano;
    void* buffer;
};

template <typename GridPtr>
auto pack_grid_to_nano(const GridPtr& grid, cudaStream_t stream) {
    using GridT = std::remove_cv_t<std::remove_reference_t<decltype(*grid)>>;
    using ValueT = typename GridT::ValueType;
    using NanoValueT = typename nanovdb::tools::MapToNano<ValueT>::type;

    auto handle = nanovdb::tools::createNanoGrid<
            GridT, NanoValueT, nanovdb::CudaDeviceBuffer
            >(*grid);

    handle.deviceUpload(stream, /*sync=*/false);
    return handle;
}

// auto pack_grid_test(const openvdb::FloatGrid::Ptr& grid, cudaStream_t stream){
//     auto handle = nanovdb::tools::createNanoGrid<
//         decltype(grid), float, nanovdb::CudaDeviceBuffer
//     >(*grid);
// }

// template<typename T>
// auto send_array_to_gpu(const T* start, size_t elements){  
//     const float3 light_colors[] = {make_float3(0.2f, 0.2f, 1.0f)}; 
//     float3* device_light_colors = nullptr;
//     CUDA_CHECK(cudaMalloc(
//         &device_light_colors,
//         sizeof(light_colors)
//     ));
//     CUDA_CHECK(cudaMemcpy(
//         device_light_colors,
//         light_colors,
//         sizeof(light_colors),
//         cudaMemcpyHostToDevice
//     ));
// }

template <typename T>
T* send_array_to_gpu(const T* start, size_t elements) {
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable for a raw CUDA copy");
    if (elements == 0) return nullptr;
    const size_t bytes = elements * sizeof(T);
    T* device_data = nullptr;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&device_data), bytes));
    CUDA_CHECK(cudaMemcpy(
        device_data,
        start,
        bytes,
        cudaMemcpyHostToDevice
    ));
    return device_data;
}

} // namespace lux