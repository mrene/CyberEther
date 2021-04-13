#ifndef RENDER_BASE_PROGRAM_H
#define RENDER_BASE_PROGRAM_H

#include "types.hpp"
#include "surface.hpp"

namespace Render {

class Program {
public:
    struct Config {
        const char* const* vertexSource;
        const char* const* fragmentSource;
        std::shared_ptr<Surface> surface;
    };

    Program(Config& c) : p(c) {};
    virtual ~Program() = default;

    virtual Result create() = 0;
    virtual Result destroy() = 0;

    virtual Result setUniform(std::string, const std::vector<int> &) = 0;
    virtual Result setUniform(std::string, const std::vector<float> &) = 0;

    virtual Result draw() = 0;

protected:
    Config& p;
};

} // namespace Render

#endif
