#ifndef JETSTREAM_MODULES_PAD_HH
#define JETSTREAM_MODULES_PAD_HH

#include "jetstream/logger.hh"
#include "jetstream/module.hh"
#include "jetstream/types.hh"

#include "jetstream/memory/base.hh"
#include "jetstream/compute/graph/base.hh"

namespace Jetstream {

#define JST_PAD_CPU(MACRO) \
    MACRO(Pad, CPU, CF32) \
    MACRO(Pad, CPU, F32)

template<Device D, typename T = CF32>
class Pad : public Module, public Compute {
 public:
    // Configuration 

    struct Config {
        U64 size = 33;
        U64 axis = 1;

        JST_SERDES(size, axis);
    };

    constexpr const Config& getConfig() const {
        return config;
    }

    // Input

    struct Input {
        Tensor<D, T> unpadded;

        JST_SERDES_INPUT(unpadded);
    };

    constexpr const Input& getInput() const {
        return input;
    }

    // Output

    struct Output {
        Tensor<D, T> padded;

        JST_SERDES_OUTPUT(padded);
    };

    constexpr const Output& getOutput() const {
        return output;
    }

    constexpr const Tensor<D, T>& getOutputPadded() const {
        return this->output.padded;
    }

    // Taint & Housekeeping

    constexpr Device device() const {
        return D;
    }

    void info() const final;

    // Constructor

    Result create();

 protected:
    Result createCompute(const RuntimeMetadata& meta) final;
    Result compute(const RuntimeMetadata& meta) final;

    JST_DEFINE_IO();
};

#ifdef JETSTREAM_MODULE_PAD_CPU_AVAILABLE
JST_PAD_CPU(JST_SPECIALIZATION);
#endif

}  // namespace Jetstream

#endif
