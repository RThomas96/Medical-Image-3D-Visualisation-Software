#include "drawable_grid.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <memory>

DrawableGrid::DrawableGrid(): gl(nullptr) {
    this->program_VolumetricViewer = 0;
    std::cout << "Create drawable grid" << std::endl;
};

void DrawableGrid::recompileShaders() {
    std::cout << "Compile shaders of drawable grid" << std::endl;
    GLuint newVolumetricProgram	 = this->compileShaders("../shaders/transfer_mesh.vert", "../shaders/transfer_mesh.geom", "../shaders/transfer_mesh.frag");
    if (newVolumetricProgram) {
        gl->glDeleteProgram(this->program_VolumetricViewer);
        this->program_VolumetricViewer = newVolumetricProgram;
    }
}

void DrawableGrid::initializeGL(ShaderCompiler::GLFunctions *functions) {
    std::cout << "Initialize drawable grid" << std::endl;
    this->gl = functions;
    this->shaderCompiler = std::make_unique<ShaderCompiler>(functions);
    this->recompileShaders();
}

GLuint DrawableGrid::compileShaders(std::string _vPath, std::string _gPath, std::string _fPath) {
    gl->glUseProgram(0);
    this->shaderCompiler->reset();
    this->shaderCompiler->pragmaReplacement_file("include_color_shader", "../shaders/colorize_new_flow.glsl");
    this->shaderCompiler->vertexShader_file(_vPath).geometryShader_file(_gPath).fragmentShader_file(_fPath);
    if (this->shaderCompiler->compileShaders()) {
        return this->shaderCompiler->programName();
    }
    std::cerr << this->shaderCompiler->errorString() << '\n';
    return 0;
}


void DrawableGrid::prepareUniforms() {

}
