#ifndef VISUALISATION_MESHES_DRAWABLE_GRID_HPP_
#define VISUALISATION_MESHES_DRAWABLE_GRID_HPP_

//#include "../geometry/surface_mesh.hpp"

#include "../../legacy/meshes/drawable/shaders.hpp"
#include "qobjectdefs.h"
#include "qt/legacy/viewer_structs.hpp"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QOpenGLFunctions_3_2_Core>

#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>

#include <iostream>
#include <memory>
#include <vector>

#include <memory>
#include <vector>

class DrawableGrid {
public:
    bool multiGridRendering;
    bool displayTetmesh;
    bool drawOnlyBoundaries;
    float blendFirstPass;

    DrawableGrid(GridGLView::Ptr grid);
    virtual ~DrawableGrid() = default;

    void initializeGL(ShaderCompiler::GLFunctions *functions);
    void drawGrid(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, glm::vec3 planePosition, glm::vec3 planeDirection, bool drawFront);

    void setMultiGridRendering(bool value);
    void recompileShaders();
    void updateMinMaxDisplayValues();

    GLuint colorScaleUser;
protected:
    GLuint program;

    GLuint vaoVolumetricBuffers;
    GLuint vboTexture3DVertPos;
    GLuint vboTexture3DVertNorm;
    GLuint vboTexture3DVertTex;
    GLuint vboTexture3DVertIdx;

    GLuint colorScaleGreyscale;
    GLuint colorScaleHsv2rgb;

    GLuint frameBuffer;
    GLuint frameDepthBuffer;
    GLuint dualRenderingTexture;

    GridGLView::Ptr grid;

    // Shader compilation management
    ShaderCompiler::GLFunctions * gl;
    std::unique_ptr<ShaderCompiler> shaderCompiler;
    GLuint compileShaders(std::string _vPath, std::string _gPath, std::string _fPath);

    // Utils
    void prepareUniforms(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, glm::vec3 planePosition, glm::vec3 planeDirection, bool drawFront);
    void createBuffers();
    void generateColorScales();
    void tex3D_buildBuffers();
    GLuint uploadTexture1D(const TextureUpload& tex);
    void setUniformBufferData(GLuint uniform_buffer, std::size_t begin_bytes, std::size_t size_bytes, GLvoid* data);
};

#endif	  // VISUALISATION_MESHES_DRAWABLE_MESH_HPP_
