#ifndef JETSTREAM_MODULES_INVERT_HH
#define JETSTREAM_MODULES_INVERT_HH

#include "jetstream/logger.hh"
#include "jetstream/module.hh"
#include "jetstream/types.hh"

#include "jetstream/memory/base.hh"
#include "jetstream/compute/graph/base.hh"

namespace Jetstream {

#define JST_INVERT_CPU(MACRO) \
    MACRO(Invert, CPU, CF32)

template<Device D, typename T = CF32>
class Invert : public Module, public Compute {
 public:
    // Configuration 

    struct Config {
        JST_SERDES();
    };

    constexpr const Config& getConfig() const {
        return config;
    }

    // Input

    struct Input {
        Tensor<D, T> buffer;

        JST_SERDES_INPUT(buffer);
    };

    constexpr const Input& getInput() const {
        return input;
    }

    // Output

    struct Output {
        Tensor<D, T> buffer;

        JST_SERDES_OUTPUT(buffer);
    };

    constexpr const Output& getOutput() const {
        return output;
    }

    constexpr const Tensor<D, T>& getOutputBuffer() const {
        return this->output.buffer;
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

#ifdef JETSTREAM_MODULE_INVERT_CPU_AVAILABLE
JST_INVERT_CPU(JST_SPECIALIZATION);
#endif

}  // namespace Jetstream

#endif
