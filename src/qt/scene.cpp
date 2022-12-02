#include "scene.hpp"
//#include "planar_viewer.hpp"

#include <GL/gl.h>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>

#include <QFileDialog>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QSurface>
#include <limits>

#include <fstream>
#include <type_traits>

#include <chrono>
#include <utility>
#include <map>
#include <vector>

#include "../core/geometry/grid.hpp"
//#include "../../core/deformation/mesh_deformer.hpp"

#include "../core/utils/apss.hpp"
#include "glm/fwd.hpp"
#include "src/core/drawable/drawable.hpp"
#include "src/core/drawable/drawable_grid.hpp"
#include "src/core/geometry/base_mesh.hpp"
#include "src/core/geometry/surface_mesh.hpp"
#include "src/core/interaction/mesh_manipulator.hpp"

#include "qobjectdefs.h"

/** This constructor not only creates the object, but also sets the default values for the Scene in order
 *  to be drawn, even if it is empty at the time of the first call to a draw function.
 */
Scene::Scene() {
    this->meshManipulator = nullptr;
    this->distanceFromCamera = 0.;
    this->cameraPosition = glm::vec3(0., 0., 0.);

    this->registrationRendering = false;
    this->funnyRender = false;

    //this->grids.clear();

    this->context			= nullptr;
    this->controlPanel		= nullptr;
    //this->gridControl		= nullptr;
    this->programStatusBar	= nullptr;

    double minTexVal		= 1;
    double maxTexVal		= std::numeric_limits<int>::max();
    this->selectedChannel_r = 0;	// by default, the R channel
    this->selectedChannel_g = 0;	// by default, the R channel

    // Default light positions : at the vertices of a unit cube.

    this->planeDirection	= glm::vec3(1., 1., 1.);
    this->planeDisplacement = glm::vec3(.0, .0, .0);
    glm::vec3 min(.0, .0, .0);
    glm::vec3 max(1., 1., 1.);
    this->rgbMode				 = ColorChannel::HandEColouring;
    this->channels_r			 = ColorFunction::SingleChannel;
    this->channels_g			 = ColorFunction::SingleChannel;

    this->default_vao = 0;
    //this->vao_VolumetricBuffers = 0;

    //std::cerr << "Allocating " << +std::numeric_limits<GridGLView::data_t>::max() << " elements for vis ...\n";

    this->shouldUpdateUserColorScales = false;
    this->needUpdateMinMaxDisplayValues		  = false;

    this->glSelection = new UITool::GL::Selection(this, glm::vec3(0., 0., 0.), glm::vec3(10., 10., 10.));

    this->currentTool = UITool::MeshManipulatorType::POSITION;
    this->planeActivation = glm::vec3(1., 1., 1.);
    this->displayGrid = true;
    this->displayMesh = true;
    this->previewCursorInPlanarView = false;
    this->displayGridBBox = false;
    this->displayXRayManipulators = false;
    this->displayTetSizeUnit = false;
}

Scene::~Scene(void) {
}

void Scene::initGl(QOpenGLContext* _context) {
    // Check if the scene has been initialized, share contexts if it has been :
    // this->h = 1024;
    // this->w = 768;
    this->h = 2024;
    this->w = 1468;
    //if (this->isInitialized == true) {
    //    if (this->context != nullptr && (_context != 0 && _context != nullptr)) {
    //        _context->setShareContext(this->context);
    //        if (_context->create() == false) {
    //            // throw std::runtime_error("Couldn't re-create context with shared context added\n");
    //            std::cerr << "Couldn't re-create context with shared context added\n";
    //        } else {
    //            std::cerr << "init() : Switching to context " << _context << '\n';
    //        }
    //    }
    //    return;
    //} else {
        // If the scene had not yet been initialized, it is now :

        // Set the context for later viewers that want to connect to the scene :
        if (_context == 0) {
            throw std::runtime_error("Warning : this->context() returned 0 or nullptr !");
        }
        if (_context == nullptr) {
            std::cerr << "Warning : Initializing a scene without a valid OpenGL context !" << '\n';
        }
        this->context = _context;

        // Get OpenGL functions from the currently bound context :
        if (this->initializeOpenGLFunctions() == false) {
            throw std::runtime_error("Could not initialize OpenGL functions.");
        }
    //}
    auto maj = this->context->format().majorVersion();
    auto min = this->context->format().minorVersion();
    std::cerr << "OpenGL version " << maj << '.' << min << '\n';

    std::cerr << "OpenGL vendor string : " << glGetString(GL_VENDOR) << '\n';
    std::cerr << "OpenGL renderer string : " << glGetString(GL_RENDERER) << '\n';

    // Create the debug logger and connect its signals :
        //this->setupGLOutput();

    // The default parameters have already been set in the constructor. We
    // need to initialize the OpenGL objects now. Shaders, VAOs, VBOs.

    // Initialize limits
    this->maximumTextureSize = 0;

    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maximumTextureSize);
    std::cerr << "Info : max texture size was set to : " << this->maximumTextureSize << "\n";

    //this->generateColorScales();

    this->shaderCompiler = std::make_unique<ShaderCompiler>(this);

    // Compile the shaders :
    this->recompileShaders(false);

    this->createBuffers();

    // Generate visibility array :

    // Generate controller positions
    //this->initGL(this->get_context());
    this->activeGrid = -1;
    //this->prepareSphere();

    //this->surfaceMesh = nullptr;
    //this->drawableMesh = nullptr;
    this->glSelection->prepare();
}

void Scene::addStatusBar(QStatusBar* _s) {
    if (this->programStatusBar != nullptr) {
        return;
    }

    this->programStatusBar = _s;
}

void Scene::createBuffers() {
    /// @brief Create a vertex array, bind it and see if it has been succesfully created server-side.
    auto createVAO = [&, this](std::string name) -> GLuint {
        GLuint buf = 0;
        this->glGenVertexArrays(1, &buf);
        this->glBindVertexArray(buf);
        if (this->glIsVertexArray(buf) == GL_FALSE) {
            std::cerr << "[ERROR][" << __FILE__ << ":" << __LINE__ << "] : Could not create VAO object " << name << '\n';
        }
        return buf;
    };

    /// @brief Create a buffer, bind it and see if it has been succesfully created server-side.
    auto createVBO = [&, this](GLenum bufType, std::string name) -> GLuint {
        GLuint buf = 0;
        this->glGenBuffers(1, &buf);
        this->glBindBuffer(bufType, buf);
        if (this->glIsBuffer(buf) == GL_FALSE) {
            std::cerr << "[ERROR][" << __FILE__ << ":" << __LINE__ << "] : Could not create buffer object " << name << '\n';
        }
        return buf;
    };

    // For the default VAO :
    this->default_vao = createVAO("vaoHandle");
    this->default_vbo_Vertice = createVBO(GL_ARRAY_BUFFER, "vboHandle");
    this->default_vbo_Normal = createVBO(GL_ARRAY_BUFFER, "vboHandle");
    this->default_vbo_Id = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle");

    this->glSelection->setVao(createVAO("vaoHandle_Selection"));
    this->glSelection->setVboVertices(createVBO(GL_ARRAY_BUFFER, "vboHandle_SelectionVertices"));
    this->glSelection->setVboIndices(createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_SelectionIndices"));
}

std::pair<uint16_t, uint16_t> Scene::sendGridValuesToGPU(int gridIdx) {

    glm::vec<4, std::size_t, glm::defaultp> dimensions{this->grids[gridIdx]->getResolution(), 2};
    TextureUpload _gridTex{};
    _gridTex.minmag.x  = GL_NEAREST;
    _gridTex.minmag.y  = GL_NEAREST;
    _gridTex.lod.y	   = -1000.f;
    _gridTex.wrap.x	   = GL_CLAMP_TO_EDGE;
    _gridTex.wrap.y	   = GL_CLAMP_TO_EDGE;
    _gridTex.wrap.z	   = GL_CLAMP_TO_EDGE;
    _gridTex.swizzle.r = GL_RED;
    if (dimensions.a > 1) {
        _gridTex.swizzle.g = GL_GREEN;
    } else {
        _gridTex.swizzle.g = GL_ZERO;
    }
    if (dimensions.a > 2) {
        _gridTex.swizzle.b = GL_BLUE;
    } else {
        _gridTex.swizzle.b = GL_ZERO;
    }
    if (dimensions.a > 3) {
        _gridTex.swizzle.a = GL_ALPHA;
    } else {
        _gridTex.swizzle.a = GL_ONE;
    }
    _gridTex.alignment.x = 1;
    _gridTex.alignment.y = 2;
    switch (dimensions.a) {
        case 1:
            _gridTex.format			= GL_RED_INTEGER;
            _gridTex.internalFormat = GL_R16UI;
            break;
        case 2:
            _gridTex.format			= GL_RG_INTEGER;
            _gridTex.internalFormat = GL_RG16UI;
            break;
        case 3:
            _gridTex.format			= GL_RGB_INTEGER;
            _gridTex.internalFormat = GL_RGB16UI;
            break;
        case 4:
            _gridTex.format			= GL_RGBA_INTEGER;
            _gridTex.internalFormat = GL_RGBA16UI;
            break;
    }
    _gridTex.type = GL_UNSIGNED_SHORT;
    std::cerr << "Made the upload texture struct.\n";

    _gridTex.size.x = dimensions.x;
    _gridTex.size.y = dimensions.y;
    _gridTex.size.z = dimensions.z;

    std::vector<std::uint16_t> slices;
    glDeleteTextures(1, &this->grids[gridIdx]->gridTexture);
    this->grids[gridIdx]->gridTexture = this->newAPI_uploadTexture3D_allocateonly(_gridTex);

    int nbSlice = this->grids[gridIdx]->getResolution()[2];

    //TODO: this computation do not belong here
    uint16_t max = std::numeric_limits<uint16_t>::min();
    uint16_t min = std::numeric_limits<uint16_t>::max();

    dimensions[0] = dimensions[0] * dimensions[3];// Because we have "a" value

    bool addArticialBoundaries = false;

    int sliceI = 0;
    for (std::size_t s = 0; s < nbSlice; ++s) {
        this->grids[gridIdx]->getGridSlice(s, slices, dimensions.a);
        if(addArticialBoundaries) {
            if(s == 0 || s == nbSlice-1){
                std::fill(slices.begin(), slices.end(), 0);
            } else {
                for(int i = 0; i < dimensions.x; ++i) {
                    slices[i] = 0;
                    slices[i+(dimensions.x*(dimensions.y-1))] = 0;
                }
                for(int i = 0; i < dimensions.y; ++i) {
                    slices[i*dimensions.x] = 0;
                    slices[i*dimensions.x+1] = 0;
                    slices[i*dimensions.x+(dimensions.x-1)] = 0;
                    slices[i*dimensions.x+(dimensions.x-2)] = 0;
                }
            }
        }
        this->newAPI_uploadTexture3D(this->grids[gridIdx]->gridTexture, _gridTex, sliceI, slices);

        max = std::max(max, *std::max_element(slices.begin(), slices.end()));
        //min = std::min(min, *std::min_element(slices.begin(), slices.end()));
        for(uint16_t& val : slices) {
            if(val > 0 && val < min)
                min = val;
        }
        slices.clear();
        sliceI++;
    }
    this->needUpdateMinMaxDisplayValues = true;
    return std::make_pair(min, max);
}

void Scene::addGrid() {

    Grid * gridView = this->grids.back();
    DrawableGrid * drawable_gridView = this->grids.back();

    glm::vec<4, std::size_t, glm::defaultp> dimensions{gridView->getResolution(), 2};

    std::pair<uint16_t, uint16_t> min_max = sendGridValuesToGPU(this->grids.size() -1);

    uint16_t min = min_max.first;
    uint16_t max = min_max.second;
    gridView->sampler.image->minValue = min;
    gridView->sampler.image->maxValue = max;

    if(this->activeGrid == 0) {
        QColor r = Qt::GlobalColor::red;
        QColor b = Qt::GlobalColor::blue;
        drawable_gridView->color_0 = glm::vec3(r.redF(), r.greenF(), r.blueF());
        drawable_gridView->color_1 = glm::vec3(b.redF(), b.greenF(), b.blueF());
    } else {
        QColor d = Qt::GlobalColor::darkCyan;
        QColor y = Qt::GlobalColor::yellow;
        drawable_gridView->color_0 = glm::vec3(d.redF(), d.greenF(), d.blueF());
        drawable_gridView->color_1 = glm::vec3(y.redF(), y.greenF(), y.blueF());
    }

    drawable_gridView->colorChannelAttributes[0].setMinVisible(1.);
    drawable_gridView->colorChannelAttributes[0].setMinColorScale(1.);
    drawable_gridView->colorChannelAttributes[0].setMaxVisible(max);
    drawable_gridView->colorChannelAttributes[0].setMaxColorScale(max);

    if(this->activeGrid == 1) {
        this->controlPanel->setMinTexValAlternate(1.);
        this->controlPanel->updateMinValueAlternate(1.);
        this->controlPanel->setMaxTexValAlternate(max);
        this->controlPanel->updateMaxValueAlternate(max);
    } else {
        this->controlPanel->setMinTexVal(1.);
        this->controlPanel->updateMinValue(1.);
        this->controlPanel->setMaxTexVal(max);
        this->controlPanel->updateMaxValue(max);
    }
    this->setColorChannel(ColorChannel::RedOnly);
    this->setColorFunction_r(ColorFunction::ColorMagnitude);
    this->setColorFunction_g(ColorFunction::ColorMagnitude);

    // Create the uniform buffer :
    drawable_gridView->uboHandle_colorAttributes = this->createUniformBuffer(4 * sizeof(ColorChannelAttributes_GL), GL_STATIC_DRAW);
    this->setUniformBufferData(drawable_gridView->uboHandle_colorAttributes, 0, 32, &drawable_gridView->colorChannelAttributes[0]);
    this->setUniformBufferData(drawable_gridView->uboHandle_colorAttributes, 32, 32, &drawable_gridView->colorChannelAttributes[0]);
    this->setUniformBufferData(drawable_gridView->uboHandle_colorAttributes, 64, 32, &drawable_gridView->colorChannelAttributes[1]);
    this->setUniformBufferData(drawable_gridView->uboHandle_colorAttributes, 96, 32, &drawable_gridView->colorChannelAttributes[2]);

    //Send grid texture
    this->grids.back()->sendTetmeshToGPU(Grid::InfoToSend(Grid::InfoToSend::VERTICES | Grid::InfoToSend::NORMALS | Grid::InfoToSend::TEXCOORD | Grid::InfoToSend::NEIGHBORS));
}

void Scene::recompileShaders(bool verbose) {
    if(this->activeGrid >= 0)
        this->grids[this->activeGrid]->recompileShaders();

    GLuint newSelectionProgram	 = this->compileShaders("../shaders/selection.vert", "", "../shaders/selection.frag", true);

    if (newSelectionProgram) {
        glDeleteProgram(this->glSelection->getProgram());
        this->glSelection->setProgram(newSelectionProgram);
    }
    GLuint prog	 = this->compileShaders("../shaders/dualPass.vert", "", "../shaders/dualPass.frag", true);
    glDeleteProgram(this->dualPass_program);
    this->dualPass_program = prog;
}

GLuint Scene::compileShaders(std::string _vPath, std::string _gPath, std::string _fPath, bool verbose) {
    glUseProgram(0);
    this->shaderCompiler->reset();
    this->shaderCompiler->pragmaReplacement_file("include_color_shader", "../shaders/colorize_new_flow.glsl");
    this->shaderCompiler->vertexShader_file(_vPath).geometryShader_file(_gPath).fragmentShader_file(_fPath);
    if (this->shaderCompiler->compileShaders()) {
        return this->shaderCompiler->programName();
    }
    std::cerr << this->shaderCompiler->errorString() << '\n';
    return 0;
}

GLuint Scene::uploadTexture1D(const TextureUpload& tex) {
    if (this->context != nullptr) {
        if (this->context->isValid() == false) {
            throw std::runtime_error("No associated valid context");
        }
    } else {
        throw std::runtime_error("nullptr as context");
    }

    glEnable(GL_TEXTURE_1D);

    GLuint texHandle = 0;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_1D, texHandle);

    // Min and mag filters :
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, tex.minmag.x);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, tex.minmag.y);

    // Set the min and max LOD values :
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_LOD, tex.lod.x);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAX_LOD, tex.lod.y);

    // Set the wrap parameters :
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, tex.wrap.x);

    // Set the swizzle the user wants :
    glTexParameteriv(GL_TEXTURE_1D, GL_TEXTURE_SWIZZLE_RGBA, glm::value_ptr(tex.swizzle));

    // Set the pixel alignment :
    glPixelStorei(GL_PACK_ALIGNMENT, tex.alignment.x);
    glPixelStorei(GL_UNPACK_ALIGNMENT, tex.alignment.y);

    glTexImage1D(GL_TEXTURE_1D,	   // GLenum : Target
      static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
      tex.internalFormat,	 // GLint  : Number of color components in the picture.
      tex.size.x,	 // GLsizei: Image width
      static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
      tex.format,	 // GLenum : Format of the pixel data
      tex.type,	   // GLenum : Type (the data type as in uchar, uint, float ...)
      tex.data	  // void*  : Data to load into the buffer
    );

    return texHandle;
}

GLuint Scene::uploadTexture3D(const TextureUpload& tex) {
    if (this->context != nullptr) {
        if (this->context->isValid() == false) {
            throw std::runtime_error("No associated valid context");
        }
    } else {
        throw std::runtime_error("nullptr as context");
    }

    glEnable(GL_TEXTURE_3D);

    GLuint texHandle = 0;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_3D, texHandle);

    // Min and mag filters :
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, tex.minmag.x);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, tex.minmag.y);

    // Set the min and max LOD values :
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_LOD, tex.lod.x);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, tex.lod.y);

    // Set the wrap parameters :
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, tex.wrap.x);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, tex.wrap.y);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, tex.wrap.z);

    // Set the swizzle the user wants :
    glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, glm::value_ptr(tex.swizzle));

    // Set the pixel alignment :
    glPixelStorei(GL_PACK_ALIGNMENT, tex.alignment.x);
    glPixelStorei(GL_UNPACK_ALIGNMENT, tex.alignment.y);

    glTexImage3D(GL_TEXTURE_3D,	   // GLenum : Target
      static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
      tex.internalFormat,	 // GLint  : Number of color components in the picture. Here grayscale so GL_RED
      tex.size.x,	 // GLsizei: Image width
      tex.size.y,	 // GLsizei: Image height
      tex.size.z,	 // GLsizei: Image depth (number of layers)
      static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
      tex.format,	 // GLenum : Format of the pixel data
      tex.type,	   // GLenum : Type (the data type as in uchar, uint, float ...)
      tex.data	  // void*  : Data to load into the buffer
    );

    return texHandle;
}

GLuint Scene::newAPI_uploadTexture3D_allocateonly(const TextureUpload& tex) {
    if (this->context != nullptr) {
        if (this->context->isValid() == false) {
            throw std::runtime_error("No associated valid context");
        }
    } else {
        throw std::runtime_error("nullptr as context");
    }

    glEnable(GL_TEXTURE_3D);

    GLuint texHandle = 0;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_3D, texHandle);

    // Min and mag filters :
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, tex.minmag.x);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, tex.minmag.y);

    // Set the min and max LOD values :
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_LOD, tex.lod.x);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, tex.lod.y);

    // Set the wrap parameters :
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, tex.wrap.x);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, tex.wrap.y);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, tex.wrap.z);

    // Set the swizzle the user wants :
    glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, glm::value_ptr(tex.swizzle));

    // Set the pixel alignment :
    glPixelStorei(GL_PACK_ALIGNMENT, tex.alignment.x);
    glPixelStorei(GL_UNPACK_ALIGNMENT, tex.alignment.y);

    glTexImage3D(GL_TEXTURE_3D,	   // GLenum : Target
      static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
      tex.internalFormat,	 // GLint  : Number of color components in the picture. Here grayscale so GL_RED
      tex.size.x,	 // GLsizei: Image width
      tex.size.y,	 // GLsizei: Image height
      tex.size.z,	 // GLsizei: Image depth (number of layers)
      static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
      tex.format,	 // GLenum : Format of the pixel data
      tex.type,	   // GLenum : Type (the data type as in uchar, uint, float ...)
      nullptr	 // no data here !
    );

    return texHandle;
}

GLuint Scene::newAPI_uploadTexture3D(const GLuint texHandle, const TextureUpload& tex, std::size_t s, std::vector<std::uint16_t>& data) {
    if (this->context != nullptr) {
        if (this->context->isValid() == false) {
            throw std::runtime_error("No associated valid context");
        }
    } else {
        throw std::runtime_error("nullptr as context");
    }

    glEnable(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, texHandle);

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, s, tex.size.x, tex.size.y, 1, tex.format, tex.type, data.data());

    return texHandle;
}

glm::vec3 Scene::computePlanePositionsWithActivation() {
    glm::vec3 planePos = this->computePlanePositions();
    for(int i = 0; i < 3; ++i) {
        if(this->planeActivation[i] == 0.) {
            planePos[i] = -1000000.;
        }
    }
    return planePos;
}

glm::vec3 Scene::computePlanePositions() {
    if(this->getBaseMesh(this->activeMesh)) {
        std::pair<glm::vec3, glm::vec3> bbox = this->getSceneBBox();
        glm::vec3 position = bbox.first;
        glm::vec3 diagonal = bbox.second - bbox.first;
        glm::vec3 planePos			= (position + this->planeDisplacement * diagonal);
        return planePos;
    } else {
        return glm::vec3(0., 0., 0.);
    }
}

std::pair<glm::vec3, glm::vec3> Scene::getSceneBBox() {
    glm::vec3 bbMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    glm::vec3 bbMax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

    auto updateBBox = [&](glm::vec3& p) {
        for(int i = 0; i < 3; ++i) {
            if(p[i] < bbMin[i])
                bbMin[i] = p[i];
            if(p[i] > bbMax[i])
                bbMax[i] = p[i];
        }
    };

    for(const auto& mesh : this->meshes) {
        updateBBox(mesh.first->bbMin);
        updateBBox(mesh.first->bbMax);
    }

    for(const auto& mesh : this->graph_meshes) {
        updateBBox(mesh.first->bbMin);
        updateBBox(mesh.first->bbMax);
    }

    for(const auto& grid : this->grids) {
        updateBBox(grid->bbMin);
        updateBBox(grid->bbMax);
    }
    return {bbMin, bbMax};
}

GLuint Scene::createUniformBuffer(std::size_t size_bytes, GLenum draw_mode) {
    GLuint uniform_to_create = 0;
    glGenBuffers(1, &uniform_to_create);
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_to_create);
    glBufferData(GL_UNIFORM_BUFFER, size_bytes, NULL, draw_mode);
    if (glIsBuffer(uniform_to_create) == GL_FALSE) {
        std::cerr << "Error : tried to create a " << size_bytes << " bytes buffer but failed.\n";
        uniform_to_create = 0;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return uniform_to_create;
}

void Scene::setUniformBufferData(GLuint uniform_buffer, std::size_t begin_bytes, std::size_t size_bytes, GLvoid* data) {
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, begin_bytes, size_bytes, data);
}

void Scene::drawScene(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, float near, float far) {
    this->cameraPosition = camPos;
    if (this->shouldUpdateUserColorScales) {
        this->updateUserColorScale();
    }
    if (this->needUpdateMinMaxDisplayValues) {
        this->updateMinMaxDisplayValues();
    }

    this->setLightPosition(camPos);

    glEnable(GL_DEPTH_TEST);
    glEnablei(GL_BLEND, 0);
    glEnable(GL_TEXTURE_3D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 transfoMat = glm::mat4(1.f);
    /* Manipulator drawing  */

    glm::mat4 mMat(1.0f);
    /***********************/

    if(this->displayGrid) {
        if(this->grids.size() > 0) {
            for(auto i : this->gridsToDraw) {
                glm::vec3 planePosition = this->computePlanePositionsWithActivation();
                //if(i == 0)
                //    planePosition += this->grids[i]->getWorldVoxelSize()/2.f;
                //glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
                //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                //this->grids[i]->drawGridFirstPass(mvMat, pMat, camPos, planePosition, this->planeDirection, false, w, h);

                glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                this->grids[i]->drawGrid(mvMat, pMat, camPos, planePosition, this->planeDirection, false, w, h);
            }

            float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                                     // positions   // texCoords
                                     -1.0f,  1.0f,  0.0f, 1.0f,
                                     -1.0f, -1.0f,  0.0f, 0.0f,
                                     1.0f, -1.0f,  1.0f, 0.0f,

                                     -1.0f,  1.0f,  0.0f, 1.0f,
                                     1.0f, -1.0f,  1.0f, 0.0f,
                                     1.0f,  1.0f,  1.0f, 1.0f
                                   };

            //glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
            // clear all relevant buffers
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(this->dualPass_program);
            unsigned int quadVBO;
            glDeleteBuffers(1, &quadVBO);
            glGenBuffers(1, &quadVBO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            auto getUniform = [&](const char* name) -> GLint {
                GLint g = glGetUniformLocation(this->dualPass_program, name);
                return g;
            };

            std::size_t tex = 0;
            glActiveTexture(GL_TEXTURE0 + tex);
            glBindTexture(GL_TEXTURE_2D, this->grids[0]->dualRenderingTexture);
            glUniform1i(getUniform("screenTexture"), tex);
            tex++;

            glActiveTexture(GL_TEXTURE0 + tex);
            glBindTexture(GL_TEXTURE_2D, this->grids[0]->depthTexture);
            glUniform1i(getUniform("depthTexture"), tex);
            tex++;

            int alphaRendering = 0;
            int dualRendering = 0;
            if(this->grids.size() == 2) {
                glActiveTexture(GL_TEXTURE0 + tex);
                glBindTexture(GL_TEXTURE_2D, this->grids[1]->dualRenderingTexture);
                glUniform1i(getUniform("screenTexture2"), tex);
                tex++;

                glActiveTexture(GL_TEXTURE0 + tex);
                glBindTexture(GL_TEXTURE_2D, this->grids[1]->depthTexture);
                glUniform1i(getUniform("depthTexture2"), tex);
                tex++;

                dualRendering = 1;
                if(this->registrationRendering)
                    alphaRendering = 1;
            }
            glUniform1ui(getUniform("dualRendering"), dualRendering);
            glUniform1ui(getUniform("alphaRendering"), alphaRendering);
            glUniform1ui(getUniform("funnyRender"), funnyRender);
            glUniform1f(getUniform("alphaBlend"), this->alphaBlend);
            glUniform1f(getUniform("near"), near);
            glUniform1f(getUniform("far"), far);

            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnable(GL_DEPTH_TEST);
        }
    }

    if(this->displayMesh) {
        for(int i = 0; i < this->meshes.size(); ++i) {
            this->meshes[i].first->draw(pMat, mvMat, glm::vec4{camPos, 1.f}, this->computePlanePositionsWithActivation(), this->planeDirection);
        }
    }

    this->glSelection->draw(mvMat, pMat, glm::value_ptr(mMat));

    //Legacy openGL draw //

    this->activateCuttingPlaneLegacyOpenGL();

    for(int i = 0; i < this->graph_meshes.size(); ++i) {
        this->graph_meshes[i].first->draw(pMat, mvMat, glm::value_ptr(mMat));
    }

    if(displayGridBBox) {
        for(auto grid : grids) {
            grid->drawBBox(this->computePlanePositions());
        }
    }

    for(auto box : boxes) {
        this->drawBox(box);
    }

    if(this->meshManipulator) {
        if(this->displayXRayManipulators)
            glClear(GL_DEPTH_BUFFER_BIT);
        this->meshManipulator->draw();
    }

    this->deactivateCuttingPlaneLegacyOpenGL();

    // We dont want the guizmo to be affected by cutting planes
    if(this->meshManipulator)
        this->meshManipulator->drawGuizmo();
}

void Scene::activateCuttingPlaneLegacyOpenGL() {
    glm::vec3 planePos = this->computePlanePositionsWithActivation();
    glm::tvec4<double>planeX(planeDirection.x, 0, 0, planeDirection.x*-planePos.x);
    glm::tvec4<double>planeY(0, planeDirection.y, 0, planeDirection.y*-planePos.y);
    glm::tvec4<double>planeZ(0, 0, planeDirection.z, planeDirection.z*-planePos.z);
    glClipPlane(GL_CLIP_PLANE0, glm::value_ptr(planeX));
    glClipPlane(GL_CLIP_PLANE1, glm::value_ptr(planeY));
    glClipPlane(GL_CLIP_PLANE2, glm::value_ptr(planeZ));
    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_CLIP_PLANE1);
    glEnable(GL_CLIP_PLANE2);
}

void Scene::deactivateCuttingPlaneLegacyOpenGL() {
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
}

void Scene::updateMinMaxDisplayValues() {
    if(grids.size() == 0)
        return;

    this->needUpdateMinMaxDisplayValues = false;
    for(auto& grid : this->grids) {
        grid->updateMinMaxDisplayValues();
    }
    Q_EMIT meshMoved();// To update the 2D viewer
}

GLuint Scene::uploadTexture2D(const TextureUpload& tex) {
    glEnable(GL_TEXTURE_2D);

    GLuint texHandle = 0;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);

    // Min and mag filters :
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex.minmag.x);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex.minmag.y);

    // Set the min and max LOD values :
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, tex.lod.x);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, tex.lod.y);

    // Set the wrap parameters :
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex.wrap.x);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex.wrap.y);

    // Set the swizzle the user wants :
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, glm::value_ptr(tex.swizzle));

    // Set the pixel alignment :
    glPixelStorei(GL_PACK_ALIGNMENT, tex.alignment.x);
    glPixelStorei(GL_UNPACK_ALIGNMENT, tex.alignment.y);

    glTexImage2D(GL_TEXTURE_2D,	   // GLenum : Target
      static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
      tex.internalFormat,	 // GLint  : Number of color components in the picture. Here grayscale so GL_RED
      tex.size.x,	 // GLsizei: Image width
      tex.size.y,	 // GLsizei: Image height
      static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
      tex.format,	 // GLenum : Format of the pixel data
      tex.type,	   // GLenum : Type (the data type as in uchar, uint, float ...)
      tex.data	  // void*  : Data to load into the buffer
    );

    return texHandle;
}

GLuint Scene::updateFBOOutputs(glm::ivec2 dimensions, GLuint fb_handle, GLuint old_texture) {
    // Silently ignore attempts to modify the 0-th FBO, which is provided by the
    // windowing system, and thus unmodifiable from GL/application code.
    if (fb_handle == 0) {
        return 0;
    }
    // Remove old textures, if updating the FBO from a resize event for example.
    if (old_texture != 0) {
        glDeleteTextures(1, &old_texture);
    }

    // Upload an _empty_ texture with no mipmapping, and nearest neighbor :
    GLuint new_tex = 0;
    TextureUpload t{};
    t.minmag		 = glm::ivec2{GL_NEAREST, GL_NEAREST};
    t.lod			 = glm::vec2{-1000.f, -1000.f};
    t.internalFormat = GL_RGBA32F;	  // RGBA 32-bit floating point, unnormalized
    t.size			 = glm::ivec3(dimensions, 0);

    // Create the texture object itself :
    new_tex = this->uploadTexture2D(t);
    if (new_tex == 0) {
        std::cerr << "Error : Could not create a framebuffer texture.\n";
        return 0;
    }

    // Bind framebuffer, and bind texture to a color attachment :
    glBindFramebuffer(GL_FRAMEBUFFER, fb_handle);
    // Bind texture 'new_tex' at level 0 to the color attachment 1 :
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, new_tex, 0);

    // Specify the texture ouptuts in the framebuffer state :
    GLenum colorAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, colorAttachments);

    // Check the framebuffer is complete :
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "An error occured while specifying the framebuffer attachments.\n";
    }

    return new_tex;
}

void Scene::setColorFunction_r(ColorFunction _c) {
    if (_c == ColorFunction::SingleChannel && this->channels_r != ColorFunction::SingleChannel) {
        this->selectedChannel_r = 0;
    }
    if (_c == ColorFunction::SingleChannel && this->channels_r == ColorFunction::SingleChannel) {
        this->selectedChannel_r = (this->selectedChannel_r + 1) % 2;
    }
    this->channels_r = _c;
    // WARNING : below is a dirty hack, since UBO expects a color scale index but this gives it a constant
    // value that only has a meaning on the host (CPU) side.
    if (_c == ColorFunction::ColorMagnitude) {
        _c = ColorFunction::HSV2RGB;
    } else if (_c == ColorFunction::HSV2RGB) {
        _c = ColorFunction::HistologyHandE;
    }
    if(grids.size() > 0) {
        grids[0]->colorChannelAttributes[0].setColorScale(static_cast<int>(_c));
        grids[0]->colorChannelAttributes[1].setColorScale(static_cast<int>(_c));
    }
    this->needUpdateMinMaxDisplayValues = true;
}

void Scene::setColorFunction_g(ColorFunction _c) {
    if (_c == ColorFunction::SingleChannel && this->channels_g != ColorFunction::SingleChannel) {
        this->selectedChannel_g = 0;
    }
    if (_c == ColorFunction::SingleChannel && this->channels_g == ColorFunction::SingleChannel) {
        this->selectedChannel_g = (this->selectedChannel_g + 1) % 2;
    }
    this->channels_g = _c;
    // WARNING : below is a dirty hack, since UBO expects a color scale index but this gives it a constant
    // value that only has a meaning on the host (CPU) side.
    if (_c == ColorFunction::HSV2RGB) {
        _c = ColorFunction::HistologyHandE;
    }
    if(grids.size() > 1) {
        grids[1]->colorChannelAttributes[0].setColorScale(static_cast<int>(_c));
        grids[1]->colorChannelAttributes[1].setColorScale(static_cast<int>(_c));
    }
    this->needUpdateMinMaxDisplayValues = true;
}

void Scene::slotSetNormalizedPlaneDisplacement(CuttingPlaneDirection direction, float scalar) {
    switch(direction) {
        case X:
            this->planeDisplacement.x = scalar;
            break;
        case Y:
            this->planeDisplacement.y = scalar;
            break;
        case Z:
            this->planeDisplacement.z = scalar;
            break;
        case XYZ:
            break;
    }
    Q_EMIT planesMoved(this->computePlanePositions());
    Q_EMIT planeControlWidgetNeedUpdate(this->planeDisplacement);
}

void Scene::slotSetPlaneDisplacement(std::string gridName, CuttingPlaneDirection direction, float scalar) {
    glm::vec3 dimension = this->grids[this->getGridIdx(gridName)]->getDimensions();
    switch(direction) {
        case X:
            this->planeDisplacement.x = scalar/dimension.x;
            break;
        case Y:
            this->planeDisplacement.y = scalar/dimension.y;
            break;
        case Z:
            this->planeDisplacement.z = scalar/dimension.z;
            break;
        case XYZ:
            break;
    }
    Q_EMIT planesMoved(this->computePlanePositions());
}

void Scene::slotTogglePlaneDirection(CuttingPlaneDirection direction) {
    switch(direction) {
        case X:
            this->planeDirection.x = -this->planeDirection.x;
            break;
        case Y:
            this->planeDirection.y = -this->planeDirection.y;
            break;
        case Z:
            this->planeDirection.z = -this->planeDirection.z;
            break;
        case XYZ:
            this->planeDirection = -this->planeDirection;
            break;
    }
    Q_EMIT planesMoved(this->computePlanePositions());
}


void Scene::slotToggleDisplayPlane(CuttingPlaneDirection direction, bool display) {
    float value = 0;
    if(display)
        value = 1;
    switch(direction) {
        case X:
            this->planeActivation.x = value;
            break;
        case Y:
            this->planeActivation.y = value;
            break;
        case Z:
            this->planeActivation.z = value;
            break;
        case XYZ:
            this->planeActivation = glm::vec3(value, value, value);
            break;
    }
    Q_EMIT planesMoved(this->computePlanePositions());
}

void Scene::setDisplayRange(int gridIdx, ValueType type, double value) {
    if(this->grids.size() > 0 && gridIdx < this->grids.size()) {
        // Always 0 here because this is the "main" channel
        // In this version the software can't handle multiple channel images
        if(type == ValueType::MIN)
            this->grids[gridIdx]->colorChannelAttributes[0].setMinVisible(value);
        else
            this->grids[gridIdx]->colorChannelAttributes[0].setMaxVisible(value);
    }
    this->updateMinMaxDisplayValues();
}

double Scene::getDisplayRange(int gridIdx, ValueType type) {
    if(this->grids.size() > 0 && gridIdx < this->grids.size()) {
        if(type == ValueType::MIN)
            return this->grids[gridIdx]->colorChannelAttributes[0].getVisibleRange().x;
        else
            return this->grids[gridIdx]->colorChannelAttributes[0].getVisibleRange().y;
    }
    return 0.;
}

void Scene::setMinMaxDisplayRange(int gridIdx, ValueType type, double value) {
    if(this->grids.size() > 0 && gridIdx < this->grids.size()) {
        // Always 0 here because this is the "main" channel
        // In this version the software can't handle multiple channel images
        if(type == ValueType::MIN)
            this->grids[gridIdx]->colorChannelAttributes[0].setMinColorScale(value);
        else
            this->grids[gridIdx]->colorChannelAttributes[0].setMaxColorScale(value);
    }
    this->updateMinMaxDisplayValues();
}

double Scene::getMinMaxDisplayRange(int gridIdx, ValueType type) {
    if(this->grids.size() > 0 && gridIdx < this->grids.size()) {
        if(type == ValueType::MIN)
            return this->grids[gridIdx]->colorChannelAttributes[0].getColorRange().x;
        else
            return this->grids[gridIdx]->colorChannelAttributes[0].getColorRange().y;
    }
    return 0.;
}

void Scene::setUserColorScale(int gridIdx, ValueType type, glm::vec3 color) {
    if(this->grids.size() > 0 && gridIdx < this->grids.size()) {
        if(type == ValueType::MIN)
            this->grids[gridIdx]->color_0 = color;
        else
            this->grids[gridIdx]->color_1 = color;
    }
    //this->updateUserColorScale();
    this->shouldUpdateUserColorScales = true;
    emit this->colorChanged();
}

void Scene::updateUserColorScale() {
    this->shouldUpdateUserColorScales = false;
    TextureUpload colorScaleUploadParameters;
    std::size_t textureSize = this->maximumTextureSize / 2u;
    float textureSize_f		= static_cast<float>(this->maximumTextureSize / 2u);
    std::vector<glm::vec3> colorScaleData_user0(textureSize);
    std::vector<glm::vec3> colorScaleData_user1(textureSize);

    // The color scale 0 first :
    glm::vec3 color0 = glm::vec3(1., 0., 0.);
    glm::vec3 color1 = glm::vec3(0., 0., 1.);
    if(this->grids.size() > 0) {
        color0 = this->grids[0]->color_0;
        color1 = this->grids[0]->color_1;
    }
    glm::vec3 color0_second = glm::vec3(1., 0., 0.);
    glm::vec3 color1_second = glm::vec3(0., 0., 1.);
    if(this->grids.size() > 1) {
        color0_second = this->grids[1]->color_0;
        color1_second = this->grids[1]->color_1;
    }
    for (std::size_t i = 0; i < textureSize; ++i) {
        colorScaleData_user0[i] = glm::mix(color0, color1, static_cast<float>(i) / textureSize_f);
        colorScaleData_user1[i] = glm::mix(color0_second, color1_second, static_cast<float>(i) / textureSize_f);
    }

    colorScaleUploadParameters.minmag.x	 = GL_LINEAR;
    colorScaleUploadParameters.minmag.y	 = GL_LINEAR;
    colorScaleUploadParameters.lod.y	 = -1000.f;
    colorScaleUploadParameters.wrap.x	 = GL_CLAMP_TO_EDGE;
    colorScaleUploadParameters.wrap.y	 = GL_CLAMP_TO_EDGE;
    colorScaleUploadParameters.wrap.z	 = GL_CLAMP_TO_EDGE;
    colorScaleUploadParameters.swizzle.r = GL_RED;
    colorScaleUploadParameters.swizzle.g = GL_GREEN;
    colorScaleUploadParameters.swizzle.b = GL_BLUE;
    colorScaleUploadParameters.swizzle.a = GL_ONE;

    colorScaleUploadParameters.level		  = 0;
    colorScaleUploadParameters.internalFormat = GL_RGB;
    colorScaleUploadParameters.size.x		  = textureSize;
    colorScaleUploadParameters.size.y		  = 1;
    colorScaleUploadParameters.size.z		  = 1;
    colorScaleUploadParameters.format		  = GL_RGB;
    colorScaleUploadParameters.type			  = GL_FLOAT;
    if(this->grids.size() > 0) {
        colorScaleUploadParameters.data			  = colorScaleData_user0.data();
        this->grids[0]->colorScaleUser = this->uploadTexture1D(colorScaleUploadParameters);
    }

    if(this->grids.size() > 1) {
        colorScaleUploadParameters.data	 = colorScaleData_user1.data();
        this->grids[1]->colorScaleUser = this->uploadTexture1D(colorScaleUploadParameters);
    }
}

// TODO: replace this function by a real update function
void Scene::updateTetmeshAllGrids(bool updateAllInfos) {
    for(auto grid : grids) {
        if(updateAllInfos)
            grid->sendTetmeshToGPU(Grid::InfoToSend(Grid::InfoToSend::VERTICES | Grid::InfoToSend::NORMALS | Grid::InfoToSend::NEIGHBORS | Grid::InfoToSend::TEXCOORD));
        else
            grid->sendTetmeshToGPU(Grid::InfoToSend(Grid::InfoToSend::VERTICES | Grid::InfoToSend::NORMALS));
    }
    Q_EMIT meshMoved();
}

/**********************************************************************/

/*********************************/
/* Slots */
/*********************************/

void Scene::toggleDisplayTetmesh(bool value) {
    if(this->activeGrid >= 0)
        this->grids[this->activeGrid]->displayTetmesh = value;
}

void Scene::setColorChannel(ColorChannel mode) {
    this->rgbMode = mode;
    std::for_each(this->grids.begin(), this->grids.end(), [this](DrawableGrid * gridView) {
        switch (this->rgbMode) {
            case ColorChannel::None:
                gridView->colorChannelAttributes[0].setHidden();
                gridView->colorChannelAttributes[1].setHidden();
                break;
            case ColorChannel::RedOnly:
                gridView->colorChannelAttributes[0].setHidden(false);
                gridView->colorChannelAttributes[1].setHidden();
                break;
            case ColorChannel::GreenOnly:
                gridView->colorChannelAttributes[0].setHidden();
                gridView->colorChannelAttributes[1].setHidden(false);
                break;
            case ColorChannel::RedAndGreen:
                gridView->colorChannelAttributes[0].setHidden(false);
                gridView->colorChannelAttributes[1].setHidden(false);
                break;
            default:
                // do nothing
                break;
        }
    });
    this->needUpdateMinMaxDisplayValues = true;
    switch (mode) {
        case ColorChannel::None:
            std::cerr << "Set mode to none" << '\n';
            break;
        case ColorChannel::RedOnly:
            std::cerr << "Set mode to red" << '\n';
            break;
        case ColorChannel::GreenOnly:
            std::cerr << "Set mode to green" << '\n';
            break;
        case ColorChannel::RedAndGreen:
            std::cerr << "Set mode to both channels" << '\n';
            break;
        case ColorChannel::HandEColouring:
            std::cerr << "Set mode to both channels" << '\n';
            break;
        default:
            std::cerr << "Cannot set unknown mode\n";
            break;
    }
}

bool Scene::openGraph(const std::string& name, const std::string& filename, const glm::vec4& color) {
    //this->surfaceMesh = new SurfaceMesh("/home/thomas/data/Data/Mesh/bunny_lowres.off");
    this->graph_meshes.push_back(std::pair<GraphMesh*, std::string>(nullptr, name));
    this->graph_meshes.back().first = new GraphMesh(filename);

    Q_EMIT meshAdded(name, false, false);
    this->changeActiveMesh(name);
    return true;
}

bool Scene::openMesh(const std::string& name, const std::string& filename, const glm::vec4& color) {
    //this->surfaceMesh = new SurfaceMesh("/home/thomas/data/Data/Mesh/bunny_lowres.off");
    this->meshes.push_back(std::pair<SurfaceMesh*, std::string>(nullptr, name));
    this->meshes.back().first = new SurfaceMesh(filename);
    this->meshes.back().first->initializeGL(this);
    this->meshes.back().first->color = color;

    Q_EMIT meshAdded(name, false, false);
    this->changeActiveMesh(name);
    return true;
}

bool Scene::linkCage(const std::string& cageName, BaseMesh * meshToDeform, const bool MVC) {
    SurfaceMesh * cage = this->getMesh(cageName);

    if(!cage) {
        std::cout << "ERROR: no cage mesh provided" << std::endl;
        return false;
    }

    if(!meshToDeform) {
        std::cout << "ERROR: no surface mesh provided" << std::endl;
        return false;
    }

    int cageIdx = this->getMeshIdx(cageName);
    //delete this->meshes[cageIdx].first;
    glm::vec4 color = this->meshes[cageIdx].first->color;
    delete this->meshes[cageIdx].first;

    TetMesh * tetMesh = dynamic_cast<TetMesh*>(meshToDeform);
    if(tetMesh) {
        //this->meshes[cageIdx].first = new CageGreenLRI(this->meshes[cageIdx].first, tetMesh);
    } else {
        if(MVC)
            this->meshes[cageIdx].first = new CageMVC(this->meshes[cageIdx].first, meshToDeform);
        else
            this->meshes[cageIdx].first = new CageGreen(this->meshes[cageIdx].first, meshToDeform);
    }

    this->meshes[cageIdx].first->initializeGL(this);
    this->meshes[cageIdx].first->color = color;

    //this->changeActiveMesh(name);

    //this->updateSceneBBox(this->meshes.back().first->bbMin, this->meshes.back().first->bbMax);
    //this->updateSceneCenter();

    // TODO change this
    //Q_EMIT meshAdded(name, false, true);
    return true;
}

bool Scene::openCage(const std::string& name, const std::string& filename, BaseMesh * surfaceMeshToDeform, const bool MVC, const glm::vec4& color) {
    //this->cage = new CageMVC("/home/thomas/data/Data/Mesh/bunny_cage.off", this->surfaceMesh);
    if(!surfaceMeshToDeform) {
        std::cout << "ERROR: no surface mesh provided" << std::endl;
        return false;
    }

    TetMesh * tetMesh = dynamic_cast<TetMesh*>(surfaceMeshToDeform);
    if(MVC) {
        this->meshes.push_back(std::pair<SurfaceMesh*, std::string>(new CageMVC(filename, surfaceMeshToDeform), name));
    } else {
        if(tetMesh) {
            this->meshes.push_back(std::pair<SurfaceMesh*, std::string>(new CageGreenLRI(filename, tetMesh), name));
        } else {
            this->meshes.push_back(std::pair<SurfaceMesh*, std::string>(new CageGreen(filename, surfaceMeshToDeform), name));
        }
    }

    this->meshes.back().first->initializeGL(this);
    this->meshes.back().first->color = color;

    Q_EMIT meshAdded(name, false, true);
    this->changeActiveMesh(name);
    return true;
}

bool Scene::openGrid(const std::string& name, const std::vector<std::string>& imgFilenames, const int subsample, const glm::vec3& sizeVoxel, const glm::vec3& nbCubeGridTransferMesh) {
    int autofitSubsample = this->autofitSubsample(subsample, imgFilenames);
    Grid * newGrid = new Grid(imgFilenames, autofitSubsample, sizeVoxel, nbCubeGridTransferMesh);
    this->addGridToScene(name, newGrid);
    return true;
}

bool Scene::openGrid(const std::string& name, const std::vector<std::string>& imgFilenames, const int subsample, const glm::vec3& sizeVoxel, const std::string& transferMeshFileName) {
    int autofitSubsample = this->autofitSubsample(subsample, imgFilenames);
    //TODO: sizeVoxel isn't take into account with loading a custom transferMesh
    Grid * newGrid = new Grid(imgFilenames, autofitSubsample, sizeVoxel, transferMeshFileName);
    this->addGridToScene(name, newGrid);
    return true;
}

void Scene::addGridToScene(const std::string& name, Grid * newGrid) {
    Grid * gridView = newGrid;
    this->grids.push_back(gridView);
    this->grids.back()->initializeGL(this);

    this->activeGrid += 1;

    this->addGrid();
    this->grids_name.push_back(name);

    this->updateSceneCenter();
    std::cout << "New grid added with BBox:" << this->grids.back()->bbMax << std::endl;

    Q_EMIT meshAdded(name, true, false);
    this->changeActiveMesh(name);

    this->gridsToDraw.push_back(this->activeGrid);
    if(this->grids.size() == 2) {
        this->grids[0]->drawSliceOnly = true;
    }
}

int Scene::autofitSubsample(int initialSubsample, const std::vector<std::string>& imgFilenames) {
    return initialSubsample;
    float percentageOfMemory = 0.7;
    int gpuMemoryInGB = 2;
    double gpuMemoryInBytes = double(gpuMemoryInGB) * double(1073741824.);

    int finalSubsample = initialSubsample;
    bool autofitSizeRequired = false;
    bool autofitMemoryRequired = false;

    TIFFReaderLibtiff tiffReader(imgFilenames);
    glm::vec3 imgResolution = tiffReader.getImageResolution();
    float maxResolution = std::max(imgResolution[0], std::max(imgResolution[1], imgResolution[2]));
    if(maxResolution > this->maximumTextureSize) {
        autofitSizeRequired = true;
        std::cout << "INFO: image too large to fit in the GPU, size " << imgResolution << std::endl;
    }

    if(autofitSizeRequired) {// Auto-fit size activated
        std::cout << "Auto-fit size activated" << std::endl;
        finalSubsample = std::max(2.f, std::ceil(maxResolution / (float(this->maximumTextureSize))));
        std::cout << "Subsample set to [" << finalSubsample << "]" << std::endl;
    }

    double dataMemory = 1;
    for(int dim = 0; dim < 3; ++dim) {
        dataMemory *= imgResolution[dim];// Because data are cast as uint16_t internally
    }
    dataMemory *= 4;
    std::cout << "GPU memory available: [" << gpuMemoryInBytes / 1073741824. << "] Go" << std::endl;
    std::cout << "Data memory usage: [" << dataMemory / 1073741824. << "] Go" << std::endl;
    if(gpuMemoryInBytes < dataMemory) {
        autofitMemoryRequired = true;
        std::cout << "INFO: image too large to fit in the GPU, memory [" << dataMemory / 1073741824. << "] Go" << std::endl;
    }

    autofitMemoryRequired = false;
    if(autofitMemoryRequired) {
        std::cout << "Auto-fit memory activated" << std::endl;
        finalSubsample = std::max(2., dataMemory / (gpuMemoryInBytes*double(percentageOfMemory)));// We want to fill 70% of the memory
        std::cout << "Subsample set to [" << finalSubsample << "]" << std::endl;
        std::cout << "GPU memory used by the data: [" << percentageOfMemory * 100. << "%]" << std::endl;
        std::cout << "GPU memory usage reduced to: [" << (dataMemory/double(finalSubsample))/1073741824. << "] Go" << std::endl;
    }
    return finalSubsample;
}

int Scene::getMeshIdx(const std::string& name) {
    for(int i = 0; i < this->meshes.size(); ++i) {
        if(this->meshes[i].second == name) {
            return i;
        }
    }
    std::cout << "Error: wrong mesh name to get idx: [" << name << "]" << std::endl;
    return -1;
}

SurfaceMesh * Scene::getMesh(const std::string& name) {
    for(int i = 0; i < this->meshes.size(); ++i) {
        if(this->meshes[i].second == name) {
            return this->meshes[i].first;
        }
    }
    std::cout << "Error: wrong mesh name: [" << name << "]" << std::endl;
    return nullptr;
}

Cage * Scene::getCage(const std::string& name) {
    return dynamic_cast<Cage*>(this->getMesh(name));
}

void Scene::changeSceneRadius(float sceneRadius) {
    if(this->graph_meshes.size() > 0)
        this->graph_meshes.front().first->zoom(sceneRadius);
    if(this->meshManipulator) {
        this->meshManipulator->zoom(sceneRadius);
    }
}

void Scene::updateSceneRadius() {
    Q_EMIT sceneRadiusChanged(this->getSceneRadius());
    this->changeSceneRadius(this->distanceFromCamera);
}

float Scene::getSceneRadius() {
    BaseMesh * mesh = this->getBaseMesh(this->activeMesh);
    if(mesh) {
        return glm::length(mesh->getDimensions());
    }
    return 1.;
}


glm::vec3 Scene::getSceneCenter() {

    BaseMesh * mesh = this->getBaseMesh(this->activeMesh);
    if(mesh) {
        std::cout << "Update scene center to origin of [" << this->activeMesh << "] which is " << mesh->getOrigin() << std::endl;
        return mesh->getOrigin();
    } //else {
        //return (this->sceneBBMax + this->sceneBBMin)/2.f;
    //}
    return glm::vec3(0., 0., 0.);
}

void Scene::updateSceneCenter() {
    Q_EMIT sceneCenterChanged(this->getSceneCenter());
}

void Scene::toggleBindMeshToCageMove() {
    this->toggleBindMeshToCageMove(this->activeMesh);
}

void Scene::toggleBindMeshToCageMove(const std::string& name) {
    Cage * cage = this->getCage(name);
    if(cage) {
        if(cage->moveMeshToDeform) {
            cage->unbindMovementWithDeformedMesh();
        } else {
            cage->bindMovementWithDeformedMesh();
        }
    } else {
        std::cout << "Error: the selected mesh isn't a cage or doesn't exist" << std::endl;
    }
}

void Scene::setBindMeshToCageMove(const std::string& name, bool state) {
    Cage * cage = this->getCage(name);
    if(cage) {
        if(state) {
            cage->bindMovementWithDeformedMesh();
        } else {
            cage->unbindMovementWithDeformedMesh();
        }
    } else {
        std::cout << "Error: the selected mesh isn't a cage or doesn't exist" << std::endl;
    }
}

bool Scene::isGrid(const std::string& name) {
    for(int i = 0; i < this->grids_name.size(); ++i) {
        if(this->grids_name[i] == name) {
            return true;
        }
    }
    return false;
}

int Scene::getGridIdxLinkToCage(const std::string& name) {
    for(int i = 0; i < this->cageToGrid.size(); ++i) {
        if(this->cageToGrid[i].first == name) {
            return this->getGridIdx(this->cageToGrid[i].second);
        }
    }
    return -1;
}

int Scene::getGridIdx(const std::string& name) {
    for(int i = 0; i < this->grids_name.size(); ++i) {
        if(this->grids_name[i] == name) {
            return i;
        }
    }
    return -1;
}

std::pair<glm::vec3, glm::vec3> Scene::getBbox(const std::string& name) {
    BaseMesh * mesh = this->getBaseMesh(name);
    if(mesh) {
        return std::make_pair(mesh->bbMin, mesh->bbMax);
    } else {
        return std::make_pair(glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.));
    }
}

void Scene::openAtlas() {
         ///home/thomas/data/Data/teletravail/
         ///home/thomas/data/Data/Demo
         //this->openGrid(std::string("atlas"), {std::string("/home/thomas/data/Data/teletravail/atlas.tiff")}, 1, std::string("/home/thomas/data/Data/teletravail/atlas-transfert.mesh"));
         this->openGrid(std::string("atlas"), {std::string("/home/thomas/data/Data/Demo/atlas/atlas.tiff")}, 1, glm::vec3(1., 1., 1.), std::string("/home/thomas/data/Data/Demo/atlas/atlas-transfert.mesh"));
         //this->openCage(std::string("cage"), std::string("/home/thomas/data/Data/teletravail/atlas-cage-hyperdilated.off"), std::string("atlas"), true);
         this->openCage(std::string("cage"), std::string("/home/thomas/data/Data/Demo/atlas/atlas-cage_fixed.off"), std::string("atlas"), true);
         //this->getCage(std::string("cage"))->setARAPDeformationMethod();
         this->getCage(std::string("cage"))->unbindMovementWithDeformedMesh();
         this->getCage(std::string("cage"))->setOrigin(this->getBaseMesh("atlas")->getOrigin());
         this->getCage(std::string("cage"))->bindMovementWithDeformedMesh();

         //this->applyCage(std::string("cage"), std::string("/home/thomas/data/Data/teletravail/atlas-cage-hyperdilated-rigidRegister-lightsheet_2.off"));

         //this->getCage(std::string("cage"))->unbindMovementWithDeformedMesh();
         //this->applyCage(std::string("cage"), std::string("/home/thomas/data/Data/Data/good_cage.off"));
         //this->getCage(std::string("cage"))->bindMovementWithDeformedMesh();

         this->changeActiveMesh("cage");
 }

 void Scene::openIRM() {
         this->openGrid(std::string("irm"), {std::string("/home/thomas/data/Data/teletravail/irm.tif")}, 1, glm::vec3(3.9, 3.9, 50));
         this->changeActiveMesh("irm");
 }

void Scene::init() {
}

BaseMesh * Scene::getBaseMesh(const std::string& name) {
    SurfaceMesh * surface = nullptr;
    for(int i = 0; i < this->meshes.size(); ++i) {
        if(this->meshes[i].second == name) {
            return this->meshes[i].first;
        }
    }
    for(int i = 0; i < this->graph_meshes.size(); ++i) {
        if(this->graph_meshes[i].second == name) {
            return this->graph_meshes[i].first;
        }
    }
    for(int i = 0; i < this->grids_name.size(); ++i) {
        if(this->grids_name[i] == name) {
            return this->grids[i];
        }
    }
    //std::cout << "Wrong base mesh name: " << name << std::endl;
    return nullptr;
}

void Scene::updateTools(UITool::MeshManipulatorType tool) {
    if(this->meshManipulator) {
        delete this->meshManipulator;
        this->meshManipulator = nullptr;
    }

    BaseMesh * mesh = this->getBaseMesh(this->activeMesh);

    if(!mesh)
        return;

    if(tool == UITool::MeshManipulatorType::NONE || !this->getBaseMesh(this->activeMesh)) {
        Q_EMIT this->needDisplayInfos("");
        return;
    }

    this->currentTool = tool;
    if(tool == UITool::MeshManipulatorType::DIRECT) {
        this->meshManipulator = new UITool::DirectManipulator(mesh);
        dynamic_cast<UITool::DirectManipulator*>(this->meshManipulator)->setDefaultManipulatorColor(glm::vec3(1., 1., 0.));
    } else if(tool == UITool::MeshManipulatorType::POSITION) {
        this->meshManipulator = new UITool::GlobalManipulator(mesh);
    } else if(tool == UITool::MeshManipulatorType::ARAP) {
        this->meshManipulator = new UITool::ARAPManipulator(mesh);
    } else if(tool == UITool::MeshManipulatorType::SLICE) {
        this->meshManipulator = new UITool::SliceManipulator(mesh);
    } else if(tool == UITool::MeshManipulatorType::MARKER) {
        if(this->grids.size() > 0) {
            this->meshManipulator = new UITool::MarkerManipulator(mesh, this->grids[0]);
            this->meshManipulator->setSize(UITool::GL::SPHERE, 0.007);
        }
    }

    QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needChangeCursor(UITool::CursorType)), this, SLOT(changeCursor(UITool::CursorType)));
    QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needChangeCursorInPlanarView(UITool::CursorType)), this, SLOT(changeCursorInPlanarView(UITool::CursorType)));
    QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needDisplayVertexInfo(std::pair<int,glm::vec3>)), this, SLOT(changeSelectedPoint(std::pair<int,glm::vec3>)));
    QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needDisplayInfos(const std::string&)), this, SIGNAL(needDisplayInfos(const std::string&)));
    QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(meshIsModified()), this, SIGNAL(meshMoved()));

    QObject::connect(this, SIGNAL(keyPressed(QKeyEvent*)), dynamic_cast<QObject*>(this->meshManipulator), SLOT(keyPressed(QKeyEvent*)));
    QObject::connect(this, SIGNAL(keyReleased(QKeyEvent*)), dynamic_cast<QObject*>(this->meshManipulator), SLOT(keyReleased(QKeyEvent*)));
    QObject::connect(this, SIGNAL(mousePressed(QMouseEvent*)), dynamic_cast<QObject*>(this->meshManipulator), SLOT(mousePressed(QMouseEvent*)));
    QObject::connect(this, SIGNAL(mouseReleased(QMouseEvent*)), dynamic_cast<QObject*>(this->meshManipulator), SLOT(mouseReleased(QMouseEvent*)));

    if(tool == UITool::MeshManipulatorType::POSITION) {
        QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needUpdateSceneCenter()), this, SLOT(updateSceneCenter()));
    }

    if(tool == UITool::MeshManipulatorType::SLICE) {
        QObject::connect(this, SIGNAL(planesMoved(const glm::vec3&)), dynamic_cast<UITool::SliceManipulator*>(this->meshManipulator), SLOT(movePlanes(const glm::vec3&)));
    }

    // Scene->MeshManipulator->Selection
    QObject::connect(this, SIGNAL(keyPressed(QKeyEvent*)), dynamic_cast<QObject*>(&this->meshManipulator->selection), SLOT(keyPressed(QKeyEvent*)));
    QObject::connect(this, SIGNAL(keyReleased(QKeyEvent*)), dynamic_cast<QObject*>(&this->meshManipulator->selection), SLOT(keyReleased(QKeyEvent*)));
    QObject::connect(&this->meshManipulator->selection, &UITool::Selection::needToRedrawSelection, this, &Scene::redrawSelection);
    // MeshManipulator->Scene

    this->updateManipulatorRadius();

    // MeshManipulator->Scene

    if(tool == UITool::MeshManipulatorType::ARAP) {
        QObject::connect(dynamic_cast<UITool::ARAPManipulator*>(this->meshManipulator), SIGNAL(needPushHandleButton()), this, SIGNAL(needPushHandleButton()));
    }

    if(tool == UITool::MeshManipulatorType::SLICE) {
        QObject::connect(dynamic_cast<UITool::SliceManipulator*>(this->meshManipulator), &UITool::SliceManipulator::needChangePointsToProject, [this](std::vector<int> selectedPoints){ this->computeProjection(selectedPoints); });
    }

    if(tool == UITool::MeshManipulatorType::MARKER) {
        QObject::connect(dynamic_cast<UITool::MarkerManipulator*>(this->meshManipulator), SIGNAL(needCastRay()), this, SIGNAL(needCastRay()));
        QObject::connect(this, &Scene::rayIsCasted, this, [this](const glm::vec3& origin, const glm::vec3& direction) {
            std::vector<bool> visuMap;
            this->grids[this->activeGrid]->getVisibilityMap(visuMap);
            dynamic_cast<UITool::MarkerManipulator*>(this->meshManipulator)->placeManipulator(origin, direction, visuMap, this->computePlanePositions());
        });
    }

    Q_EMIT this->needDisplayInfos(this->meshManipulator->instructions);
}

void Scene::selectSlice(UITool::SliceOrientation sliceOrientation) {
    UITool::SliceManipulator* manipulator = dynamic_cast<UITool::SliceManipulator*>(this->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->selectSlice(sliceOrientation);
}

void Scene::changeSliceToSelect(UITool::SliceOrientation sliceOrientation) {
    UITool::SliceManipulator* manipulator = dynamic_cast<UITool::SliceManipulator*>(this->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->updateSliceToSelect(sliceOrientation);
}

bool Scene::isCage(const std::string& name) {
    return this->getCage(name);
}

std::vector<std::string> Scene::getAllNonTetrahedralMeshesName() {
    std::vector<std::string> res;
    for(int i = 0; i < this->meshes.size(); ++i) {
        std::string name = this->meshes[i].second;
        res.push_back(name);
    }
    return res;
}

std::vector<std::string> Scene::getAllBaseMeshesName() {
    std::vector<std::string> res;
    for(int i = 0; i < this->meshes.size(); ++i) {
        std::string name = this->meshes[i].second;
        res.push_back(name);
    }
    for(int i = 0; i < this->grids_name.size(); ++i) {
        res.push_back(this->grids_name[i]);
    }
    return res;
}

std::vector<std::string> Scene::getAllCagesName() {
    std::vector<std::string> res;
    for(int i = 0; i < this->meshes.size(); ++i) {
        std::string name = this->meshes[i].second;
        if(this->isCage(name))
            res.push_back(name);
    }
    return res;
}

std::vector<std::string> Scene::getAllGridsName() {
    return grids_name;
}

bool Scene::openCage(const std::string& name, const std::string& filename, const std::string& surfaceMeshToDeformName, const bool MVC, const glm::vec4& color) {
    this->cageToGrid.push_back(std::make_pair(name, surfaceMeshToDeformName));
    return this->openCage(name, filename, this->getBaseMesh(surfaceMeshToDeformName), MVC, color);
}

bool Scene::saveMesh(const std::string& name, const std::string& filename) {
    if(this->isGrid(this->activeMesh))
        return false;
    GraphMesh* graph = dynamic_cast<GraphMesh*>(this->getBaseMesh(name));
    if(graph) {
        graph->saveOFF(filename.c_str());
    } else {
        this->getMesh(name)->saveOFF(filename.c_str());
    }
    return true;
}

bool Scene::saveActiveMesh(const std::string& filename) {
    return this->saveMesh(this->activeMesh, filename);
}

bool Scene::saveActiveCage(const std::string& filename) {
    if(this->isCage(this->activeMesh)) {
        this->getMesh(this->activeMesh)->saveOFF(filename.c_str());
        return true;
    } else {
        return false;
    }
}

void Scene::applyCage(const std::string& name, const std::string& filename) {
    SurfaceMesh cageToApply(filename);
    this->getCage(name)->applyCage(cageToApply.getVertices());
    this->updateTetmeshAllGrids();
}

void Scene::redrawSelection(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec4& color) {
    this->glSelection->setSelectionBB(p0, p1, p2, p3);
    this->glSelection->setColor(color);
    this->glSelection->prepare();
}

void Scene::setLightPosition(const glm::vec3& lighPosition) {
    for(int i = 0; i < this->meshes.size(); ++i) {
        this->meshes[i].first->lightPosition = lighPosition;
    }
}

void Scene::changeActiveMesh(const std::string& name) {
    this->activeMesh = name;
    this->updateSceneCenter();
    this->updateSceneRadius();
    if(this->isGrid(activeMesh)) {
        this->activeGrid = this->getGridIdx(activeMesh);;
    } else if (this->isCage(activeMesh)) {
        this->activeGrid = this->getGridIdxLinkToCage(activeMesh);
    }
    Q_EMIT activeMeshChanged();
    this->changeCurrentTool(this->currentTool);
    this->shouldUpdateUserColorScales = true;
}

void Scene::changeCurrentTool(UITool::MeshManipulatorType newTool) {
    this->currentTool = newTool;
    this->updateTools(newTool);
}

void Scene::changeSelectedPoint(std::pair<int, glm::vec3> selectedPoint) {
    Q_EMIT selectedPointChanged(selectedPoint);
}

bool Scene::isRightTool(const UITool::MeshManipulatorType& typeToCheck) {
    bool isRightType = (this->currentTool == typeToCheck);
    if(!isRightType)
        std::cout << "WARNING: not the right tool" << std::endl;
    return isRightType;
}

void Scene::moveTool_toggleEvenMode() { auto * toolPtr = this->getMeshTool<UITool::GlobalManipulator>(); if(toolPtr) { toolPtr->toggleEvenMode(); } };

void Scene::ARAPTool_toggleEvenMode(bool value) { auto * toolPtr = this->getMeshTool<UITool::ARAPManipulator>(); if(toolPtr) { toolPtr->toggleEvenMode(value); } };

void Scene::moveInHistory(bool backward, bool reset) {
    BaseMesh * mesh = this->getBaseMesh(this->activeMesh);
    if(mesh && mesh->history) {
        std::vector<glm::vec3> pointsBefore;
        std::array<glm::vec3, 3> coordinates;
        bool success = false;
        if(reset) {
            success = mesh->history->reset(pointsBefore, coordinates);
        } else {
            if(backward) {
                success = mesh->history->undo(pointsBefore, coordinates);
            } else {
                success = mesh->history->redo(pointsBefore, coordinates);
            }
        }
        if(success) {
            mesh->movePoints(pointsBefore);
            mesh->coordinate_system = coordinates;
            this->meshManipulator->updateWithMeshVertices();
            this->updateManipulatorRadius();
            this->updateTetmeshAllGrids();
            this->updateSceneCenter();
        }
    } else {
        std::cout << "WARNING: the active mesh do not contain any history" << std::endl;
        return;
    }
}

void Scene::undo() {
    this->moveInHistory(true);
}

void Scene::redo() {
    this->moveInHistory(false);
}

void Scene::reset() {
    this->moveInHistory(true, true);
}

void Scene::writeDeformedImage(const std::string& filename, const std::string& gridName, bool useColorMap) {
    Grid * grid = this->grids[this->getGridIdx(gridName)];
    this->writeDeformedImage(filename, gridName, grid->bbMin, grid->bbMax, useColorMap, grid->getVoxelSize());
}

void Scene::writeDeformedImage(const std::string& filename, const std::string& gridName, bool useColorMap, const glm::vec3& voxelSize) {
    Grid * grid = this->grids[this->getGridIdx(gridName)];
    this->writeDeformedImage(filename, gridName, grid->bbMin, grid->bbMax, useColorMap, voxelSize);
}

void Scene::writeDeformedImage(const std::string& filename, const std::string& gridName, const glm::vec3& bbMin, const glm::vec3& bbMax, bool useColorMap, const glm::vec3 &voxelSize) {
    Grid * fromGrid = this->grids[this->getGridIdx(gridName)];
    if(useColorMap)
        this->writeDeformedImageGeneric(filename, gridName, bbMin, bbMax, (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16), useColorMap, voxelSize);
    else
        this->writeDeformedImageGeneric(filename, gridName, bbMin, bbMax, fromGrid->sampler.getInternalDataType(), useColorMap, voxelSize);
}

void Scene::writeDeformedImageGeneric(const std::string& filename, const std::string& gridName, const glm::vec3& bbMin, const glm::vec3& bbMax, Image::ImageDataType imgDataType, bool useColorMap, const glm::vec3& voxelSize) {
    if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8)) {
        this->writeDeformedImageTemplated<uint8_t>(filename, gridName, bbMin, bbMax, 8, imgDataType, useColorMap, voxelSize);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16)) {
        this->writeDeformedImageTemplated<uint16_t>(filename, gridName, bbMin, bbMax, 16, imgDataType, useColorMap, voxelSize);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32)) {
        this->writeDeformedImageTemplated<uint32_t>(filename, gridName, bbMin, bbMax, 32, imgDataType, useColorMap, voxelSize);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64)) {
        this->writeDeformedImageTemplated<uint64_t>(filename, gridName, bbMin, bbMax, 64, imgDataType, useColorMap, voxelSize);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_8)) {
        this->writeDeformedImageTemplated<int8_t>(filename, gridName, bbMin, bbMax, 8, imgDataType, useColorMap, voxelSize);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_16)) {
        this->writeDeformedImageTemplated<int16_t>(filename, gridName, bbMin, bbMax, 16, imgDataType, useColorMap, voxelSize);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_32)) {
        this->writeDeformedImageTemplated<int32_t>(filename, gridName, bbMin, bbMax, 32, imgDataType, useColorMap, voxelSize);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_64)) {
        this->writeDeformedImageTemplated<int64_t>(filename, gridName, bbMin, bbMax, 64, imgDataType, useColorMap, voxelSize);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_32)) {
        this->writeDeformedImageTemplated<float>(filename, gridName, bbMin, bbMax, 32, imgDataType, useColorMap, voxelSize);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_64)) {
        this->writeDeformedImageTemplated<double>(filename, gridName, bbMin, bbMax, 64, imgDataType, useColorMap, voxelSize);
    } 
}

template<typename DataType>
void Scene::writeDeformedImageTemplated(const std::string& filename, const std::string& gridName, const glm::vec3& bbMin, const glm::vec3& bbMax, int bit, Image::ImageDataType dataType, bool useColorMap, const glm::vec3& imageVoxelSize) {
    // To expose as parameters
    bool smallFile = true;
    int cacheSize = 2;
    bool useCustomColor = useColorMap;
    glm::ivec3 sceneImageSize = glm::vec3(0, 0, 0);
    //ResolutionMode resolution = ResolutionMode::FULL_RESOLUTION;

    Grid * fromGrid = this->grids[this->getGridIdx(gridName)];
    glm::vec3 worldSize = fromGrid->getDimensions();
    //glm::vec3 voxelSize = fromGrid->getVoxelSize(resolution);

    auto start = std::chrono::steady_clock::now();

    glm::vec3 fromImageToCustomImage = glm::vec3(0., 0., 0.);

    auto fromWorldToImage = [&](glm::vec3& p, bool ceil) {
        p -= fromGrid->bbMin;
        for(int i = 0; i < 3; ++i) {
            if(ceil)
                p[i] = std::ceil((p[i]/imageVoxelSize[i]));
            else
                p[i] = std::floor((p[i]/imageVoxelSize[i]));
        }
        p /= fromImageToCustomImage;
    };

    auto getWorldCoordinates = [&](glm::vec3& p) {
        for(int i = 0; i < 3; ++i) {
            p[i] = (float(std::ceil((p[i] * fromImageToCustomImage[i])) + 0.5) * imageVoxelSize[i]);
        }
        p += fromGrid->bbMin;
    };

    glm::ivec3 n(0, 0, 0);
    for(int i = 0 ; i < 3 ; i++) {
        n[i] = std::ceil(fabs((worldSize[i])/imageVoxelSize[i]));
    }

    if(sceneImageSize == glm::ivec3(0., 0., 0.))
        sceneImageSize = n;

    fromImageToCustomImage = glm::vec3(n) / glm::vec3(sceneImageSize);

    glm::ivec3 bbMinWrite = ((bbMin - fromGrid->bbMin)/imageVoxelSize);
    glm::ivec3 bbMaxWrite = ((bbMax - fromGrid->bbMin)/imageVoxelSize);
    glm::ivec3 imageSize = bbMaxWrite - bbMinWrite;

    std::cout << "BBmin: " << bbMinWrite << std::endl;
    std::cout << "BBmax: " << bbMaxWrite << std::endl;
    std::cout << "ImageSize: " << imageSize << std::endl;
    std::cout << "Original size: " << sceneImageSize << std::endl;

    TinyTIFFWriterFile * tif = nullptr;
    if(useCustomColor) {
        tif = TinyTIFFWriter_open(filename.c_str(), 8, TinyTIFFWriter_UInt, 3, imageSize[0], imageSize[1], TinyTIFFWriter_RGB);
    } else {
        if(dataType & Image::ImageDataType::Unsigned)
            tif = TinyTIFFWriter_open(filename.c_str(), bit, TinyTIFFWriter_UInt, 1, imageSize[0], imageSize[1], TinyTIFFWriter_Greyscale);
        else if(dataType & Image::ImageDataType::Signed)
            tif = TinyTIFFWriter_open(filename.c_str(), bit, TinyTIFFWriter_Int, 1, imageSize[0], imageSize[1], TinyTIFFWriter_Greyscale);
        else if(dataType & Image::ImageDataType::Floating)
            tif = TinyTIFFWriter_open(filename.c_str(), bit, TinyTIFFWriter_Float, 1, imageSize[0], imageSize[1], TinyTIFFWriter_Greyscale);
        else
            std::cout << "WARNING: image data type no take in charge to export" << std::endl;
    }

    if(tif == nullptr)
        return;

    std::vector<std::vector<DataType>> img = std::vector<std::vector<DataType>>(sceneImageSize.z, std::vector<DataType>(sceneImageSize.x * sceneImageSize.y, 0.));

    std::vector<std::vector<uint8_t>> img_color;

    std::vector<bool> data;
    std::vector<glm::vec3> data_color;

    if(useCustomColor) {
        img_color = std::vector<std::vector<uint8_t>>(sceneImageSize.z, std::vector<uint8_t>(sceneImageSize.x * sceneImageSize.y * 3, 0));

        auto upperGrid = this->grids[this->getGridIdx(gridName)];
        auto drawable_upperGrid = this->grids[this->getGridIdx(gridName)];
        float maxValue = upperGrid->getMaxValue();

        for(int i = 0; i <= maxValue; ++i) {
            data.push_back(false);
            data_color.push_back(glm::vec3(0., 0., 0.));
        }
        for(int i = 0; i < upperGrid->displayRangeSegmentedData.size(); ++i) {
            for(int j = upperGrid->displayRangeSegmentedData[i].first; j <= upperGrid->displayRangeSegmentedData[i].second; ++j) {
                if(j < data.size()) {
                    data[j] = true;
                    data_color[j] = upperGrid->displayColorSegmentedData[i];
                }
            }
        }
    }

    int cacheMaxNb = std::floor(fromGrid->sampler.image->tiffImageReader->imgResolution.z / float(cacheSize));
    std::map<int, std::vector<DataType>> cache;

    if(smallFile)
        for(int i = 0; i < fromGrid->sampler.image->tiffImageReader->imgResolution.z; ++i)
            fromGrid->sampler.image->tiffImageReader->getImage<DataType>(i, cache[i], {glm::vec3(0., 0., 0.), fromGrid->sampler.image->tiffImageReader->imgResolution});

    #pragma omp parallel for schedule(static) if(smallFile)
    for(int tetIdx = 0; tetIdx < fromGrid->mesh.size(); ++tetIdx) {
        //std::cout << "Tet: " << tetIdx << "/" << fromGrid->mesh.size() << std::endl;
        const Tetrahedron& tet = fromGrid->mesh[tetIdx];
        glm::vec3 bbMinTet = tet.getBBMin();
        glm::vec3 bbMaxTet = tet.getBBMax();
        fromWorldToImage(bbMinTet, false);
        fromWorldToImage(bbMaxTet, true);
        int X = bbMaxTet.x;
        int Y = bbMaxTet.y;
        int Z = bbMaxTet.z;
        for(int k = bbMinTet.z; k < Z; ++k) {
            for(int j = bbMinTet.y; j < Y; ++j) {
                for(int i = bbMinTet.x; i < X; ++i) {
                    glm::vec3 p(i, j, k);
                    if(p.x < bbMinWrite.x ||
                       p.y < bbMinWrite.y ||
                       p.z < bbMinWrite.z ||
                       p.x > bbMaxWrite.x ||
                       p.y > bbMaxWrite.y ||
                       p.z > bbMaxWrite.z)
                        continue;
                    getWorldCoordinates(p);
                    if(tet.isInTetrahedron(p)) {
                        if(fromGrid->getCoordInInitial(fromGrid->initialMesh, p, p, tetIdx)) {
                            int insertIdx = i + j*sceneImageSize[0];

                            //p *= fromGrid->sampler.resolutionRatio;
                            //p += glm::vec3(.5, .5, .5);
                            int imgIdxLoad = std::floor(p.z);
                            int idxLoad = std::floor(p.x) + std::floor(p.y) * fromGrid->sampler.image->tiffImageReader->imgResolution.x;

                            if(img[k][insertIdx] == 0) {

                                bool isInBBox = true;
                                for(int l = 0; l < 3; ++l) {
                                    if(p[l] < 0. || p[l] >= fromGrid->sampler.image->tiffImageReader->imgResolution[l])
                                        isInBBox = false;
                                }

                                if(isInBBox) {
                                    bool imgAlreadyLoaded = cache.find(imgIdxLoad) != cache.end();
                                    if(!imgAlreadyLoaded) {
                                        if(cache.size() > cacheMaxNb) {
                                            cache.erase(cache.begin());
                                        }
                                        fromGrid->sampler.image->tiffImageReader->getImage<DataType>(imgIdxLoad, cache[imgIdxLoad], {glm::vec3(0., 0., 0.), fromGrid->sampler.image->tiffImageReader->imgResolution});
                                    }
                                    if(useCustomColor) {
                                        if(k >= 0 && k < img_color.size() && insertIdx*3 < img_color[0].size() && insertIdx >= 0) {
                                            if(imgIdxLoad >= 0 && imgIdxLoad < cache.size() && idxLoad < cache[0].size() && idxLoad >= 0) {
                                                uint16_t value = 0;
                                                value = cache[imgIdxLoad][idxLoad];
                                                if(data[value]) {
                                                    glm::vec3 color = data_color[value];
                                                    insertIdx *= 3;
                                                    img_color[k][insertIdx] = static_cast<uint8_t>(color.r * 255.);
                                                    img_color[k][insertIdx+1] = static_cast<uint8_t>(color.g * 255.);
                                                    img_color[k][insertIdx+2] = static_cast<uint8_t>(color.b * 255.);
                                                }
                                            }
                                        }
                                    } else {
                                        if(k >= 0 && k < img.size() && insertIdx < img[0].size() && insertIdx >= 0)
                                            if(imgIdxLoad >= 0 && imgIdxLoad < cache.size() && idxLoad < cache[0].size() && idxLoad >= 0)
                                                img[k][insertIdx] = cache[imgIdxLoad][idxLoad];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //this->writeGreyscaleTIFFImage(filename, n, img);

    std::cout << "saving" << std::endl;
    if(useCustomColor) {
        std::vector<std::vector<uint8_t>> finalImg = std::vector<std::vector<uint8_t>>(imageSize.z, std::vector<uint8_t>(imageSize.x * imageSize.y * 3, 0.));
        for(int k = 0; k < imageSize.z; ++k) {
            for(int j = 0; j < imageSize.y; ++j) {
                for(int i = 0; i < imageSize.x; ++i) {
                    int idx = i+j*imageSize.x;
                    int idx2 = (i+bbMinWrite.x)+(j+bbMinWrite.y)*sceneImageSize.x;
                    finalImg[k][idx*3] = img_color[k+bbMinWrite.z][idx2*3];
                    finalImg[k][idx*3+1] = img_color[k+bbMinWrite.z][idx2*3+1];
                    finalImg[k][idx*3+2] = img_color[k+bbMinWrite.z][idx2*3+2];
                }
            }
        }
        for(int i = 0; i < finalImg.size(); ++i) {
            TinyTIFFWriter_writeImage(tif, finalImg[i].data());
        }
    } else {
        std::vector<std::vector<DataType>> finalImg = std::vector<std::vector<DataType>>(imageSize.z, std::vector<DataType>(imageSize.x * imageSize.y, 0.));
        for(int k = 0; k < imageSize.z; ++k) {
            for(int j = 0; j < imageSize.y; ++j) {
                for(int i = 0; i < imageSize.x; ++i) {
                    finalImg[k][i+j*imageSize.x] = img[k+bbMinWrite.z][(i+bbMinWrite.x)+(j+bbMinWrite.y)*sceneImageSize.x];
                }
            }
        }
        for(int i = 0; i < finalImg.size(); ++i) {
            TinyTIFFWriter_writeImage(tif, finalImg[i].data());
        }
    }
    TinyTIFFWriter_close(tif);
    std::cout << "Destination: " << filename << std::endl;
    std::cout << "Save sucessfull" << std::endl;

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Duration time: " << elapsed_seconds.count() << "s / " << elapsed_seconds.count()/60. << "m" << std::endl;
}

glm::vec3 Scene::getTransformedPoint(const glm::vec3& inputPoint, const std::string& from, const std::string& to) {
    glm::vec3 result = glm::vec3(0., 0., 0.);

    Grid * fromGrid = this->grids[this->getGridIdx(from)];
    Grid * toGrid = this->grids[this->getGridIdx(to)];

    // From initial to deformed
    glm::vec3 inputPointInFromGrid = inputPoint;
    std::cout << "-------------" << std::endl;
    std::cout << "Input point: " << inputPoint << std::endl;
    std::cout << "From [" << from << "].image to [" << from << "].sampler: " << inputPointInFromGrid;
    fromGrid->sampler.fromImageToSampler(inputPointInFromGrid);
    std::cout << " -> " << inputPointInFromGrid << std::endl;

    bool ptIsInInitial = fromGrid->initialMesh.getCoordInInitial(*fromGrid, inputPointInFromGrid, result);
    std::cout << "From [" << from << "].initial to [" << from << "].deformed: " << inputPointInFromGrid << " -> " << result << std::endl;
    if(!ptIsInInitial) {
        std::cout << "Input point isn't in the original image" << std::endl;
    }

    std::cout << "From [" << from  << "].deformed to [" << to << "].deformed: manually done" << std::endl;

    std::cout << "From [" << to  << "].deformed to [" << to << "].initial: " << result;
    ptIsInInitial = toGrid->getCoordInInitial(toGrid->initialMesh, result, result);
    std::cout << " -> " << result << std::endl;
    if(!ptIsInInitial) {
        std::cout << "Input point isn't in the original image" << std::endl;
    }

    std::cout << "From [" << to << "].sampler to [" << to << "].image: " << result;
    toGrid->sampler.fromSamplerToImage(result);
    std::cout << " -> " << result << std::endl;

    std::cout << "-------------" << std::endl;
    std::cout << "From [" << from  << "] to [" << to << "]grid: " << inputPoint << " -> " << result << std::endl;
    std::cout << "-------------" << std::endl;

    return result;
}

void Scene::getValues(const std::string& gridName, const glm::vec3& slice, const std::pair<glm::vec3, glm::vec3>& area, const glm::vec3& resolution, std::vector<uint16_t>& data, Interpolation::Method interpolationMethod) {
    this->grids[this->getGridIdx(gridName)]->sampleSliceGridValues(slice, area, resolution, data, interpolationMethod);
}

void Scene::writeGreyscaleTIFFImage(const std::string& filename, const glm::vec3& imgDimensions, const std::vector<std::vector<uint16_t>>& data) {
    TinyTIFFWriterFile * tif = TinyTIFFWriter_open(filename.c_str(), 16, TinyTIFFWriter_UInt, 1, imgDimensions[0], imgDimensions[1], TinyTIFFWriter_Greyscale);
    for(int img = 0; img < data.size(); ++img) {
        TinyTIFFWriter_writeImage(tif, data[img].data());
    }
    TinyTIFFWriter_close(tif);
    std::cout << "Destination: " << filename << std::endl;
    std::cout << "Save sucessfull" << std::endl;
}

void Scene::clear() {
    this->updateTools(UITool::MeshManipulatorType::NONE);
    this->grids_name.clear();
    this->grids.clear();
    this->meshes.clear();
    this->graph_meshes.clear();
    this->gridsToDraw.clear();
    activeGrid = -1;
}

glm::vec3 Scene::getGridImgSize(const std::string& name) {
    return this->grids[this->getGridIdx(name)]->getResolution();
}

glm::vec3 Scene::getGridVoxelSize(const std::string &name) {
    return this->grids[this->getGridIdx(name)]->getVoxelSize();
}

std::pair<uint16_t, uint16_t> Scene::getGridMinMaxValues(const std::string& name) {
    return std::make_pair(this->grids[this->getGridIdx(name)]->getMinValue(), this->grids[this->getGridIdx(name)]->getMaxValue());
}

std::pair<uint16_t, uint16_t> Scene::getGridMinMaxValues() {
    if(activeGrid != -1)
        return std::make_pair(this->grids[this->activeGrid]->getMinValue(), this->grids[this->activeGrid]->getMaxValue());
    return std::make_pair(0, 0);
}

std::vector<bool> Scene::getGridUsageValues(int minValue) {
    std::vector<int> histogram = this->grids[this->activeGrid]->sampler.getHistogram();
    if(activeGrid != -1) {
        std::vector<bool> usage;
        usage.reserve(histogram.size());
        for(auto& value : histogram) {
            if(value >= minValue) {
                usage.push_back(true);
            } else {
                usage.push_back(false);
            }
        }
        return usage;
    }
    return std::vector<bool>{false};
}

template<typename MeshToolType>
MeshToolType* Scene::getMeshTool() { return dynamic_cast<MeshToolType*>(this->meshManipulator); };

void Scene::setGridsToDraw(std::vector<int> indices) {
    this->gridsToDraw = indices;
}

void Scene::setMultiGridRendering(bool value) {
    this->registrationRendering = value;
};

void Scene::setDrawSliceOnly(bool value) {
    if(this->activeGrid >= 0)
        this->grids[this->activeGrid]->drawSliceOnly = value;
}

void Scene::setBlend(float value) {
    std::cout << this->alphaBlend << std::endl;
    this->alphaBlend = value;
}

void Scene::setRenderSize(int h, int w) {
    this->h = h;
    this->w = w;
}

void Scene::resetRanges() {
   if(this->activeGrid == -1)
       return;
   auto& grid = this->grids[this->activeGrid];
   grid->displayRangeSegmentedData.clear();
   grid->displayColorSegmentedData.clear();
   grid->displaySegmentedData.clear();
   this->updateMinMaxDisplayValues();
}

void Scene::addRange(uint16_t min, uint16_t max, glm::vec3 color, bool visible, bool updateUBO) {
   if(this->activeGrid == -1)
       return;
   auto& grid = this->grids[this->activeGrid];
   grid->displayRangeSegmentedData.push_back(std::make_pair(min, max));
   grid->displayColorSegmentedData.push_back(color);
   grid->displaySegmentedData.push_back(visible);
   if(updateUBO)
       this->updateMinMaxDisplayValues();
}

void Scene::getRanges(std::vector<std::pair<uint16_t, uint16_t>>& ranges) {
   if(this->activeGrid == -1)
       return;
   auto& grid = this->grids[this->activeGrid];
   ranges = grid->displayRangeSegmentedData;
}

void Scene::getRangesColor(std::vector<glm::vec3>& colors) {
   if(this->activeGrid == -1)
       return;
   auto& grid = this->grids[this->activeGrid];
   colors = grid->displayColorSegmentedData;
}

void Scene::getRangesVisu(std::vector<bool>& visu) {
   if(this->activeGrid == -1)
       return;
   auto& grid = this->grids[this->activeGrid];
   visu = grid->displaySegmentedData;
}

bool Scene::hasTwoOrMoreGrids() {
    return grids.size() >= 2;
}

void Scene::updateManipulatorRadius() {
    Q_EMIT sceneRadiusOutOfDate();
}

void Scene::computeProjection(const std::vector<int>& vertexIndices) {
    int knnMaxNode = 15;

    std::vector<glm::vec3> newPositions;

    struct KnnNode {
        int indice;
        float distance;

        KnnNode(): indice(-1), distance(std::numeric_limits<float>::max()) {}

        bool operator<(const KnnNode& other) const {
            return (distance < other.distance);
        }

        bool operator>(const KnnNode& other) const {
            return (distance > other.distance);
        }
    };

    BaseMesh * meshToProject = this->meshes[0].first;
    BaseMesh * mesh = this->meshes[1].first;
    std::vector<glm::vec3> positions = mesh->getVertices();
    std::vector<glm::vec3> normals = mesh->verticesNormals;
    for(int k = 0; k < vertexIndices.size(); ++k) {
        int vertexIdx = vertexIndices[k];

        glm::vec3 inputPoint = meshToProject->getVertice(vertexIdx);
        std::vector<KnnNode> knn(knnMaxNode);

        for(int i = 0; i < positions.size(); ++i) {
            if(i != vertexIdx) {
                float distance = glm::distance(inputPoint, positions[i]);
                if(distance < knn.back().distance) {
                    knn.back().indice = i;
                    knn.back().distance = distance;
                    std::sort(knn.begin(), knn.end());
                }
            }
        }

        std::vector<int> knn_indices(knnMaxNode);
        std::vector<float> knn_distances(knnMaxNode);
        for(int i = 0; i < knn.size(); ++i) {
            knn_indices[i] = knn[i].indice;
            knn_distances[i] = knn[i].distance * knn[i].distance;
        }

        ProjectedPoint res;
        res = apss(inputPoint,
                   positions,
                   normals,
                   knn_indices,// nth closest points from inputPoint
                   knn_distances);// same but distances

        newPositions.push_back(res.position);
    }
    meshToProject->movePoints(vertexIndices, newPositions);

    dynamic_cast<UITool::SliceManipulator*>(this->meshManipulator)->updateWithMeshVertices();
    dynamic_cast<UITool::SliceManipulator*>(this->meshManipulator)->moveGuizmo();
}

void Scene::drawBox(const Box& box) {
   glDisable(GL_LIGHTING);
   glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
   glColor4f(0.2,0.2,0.9, 1);
   glLineWidth(2.f);

   BasicGL::drawBBox<glm::vec3>(box.first, box.second);

   glEnable(GL_LIGHTING);
   glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
}

void Scene::addBox(const Box& box) {
    Box boxWithNoise = box;
    boxWithNoise.first += glm::vec3(0.001, 0.001, 0.001);
    this->boxes.push_back(boxWithNoise);
}

void Scene::clearBoxes() {
    this->boxes.clear();
}
