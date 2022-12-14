#ifndef VISUALISATION_MESHES_DRAWABLE_GRID_HPP_
#define VISUALISATION_MESHES_DRAWABLE_GRID_HPP_

//#include "../geometry/surface_mesh.hpp"

#include "../../legacy/meshes/drawable/shaders.hpp"
#include "qobjectdefs.h"
#include "../../qt/legacy/viewer_structs.hpp"

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

class Grid;


//! @ingroup gl
class DrawableGrid {
public:
    bool multiGridRendering;
    bool displayTetmesh;
    bool drawSliceOnly;
    bool drawTetIdx;
    bool drawSizeTetUnit;
    float blendFirstPass;

    // Segmented data display control
    std::vector<std::pair<uint16_t, uint16_t>> displayRangeSegmentedData;
    std::vector<glm::vec3> displayColorSegmentedData;
    std::vector<bool> displaySegmentedData;
    std::vector<glm::vec3> visu_map;
    std::vector<glm::vec3> color_map;
    glm::vec3 color_0;
    glm::vec3 color_1;

    std::array<ColorChannelAttributes_GL, 3> colorChannelAttributes;

    DrawableGrid(Grid * grid);
    virtual ~DrawableGrid() = default;

    void initializeGL(ShaderCompiler::GLFunctions *functions);
    void drawGrid(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, glm::vec3 planePosition, glm::vec3 planeDirection, bool drawFront, int w, int h);
    void drawGridFirstPass(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, glm::vec3 planePosition, glm::vec3 planeDirection, bool drawFront, int w, int h);
    void drawBBox(const glm::vec3& planePos);

    void setMultiGridRendering(bool value);
    void recompileShaders();
    void updateMinMaxDisplayValues();
    void getVisibilityMap(std::vector<bool>& visMap);

    // Choose which data of the tetmesh to send to the GPU
    enum InfoToSend {
        VERTICES  = 0b00000001,
        NORMALS   = 0b00000010,
        TEXCOORD  = 0b00000100,
        NEIGHBORS = 0b00001000
    };
    void sendTetmeshToGPU(const InfoToSend infoToSend);

    // Tetrahedral mesh rendering
    GLuint vertexPositions;
    GLuint textureCoordinates;
    GLuint neighborhood;
    GLuint faceNormals;
    GLuint visibilityMap;
    GLsizei tetrahedraCount;

    GLuint colorScaleUser;
    GLuint uboHandle_colorAttributes;
    GLuint gridTexture;

    GLuint frameBuffer;
    GLuint frameBuffer2;
    GLuint rbo;
    GLuint frameDepthBuffer;
    GLuint depthTexture;
    GLuint depthTexture2;
    GLuint dualRenderingTexture;
    GLuint firstPassTexture;

protected:
    GLuint program;

    // Grid rendering
    GLuint vaoVolumetricBuffers;
    GLuint vboTexture3DVertPos;
    GLuint vboTexture3DVertNorm;
    GLuint vboTexture3DVertTex;
    GLuint vboTexture3DVertIdx;

    GLuint colorScaleGreyscale;
    GLuint colorScaleHsv2rgb;

    Grid * grid;

    GLuint colorRanges;
    GLuint valuesRangeToDisplay;
    GLuint valuesRangeColorToDisplay;

    // Shader compilation management
    ShaderCompiler::GLFunctions * gl;
    std::unique_ptr<ShaderCompiler> shaderCompiler;
    GLuint compileShaders(std::string _vPath, std::string _gPath, std::string _fPath);

    // Utils
    void prepareUniforms(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, glm::vec3 planePosition, glm::vec3 planeDirection, bool drawFront, float w, float h);
    void createBuffers();
    void generateColorScales();
    void tex3D_buildBuffers();
    GLuint uploadTexture1D(const TextureUpload& tex);
    GLuint uploadTexture2D(const TextureUpload& tex);
    void setUniformBufferData(GLuint uniform_buffer, std::size_t begin_bytes, std::size_t size_bytes, GLvoid* data);
};

#endif	  // VISUALISATION_MESHES_DRAWABLE_MESH_HPP_
