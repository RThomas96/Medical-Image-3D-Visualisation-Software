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
    DrawableGrid(GridGLView::Ptr grid);
    virtual ~DrawableGrid() = default;

    void initializeGL(ShaderCompiler::GLFunctions *functions);

    void prepareUniforms();
    void recompileShaders();

    // TODO: move
    GLuint program_VolumetricViewer;
    GLuint tex_ColorScaleGrid;
    GLuint tex_ColorScaleGridAlternate;
    GLuint dualRenderingTexture;
    GLuint frameDepthBuffer;
protected:

    GridGLView::Ptr grid;

    ShaderCompiler::GLFunctions* gl;
    std::unique_ptr<ShaderCompiler> shaderCompiler;
    GLuint compileShaders(std::string _vPath, std::string _gPath, std::string _fPath);


    void createBuffers();
};

#endif	  // VISUALISATION_MESHES_DRAWABLE_MESH_HPP_
