#ifndef RENDER_GLES_API_H
#define RENDER_GLES_API_H

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "base/instance.hpp"
#include "base/program.hpp"
#include "base/surface.hpp"

namespace Render {

class GLES {
public:
    class State;
    class Instance;
    class Program;
	class Surface;

    GLES();

    std::shared_ptr<Instance> createInstance(Render::Instance::Config&);
	std::shared_ptr<Program> createProgram(Render::Program::Config&);

private:
    std::unique_ptr<State> state;
	std::shared_ptr<Instance> instance;

	static Result getError(std::string func, std::string file, int line);
};

} // namespace Render

#endif
