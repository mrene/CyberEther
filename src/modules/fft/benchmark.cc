#include "jetstream/modules/fft.hh"

namespace Jetstream {

template<template<Device, typename...> class Module, Device D, typename T>
void benchmark(ankerl::nanobench::Bench& bench, std::string name) {
    JST_BENCHMARK_RUN("128x8000 Forward", {
        .forward = true COMMA
    }, {
        .buffer = Tensor<D COMMA T>({128 COMMA 8000}) COMMA
    }, T);

    JST_BENCHMARK_RUN("128x8000 Backward", {
        .forward = false COMMA
    }, {
        .buffer = Tensor<D COMMA T>({128 COMMA 8000}) COMMA
    }, T);
}

}  // namespace Jetstream