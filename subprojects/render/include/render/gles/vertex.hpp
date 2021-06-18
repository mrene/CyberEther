#ifndef RENDER_GLES_VERTEX_H
#define RENDER_GLES_VERTEX_H

#include "render/gles/instance.hpp"

#ifdef RENDER_CUDA_AVAILABLE
#include <cuda_gl_interop.h>
#endif

namespace Render {

class GLES::Vertex : public Render::Vertex {
public:
    Vertex(Config& cfg, GLES& i) : Render::Vertex(cfg), inst(i) {};

    Result update();

protected:
    GLES& inst;

    uint vao, ebo;
    uint vertex_count;

    Result create();
    Result destroy();
    Result start();
    Result end();

    uint count();
    uint buffered();

#ifdef RENDER_CUDA_AVAILABLE
    cudaStream_t stream;
#endif

    friend class GLES::Draw;
};

} // namespace Render

#endif
