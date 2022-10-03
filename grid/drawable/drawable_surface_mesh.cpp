#include "drawable_surface_mesh.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <memory>

void DrawableMesh::initializeGL(ShaderCompiler::GLFunctions *functions) {
    this->gl = functions;
    // Create the right shader program :
    auto compiler = std::make_unique<ShaderCompiler>(functions);
    compiler->vertexShader_file("../shaders/base_mesh.vert").fragmentShader_file("../shaders/base_mesh.frag");
    if (compiler->compileShaders()) {
        this->program = compiler->programName();
    } else {
        std::cerr << "Error while building shaders for drawable mesh.\n" << compiler->errorString() << "\n";
    }

    this->color = glm::vec4(1., 0., 0., 1.);
    this->lightPosition = glm::vec3(500., 500., 500.);
}
