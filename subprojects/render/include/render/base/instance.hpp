#ifndef RENDER_BASE_INSTANCE_H
#define RENDER_BASE_INSTANCE_H

#include "render/types.hpp"
#include "program.hpp"
#include "surface.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace Render {

class Instance {
public:
    struct Config {
        int width = 1280;
        int height = 720;
        bool resizable = false;
        bool enableImgui = false;
        std::string title = "Render";
    };

    Instance(Config& c) : cfg(c) {};
    virtual ~Instance() = default;

    virtual Result create() = 0;
    virtual Result destroy() = 0;
    virtual Result start() = 0;
    virtual Result end() = 0;

    Result bind(std::shared_ptr<Surface>);

    virtual bool keepRunning() = 0;

    virtual std::string renderer_str() = 0;
    virtual std::string version_str() = 0;
    virtual std::string vendor_str() = 0;
    virtual std::string glsl_str() = 0;

    template<class T> static std::shared_ptr<T> Create(Instance::Config&);
    template<class T> std::shared_ptr<Program> create(Program::Config&);
    template<class T> std::shared_ptr<Surface> create(Surface::Config&);
    template<class T> std::shared_ptr<Texture> create(Texture::Config&);
    template<class T> std::shared_ptr<Vertex> create(Vertex::Config&);

protected:
    Config& cfg;
    struct State* state;

    std::vector<std::shared_ptr<Surface>> surfaces;

    virtual Result createImgui() = 0;
    virtual Result destroyImgui() = 0;
    virtual Result startImgui() = 0;
    virtual Result endImgui() = 0;

    static Result getError(std::string func, std::string file, int line);

    std::string cached_renderer_str;
    std::string cached_version_str;
    std::string cached_vendor_str;
    std::string cached_glsl_str;
};

} // namespace Render

#endif
