#pragma once

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

    auto handle = nanovdb::tools::createNanoGrid<
            GridT, ValueT, nanovdb::CudaDeviceBuffer
            >(*grid);

    handle.deviceUpload(stream, /*sync=*/false);
    return handle;
}


}