#include "drawable_grid.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <memory>

DrawableGrid::DrawableGrid(GridGLView::Ptr grid): gl(nullptr) {
    this->program_VolumetricViewer = 0;
    this->tex_ColorScaleGrid			= 0;
    this->tex_ColorScaleGridAlternate = 0;
    this->tex_ColorScaleGrid = 0;
    this->grid = grid;
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
    this->createBuffers();
}

void DrawableGrid::createBuffers() {
    glGenTextures(1, &this->dualRenderingTexture);

    glBindTexture(GL_TEXTURE_2D, this->dualRenderingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2024, 1468, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenTextures(1, &this->frameDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, this->frameDepthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT32, 2024, 1468, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    gl->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->frameDepthBuffer, 0);

    // Set "renderedTexture" as our colour attachement #0
    gl->glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->dualRenderingTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    gl->glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if(gl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "WARNING: framebuffer doesn't work !!" << std::endl;
    else
        std::cout << "Framebuffer works perfectly :) !!" << std::endl;

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
    /// @brief Shortcut for glGetUniform, since this can result in long lines.
    auto getUniform = [&](const char* name) -> GLint {
        GLint g = gl->glGetUniformLocation(program_VolumetricViewer, name);
        return g;
    };

    std::size_t tex = 0;
    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.vertexPositions);
    gl->glUniform1i(getUniform("vertices_translations"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.faceNormals);
    gl->glUniform1i(getUniform("normals_translations"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.visibilityMap);
    gl->glUniform1i(getUniform("visibility_texture"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.textureCoordinates);
    gl->glUniform1i(getUniform("texture_coordinates"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.neighborhood);
    gl->glUniform1i(getUniform("neighbors"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_3D, grid->gridTexture);
    gl->glUniform1i(getUniform("texData"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, this->tex_ColorScaleGrid);
    gl->glUniform1i(getUniform("visiblity_map"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, this->tex_ColorScaleGridAlternate);
    gl->glUniform1i(getUniform("visiblity_map_alternate"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, this->dualRenderingTexture);
    gl->glUniform1i(getUniform("firstPass_texture"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, this->frameDepthBuffer);
    gl->glUniform1i(getUniform("firstPass_depthTexture"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, grid->valuesRangeToDisplay);
    gl->glUniform1i(getUniform("valuesRangeToDisplay"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, grid->valuesRangeColorToDisplay);
    gl->glUniform1i(getUniform("colorRangeToDisplay"), tex);
    tex++;
}
