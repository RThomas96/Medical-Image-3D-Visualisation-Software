#include "drawable_grid.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <memory>

void DrawableGrid::initializeGL(ShaderCompiler::GLFunctions *functions) {
    this->gl = functions;
}
