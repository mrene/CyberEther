#include "jetstream/lineplot/cuda.hpp"

namespace Jetstream {
namespace Lineplot {

CUDA::CUDA(const Config & c) : Generic(c) {
    plot_len = plot.size() * sizeof(plot[0]);
    CUDA_CHECK_THROW(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
    CUDA_CHECK_THROW(cudaMalloc(&plot_dptr, plot_len));
    CUDA_CHECK_THROW(cudaMemcpy(plot_dptr, plot.data(), plot_len, cudaMemcpyHostToDevice));
    JETSTREAM_CHECK_THROW(this->_initRender(plot_dptr, cfg.render->cudaInteropSupported()));
}

CUDA::~CUDA() {
    cudaFree(plot_dptr);
    cudaStreamDestroy(stream);
}

Result CUDA::_compute() {
    size_t elementSize = sizeof(float);
    size_t srcPitchInBytes = 1 * elementSize;
    size_t dstPitchInBytes = 3 * elementSize;
    size_t width = 1 * elementSize;
    size_t height = in.buf.size();

    CUDA_CHECK(cudaMemcpy2DAsync(plot_dptr + 1, dstPitchInBytes, in.buf.data(), srcPitchInBytes,
        width, height, cudaMemcpyDeviceToDevice, stream));
    CUDA_CHECK(cudaStreamSynchronize(stream));

    return Result::SUCCESS;
}

Result CUDA::_present() {
    lineVertex->update();

    return Result::SUCCESS;
}

} // namespace Lineplot
} // namespace Jetstream