#include "../include/scene.hpp"
#include "../include/planar_viewer.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>

#include <QFileDialog>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QSurface>

#include <fstream>
#include <type_traits>

#include <chrono>
#include <utility>

#include "../../grid/geometry/grid.hpp"
#include "../../grid/drawable/drawable_manipulator.hpp"
#include "../../grid/deformation/mesh_deformer.hpp"

inline unsigned int planeHeadingToIndex(planeHeading _heading) {
	switch (_heading) {
		case planeHeading::North:
			return 1;
		case planeHeading::East:
			return 2;
		case planeHeading::South:
			return 3;
		case planeHeading::West:
			return 4;
		default:
			return 1;	 // If no value is recognized, return a north heading (default value)
	}
}

inline void __GetTexSize(std::size_t numTexNeeded, std::size_t* opt_width, std::size_t* opt_height) {
	std::size_t iRoot = (std::size_t) sqrtf(static_cast<float>(numTexNeeded));

	*opt_width	= iRoot + 1;
	*opt_height = iRoot;
	if (((*opt_width) * (*opt_height)) < numTexNeeded) {
		(*opt_height)++;
	}
	if (((*opt_width) * (*opt_height)) < numTexNeeded) {
		(*opt_width)++;
	}
}

/** This constructor not only creates the object, but also sets the default values for the Scene in order
 *  to be drawn, even if it is empty at the time of the first call to a draw function.
 */
Scene::Scene() :
	glMeshManipulator(new UITool::GL::MeshManipulator(&this->sceneGL, nullptr, std::vector<glm::vec3>())) {
	this->isInitialized	   = false;
	this->showVAOstate	   = false;
	this->shouldDeleteGrid = false;
    this->distanceFromCamera = 0.;
    this->cameraPosition = glm::vec3(0., 0., 0.);

	//this->grids.clear();

	this->context			= nullptr;
	this->controlPanel		= nullptr;
	//this->gridControl		= nullptr;
	this->programStatusBar	= nullptr;

	double minTexVal		= 1;
	double maxTexVal		= std::numeric_limits<int>::max();
	this->textureBounds0	= GridGLView::data_2(minTexVal, maxTexVal);
	this->textureBounds1	= GridGLView::data_2(minTexVal, maxTexVal);
	this->colorBounds0		= GridGLView::data_2(0, maxTexVal);
	this->colorBounds1		= GridGLView::data_2(0, maxTexVal);
	this->selectedChannel_r = 0;	// by default, the R channel
	this->selectedChannel_g = 0;	// by default, the R channel

	this->renderSize = 0;

	// Default light positions : at the vertices of a unit cube.
	this->lightPositions = {
	  glm::vec3(.0, .0, .0), glm::vec3(1., .0, .0),
	  glm::vec3(.0, 1., .0), glm::vec3(1., 1., .0),
	  glm::vec3(.0, .0, 1.), glm::vec3(1., .0, 1.),
	  glm::vec3(.0, 1., 1.), glm::vec3(1., 1., 1.)};

	this->planeDirection	= glm::vec3(1., 1., 1.);
	this->planeDisplacement = glm::vec3(.0, .0, .0);
	Image::bbox_t::vec min(.0, .0, .0);
	Image::bbox_t::vec max(1., 1., 1.);
	this->sceneBB				 = Image::bbox_t(min, max);
	this->clipDistanceFromCamera = 5.f;
	this->drawMode				 = DrawMode::Solid;
	this->rgbMode				 = ColorChannel::HandEColouring;
	this->channels_r			 = ColorFunction::SingleChannel;
	this->channels_g			 = ColorFunction::SingleChannel;

	// Show all planes as default :
	this->planeVisibility = glm::vec<3, bool, glm::defaultp>(true, true, true);

	this->vao					  = 0;
	this->vao_VolumetricBuffers = 0;
	this->vao_boundingBox		  = 0;

	this->vbo_VertPos				= 0;
	this->vbo_VertNorm			= 0;
	this->vbo_VertTex				= 0;
	this->vbo_Element				= 0;
	this->vbo_PlaneElement		= 0;
	this->vbo_SinglePlaneElement	= 0;
	this->vbo_boundingBoxVertices = 0;
	this->vbo_boundingBoxIndices	= 0;

	this->program_projectedTex	 = 0;
	this->program_Plane3D			 = 0;
	this->program_PlaneViewer		 = 0;
	this->program_VolumetricViewer = 0;
	this->program_BoundingBox		 = 0;

	this->tex_ColorScaleGrid			= 0;
	this->tex_ColorScaleGridAlternate = 0;
	this->volumetricMesh					= {};
	this->vbo_Texture3D_VertPos		= 0;
	this->vbo_Texture3D_VertNorm		= 0;
	this->vbo_Texture3D_VertTex		= 0;
	this->vbo_Texture3D_VertIdx		= 0;

    this->state_idx = 0;
    this->pos_idx = 0;

	this->color0		= glm::vec3(1., .0, .0);
	this->color1		= glm::vec3(.0, .0, 1.);
	this->color0_second = glm::vec3(1., .0, .0);
	this->color1_second = glm::vec3(.0, .0, 1.);

	std::cerr << "Allocating " << +std::numeric_limits<GridGLView::data_t>::max() << " elements for vis ...\n";

	this->pb_loadProgress			  = nullptr;
	this->timer_refreshProgress		  = nullptr;
	this->isFinishedLoading			  = false;
	this->shouldUpdateUserColorScales = false;
	this->shouldUpdateUBOData		  = false;

	this->posFrame = nullptr;

    this->glSelection = new UITool::GL::Selection(&this->sceneGL, glm::vec3(0., 0., 0.), glm::vec3(10., 10., 10.));

    this->currentTool = UITool::MeshManipulatorType::POSITION;
    this->currentDeformMethod = DeformMethod::NORMAL;
    this->planeActivation = glm::vec3(1., 1., 1.);
    this->displayGrid = true;
    this->displayMesh = true;
    this->previewCursorInPlanarView = false;
    this->multiGridRendering = false;
    this->sortingRendering = false;
}

Scene::~Scene(void) {
}

void Scene::initGl(QOpenGLContext* _context) {
	// Check if the scene has been initialized, share contexts if it has been :
	if (this->isInitialized == true) {
		if (this->context != nullptr && (_context != 0 && _context != nullptr)) {
			_context->setShareContext(this->context);
			if (_context->create() == false) {
				// throw std::runtime_error("Couldn't re-create context with shared context added\n");
				std::cerr << "Couldn't re-create context with shared context added\n";
			} else {
				std::cerr << "init() : Switching to context " << _context << '\n';
			}
		}
		return;
	} else {
		// If the scene had not yet been initialized, it is now :
		this->isInitialized = true;

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
	}
	this->sceneGL.initGl(_context);

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

	this->newSHADERS_generateColorScales();

	this->shaderCompiler = std::make_unique<ShaderCompiler>(this);

	// Compile the shaders :
	this->recompileShaders(false);

	// Create VAO, VBO elements and populate them in generateSceneData() :
	this->createBuffers();
	this->generateSceneData();

	// Generate visibility array :
	this->tex_ColorScaleGrid = 0;

	// Generate controller positions
	//this->glMeshManipulator->initGL(this->get_context());
	this->sceneGL.initGl(this->get_context());
    this->gridToDraw = -1;
	//this->glMeshManipulator->prepareSphere();

    //this->surfaceMesh = nullptr;
    //this->drawableMesh = nullptr; 
    this->glSelection->prepare();
}

void Scene::generateColorScales() {
	TextureUpload colorScaleUploadParameters;

	std::size_t textureSize = this->maximumTextureSize / 2u;
	float textureSize_f		= static_cast<float>(this->maximumTextureSize / 2u);

	std::vector<glm::vec3> colorScaleData_greyscale(textureSize);
	std::vector<glm::vec3> colorScaleData_hsv2rgb(textureSize);

	// Generate the greyscale :
	for (std::size_t i = 0; i < textureSize; ++i) {
		float intensity = static_cast<float>(i) / textureSize_f;
		glm::vec3 currentGreyscale(intensity, intensity, intensity);
		colorScaleData_greyscale[i] = currentGreyscale;
	}

	std::cerr << "Generated data for the greyscale color scale\n";

	// Generate the HSV2RGB :
	for (std::size_t i = 0; i < textureSize; ++i) {
		glm::vec3 hsv						= glm::vec3(float(i) / textureSize_f, 1., 1.);
		hsv.x								= glm::mod(100.0 + hsv.x, 1.0);	   // Ensure [0,1[
		float HueSlice						= 6.0 * hsv.x;	  // In [0,6[
		float HueSliceInteger				= floor(HueSlice);
		float HueSliceInterpolant			= HueSlice - HueSliceInteger;	 // In [0,1[ for each hue slice
		glm::vec3 TempRGB					= glm::vec3(hsv.z * (1.0 - hsv.y), hsv.z * (1.0 - hsv.y * HueSliceInterpolant), hsv.z * (1.0 - hsv.y * (1.0 - HueSliceInterpolant)));
		float IsOddSlice					= glm::mod(HueSliceInteger, 2.0f);	  // 0 if even (slices 0, 2, 4), 1 if odd (slices 1, 3, 5)
		float ThreeSliceSelector			= 0.5 * (HueSliceInteger - IsOddSlice);	   // (0, 1, 2) corresponding to slices (0, 2, 4) and (1, 3, 5)
		glm::vec3 ScrollingRGBForEvenSlices = glm::vec3(hsv.z, TempRGB.z, TempRGB.x);	 // (V, Temp Blue, Temp Red) for even slices (0, 2, 4)
		glm::vec3 ScrollingRGBForOddSlices	= glm::vec3(TempRGB.y, hsv.z, TempRGB.x);	 // (Temp Green, V, Temp Red) for odd slices (1, 3, 5)
		glm::vec3 ScrollingRGB				= mix(ScrollingRGBForEvenSlices, ScrollingRGBForOddSlices, IsOddSlice);
		float IsNotFirstSlice				= glm::clamp(ThreeSliceSelector, 0.0f, 1.0f);	 // 1 if NOT the first slice (true for slices 1 and 2)
		float IsNotSecondSlice				= glm::clamp(ThreeSliceSelector - 1.0f, 0.0f, 1.f);	   // 1 if NOT the first or second slice (true only for slice 2)
		colorScaleData_hsv2rgb[i]			= glm::vec4(glm::mix(glm::vec3(ScrollingRGB), glm::mix(glm::vec3(ScrollingRGB.z, ScrollingRGB.x, ScrollingRGB.y), glm::vec3(ScrollingRGB.y, ScrollingRGB.z, ScrollingRGB.x), IsNotSecondSlice), IsNotFirstSlice), 1.f);	   // Make the RGB rotate right depending on final slice index
	}

	std::cerr << "Generated data for the HSV2RGB color scale" << '\n';

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
	colorScaleUploadParameters.data			  = colorScaleData_greyscale.data();

    glDeleteTextures(1, &this->tex_colorScale_greyscale);
	this->tex_colorScale_greyscale = this->uploadTexture1D(colorScaleUploadParameters);

	colorScaleUploadParameters.data	   = colorScaleData_hsv2rgb.data();
    glDeleteTextures(1, &this->tex_colorScale_hsv2rgb);
	this->tex_colorScale_hsv2rgb = this->uploadTexture1D(colorScaleUploadParameters);
}

void Scene::addStatusBar(QStatusBar* _s) {
	if (this->programStatusBar != nullptr) {
		return;
	}

	this->programStatusBar = _s;
}

void Scene::printOpenGLMessage(const QOpenGLDebugMessage& message) {
	std::string src = "";
	std::string sev = "";
	std::string typ = "";
	QFlags severity = message.severity();
	QFlags type		= message.type();
	QFlags source	= message.source();

	// clang-format off

	//if (severity & QOpenGLDebugMessage::Severity::NotificationSeverity)	{ sev += "{Notif}"; }
	if (severity & QOpenGLDebugMessage::Severity::LowSeverity)		{ sev += "{Low}"; }
	if (severity & QOpenGLDebugMessage::Severity::MediumSeverity)	{ sev += "{Med}"; }
	if (severity & QOpenGLDebugMessage::Severity::HighSeverity)		{ sev += "{High}"; }

	if (type & QOpenGLDebugMessage::Type::ErrorType)				{ typ += "[ERROR]"; }
	if (type & QOpenGLDebugMessage::Type::DeprecatedBehaviorType)	{ typ += "[DEPR. BEHAVIOUR]"; }
	if (type & QOpenGLDebugMessage::Type::UndefinedBehaviorType)	{ typ += "[UNDEF. BEHAVIOUR]"; }
	if (type & QOpenGLDebugMessage::Type::PortabilityType)			{ typ += "[PORTABILITY]"; }
	if (type & QOpenGLDebugMessage::Type::PerformanceType)			{ typ += "[PERF]"; }
	if (type & QOpenGLDebugMessage::Type::OtherType)				{ typ += "[OTHER]"; }
	if (type & QOpenGLDebugMessage::Type::MarkerType)				{ typ += "[MARKER]"; }
	if (type & QOpenGLDebugMessage::Type::GroupPushType)			{ typ += "[GROUP PUSH]"; }
	if (type & QOpenGLDebugMessage::Type::GroupPopType)				{ typ += "[GROUP POP]"; }

	if (source & QOpenGLDebugMessage::Source::APISource)			{ src += "[OpenGL]"; }
	if (source & QOpenGLDebugMessage::Source::WindowSystemSource)	{ src += "[WinSys]"; }
	if (source & QOpenGLDebugMessage::Source::ShaderCompilerSource)	{ src += "[ShaComp]"; }
	if (source & QOpenGLDebugMessage::Source::ThirdPartySource)		{ src += "[3rdParty]"; }
	if (source & QOpenGLDebugMessage::Source::OtherSource)			{ src += "[Other]"; }

	// Currently outputs any message on the GL stack, regardless of severity, type, or source :
	std::cerr << sev << ' ' << typ << ' ' << src << ' ' << +message.id() << " : " << message.message().toStdString() << '\n';

	// clang-format on
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
	this->vao					   = createVAO("vaoHandle");
	this->vbo_VertPos			   = createVBO(GL_ARRAY_BUFFER, "vboHandle_VertPos");
	this->vbo_VertNorm		   = createVBO(GL_ARRAY_BUFFER, "vboHandle_VertNorm");
	this->vbo_VertTex			   = createVBO(GL_ARRAY_BUFFER, "vboHandle_VertTex");
	this->vbo_Element			   = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_Element");
	this->vbo_PlaneElement	   = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_PlaneElement");
	this->vbo_SinglePlaneElement = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_SinglePlaneElement");

	// For the texture3D visualization method :
	this->vao_VolumetricBuffers  = createVAO("vaoHandle_VolumetricBuffers");
	this->vbo_Texture3D_VertPos  = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertPos");
	this->vbo_Texture3D_VertNorm = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertNorm");
	this->vbo_Texture3D_VertTex  = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertTex");
	this->vbo_Texture3D_VertIdx  = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_Texture3D_VertIdx");

	// For the bounding boxes we have to create/show :
	this->vao_boundingBox			= createVAO("vaoHandle_boundingBox");
	this->vbo_boundingBoxVertices = createVBO(GL_ARRAY_BUFFER, "vboHandle_boundingBoxVertices");
	this->vbo_boundingBoxIndices	= createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_boundingBoxIndices");

	this->glMeshManipulator->vao = createVAO("vaoHandle_Sphere");
	this->glMeshManipulator->vboVertices = createVBO(GL_ARRAY_BUFFER, "vboHandle_SphereVertices");
	this->glMeshManipulator->vboIndices = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_SphereIndices");

	this->glSelection->setVao(createVAO("vaoHandle_Selection"));
	this->glSelection->setVboVertices(createVBO(GL_ARRAY_BUFFER, "vboHandle_SelectionVertices"));
	this->glSelection->setVboIndices(createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_SelectionIndices"));

    //glGenFramebuffers(1, &this->frameBuffer);
    //glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);
    //glGenTextures(1, &this->dualRenderingTexture);

    //glBindTexture(GL_TEXTURE_2D, this->dualRenderingTexture);
    //glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 1024, 768, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //glGenRenderbuffers(1, &this->frameDepthBuffer);
    //glBindRenderbuffer(GL_RENDERBUFFER, this->frameDepthBuffer);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->frameDepthBuffer);

    //// Set "renderedTexture" as our colour attachement #0
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->dualRenderingTexture, 0);

    //// Set the list of draw buffers.
    //GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    //glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    //// Always check that our framebuffer is ok
    //if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    //    std::cout << "WARNING: framebuffer doesn't work !!" << std::endl;
    //else
    //    std::cout << "Framebuffer works perfectly :) !!" << std::endl;

    // The fullscreen quad's FBO
    /***/

    static const GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    //static const GLfloat g_quad_vertex_buffer_data[] = {
    //    -1.0f, -1.0f, 0.0f,
    //    1.0f, -1.0f, 0.0f,
    //    -1.0f,  1.0f, 0.0f,
    //    -1.0f,  1.0f, 0.0f,
    //    1.0f, -1.0f, 0.0f,
    //    1.0f,  1.0f, 0.0f,
    //};

    glGenVertexArrays(1, &quad_VertexArrayID);
    glGenBuffers(1, &quad_vertexbuffer);

    glBindVertexArray(quad_VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    /***/

    // Create and compile our GLSL program from the shaders
    //this->quad_programID = this->compileShaders("../shaders/plane.vert", "", "../shaders/plane.frag", verbose);
    //GLuint texID = glGetUniformLocation(quad_programID, "renderedTexture");
    //GLuint timeID = glGetUniformLocation(quad_programID, "time");

	return;
}

void Scene::loadGridROI() {
	LOG_ENTER(Scene::loadGridROI)
	// It will load the grid, based on the visu box coordinates !
	// The context will be made current at this stage, no need to worry :)

	std::cerr << "Error: visu box controller is broken for now due to new API update" << std::endl;
}

void Scene::updateProgressBar() {
	if (this->tasks.size() == 0) {
		return;
	}

	// Track progress of tasks :
	std::size_t current	 = 0;
	std::size_t maxSteps = 0;
	std::for_each(this->tasks.cbegin(), this->tasks.cend(), [&current, &maxSteps](const Image::ThreadedTask::Ptr& task) {
		maxSteps += task->getMaxSteps();
		current += task->getAdvancement();
	});

	// Update the progress :
	this->pb_loadProgress->setFormat("Loading high-res grid ... %v/%m (%p%)");
	this->pb_loadProgress->setValue(current);
	this->pb_loadProgress->setMaximum(maxSteps);

	if (current >= maxSteps) {
		// stop this function, and remove widget
		this->timer_refreshProgress->stop();
		this->pb_loadProgress->setVisible(false);
		this->programStatusBar->removeWidget(this->pb_loadProgress);
		// delete pointers :
		delete this->programStatusBar;
		delete this->timer_refreshProgress;
		// remove pointer values :
		this->programStatusBar		= nullptr;
		this->timer_refreshProgress = nullptr;

		// Join threads :
		std::for_each(this->runningThreads.begin(), this->runningThreads.end(), [this](std::shared_ptr<std::thread>& th) {
			if (th->joinable()) {
				std::lock_guard(this->mutexout);
				std::cerr << "Waiting for thread " << th->get_id() << " ...\n";
				th->join();
				th.reset();
			} else {
				std::lock_guard(this->mutexout);
				std::cerr << "Warning : cannot join thread " << th->get_id() << '\n';
			}
		});

		this->runningThreads.clear();
		this->isFinishedLoading = true;
	}

	return;
}

uint16_t Scene::sendGridValuesToGPU(int gridIdx) {

	glm::vec<4, std::size_t, glm::defaultp> dimensions{this->grids[gridIdx]->grid->getResolution(), 2};
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

    int nbSlice = this->grids[gridIdx]->grid->getResolution()[2];

    //TODO: this computation do not belong here
    uint16_t max = std::numeric_limits<uint16_t>::min();

    dimensions[0] = dimensions[0] * dimensions[3];// Because we have "a" value

    bool addArticialBoundaries = false;

    int sliceI = 0;
	for (std::size_t s = 0; s < nbSlice; ++s) {
        this->grids[gridIdx]->grid->getGridSlice(s, slices, dimensions.a);
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
        slices.clear();
        sliceI++;
	}
    this->shouldUpdateUBOData = true;

    return max;
}

void Scene::addGrid() {

    GridGLView::Ptr gridView = this->grids.back();

	glm::vec<4, std::size_t, glm::defaultp> dimensions{gridView->grid->getResolution(), 2};

    uint16_t max = sendGridValuesToGPU(this->grids.size() -1);

    std::cout << "Max value: " << max << std::endl;
    gridView->colorChannelAttributes[0].setMaxVisible(max);
    gridView->colorChannelAttributes[0].setMaxColorScale(max);
    gridView->colorChannelAttributes[1].setMaxVisible(max);
    gridView->colorChannelAttributes[1].setMaxColorScale(max);
    gridView->colorChannelAttributes[2].setMaxVisible(max);
    gridView->colorChannelAttributes[2].setMaxColorScale(max);

    if(this->gridToDraw == 1) {
        this->controlPanel->setMaxTexValAlternate(max);
        this->controlPanel->updateMaxValueAlternate(max);
        this->slotSetMaxColorValueAlternate(max);
    } else {
        this->controlPanel->setMaxTexVal(max);
        this->controlPanel->updateMaxValue(max);
        this->slotSetMaxColorValue(max);
    }
	this->setColorChannel(ColorChannel::RedOnly);
	this->setColorFunction_r(ColorFunction::ColorMagnitude);
	this->setColorFunction_g(ColorFunction::ColorMagnitude);

	gridView->boundingBoxColor = glm::vec3(.4, .6, .3);	   // olive-colored by default
	gridView->nbChannels	   = 2;	   // loaded 2 channels in the image

	// Create the uniform buffer :
	auto mainColorChannel				= gridView->mainColorChannelAttributes();
	gridView->uboHandle_colorAttributes = this->createUniformBuffer(4 * sizeof(colorChannelAttributes_GL), GL_STATIC_DRAW);
	this->setUniformBufferData(gridView->uboHandle_colorAttributes, 0, 32, &mainColorChannel);
	this->setUniformBufferData(gridView->uboHandle_colorAttributes, 32, 32, &gridView->colorChannelAttributes[0]);
	this->setUniformBufferData(gridView->uboHandle_colorAttributes, 64, 32, &gridView->colorChannelAttributes[1]);
	this->setUniformBufferData(gridView->uboHandle_colorAttributes, 96, 32, &gridView->colorChannelAttributes[2]);

    //Send grid texture
    this->sendTetmeshToGPU(this->gridToDraw, InfoToSend(InfoToSend::VERTICES | InfoToSend::NORMALS | InfoToSend::TEXCOORD | InfoToSend::NEIGHBORS));
	this->tex3D_buildBuffers(gridView->volumetricMesh);

    //Add manipulators
    //this->createNewMeshManipulator(std::string("bunny"), 0, false);
	//this->glMeshManipulator->toggleActivation();

    //this->updateManipulatorPositions();
	this->prepareManipulators();

	this->updateBoundingBox();
	//this->setVisuBoxMinCoord(glm::uvec3());
	//this->setVisuBoxMaxCoord(gridLoaded->getResolution());
	this->resetVisuBox();

    // Update mesh scale
    //glm::vec3 gridDim = this->grids[0]->grid->bbMin - this->grids[0]->grid->bbMax;
    //glm::vec3 meshDim = this->getMesh("bunny")->bbMin - this->getMesh("bunny")->bbMax;
    //glm::vec3 diff = gridDim - meshDim;
    //float scaleFactor = std::abs(std::max(diff[0], std::max(diff[1], diff[2])));
    //glm::vec3 scale = glm::vec3(scaleFactor, scaleFactor, scaleFactor);
    //for(int i = 0; i < this->getMesh("bunny")->getNbVertices(); ++i)
    //    this->getMesh("bunny")->vertices[i] *= scale;

    //for(int i = 0; i < this->getMesh("bunny_cage")->getNbVertices(); ++i)
    //    this->getMesh("bunny_cage")->vertices[i] *= scale;

    //this->getMesh("bunny")->updatebbox();
    //this->getMesh("bunny")->computeNormals();
    //this->getMesh("bunny_cage")->updatebbox();
    //this->getMesh("bunny_cage")->computeNormals();
    //this->getDrawableMesh("bunny")->makeVAO();
    //this->getDrawableMesh("bunny_cage")->makeVAO();

}

void Scene::updateBoundingBox(void) {
	std::cerr << "Error: updateBoundingBox brocken due to new API update" << std::endl;
	this->sceneBB	  = Image::bbox_t();
	this->sceneDataBB = Image::bbox_t();

	for (std::size_t i = 0; i < this->grids.size(); ++i) {
		const Grid * _g = this->grids[i]->grid;
		Image::bbox_t box		  = _g->getBoundingBox();
		Image::bbox_t dbox		  = _g->getBoundingBox();
		this->sceneBB.addPoints(box.getAllCorners());
		this->sceneDataBB.addPoints(dbox.getAllCorners());
	}

	// Update light positions :
	auto corners = this->sceneBB.getAllCorners();
	for (std::size_t i = 0; i < corners.size(); ++i) {
		this->lightPositions[i] = glm::convert_to<float>(corners[i]);
	}

	return;
}

void Scene::recompileShaders(bool verbose) {

	GLuint newProgram			 = this->compileShaders("../shaders/voxelgrid.vert", "../shaders/voxelgrid.geom", "../shaders/voxelgrid.frag", verbose);
	GLuint newPlaneProgram		 = this->compileShaders("../shaders/plane.vert", "", "../shaders/plane.frag", verbose);
	GLuint newPlaneViewerProgram = this->compileShaders("../shaders/texture_explorer.vert", "", "../shaders/texture_explorer.frag", verbose);
	GLuint newVolumetricProgram	 = this->compileShaders("../shaders/transfer_mesh.vert", "../shaders/transfer_mesh.geom", "../shaders/transfer_mesh.frag", verbose);
	GLuint newBoundingBoxProgram = this->compileShaders("../shaders/bounding_box.vert", "", "../shaders/bounding_box.frag", verbose);
	GLuint newSphereProgram		 = this->compileShaders("../shaders/sphere.vert", "", "../shaders/sphere.frag", true);
	GLuint newSelectionProgram	 = this->compileShaders("../shaders/selection.vert", "", "../shaders/selection.frag", true);
    GLuint newDoublePassProgram	 = this->compileShaders("../shaders/double_pass.vert", "", "../shaders/double_pass.frag", true);

	if (newProgram) {
		glDeleteProgram(this->program_projectedTex);
		this->program_projectedTex = newProgram;
	}
	if (newPlaneProgram) {
		glDeleteProgram(this->program_Plane3D);
		this->program_Plane3D = newPlaneProgram;
	}
	if (newPlaneViewerProgram) {
		glDeleteProgram(this->program_PlaneViewer);
		this->program_PlaneViewer = newPlaneViewerProgram;
	}
	if (newVolumetricProgram) {
		glDeleteProgram(this->program_VolumetricViewer);
		this->program_VolumetricViewer = newVolumetricProgram;
	}
	if (newBoundingBoxProgram) {
		glDeleteProgram(this->program_BoundingBox);
		this->program_BoundingBox = newBoundingBoxProgram;
	}
	if (newSphereProgram) {
		glDeleteProgram(this->glMeshManipulator->program);
		this->glMeshManipulator->program = newSphereProgram;
	}
	if (newSelectionProgram) {
		glDeleteProgram(this->glSelection->getProgram());
		this->glSelection->setProgram(newSelectionProgram);
	}
    if (newDoublePassProgram) {
        glDeleteProgram(this->program_doublePass);
        this->program_doublePass = newDoublePassProgram;
    }
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

GLuint Scene::uploadTexture2D(const TextureUpload& tex) {
	if (this->context != nullptr) {
		if (this->context->isValid() == false) {
			throw std::runtime_error("No associated valid context");
		}
	} else {
		throw std::runtime_error("nullptr as context");
	}

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

void Scene::removeController() {
	//this->gridControl = nullptr;
}

void Scene::deleteGridNow() {
	std::cerr << "WARNING: try to delete old API grid" << std::endl;
	// TODO: port this function to new API
	// this->shouldDeleteGrid = false;
	// for (std::size_t g : this->delGrid) {
	// 	// Delete 'raw' grid texture
	// 	glDeleteTextures(1, &this->grids[g]->gridTexture);
	// 	// Delete mesh textures :
	// 	glDeleteTextures(1, &this->grids[g]->volumetricMesh.visibilityMap);
	// 	glDeleteTextures(1, &this->grids[g]->volumetricMesh.vertexPositions);
	// 	glDeleteTextures(1, &this->grids[g]->volumetricMesh.textureCoordinates);
	// 	glDeleteTextures(1, &this->grids[g]->volumetricMesh.neighborhood);
	// 	glDeleteTextures(1, &this->grids[g]->volumetricMesh.faceNormals);
	// 	this->grids[g]->nbChannels = 0;
	// }

	// // Remove all items from grids which have no channels :
	// this->grids.erase(std::remove_if(this->grids.begin(), this->grids.end(), [](const GridGLView::Ptr& v) {
	// 	if (v->nbChannels == 0) {
	// 		return true;
	// 	}
	// 	return false;
	// }),
	//   this->grids.end());
	// this->delGrid.clear();

	// this->updateBoundingBox();
}

void Scene::drawGridMonoPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading, float zoomRatio, glm::vec2 offset) {
	if (this->grids.empty()) {
		return;
	}

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	glEnablei(GL_BLEND, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	uint min = 0;	 // min index for drawing commands
	if (_plane == planes::x) {
		min = 0;
	}
	if (_plane == planes::y) {
		min = 6;
	}
	if (_plane == planes::z) {
		min = 12;
	}

	glUseProgram(this->program_PlaneViewer);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_SinglePlaneElement);

	if (not this->grids.empty()) {
		this->prepareUniformsMonoPlaneView(_plane, _heading, fbDims, zoomRatio, offset, this->grids[this->gridToDraw]);

		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*) (min * sizeof(GLuint)));
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	this->showVAOstate = false;

	return;
}

void Scene::drawGridVolumetricView(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, const GridGLView::Ptr& grid) {
	if (grid->gridTexture > 0) {
		glUseProgram(this->program_VolumetricViewer);

        if(sortingRendering) {
            this->sendTetmeshToGPU(this->gridToDraw, InfoToSend(InfoToSend::VERTICES | InfoToSend::NORMALS), sortingRendering);
        }

        this->prepareUniformsGridVolumetricView(mvMat, pMat, camPos, grid);

		this->tex3D_bindVAO();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_Texture3D_VertIdx);

        // Version to draw in each framebuffer
        //
        //std::cout << this->gridToDraw << std::endl;
        //if(this->gridToDraw == 0) {
        //    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &this->defaultFBO);
        //    glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);
        //    glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, (void*) 0, grid->volumetricMesh.tetrahedraCount);
        //    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFBO);
        //    //glViewport(0,0,1024,768); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        //} else {
        //    glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, (void*) 0, grid->volumetricMesh.tetrahedraCount);
        //}

        // Original version
        //
        glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, (void*) 0, grid->volumetricMesh.tetrahedraCount);

        // Version to draw a simple texture
        //


		// Unbind program, buffers and VAO :
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}

	// draw grid BB :
    // Deactivate for now because useless and create a bug with the normal display
	//this->drawBoundingBox(grid->grid->getBoundingBox(), grid->boundingBoxColor, mvMat, pMat);
}

void Scene::drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	// Plane X :
	glUseProgram(this->program_Plane3D);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_PlaneElement);
	if (not this->grids.empty()) {
		this->prepareUniformsPlanes(mvMat, pMat, planes::x, this->grids[this->gridToDraw], showTexOnPlane);
		this->setupVAOPointers();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, static_cast<GLvoid*>(0));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	// Plane Y :
	glUseProgram(this->program_Plane3D);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_PlaneElement);
	if (not this->grids.empty()) {
		this->prepareUniformsPlanes(mvMat, pMat, planes::y, this->grids[this->gridToDraw], showTexOnPlane);
		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*) (6 * sizeof(GLuint)));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	// Plane Z :
	glUseProgram(this->program_Plane3D);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_PlaneElement);
	if (not this->grids.empty()) {
		this->prepareUniformsPlanes(mvMat, pMat, planes::z, this->grids[this->gridToDraw], showTexOnPlane);
		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*) (12 * sizeof(GLuint)));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
}

glm::vec3 Scene::computePlanePositions() {
	//Image::bbox_t::vec position = this->sceneBB.getMin();
    if(this->grids.size() > 0) {
	    Image::bbox_t::vec position = this->grids.back()->grid->bbMin;
	    Image::bbox_t::vec diagonal = this->grids.back()->grid->getDimensions();
	    //Image::bbox_t::vec position = this->sceneBB.getMin();
	    //Image::bbox_t::vec diagonal = this->sceneBB.getDiagonal();
	    glm::vec3 planePos			= (position + this->planeDisplacement * diagonal);
        for(int i = 0; i < 3; ++i) {
            if(this->planeActivation[i] == 0.) {
                planePos[i] = -1000000.;
            }
        }
	    return planePos;
    } else {
        return glm::vec3(0., 0., 0.);
    }
}

void Scene::prepareUniformsGridPlaneView(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, const GridGLView::Ptr& gridView) {
	// Get the world to grid transform :
	glm::mat4 transfoMat						= baseMatrix;
#warning Transform API is still in-progress.

	auto getUniform = [&](const char* name) -> GLint {
		GLint g = glGetUniformLocation(this->program_projectedTex, name);
		if (this->showVAOstate) {
			if (g >= 0) {
				std::cerr << "[LOG]\tLocation [" << +g << "] for uniform " << name << '\n';
			} else {
				std::cerr << "[LOG]\tCannot find uniform \"" << name << "\"\n";
			}
		}
		return g;
	};

	if (this->showVAOstate) {
		LOG_ENTER(Scene::prepareUniforms_3DSolid);
		std::cerr << "[LOG] Uniform locations for " << __FUNCTION__ << " : \n";
		this->newSHADERS_print_all_uniforms(this->program_projectedTex);
		glUseProgram(this->program_projectedTex);
	}

	// Get the uniform locations :
	GLint mMatrix_Loc			   = getUniform("mMatrix");
	GLint vMatrix_Loc			   = getUniform("vMatrix");
	GLint pMatrix_Loc			   = getUniform("pMatrix");
	GLint lightPos_Loc			   = getUniform("lightPos");
	GLint voxelGridOrigin_Loc	   = getUniform("voxelGridOrigin");
	GLint voxelGridSize_Loc		   = getUniform("voxelGridSize");
	GLint voxelSize_Loc			   = getUniform("voxelSize");
	GLint drawMode_Loc			   = getUniform("drawMode");
	GLint texDataLoc			   = getUniform("texData");
	GLint planePositionsLoc		   = getUniform("planePositions");
	GLint location_planeDirections = getUniform("planeDirections");
	GLint gridPositionLoc		   = getUniform("gridPosition");

	GLint location_colorScales0 = getUniform("colorScales[0]");
	GLint location_colorScales1 = getUniform("colorScales[1]");
	GLint location_colorScales2 = getUniform("colorScales[2]");
	GLint location_colorScales3 = getUniform("colorScales[3]");

	//Image::bbox_t::vec origin	= Image::bbox_t(gridView->grid->getBoundingBox()).getMin();
	//Image::bbox_t::vec originWS = Image::bbox_t(gridView->grid->getBoundingBox()).getMin();
	//glm::vec3 dims				= gridView->grid->getResolution();

    // Do not use the bounding anymore as it can change due to deformation
	Image::bbox_t::vec origin	= gridView->grid->bbMin;
	Image::bbox_t::vec originWS = gridView->grid->bbMin;
	glm::vec3 dims				= gridView->grid->getResolution();

	glUniform3fv(voxelGridOrigin_Loc, 1, glm::value_ptr(origin));
	glUniform3fv(voxelGridSize_Loc, 1, glm::value_ptr(dims));
	glUniform3fv(voxelSize_Loc, 1, glm::value_ptr(gridView->voxelDimensions));
	glUniform1ui(drawMode_Loc, this->drawMode);

	glm::vec3 planePos = this->computePlanePositions();

	glUniform3fv(planePositionsLoc, 1, glm::value_ptr(planePos));
	glUniform3fv(location_planeDirections, 1, glm::value_ptr(this->planeDirection));
	glUniform3fv(gridPositionLoc, 1, glm::value_ptr(originWS));

	// Apply the uniforms :
	glUniformMatrix4fv(mMatrix_Loc, 1, GL_FALSE, glm::value_ptr(transfoMat));
	glUniformMatrix4fv(vMatrix_Loc, 1, GL_FALSE, &mvMat[0]);
	glUniformMatrix4fv(pMatrix_Loc, 1, GL_FALSE, &pMat[0]);
	glUniform4fv(lightPos_Loc, 1, glm::value_ptr(lightPos));

	GLuint enabled_textures = 0;

	// Textures :
	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, gridView->gridTexture);
	glUniform1i(texDataLoc, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glEnable(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_greyscale);
	glUniform1i(location_colorScales0, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glEnable(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_hsv2rgb);
	glUniform1i(location_colorScales1, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_user0);
	glUniform1i(location_colorScales2, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_user1);
	glUniform1i(location_colorScales3, enabled_textures);
	enabled_textures++;

	/**
	 * NOTE : assumes the main color channel is always green, a.k.a. colorChannels[1]
	 * Load all the available color channels in the shader program, using the discrete identifiers :
	 */
	const GLchar uniform_block_name[] = "ColorBlock";
	GLuint colorBlock_index			  = glGetUniformBlockIndex(this->program_projectedTex, uniform_block_name);
	glUniformBlockBinding(this->program_projectedTex, colorBlock_index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, gridView->uboHandle_colorAttributes);

	if (this->showVAOstate) {
		this->printAllUniforms(this->program_projectedTex);
	}
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

void Scene::printAllUniforms(GLuint _shader_program) {
	GLint i;
	GLint count;	// count of how many variables are there

	GLint size;	   // size of the variable
	GLenum type;	// type of the variable (float, vec3 or mat4, etc)

	const GLsizei bufSize = 64;	   // maximum name length
	GLchar name[bufSize];	 // variable name in GLSL
	GLsizei length;	   // name length

	glGetProgramiv(_shader_program, GL_ACTIVE_UNIFORMS, &count);
	fprintf(stderr, "Active Uniforms: %d\n", count);

	for (i = 0; i < count; i++)
	{
		glGetActiveUniform(_shader_program, (GLuint) i, bufSize, &length, &size, &type, name);
		fprintf(stderr, "\t- Uniform #%d : \"%s\"\n", i, name);
	}
}

void Scene::prepareUniformsPlanes(GLfloat* mvMat, GLfloat* pMat, planes _plane, const GridGLView::Ptr& grid, bool showTexOnPlane) {
	bool shouldHide = false;
	if (_plane == planes::x) {
		shouldHide = this->planeVisibility.x;
	}
	if (_plane == planes::y) {
		shouldHide = this->planeVisibility.y;
	}
	if (_plane == planes::z) {
		shouldHide = this->planeVisibility.z;
	}

	auto getUniform = [&](const char* name) -> GLint {
		GLint g = glGetUniformLocation(this->program_Plane3D, name);
		if (this->showVAOstate) {
			if (g >= 0) {
				std::cerr << "[LOG]\tLocation [" << +g << "] for uniform " << name << '\n';
			} else {
				std::cerr << "[LOG]\tCannot find uniform " << name << "\n";
			}
		}
		return g;
	};

	if (this->showVAOstate) {
		LOG_ENTER(Scene::prepareUniforms_3DPlane);
		std::cerr << "[LOG] Uniform locations for " << __FUNCTION__ << " : \n";
	}

	// Get uniform locations for the program :
	GLint location_mMatrix		  = getUniform("model_Mat");
	GLint location_vMatrix		  = getUniform("view_Mat");
	GLint location_pMatrix		  = getUniform("projection_Mat");
	GLint location_gridTransform  = getUniform("gridTransform");
	GLint location_gridSize		  = getUniform("gridSize");
	GLint location_gridDimensions = getUniform("gridDimensions");

	// Shader uniforms :
	GLint location_texData				  = getUniform("texData");
	GLint location_planePosition		  = getUniform("planePositions");
	GLint location_planeDirection		  = getUniform("planeDirections");
	GLint location_sceneBBPosition		  = getUniform("sceneBBPosition");
	GLint location_sceneBBDiagonal		  = getUniform("sceneBBDiagonal");
	GLint location_colorBounds			  = getUniform("colorBounds");
	GLint location_textureBounds		  = getUniform("textureBounds");
	GLint location_colorBoundsAlternate	  = getUniform("colorBoundsAlternate");
	GLint location_textureBoundsAlternate = getUniform("textureBoundsAlternate");
	GLint location_currentPlane			  = getUniform("currentPlane");
	GLint location_showTex				  = getUniform("showTex");
	GLint location_drawOnlyData			  = getUniform("drawOnlyData");

	GLint location_color0			 = getUniform("color0");
	GLint location_color1			 = getUniform("color1");
	GLint location_color0Alt		 = getUniform("color0Alternate");
	GLint location_color1Alt		 = getUniform("color1Alternate");
	GLint location_r_channelView	 = getUniform("r_channelView");
	GLint location_r_selectedChannel = getUniform("r_selectedChannel");
	GLint location_r_nbChannels		 = getUniform("r_nbChannels");
	GLint location_g_channelView	 = getUniform("g_channelView");
	GLint location_g_selectedChannel = getUniform("g_selectedChannel");
	GLint location_g_nbChannels		 = getUniform("g_nbChannels");
	GLint location_rgbMode			 = getUniform("rgbMode");

	if (grid->nbChannels > 1) {
		glUniform1ui(location_r_selectedChannel, this->selectedChannel_r);
		glUniform1ui(location_g_selectedChannel, this->selectedChannel_g);
	} else {
		glUniform1ui(location_r_selectedChannel, 0);
		glUniform1ui(location_g_selectedChannel, 0);
	}
	glUniform3fv(location_color0, 1, glm::value_ptr(this->color0));
	glUniform3fv(location_color1, 1, glm::value_ptr(this->color1));
	glUniform3fv(location_color0Alt, 1, glm::value_ptr(this->color0_second));
	glUniform3fv(location_color1Alt, 1, glm::value_ptr(this->color1_second));
	glUniform1ui(location_rgbMode, this->rgbMode);

	// Generate the data we need :
#warning Transform API is still in-progress.
    Image::bbox_t bbws = Image::bbox_t(grid->grid->getBoundingBox());
	glm::vec3 dims	   = glm::convert_to<glm::vec3::value_type>(grid->grid->getResolution()) * grid->voxelDimensions;
	glm::vec3 size	   = bbws.getDiagonal();
	GLint plIdx		   = (_plane == planes::x) ? 1 : (_plane == planes::y) ? 2 :
																			   3;

	Image::bbox_t::vec position = this->grids.back()->grid->bbMin;
	Image::bbox_t::vec diagonal = this->grids.back()->grid->getDimensions();
	glm::vec3 planePos			= this->computePlanePositions();

	glm::mat4 transform							= glm::mat4(1.f);
	glUniformMatrix4fv(location_mMatrix, 1, GL_FALSE, glm::value_ptr(transform));
	glUniformMatrix4fv(location_vMatrix, 1, GL_FALSE, mvMat);
	glUniformMatrix4fv(location_pMatrix, 1, GL_FALSE, pMat);
	glUniformMatrix4fv(location_gridTransform, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(location_sceneBBPosition, 1, glm::value_ptr(position));
	glUniform3fv(location_sceneBBDiagonal, 1, glm::value_ptr(diagonal));
	glUniform3fv(location_gridSize, 1, glm::value_ptr(size));
	glUniform3fv(location_gridDimensions, 1, glm::value_ptr(dims));
	glUniform1i(location_currentPlane, plIdx);
	glUniform1ui(location_r_nbChannels, 1);
	glUniform1ui(location_g_nbChannels, 1);
	glUniform1ui(location_r_channelView, this->colorFunctionToUniform(this->channels_r));
	glUniform1ui(location_g_channelView, this->colorFunctionToUniform(this->channels_g));
	glUniform1ui(location_drawOnlyData, shouldHide ? 1 : 0);

	glm::vec2 tb0 = glm::convert_to<float>(this->textureBounds0);
	glm::vec2 tb1 = glm::convert_to<float>(this->textureBounds1);

	glUniform2fv(location_colorBounds, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds0)));
	glUniform2fv(location_colorBoundsAlternate, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds1)));
	glUniform2f(location_textureBounds, tb0.x, tb0.y);
	glUniform2f(location_textureBoundsAlternate, tb1.x, tb1.y);

	glUniform3fv(location_planePosition, 1, glm::value_ptr(planePos));
	glUniform3fv(location_planeDirection, 1, glm::value_ptr(this->planeDirection));
	glUniform1i(location_showTex, showTexOnPlane ? 1 : 0);

	GLint location_colorScales0 = getUniform("colorScales[0]");
	GLint location_colorScales1 = getUniform("colorScales[1]");
	GLint location_colorScales2 = getUniform("colorScales[2]");
	GLint location_colorScales3 = getUniform("colorScales[3]");

	GLint enabled_textures = 0;
	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, grid->gridTexture);
	glUniform1i(location_texData, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_greyscale);
	glUniform1i(location_colorScales0, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_hsv2rgb);
	glUniform1i(location_colorScales1, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_user0);
	glUniform1i(location_colorScales2, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_user1);
	glUniform1i(location_colorScales3, enabled_textures);
	enabled_textures++;

	const GLchar uniform_block_name[] = "ColorBlock";
	GLuint colorBlock_index			  = glGetUniformBlockIndex(this->program_projectedTex, uniform_block_name);
	glUniformBlockBinding(this->program_projectedTex, colorBlock_index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, grid->uboHandle_colorAttributes);
}

void Scene::prepareUniformsMonoPlaneView(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, glm::vec2 offset, const GridGLView::Ptr& _grid) {
	glUseProgram(this->program_PlaneViewer);
	// The BB used is the scene's bounding box :// NO not anymore, because the scene can be deformed
	//const Image::bbox_t::vec& bbox	 = this->sceneBB.getDiagonal();
	//const Image::bbox_t::vec& posBox = this->sceneBB.getMin();

	//const Image::bbox_t::vec& bbox	 = this->grids[this->gridToDraw]->grid->getResolution();
	const glm::vec3 bbox	 = this->grids[this->gridToDraw]->grid->getDimensions();
	const Image::bbox_t::vec& posBox = glm::vec3(0., 0., 0.);

	// The correct bounding box coordinates :
	glm::vec2 gridBBDims;
	if (_plane == planes::x) {
		gridBBDims.x = bbox.y;
		gridBBDims.y = bbox.z;
	}
	if (_plane == planes::y) {
		gridBBDims.x = bbox.x;
		gridBBDims.y = bbox.z;
	}
	if (_plane == planes::z) {
		gridBBDims.x = bbox.x;
		gridBBDims.y = bbox.y;
	}

	// Plane heading as a integer value (valid for shaders) :
	uint plane_heading							= planeHeadingToIndex(_heading);
#warning Transform API is still in-progress.
	// Grid dimensions :
	glm::vec3 gridDimensions = this->grids[this->gridToDraw]->grid->getDimensions();
	// Depth of the plane :
	glm::vec3 planePos = this->computePlanePositions();

	auto getUniform = [&](const char* name) -> GLint {
		GLint g = glGetUniformLocation(this->program_PlaneViewer, name);
		if (this->showVAOstate) {
			if (g >= 0) {
				std::cerr << "[LOG]\tLocation [" << +g << "] for uniform " << name << '\n';
			} else {
				std::cerr << "[LOG]\tCannot find uniform " << name << "\n";
			}
		}
		return g;
	};

	if (this->showVAOstate) {
		LOG_ENTER(Scene::prepareUniforms_PlaneViewer);
		std::cerr << "[LOG] Uniform locations for " << __FUNCTION__ << " : \n";
	}

	TextureUpload texParamsState;
    texParamsState.minmag.x = GL_NEAREST;
    texParamsState.minmag.y = GL_NEAREST;
    texParamsState.lod.y	 = -1000.f;
    texParamsState.wrap.s	 = GL_CLAMP;
    texParamsState.wrap.t	 = GL_CLAMP;
    
    texParamsState.internalFormat = GL_RGB32F;
    texParamsState.size.y		   = 1;
    texParamsState.size.z		   = 1;
    texParamsState.format		   = GL_RGB;
    texParamsState.type		   = GL_FLOAT;

	TextureUpload texParamsPos;
    texParamsPos.minmag.x = GL_NEAREST;
    texParamsPos.minmag.y = GL_NEAREST;
    texParamsPos.lod.y	 = -1000.f;
    texParamsPos.wrap.s	 = GL_CLAMP;
    texParamsPos.wrap.t	 = GL_CLAMP;
    
    texParamsPos.internalFormat = GL_RGB32F;
    texParamsPos.size.y		   = 1;
    texParamsPos.size.z		   = 1;
    texParamsPos.format		   = GL_RGB;
    texParamsPos.type		   = GL_FLOAT;

	// Uniform locations :
	// VShader :
	GLint location_fbDims		  = getUniform("fbDims");
	GLint location_bbDims		  = getUniform("bbDims");
	GLint location_planeIndex	  = getUniform("planeIndex");
	GLint location_gridTransform  = getUniform("gridTransform");
	GLint location_gridDimensions = getUniform("gridDimensions");
	GLint location_gridBBDiagonal = getUniform("sceneBBDiagonal");
	GLint location_gridBBPosition = getUniform("sceneBBPosition");
	GLint location_planePositions = getUniform("planePositions");
	GLint location_heading		  = getUniform("heading");
	GLint location_zoom			  = getUniform("zoom");
	GLint location_offset		  = getUniform("offset");
	// FShader :
	GLint location_texData				  = getUniform("texData");
	GLint location_colorBounds			  = getUniform("colorBounds");
	GLint location_colorBoundsAlternate	  = getUniform("colorBoundsAlternate");
	GLint location_textureBounds		  = getUniform("textureBounds");
	GLint location_textureBoundsAlternate = getUniform("textureBoundsAlternate");
	GLint location_color0				  = getUniform("color0");
	GLint location_color1				  = getUniform("color1");
	GLint location_color0Alt			  = getUniform("color0Alternate");
	GLint location_color1Alt			  = getUniform("color1Alternate");
	GLint location_r_channelView		  = getUniform("r_channelView");
	GLint location_r_selectedChannel	  = getUniform("r_selectedChannel");
	GLint location_r_nbChannels			  = getUniform("r_nbChannels");
	GLint location_g_channelView		  = getUniform("g_channelView");
	GLint location_g_selectedChannel	  = getUniform("g_selectedChannel");
	GLint location_g_nbChannels			  = getUniform("g_nbChannels");
	GLint location_rgbMode				  = getUniform("rgbMode");

    // Manipulator display
    //uniform uint displayManipulator;
    //uniform float sphereRadius;
    //uniform uint nbManipulator;
    //uniform usampler1D manipulatorPositions;	
    GLint location_manipulatorPositions = getUniform("manipulatorPositions");
    //uniform usampler1D states

    /***/

    unsigned int shouldDisplay = 1;
    //float radius = this->glMeshManipulator->meshManipulator->getManipulatorSize();
    float radius = this->glMeshManipulator->planeViewRadius;
    
    std::vector<glm::vec3> manipulatorPositions;
    if(this->glMeshManipulator->meshManipulator) {
        this->glMeshManipulator->meshManipulator->getAllPositions(manipulatorPositions);
    }

	glUniform1ui(getUniform("displayManipulator"), shouldDisplay);
	glUniform1f(getUniform("sphereRadius"), radius);
	glUniform1ui(getUniform("nbManipulator"), manipulatorPositions.size());

	std::vector<UITool::State> rawState;
    if(this->glMeshManipulator->meshManipulator) {
        this->glMeshManipulator->meshManipulator->getManipulatorsState(rawState);
    }

	std::vector<glm::vec3> state;
    for(int i = 0; i < rawState.size(); ++i) {
        int value = int(rawState[i]);
        state.push_back(glm::vec3(value, value, value));
    }

	texParamsState.data   = state.data();
	texParamsState.size.x = state.size();

    glDeleteTextures(1, &this->state_idx);
	this->state_idx = this->uploadTexture1D(texParamsState);

	std::vector<glm::vec3> allPositions;
    if(this->glMeshManipulator->meshManipulator) {
	    this->glMeshManipulator->meshManipulator->getAllPositions(allPositions);
    }
	texParamsPos.data   = allPositions.data();
	texParamsPos.size.x = allPositions.size();
    glDeleteTextures(1, &this->pos_idx);
	this->pos_idx = this->uploadTexture1D(texParamsPos);

    /***/

	glUniform1ui(location_rgbMode, this->rgbMode);

	glUniform1ui(location_r_channelView, this->colorFunctionToUniform(this->channels_r));
	glUniform1ui(location_g_channelView, this->colorFunctionToUniform(this->channels_g));

	glUniform3fv(location_color0, 1, glm::value_ptr(this->color0));
	glUniform3fv(location_color1, 1, glm::value_ptr(this->color1));
	glUniform3fv(location_color0Alt, 1, glm::value_ptr(this->color0_second));
	glUniform3fv(location_color1Alt, 1, glm::value_ptr(this->color1_second));
	if (_grid->nbChannels > 1) {
		glUniform1ui(location_r_selectedChannel, this->selectedChannel_r);
		glUniform1ui(location_g_selectedChannel, this->selectedChannel_g);
	} else {
		glUniform1ui(location_r_selectedChannel, 0);
		glUniform1ui(location_g_selectedChannel, 0);
	}

	glm::vec2 tb0 = glm::convert_to<float>(this->textureBounds0);
	glm::vec2 tb1 = glm::convert_to<float>(this->textureBounds1);

	// Uniform variables :
	glUniform2fv(location_fbDims, 1, glm::value_ptr(fbDims));
	glUniform2fv(location_bbDims, 1, glm::value_ptr(gridBBDims));
	glUniform1ui(location_planeIndex, (_plane == planes::x) ? 1 : (_plane == planes::y) ? 2 : 3);
	glUniformMatrix4fv(location_gridTransform, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	glUniform4f(location_gridDimensions, gridDimensions.x, gridDimensions.y, gridDimensions.z, 1.f);
	glUniform4f(location_gridBBDiagonal, bbox.x, bbox.y, bbox.z, 1.f);
	glUniform4f(location_gridBBPosition, posBox.x, posBox.y, posBox.z, .0f);
	glUniform3fv(location_planePositions, 1, glm::value_ptr(planePos));
	glUniform2fv(location_colorBounds, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds0)));
	glUniform2fv(location_colorBoundsAlternate, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds1)));
	glUniform2f(location_textureBounds, tb0.x, tb0.y);
	glUniform2f(location_textureBoundsAlternate, tb1.x, tb1.y);
	glUniform1ui(location_heading, plane_heading);
	glUniform1f(location_zoom, zoomRatio);
	glUniform1ui(location_r_nbChannels, 1);
	glUniform1ui(location_g_nbChannels, 1);
	glm::vec2 realOffset = offset;
	realOffset.y		 = -realOffset.y;
	glUniform2fv(location_offset, 1, glm::value_ptr(realOffset));

	// Uniform samplers :
	GLint tex = 0;
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_3D, _grid->gridTexture);
	glUniform1i(location_texData, tex);
	tex++;

	GLint location_colorScales0 = getUniform("colorScales[0]");
	GLint location_colorScales1 = getUniform("colorScales[1]");
	GLint location_colorScales2 = getUniform("colorScales[2]");
	GLint location_colorScales3 = getUniform("colorScales[3]");

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_greyscale);
	glUniform1i(location_colorScales0, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_hsv2rgb);
	glUniform1i(location_colorScales1, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_user0);
	glUniform1i(location_colorScales2, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_user1);
	glUniform1i(location_colorScales3, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, state_idx);
	glUniform1i(getUniform("states"), tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, pos_idx);
	glUniform1i(getUniform("manipulatorPositions"), tex);
	tex++;

	const GLchar uniform_block_name[] = "ColorBlock";
	GLuint colorBlock_index			  = glGetUniformBlockIndex(this->program_projectedTex, uniform_block_name);
	glUniformBlockBinding(this->program_projectedTex, colorBlock_index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, _grid->uboHandle_colorAttributes);
}

void Scene::prepareUniformsGridVolumetricView(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, const GridGLView::Ptr& _grid) {
	// We assume the right program has been bound.

	/// @brief Shortcut for glGetUniform, since this can result in long lines.
	auto getUniform = [&](const char* name) -> GLint {
		GLint g = glGetUniformLocation(this->program_VolumetricViewer, name);
		if (this->showVAOstate) {
			if (g >= 0) {
				std::cerr << "[LOG]\tLocation [" << +g << "] for uniform " << name << '\n';
			} else {
				std::cerr << "[LOG]\tCannot find uniform " << name << "\n";
			}
		}
		return g;
	};

	if (this->showVAOstate) {
		LOG_ENTER(Scene::drawVolumetric)
		std::cerr << "[LOG] Uniform locations for " << __FUNCTION__ << " : \n";
	}

	// Texture handles :
	GLint location_vertices_translation	  = getUniform("vertices_translations");
	GLint location_normals_translation	  = getUniform("normals_translations");
	GLint location_visibility_texture	  = getUniform("visibility_texture");
	GLint location_texture_coordinates	  = getUniform("texture_coordinates");
	GLint location_neighbors			  = getUniform("neighbors");
	GLint location_Mask					  = getUniform("texData");
	GLint location_visibilityMap		  = getUniform("visiblity_map");
	GLint location_visibilityMapAlternate = getUniform("visiblity_map_alternate");

	std::size_t tex = 0;
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, _grid->volumetricMesh.vertexPositions);
	glUniform1i(location_vertices_translation, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, _grid->volumetricMesh.faceNormals);
	glUniform1i(location_normals_translation, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, _grid->volumetricMesh.visibilityMap);
	glUniform1i(location_visibility_texture, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, _grid->volumetricMesh.textureCoordinates);
	glUniform1i(location_texture_coordinates, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, _grid->volumetricMesh.neighborhood);
	glUniform1i(location_neighbors, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_3D, _grid->gridTexture);
	glUniform1i(location_Mask, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, this->tex_ColorScaleGrid);
	glUniform1i(location_visibilityMap, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, this->tex_ColorScaleGridAlternate);
	glUniform1i(location_visibilityMapAlternate, tex);
	tex++;

	// Scalars :
	GLint location_voxelSize = getUniform("voxelSize");
	GLint location_gridSize	 = getUniform("gridSize");

	glm::vec3 floatres = glm::convert_to<float>(_grid->grid->getDimensions());

    glUniform3fv(location_voxelSize, 1, glm::value_ptr(_grid->grid->getWorldVoxelSize()));
	glUniform3fv(location_gridSize, 1, glm::value_ptr(floatres));

	// Vectors/arrays :
	GLint location_cam					  = getUniform("cam");
	GLint location_cut					  = getUniform("cut");
	GLint location_cutDirection			  = getUniform("cutDirection");
	GLint location_clipDistanceFromCamera = getUniform("clipDistanceFromCamera");
	GLint location_visuBBMin			  = getUniform("visuBBMin");
	GLint location_visuBBMax			  = getUniform("visuBBMax");
	GLint location_shouldUseBB			  = getUniform("shouldUseBB");
	GLint location_displayWireframe		  = getUniform("displayWireframe");
	GLint location_volumeEpsilon		  = getUniform("volumeEpsilon");

	glm::vec3 planePos	   = this->computePlanePositions();
    Image::bbox_t::vec min = _grid->grid->bbMin;
    Image::bbox_t::vec max = _grid->grid->bbMax;

	glUniform3fv(location_cam, 1, glm::value_ptr(camPos));
	glUniform3fv(location_cut, 1, glm::value_ptr(planePos));
	glUniform3fv(location_cutDirection, 1, glm::value_ptr(this->planeDirection));
	glUniform1f(location_clipDistanceFromCamera, this->clipDistanceFromCamera);
	glUniform3fv(location_visuBBMin, 1, glm::value_ptr(min));
	glUniform3fv(location_visuBBMax, 1, glm::value_ptr(max));
	//glUniform1ui(location_shouldUseBB, ((this->drawMode == DrawMode::VolumetricBoxed) ? 1 : 0));
	glUniform1ui(location_shouldUseBB, 0);
	glUniform1ui(location_displayWireframe, this->glMeshManipulator->isWireframeDisplayed());
	glUniform3fv(location_volumeEpsilon, 1, glm::value_ptr(_grid->defaultEpsilon));

	// Matrices :
	GLint location_mMat = getUniform("mMat");
	GLint location_vMat = getUniform("vMat");
	GLint location_pMat = getUniform("pMat");

    glUniformMatrix4fv(location_mMat, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
	glUniformMatrix4fv(location_vMat, 1, GL_FALSE, mvMat);
	glUniformMatrix4fv(location_pMat, 1, GL_FALSE, pMat);

	// Light positions :
	GLint location_light0 = getUniform("lightPositions[0]");
	GLint location_light1 = getUniform("lightPositions[1]");
	GLint location_light2 = getUniform("lightPositions[2]");
	GLint location_light3 = getUniform("lightPositions[3]");
	GLint location_light4 = getUniform("lightPositions[4]");
	GLint location_light5 = getUniform("lightPositions[5]");
	GLint location_light6 = getUniform("lightPositions[6]");
	GLint location_light7 = getUniform("lightPositions[7]");

	glUniform3fv(location_light0, 1, glm::value_ptr(this->lightPositions[0]));
	glUniform3fv(location_light1, 1, glm::value_ptr(this->lightPositions[1]));
	glUniform3fv(location_light2, 1, glm::value_ptr(this->lightPositions[2]));
	glUniform3fv(location_light3, 1, glm::value_ptr(this->lightPositions[3]));
	glUniform3fv(location_light4, 1, glm::value_ptr(this->lightPositions[4]));
	glUniform3fv(location_light5, 1, glm::value_ptr(this->lightPositions[5]));
	glUniform3fv(location_light6, 1, glm::value_ptr(this->lightPositions[6]));
	glUniform3fv(location_light7, 1, glm::value_ptr(this->lightPositions[7]));

	// Color and shading parameters :
	GLint location_specRef	  = getUniform("specRef");
	GLint location_shininess  = getUniform("shininess");
	GLint location_diffuseRef = getUniform("diffuseRef");
	glUniform1f(location_specRef, .8f);
	glUniform1f(location_shininess, .8f);
	glUniform1f(location_diffuseRef, .8f);

	// User-defined colors :
	GLint location_color0	 = getUniform("color0");
	GLint location_color1	 = getUniform("color1");
	GLint location_color0Alt = getUniform("color0Alternate");
	GLint location_color1Alt = getUniform("color1Alternate");

    glUniform3fv(location_color0, 1, glm::value_ptr(this->color0));
    glUniform3fv(location_color1, 1, glm::value_ptr(this->color1));
    glUniform3fv(location_color0Alt, 1, glm::value_ptr(this->color0_second));
    glUniform3fv(location_color1Alt, 1, glm::value_ptr(this->color1_second));

	GLint location_colorScales0 = getUniform("colorScales[0]");
	GLint location_colorScales1 = getUniform("colorScales[1]");
	GLint location_colorScales2 = getUniform("colorScales[2]");
	GLint location_colorScales3 = getUniform("colorScales[3]");

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_greyscale);
	glUniform1i(location_colorScales0, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->tex_colorScale_hsv2rgb);
	glUniform1i(location_colorScales1, tex);
	tex++;

    GLuint colorScale = this->tex_colorScale_user0;
    if(this->gridToDraw > 0)
        colorScale = this->tex_colorScale_user1;

	glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, colorScale);
	glUniform1i(location_colorScales2, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, colorScale);
	glUniform1i(location_colorScales3, tex);
	tex++;

	const GLchar uniform_block_name[] = "ColorBlock";
	GLuint colorBlock_index			  = glGetUniformBlockIndex(this->program_projectedTex, uniform_block_name);
	glUniformBlockBinding(this->program_projectedTex, colorBlock_index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, _grid->uboHandle_colorAttributes);

	// print uniform values :
	if (this->showVAOstate) {
		LOG_LEAVE(Scene::drawVolumetric)
	}
}

void Scene::drawGridPlaneView(GLfloat* mvMat, GLfloat* pMat, glm::mat4 baseMatrix, const GridGLView::Ptr& grid) {
	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	// If the grid has been uploaded, show it :
	if (grid->gridTexture > 0) {
		glUseProgram(this->program_projectedTex);
		glBindVertexArray(this->vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_Element);

		this->prepareUniformsGridPlaneView(mvMat, pMat, lightPos, baseMatrix, grid);
		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*) 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}

	// No matter the grid's state, print the bounding box :
	this->drawBoundingBox(grid->grid->getBoundingBox(), grid->boundingBoxColor, mvMat, pMat);
}

void Scene::newSHADERS_print_all_uniforms(GLuint _shader_program) {
	// Assume the program used is the right one :
	glUseProgram(_shader_program);
	GLuint uni_block = glGetUniformBlockIndex(_shader_program, "ColorBlock");
	if (uni_block == GL_INVALID_INDEX) {
		std::cerr << "Color block uniform has an invalid index\n";
	} else {
		std::cerr << "Color block binding is : " << uni_block << '\n';
	}
	std::cerr << "Color attributes uniforms :\n";

	std::string baseArrayName	 = "ColorBlock.attributes";
	std::string uniformArrayName = "";
	for (std::size_t i = 0; i < 4; ++i) {
		uniformArrayName	  = baseArrayName + '[' + std::to_string(i) + ']';
		GLint block_loc		  = glGetUniformLocation(_shader_program, (uniformArrayName).c_str());
		GLint uni_visible	  = glGetUniformLocation(_shader_program, (uniformArrayName + ".isVisible").c_str());
		GLint uni_index		  = glGetUniformLocation(_shader_program, (uniformArrayName + ".colorScaleIndex").c_str());
		GLint uni_visbounds	  = glGetUniformLocation(_shader_program, (uniformArrayName + ".visibleBounds").c_str());
		GLint uni_colorBounds = glGetUniformLocation(_shader_program, (uniformArrayName + ".colorScaleBounds").c_str());
		std::cerr << "\t" << uniformArrayName << " = " << block_loc << " {\n";
		std::cerr << "\t\t.isVisible = " << uni_visible << ",\n";
		std::cerr << "\t\t.colorScaleIndex = " << uni_index << ",\n";
		std::cerr << "\t\t.visibleBounds = " << uni_visbounds << ",\n";
		std::cerr << "\t\t.colorScaleBounds = " << uni_colorBounds << "\n";
		std::cerr << "\t},\n";
	}
	std::cerr << "Ending of new color uniforms\n";
	glUseProgram(0);
}

void Scene::newSHADERS_generateColorScales() {
	this->generateColorScales();
}

void Scene::draw3DView(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, bool showTexOnPlane) {
    this->cameraPosition = camPos;
	if (this->shouldDeleteGrid) {
		this->deleteGridNow();
	}
	if (this->shouldUpdateUserColorScales) {
		this->newSHADERS_updateUserColorScales();
	}
	if (this->shouldUpdateUBOData) {
		this->newSHADERS_updateUBOData();
	}

    this->setLightPosition(camPos);

	glEnable(GL_DEPTH_TEST);
	glEnablei(GL_BLEND, 0);
	glEnable(GL_TEXTURE_3D);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //return;


	glm::mat4 transfoMat = glm::mat4(1.f);
	/* Manipulator drawing  */

	glm::mat4 mMat(1.0f);
    this->glMeshManipulator->draw(mvMat, pMat, glm::value_ptr(mMat), this->computePlanePositions());

	/***********************/


	//if (this->drawMode == DrawMode::Solid) {
	//	for (std::size_t i = 0; i < this->grids.size(); ++i) {
	//		this->drawGridPlaneView(mvMat, pMat, transfoMat, this->grids[i]);
	//	}
	//} else if (this->drawMode == DrawMode::Volumetric || this->drawMode == DrawMode::VolumetricBoxed) {
    if(this->displayGrid) {
        if(this->multiGridRendering) {
            if(this->grids.size() > 0) {
                int originalGridToDraw = this->gridToDraw;
                for(auto i : this->gridsToDraw) {
                    if(i >= 0 && i < this->grids.size()) {
                        this->gridToDraw = i;
                        this->drawGridVolumetricView(mvMat, pMat, camPos, this->grids[gridToDraw]);
                        //this->drawPlanes(mvMat, pMat, this->drawMode == DrawMode::Solid);
                    }
                }
                this->gridToDraw = originalGridToDraw;
            }
        } else {
            if(this->grids.size() > 0) {
                this->drawGridVolumetricView(mvMat, pMat, camPos, this->grids[gridToDraw]);
            }
        }
    }

//		if (this->drawMode == DrawMode::VolumetricBoxed) {
//			this->drawBoundingBox(this->visuBox, glm::vec3(1., .0, .0), mvMat, pMat);
//		}
//	}

    if(this->displayMesh) {
        for(int i = 0; i < this->drawableMeshes.size(); ++i) {
            this->drawableMeshes[i].first->makeVAO();
            this->drawableMeshes[i].first->draw(pMat, mvMat, glm::vec4{camPos, 1.f});
        }
    }

	//this->drawBoundingBox(this->sceneBB, glm::vec4(.5, .5, .0, 1.), mvMat, pMat);
	this->showVAOstate = false;

    this->glSelection->draw(mvMat, pMat, glm::value_ptr(mMat));

    glUseProgram(this->program_doublePass);
    glBindVertexArray(this->quad_VertexArrayID);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

}

void Scene::newSHADERS_updateUBOData() {
	this->shouldUpdateUBOData = false;
	for (const auto& grid : this->grids) {
        this->setUniformBufferData(grid->uboHandle_colorAttributes,  0, 32, &grid->mainColorChannelAttributes());
		this->setUniformBufferData(grid->uboHandle_colorAttributes, 32, 32, &grid->colorChannelAttributes[0]);
		this->setUniformBufferData(grid->uboHandle_colorAttributes, 64, 32, &grid->colorChannelAttributes[1]);
		this->setUniformBufferData(grid->uboHandle_colorAttributes, 96, 32, &grid->colorChannelAttributes[2]);
	}
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

glm::vec4 Scene::readFramebufferContents(GLuint fb_handle, glm::ivec2 image_coordinates) {
	// The container of pixel values :
	glm::vec4 pixelValue = glm::vec4();

	// Bind the framebuffer to read from :
	glBindFramebuffer(GL_FRAMEBUFFER, fb_handle);
	// Specify reading from color attachment 1:
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	// Read pixels :
	glReadPixels(image_coordinates.x, image_coordinates.y, 1, 1, GL_RGBA, GL_FLOAT, glm::value_ptr(pixelValue));

	return pixelValue;
}

void Scene::generateSceneData() {
	SimpleVolMesh mesh{};
	this->generateTexCube(mesh);
	this->renderSize = mesh.indices.size();
	this->generatePlanesArray(mesh);
	this->setupVBOData(mesh);
	this->createBoundingBoxBuffers();

	this->generateSphereData();
}

void Scene::generateSphereData() {
	std::size_t segments_around = 8;
	std::size_t segments_height = 8;

	std::vector<glm::vec4> positions;	 // also serves as normals ...
	std::vector<unsigned int> indices;

	// Iterate and create positions around the sphere :
	for (std::size_t i = 1; i <= segments_height; ++i) {
		for (std::size_t j = 0; j < segments_around; ++j) {
			double theta = 2 * M_PI * static_cast<float>(j) / static_cast<float>(segments_around);
			double phi	 = M_PI * static_cast<float>(i) / static_cast<float>(segments_height + 1);
			positions.push_back(glm::normalize(glm::vec4{
			  static_cast<float>(sin(phi) * cos(theta)),
			  static_cast<float>(sin(phi) * sin(theta)),
			  static_cast<float>(cos(phi)), 1.f}));
		}
	}
	positions.push_back(glm::normalize(glm::vec4{
	  static_cast<float>(sin(.0f) * cos(2 * M_PI)),
	  static_cast<float>(sin(.0f) * sin(2 * M_PI)),
	  static_cast<float>(cos(.0f)), 1.f}));
	positions.push_back(glm::normalize(glm::vec4{
	  static_cast<float>(sin(M_PI) * cos(2 * M_PI)),
	  static_cast<float>(sin(M_PI) * sin(2 * M_PI)),
	  static_cast<float>(cos(M_PI)), 1.f}));
	// link the sphere faces :
	for (std::size_t i = 0; i < segments_height - 1; ++i) {
		std::size_t next_segment_height = i + 1;
		for (std::size_t j = 0; j < segments_around; ++j) {
			std::size_t next_segment_around = j + 1;
			if (next_segment_around == segments_around) {
				next_segment_around = 0;
			}

			// The four corners below define a quad (region of the sphere).
			std::size_t p1 = i * segments_around + j;
			std::size_t p2 = i * segments_around + next_segment_around;
			std::size_t p3 = next_segment_height * segments_around + j;
			std::size_t p4 = next_segment_height * segments_around + next_segment_around;
			// Link them as two triangles :
			indices.push_back(p1);
			indices.push_back(p2);
			indices.push_back(p3);
			indices.push_back(p3);
			indices.push_back(p2);
			indices.push_back(p4);
		}
	}
	// Link the top and bottom 'cones' :
	for (std::size_t i = 0; i < segments_around; ++i) {
		std::size_t next_segment_around = i + 1;
		if (next_segment_around == segments_around) {
			next_segment_around = 0;
		}
		indices.push_back(i);
		indices.push_back(next_segment_around);
		indices.push_back(positions.size() - 2);	// actually the top point
	}
	for (std::size_t i = 0; i < segments_around; ++i) {
		std::size_t next_segment_around = i + 1;
		if (next_segment_around == segments_around) {
			next_segment_around = 0;
		}
		indices.push_back(positions.size() - segments_around + i);
		indices.push_back(positions.size() - segments_around + next_segment_around);
		indices.push_back(positions.size() - 1);	// actually the bottom point
	}

	// Create VAO/VBO and upload data :
	glGenVertexArrays(1, &this->vao_spheres);
	glBindVertexArray(this->vao_spheres);

	glGenBuffers(1, &this->vbo_spherePositions);
	glGenBuffers(1, &this->vbo_sphereNormals);
	glGenBuffers(1, &this->vbo_sphereIndices);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_spherePositions);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4::value_type) * 4 * positions.size(), positions.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_sphereNormals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4::value_type) * 4 * positions.size(), positions.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_sphereIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_spherePositions);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_sphereNormals);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	if (this->shaderCompiler) {
		this->shaderCompiler.reset();
	}
	this->shaderCompiler = std::make_unique<ShaderCompiler>(this);

	this->shaderCompiler->vertexShader_file("../shaders/base_sphere.vert").fragmentShader_file("../shaders/base_sphere.frag");
	bool program_valid = this->shaderCompiler->compileShaders();
	if (program_valid) {
		this->program_sphere = this->shaderCompiler->programName();
	} else {
		std::cerr << "Error while compiling sphere shaders.\n";
	}
	std::cerr << "Messages for sphere shaders !!!\n"
			  << this->shaderCompiler->errorString() << '\n';

	this->sphere_size_to_draw = indices.size();
}

void Scene::generatePlanesArray(SimpleVolMesh& _mesh) {
	_mesh.cutting_planes.clear();
	_mesh.planar_view.clear();
	// Refer to the generateTexCube() function to see which
	// way the cube's vertices are inserted into the array.

	unsigned int a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6;
	std::vector otheridx{a, g, e, g, a, c, a, e, b, b, e, f, a, b, c, c, b, d};
	_mesh.cutting_planes.swap(otheridx);

	// base index for the index array :
	unsigned int base = _mesh.positions.size();

	glm::vec4 center = glm::vec4(.5f, .5f, .5f, 1.f);
	// Create new vertices :
	glm::vec4 apos	= glm::vec4(.0, .0, .0, 1.);
	glm::vec4 anorm = apos - center;
	glm::vec4 bpos	= glm::vec4(1., .0, .0, 1.);
	glm::vec4 bnorm = bpos - center;
	glm::vec4 cpos	= glm::vec4(.0, 1., .0, 1.);
	glm::vec4 cnorm = cpos - center;
	glm::vec4 dpos	= glm::vec4(1., 1., .0, 1.);
	glm::vec4 dnorm = dpos - center;
	glm::vec4 epos	= glm::vec4(.0, .0, 1., 1.);
	glm::vec4 enorm = epos - center;
	glm::vec4 fpos	= glm::vec4(1., .0, 1., 1.);
	glm::vec4 fnorm = fpos - center;
	glm::vec4 gpos	= glm::vec4(.0, 1., 1., 1.);
	glm::vec4 gnorm = gpos - center;
	// Create texture coordinates serving as framebuffer positions once all is done :
	glm::vec3 ll = glm::vec3(-1., -1., .0);	   // lower-left
	glm::vec3 lr = glm::vec3(-1., 1., .0);	  // lower-right
	glm::vec3 hl = glm::vec3(1., -1., .0);	  // higher-left
	glm::vec3 hr = glm::vec3(1., 1., .0);	 // higher-right

	// clang-format off
	// Push them into the vectors, so they can be drawn without indices :
	// Plane X :
	_mesh.positions.push_back(apos); _mesh.normals.push_back(anorm); _mesh.texture.push_back(ll);
	_mesh.positions.push_back(cpos); _mesh.normals.push_back(cnorm); _mesh.texture.push_back(lr);
	_mesh.positions.push_back(epos); _mesh.normals.push_back(enorm); _mesh.texture.push_back(hl);
	_mesh.positions.push_back(epos); _mesh.normals.push_back(enorm); _mesh.texture.push_back(hl);
	_mesh.positions.push_back(cpos); _mesh.normals.push_back(cnorm); _mesh.texture.push_back(lr);
	_mesh.positions.push_back(gpos); _mesh.normals.push_back(gnorm); _mesh.texture.push_back(hr);
	// Plane Y :
	_mesh.positions.push_back(apos); _mesh.normals.push_back(anorm); _mesh.texture.push_back(ll);
	_mesh.positions.push_back(bpos); _mesh.normals.push_back(bnorm); _mesh.texture.push_back(lr);
	_mesh.positions.push_back(epos); _mesh.normals.push_back(enorm); _mesh.texture.push_back(hl);
	_mesh.positions.push_back(epos); _mesh.normals.push_back(enorm); _mesh.texture.push_back(hl);
	_mesh.positions.push_back(bpos); _mesh.normals.push_back(bnorm); _mesh.texture.push_back(lr);
	_mesh.positions.push_back(fpos); _mesh.normals.push_back(fnorm); _mesh.texture.push_back(hr);
	// Plane Z :
	_mesh.positions.push_back(apos); _mesh.normals.push_back(anorm); _mesh.texture.push_back(ll);
	_mesh.positions.push_back(bpos); _mesh.normals.push_back(bnorm); _mesh.texture.push_back(lr);
	_mesh.positions.push_back(cpos); _mesh.normals.push_back(cnorm); _mesh.texture.push_back(hl);
	_mesh.positions.push_back(cpos); _mesh.normals.push_back(cnorm); _mesh.texture.push_back(hl);
	_mesh.positions.push_back(bpos); _mesh.normals.push_back(bnorm); _mesh.texture.push_back(lr);
	_mesh.positions.push_back(dpos); _mesh.normals.push_back(dnorm); _mesh.texture.push_back(hr);
	//clang-format on

	// Push back enough indices to draw the planes all at once :
	for (unsigned int i = 0; i < _mesh.positions.size() - base; ++i) {
		_mesh.planar_view.push_back(base + i);
	}

	return;
}

void Scene::setPlaneHeading(planes _plane, planeHeading _heading) {
	/**
	 * We know the way the vertices are drawn and arranged (see Scene::generatePlanesArray() for the order). We can
	 * just modify the 'texture' coordinates in order to directly modify the gl_Position of a vertex. Rotating them
	 * will rotate the image in the framebuffer, but the computation of the plane position/data will be able to be
	 * done independently of it.
	 *
	 * We need to modify the texture coordinates with glBufferSubData
	 */

	// Create texture coordinates serving as framebuffer positions once all is done :
	glm::vec3 ll = glm::vec3(-1., -1., .0);	   // lower-left
	glm::vec3 lr = glm::vec3(-1., 1., .0);	  // lower-right
	glm::vec3 hl = glm::vec3(1., -1., .0);	  // higher-left
	glm::vec3 hr = glm::vec3(1., 1., .0);	 // higher-right

	// Create arrays of 'positions' depending on the heading :
	std::vector<glm::vec3> north{ll, lr, hl, hl, lr, hr};	 // default orientation
	std::vector<glm::vec3> east{lr, hr, ll, ll, hr, hl};
	std::vector<glm::vec3> west{hl, ll, hr, hr, ll, lr};
	std::vector<glm::vec3> south{hr, hl, lr, lr, hl, ll};

	// Get the offset into the buffer to read from :
	GLsizeiptr planeBegin = 0;
	QString planeName;
	switch (_plane) {
		case planes::x:
			planeBegin = 0;
			planeName  = "X";
			break;
		case planes::y:
			planeBegin = 6;
			planeName  = "Y";
			break;
		case planes::z:
			planeBegin = 12;
			planeName  = "Z";
			break;
	}

	// Add 8 here because the 8 vertices of the cube will be placed
	// before the tex coordinates we're interested in :
	GLsizeiptr begin = (planeBegin + 8) * sizeof(glm::vec3);

	QString h;
	// Select the vertices' tex coordinates in order to draw it the 'right' way up :
	void* data = nullptr;
	switch (_heading) {
		case North:
			data = (void*) north.data();
			h	 = "N";
			break;
		case East:
			data = (void*) east.data();
			h	 = "E";
			break;
		case West:
			data = (void*) west.data();
			h	 = "W";
			break;
		case South:
			data = (void*) south.data();
			h	 = "S";
			break;
	}

	// Bind vertex texture coordinate buffer :
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_VertTex);
	// Modify a part of the data :
	glBufferSubData(GL_ARRAY_BUFFER, begin, 6 * sizeof(glm::vec3), data);

	if (this->programStatusBar != nullptr) {
		QString message = "Plane " + planeName + " is now oriented " + h;
		this->programStatusBar->showMessage(message, 5000);
	}

	return;
}

void Scene::generateTexCube(SimpleVolMesh& _mesh) {
	// Generate statically a cube of dimension gridSize :
	glm::vec4 center = glm::vec4(.5f, .5f, .5f, 1.f);
	/**
	 * Name references :
	 *     +z
	 *     A
	 *     e-------g
	 *    /|      /|
	 *   f-------h |
	 *   | |     | |
	 *   | a-----|-c-->+y
	 *   |/      |/
	 *   b-------d
	 *  /
	 *+x
	 */
	// clang-format off
	// Build position, normals, and tex coordinates all in one line for each vertex
	glm::vec4 apos	= glm::vec4(.0, .0, .0, 1.); glm::vec4 anorm = apos - center; glm::vec3 atex = glm::vec3(.0, .0, .0);
	glm::vec4 bpos	= glm::vec4(1., .0, .0, 1.); glm::vec4 bnorm = bpos - center; glm::vec3 btex = glm::vec3(1., .0, .0);
	glm::vec4 cpos	= glm::vec4(.0, 1., .0, 1.); glm::vec4 cnorm = cpos - center; glm::vec3 ctex = glm::vec3(.0, 1., .0);
	glm::vec4 dpos	= glm::vec4(1., 1., .0, 1.); glm::vec4 dnorm = dpos - center; glm::vec3 dtex = glm::vec3(1., 1., .0);
	glm::vec4 epos	= glm::vec4(.0, .0, 1., 1.); glm::vec4 enorm = epos - center; glm::vec3 etex = glm::vec3(.0, .0, 1.);
	glm::vec4 fpos	= glm::vec4(1., .0, 1., 1.); glm::vec4 fnorm = fpos - center; glm::vec3 ftex = glm::vec3(1., .0, 1.);
	glm::vec4 gpos	= glm::vec4(.0, 1., 1., 1.); glm::vec4 gnorm = gpos - center; glm::vec3 gtex = glm::vec3(.0, 1., 1.);
	glm::vec4 hpos	= glm::vec4(1., 1., 1., 1.); glm::vec4 hnorm = hpos - center; glm::vec3 htex = glm::vec3(1., 1., 1.);
	_mesh.positions.push_back(apos); _mesh.normals.push_back(anorm); _mesh.texture.push_back(atex);
	_mesh.positions.push_back(bpos); _mesh.normals.push_back(bnorm); _mesh.texture.push_back(btex);
	_mesh.positions.push_back(cpos); _mesh.normals.push_back(cnorm); _mesh.texture.push_back(ctex);
	_mesh.positions.push_back(dpos); _mesh.normals.push_back(dnorm); _mesh.texture.push_back(dtex);
	_mesh.positions.push_back(epos); _mesh.normals.push_back(enorm); _mesh.texture.push_back(etex);
	_mesh.positions.push_back(fpos); _mesh.normals.push_back(fnorm); _mesh.texture.push_back(ftex);
	_mesh.positions.push_back(gpos); _mesh.normals.push_back(gnorm); _mesh.texture.push_back(gtex);
	_mesh.positions.push_back(hpos); _mesh.normals.push_back(hnorm); _mesh.texture.push_back(htex);
	unsigned int a = 0;
	unsigned int b = 1;
	unsigned int c = 2;
	unsigned int d = 3;
	unsigned int e = 4;
	unsigned int f = 5;
	unsigned int g = 6;
	unsigned int h = 7;
	unsigned int faceIdx1[] = {a, d, c, a, b, d, e, b, a, e, f, b, g, e, a, g, a, c};
	unsigned int faceIdx2[] = {h, e, g, h, f, e, h, g, c, h, c, d, h, d, b, h, f, b};
	_mesh.indices.insert(_mesh.indices.end(), faceIdx1, faceIdx1 + 18);
	_mesh.indices.insert(_mesh.indices.end(), faceIdx2, faceIdx2 + 18);
	// clang-format on
}

void Scene::setupVBOData(const SimpleVolMesh& _mesh) {
	if (glIsVertexArray(this->vao) == GL_FALSE) {
		throw std::runtime_error("The vao handle generated by OpenGL has been invalidated !");
	}

	// UPLOAD DATA (VBOs have previously been created in createBuffers() :

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_VertPos);
	glBufferData(GL_ARRAY_BUFFER, _mesh.positions.size() * sizeof(glm::vec4), _mesh.positions.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_VertNorm);
	glBufferData(GL_ARRAY_BUFFER, _mesh.normals.size() * sizeof(glm::vec4), _mesh.normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_VertTex);
	glBufferData(GL_ARRAY_BUFFER, _mesh.texture.size() * sizeof(glm::vec3), _mesh.texture.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_Element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(unsigned int), _mesh.indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_PlaneElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.cutting_planes.size() * sizeof(unsigned int), _mesh.cutting_planes.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_SinglePlaneElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.planar_view.size() * sizeof(unsigned int), _mesh.planar_view.data(), GL_STATIC_DRAW);

	this->setupVAOPointers();
}

void Scene::setupVAOPointers() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_VertPos);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_VertNorm);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_VertTex);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
}

glm::vec3 Scene::getSceneBoundaries() const {
	return this->sceneBB.getDiagonal();
}

void Scene::createBoundingBoxBuffers() {
	/**
	 * The bounding boxes will be drawn using GL_LINES. As such, we have to get all the positions of the corners of
	 * the bounding boxes uploaded to OpenGL, and then use an index buffer to make the lines to be drawn. (12 lines
	 * in total to draw a 'cube').
	 */
	// Create a basic bounding box :
	Image::bbox_t defaultBB = this->sceneBB;
	// Get all corners and put them sequentially in an array :
	std::vector<Image::bbox_t::vec> corners = defaultBB.getAllCorners();
	GLfloat* rawVertices					= new GLfloat[corners.size() * 3];
	for (std::size_t i = 0; i < corners.size(); ++i) {
		rawVertices[3 * i + 0] = corners[i].x;
		rawVertices[3 * i + 1] = corners[i].y;
		rawVertices[3 * i + 2] = corners[i].z;
	}

	/**
	 * The corners of a BB are given in a special order (see BoundingBox_General::getAllCorners())
	 * The lines are constructed from the indices
	 */
	GLuint a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6, h = 7;
	GLuint rawIndices[] = {a, b, b, d, d, c, c, a, e, f, f, h, h, g, g, e, a, e, b, f, c, g, d, h};

	// The VAO and VBOs have been created previously in createBuffers(). No need to create them again here.

	// Bind the vertex buffer :
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_boundingBoxVertices);
	// Upload data to OpenGL :
	glBufferData(GL_ARRAY_BUFFER, corners.size() * 3 * sizeof(GLfloat), rawVertices, GL_STATIC_DRAW);

	// Bind the index buffer :
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_boundingBoxIndices);
	// Upload data to OpenGL :
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLuint), &(rawIndices[0]), GL_STATIC_DRAW);
	// Unbind the buffer :
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Setup VAO pointer for BB drawing :
	this->setupVAOBoundingBox();

	// Unbind the buffer :
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind VAO :
	glBindVertexArray(0);

	// Free memory :
	delete[] rawVertices;
}

void Scene::setupVAOBoundingBox() {
	glBindVertexArray(this->vao_boundingBox);

	// Bind vertex buffer :
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_boundingBoxVertices);
	// Enable VAO pointer 0 : vertices
	glEnableVertexAttribArray(0);
	// Enable VAO pointer for 3-component vectors, non-normalized starting at index 0 :
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
}

void Scene::drawBoundingBox(const Image::bbox_t& _box, glm::vec3 color, GLfloat* vMat, GLfloat* pMat) {
	glUseProgram(this->program_BoundingBox);

	glLineWidth(2.0f);

	// Locate uniforms :
	GLint location_pMat	   = glGetUniformLocation(this->program_BoundingBox, "pMat");
	GLint location_vMat	   = glGetUniformLocation(this->program_BoundingBox, "vMat");
	GLint location_bbColor = glGetUniformLocation(this->program_BoundingBox, "bbColor");
	GLint location_bbSize  = glGetUniformLocation(this->program_BoundingBox, "bbSize");
	GLint location_bbPos   = glGetUniformLocation(this->program_BoundingBox, "bbPos");

	Image::bbox_t::vec min	= _box.getMin();
	Image::bbox_t::vec diag = _box.getDiagonal();

	// Set uniforms :
	glUniformMatrix4fv(location_pMat, 1, GL_FALSE, pMat);
	glUniformMatrix4fv(location_vMat, 1, GL_FALSE, vMat);
	glUniform3fv(location_bbColor, 1, glm::value_ptr(color));
	glUniform3fv(location_bbSize, 1, glm::value_ptr(diag));
	glUniform3fv(location_bbPos, 1, glm::value_ptr(min));

	if (this->showVAOstate) {
		std::cerr << "[LOG] Uniform locations : \n";
		PRINTVAL(location_pMat)
		PRINTVAL(location_vMat)
		PRINTVAL(location_bbColor)
		PRINTVAL(location_bbSize)
		PRINTVAL(location_bbPos)
		_box.printInfo("BB coordinates", pnt(Scene::drawBoundingBox));
	}

	glBindVertexArray(this->vao_boundingBox);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_boundingBoxIndices);
	this->setupVAOBoundingBox();

	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void*) 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

void Scene::setVisuBoxMinCoord(glm::uvec3 coor_min) {
	this->visuMin = coor_min;
	this->updateVisuBoxCoordinates();
}

void Scene::setVisuBoxMaxCoord(glm::uvec3 coor_max) {
	this->visuMax = coor_max;
	this->updateVisuBoxCoordinates();
}

void Scene::updateVisuBoxCoordinates() {
	if (this->grids.size() == 0) {
		return;
	}

	// Reset the visu box :
	using vec	  = Image::bbox_t::vec;
	this->visuBox = Image::bbox_t();

	if (this->grids.size()) {
		const Grid * g	 = this->grids[this->gridToDraw]->grid;
		auto min			 = glm::vec3(.0);
		auto max			 = glm::convert_to<float>(g->getResolution());
		Image::bbox_t imgBox = Image::bbox_t(min, max);
		this->visuBox.addPoints(imgBox.getAllCorners());
	}

	return;
}

void Scene::resetVisuBox() {
	this->visuBox = this->sceneDataBB;

	if (this->grids.size()) {
		this->visuMin	 = glm::uvec3(0, 0, 0);
		Image::svec3 max = this->grids[this->gridToDraw]->grid->getResolution();
		this->visuMax	 = glm::uvec3(max.x, max.y, max.z);
		this->updateVisuBoxCoordinates();
	}
}

//float Scene::getSceneRadius() {
//	// in case of box visu, it's easy :
//	if (this->drawMode == VolumetricBoxed) {
//		return glm::length(this->visuBox.getDiagonal());
//	}
//	// if draw solid or volumetric, focus on the part of the scene's BB we want to see
//	// need to compute plane positions, and get the god min/max points according to it
//	if (this->drawMode == Solid || this->drawMode == Volumetric) {
//		// get plane positions :
//		glm::vec3 bbmin	   = this->sceneBB.getMin();
//		glm::vec3 bbmax	   = this->sceneBB.getMax();
//		glm::vec3 planePos = (this->sceneBB.getMin() + this->planeDisplacement * this->sceneBB.getDiagonal());
//		glm::vec3 min = glm::vec3(), max = glm::vec3();
//
//		if (this->planeDirection.x < 0) {
//			min.x = bbmin.x;
//			max.x = planePos.x;
//		} else {
//			min.x = planePos.x;
//			max.x = bbmax.x;
//		}
//		if (this->planeDirection.y < 0) {
//			min.y = bbmin.y;
//			max.y = planePos.y;
//		} else {
//			min.y = planePos.y;
//			max.y = bbmax.y;
//		}
//		if (this->planeDirection.z < 0) {
//			min.z = bbmin.z;
//			max.z = planePos.z;
//		} else {
//			min.z = planePos.z;
//			max.z = bbmax.z;
//		}
//
//		glm::vec3 diag = max - min;
//		return glm::length(diag);
//	}
//	//default value when no things are loaded
//	return 1.f;
//}

//glm::vec3 Scene::getSceneCenter() {
//	// in case of box visu, it's easy :
//	if (this->drawMode == VolumetricBoxed) {
//		return this->visuBox.getMin() + (this->visuBox.getDiagonal() / 2.f);
//	}
//	// if draw solid, or volumetric focus on the part of the scene's BB we want to see
//	// need to compute plane positions, and get the god min/max points according to it
//	if (this->drawMode == Solid || this->drawMode == Volumetric) {
//		// get plane positions :
//		glm::vec3 bbmin	   = this->sceneBB.getMin();
//		glm::vec3 bbmax	   = this->sceneBB.getMax();
//		glm::vec3 planePos = (this->sceneBB.getMin() + this->planeDisplacement * this->sceneBB.getDiagonal());
//		glm::vec3 min = glm::vec3(), max = glm::vec3();
//
//		if (this->planeDirection.x < 0) {
//			min.x = bbmin.x;
//			max.x = planePos.x;
//		} else {
//			min.x = planePos.x;
//			max.x = bbmax.x;
//		}
//		if (this->planeDirection.y < 0) {
//			min.y = bbmin.y;
//			max.y = planePos.y;
//		} else {
//			min.y = planePos.y;
//			max.y = bbmax.y;
//		}
//		if (this->planeDirection.z < 0) {
//			min.z = bbmin.z;
//			max.z = planePos.z;
//		} else {
//			min.z = planePos.z;
//			max.z = bbmax.z;
//		}
//
//		glm::vec3 diag = max - min;
//		return min + diag / 2.f;
//	}
//	// default value, for no
//	return glm::vec3(.5, .5, .5);
//}

uint Scene::colorFunctionToUniform(ColorFunction _c) {
	switch (_c) {
		case ColorFunction::SingleChannel:
			return 1;
		case ColorFunction::HistologyHandE:
			return 2;
		case ColorFunction::HSV2RGB:
			return 3;
		case ColorFunction::ColorMagnitude:
			return 4;
	}
	return 0;
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
	this->shouldUpdateUBOData = true;
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
    this->shouldUpdateUBOData = true;
}

void Scene::slotSetPlaneDisplacementX(float scalar) {
	this->planeDisplacement.x = scalar;
    Q_EMIT planesMoved(this->computePlanePositions());
}
void Scene::slotSetPlaneDisplacementY(float scalar) {
	this->planeDisplacement.y = scalar;
    Q_EMIT planesMoved(this->computePlanePositions());
}
void Scene::slotSetPlaneDisplacementZ(float scalar) {
	this->planeDisplacement.z = scalar;
    Q_EMIT planesMoved(this->computePlanePositions());
}

void Scene::slotTogglePlaneDirectionX() {
	this->planeDirection.x = -this->planeDirection.x;
}
void Scene::slotTogglePlaneDirectionY() {
	this->planeDirection.y = -this->planeDirection.y;
}
void Scene::slotTogglePlaneDirectionZ() {
	this->planeDirection.z = -this->planeDirection.z;
}
void Scene::toggleAllPlaneDirections() {
	this->planeDirection = -this->planeDirection;
}

void Scene::slotTogglePlaneX(bool display) {
    if(this->planeActivation[0] != 0.) {
        this->planeActivation[0] = 0.;
    } else {
        this->planeActivation[0] = 1.;
    }
}

void Scene::slotTogglePlaneY(bool display) {
    if(this->planeActivation[1] != 0.) {
        this->planeActivation[1] = 0.;
    } else {
        this->planeActivation[1] = 1.;
    }
}

void Scene::slotTogglePlaneZ(bool display) {
    if(this->planeActivation[2] != 0.) {
        this->planeActivation[2] = 0.;
    } else {
        this->planeActivation[2] = 1.;
    }
}

void Scene::slotSetMinTexValue(double val) {
	this->textureBounds0.x = val;
	this->updateCVR();
}
void Scene::slotSetMaxTexValue(double val) {
	this->textureBounds0.y = val;
	this->updateCVR();
}
void Scene::slotSetMinTexValueAlternate(double val) {
	this->textureBounds1.x = val;
	this->updateCVR();
}
void Scene::slotSetMaxTexValueAlternate(double val) {
	this->textureBounds1.y = val;
	this->updateCVR();
}

void Scene::slotSetMinColorValue(double val) {
	this->colorBounds0.x = val;
	this->updateCVR();
}
void Scene::slotSetMaxColorValue(double val) {
	this->colorBounds0.y = val;
	this->updateCVR();
}
void Scene::slotSetMinColorValueAlternate(double val) {
	this->colorBounds1.x = val;
	this->updateCVR();
}
void Scene::slotSetMaxColorValueAlternate(double val) {
	this->colorBounds1.y = val;
	this->updateCVR();
}

Image::bbox_t Scene::getSceneBoundingBox() const {
	return this->sceneBB;
}

void Scene::setColor0(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r, g, b);
	this->color0 = glm::convert_to<float>(qtcolor);
	this->signal_updateUserColorScales();
    emit this->colorChanged();
	return;
}

void Scene::setColor1(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r, g, b);
	this->color1 = glm::convert_to<float>(qtcolor);
	this->signal_updateUserColorScales();
    emit this->colorChanged();
    return;
}

void Scene::setColor0Alternate(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r, g, b);
	this->color0_second = glm::convert_to<float>(qtcolor);
	this->signal_updateUserColorScales();
    emit this->colorChanged();
    return;
}

void Scene::setColor1Alternate(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r, g, b);
	this->color1_second = glm::convert_to<float>(qtcolor);
	this->signal_updateUserColorScales();
    emit this->colorChanged();
    return;
}

void Scene::signal_updateUserColorScales() {
	this->shouldUpdateUserColorScales = true;
}

void Scene::newSHADERS_updateUserColorScales() {
	this->shouldUpdateUserColorScales = false;
	TextureUpload colorScaleUploadParameters;
	std::size_t textureSize = this->maximumTextureSize / 2u;
	float textureSize_f		= static_cast<float>(this->maximumTextureSize / 2u);
	std::vector<glm::vec3> colorScaleData_user0(textureSize);
	std::vector<glm::vec3> colorScaleData_user1(textureSize);

	// The color scale 0 first :
	for (std::size_t i = 0; i < textureSize; ++i) {
        colorScaleData_user0[i] = glm::mix(this->color0, this->color1, static_cast<float>(i) / textureSize_f);
        colorScaleData_user1[i] = glm::mix(this->color0_second, this->color1_second, static_cast<float>(i) / textureSize_f);
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
	colorScaleUploadParameters.data			  = colorScaleData_user0.data();
	this->tex_colorScale_user0		  = this->uploadTexture1D(colorScaleUploadParameters);

	colorScaleUploadParameters.data	 = colorScaleData_user1.data();
	this->tex_colorScale_user1 = this->uploadTexture1D(colorScaleUploadParameters);
}

void Scene::updateCVR() {
	// For all grids, signal they need to be
    int i = 0;
	for (const auto& grid : this->grids) {

        glm::vec2 textureBounds = this->textureBounds0;
        glm::vec2 colorBounds = this->colorBounds0;
        if(i > 0) {
            textureBounds = this->textureBounds1;
            colorBounds = this->colorBounds1;
        }

        grid->colorChannelAttributes[0].setMinVisible(textureBounds.x);
        grid->colorChannelAttributes[0].setMaxVisible(textureBounds.y);
        grid->colorChannelAttributes[0].setMinColorScale(colorBounds.x);
        grid->colorChannelAttributes[0].setMaxColorScale(colorBounds.y);
        grid->colorChannelAttributes[1].setMinVisible(textureBounds.x);
        grid->colorChannelAttributes[1].setMaxVisible(textureBounds.y);
        grid->colorChannelAttributes[1].setMinColorScale(colorBounds.x);
        grid->colorChannelAttributes[1].setMaxColorScale(colorBounds.y);
        i++;
        // For now handle only one canal
        //grid->colorChannelAttributes[1].setMinVisible(this->textureBounds1.x);
        //grid->colorChannelAttributes[1].setMaxVisible(this->textureBounds1.y);
        //grid->colorChannelAttributes[1].setMinColorScale(this->colorBounds1.x);
        //grid->colorChannelAttributes[1].setMaxColorScale(this->colorBounds1.y);
	}

	this->shouldUpdateUBOData = true;
}

void Scene::setDrawMode(DrawMode _mode) {
	this->drawMode = _mode;
	if (this->programStatusBar != nullptr) {
		switch (_mode) {
			case DrawMode::Solid:
				this->programStatusBar->showMessage("Set draw mode to Solid.\n", 5000);
				break;
			case DrawMode::Volumetric:
				this->programStatusBar->showMessage("Set draw mode to Volumetric.\n", 5000);
				break;
			case DrawMode::VolumetricBoxed:
				this->programStatusBar->showMessage("Set draw mode to VolumetricBoxed.\n", 5000);
				break;
		}
	}
}

void Scene::togglePlaneVisibility(planes _plane) {
	if (_plane == planes::x) {
		this->planeVisibility.x = not this->planeVisibility.x;
	}
	if (_plane == planes::y) {
		this->planeVisibility.y = not this->planeVisibility.y;
	}
	if (_plane == planes::z) {
		this->planeVisibility.z = not this->planeVisibility.z;
	}
	std::cerr << "Visibilities : {" << std::boolalpha << this->planeVisibility.x << ',' << this->planeVisibility.y << ',' << this->planeVisibility.z << "}\n";
	return;
}

void Scene::toggleAllPlaneVisibilities() {
	this->planeVisibility.x = not this->planeVisibility.x;
	this->planeVisibility.y = not this->planeVisibility.y;
	this->planeVisibility.z = not this->planeVisibility.z;
}

bool compPt(std::pair<glm::vec4, std::vector<std::vector<int>>> i, std::pair<glm::vec4, std::vector<std::vector<int>>> j) {
	for (int x = 2; x >= 0; --x) {
		if (i.first[x] < j.first[x])
			return true;

		if (i.first[x] > j.first[x])
			return false;
	}

	return false;
}

bool contain(const InfoToSend& value, const InfoToSend& contain) {
    return value & contain > 0;
}

// TODO: replace this function by a real update function
void Scene::sendFirstTetmeshToGPU() {
    std::cout << "Send tetmesh " << this->gridToDraw << std::endl;
    if(this->grids.size() > 0)
        this->sendTetmeshToGPU(this->gridToDraw, InfoToSend(InfoToSend::VERTICES | InfoToSend::NORMALS), this->sortingRendering);
    if(this->glMeshManipulator->meshManipulator) {
        this->glMeshManipulator->meshManipulator->setAllManipulatorsPosition(this->getBaseMesh(this->activeMesh)->getMeshPositions());
    }
    Q_EMIT meshMoved();
}

void Scene::sendTetmeshToGPU(int gridIdx, const InfoToSend infoToSend, bool sort) {
    std::cout << "Send to GPU" << std::endl;
    if(sort)
        std::cout << "Sorting" << std::endl;

	std::size_t vertWidth = 0, vertHeight = 0;
	std::size_t normWidth = 0, normHeight = 0;
	std::size_t coorWidth = 0, coorHeight = 0;
	std::size_t neighbWidth = 0, neighbHeight = 0;

	//TetMesh& newMesh = *this->grids[gridIdx]->grid->grid->tetmesh;
	TetMesh& newMesh = *this->grids[gridIdx]->grid;

    if(contain(infoToSend, InfoToSend::NORMALS)) {
        newMesh.computeNormals();
    }
	__GetTexSize(newMesh.mesh.size() * 4 * 3, &vertWidth, &vertHeight);
	__GetTexSize(newMesh.mesh.size() * 4, &normWidth, &normHeight);
	__GetTexSize(newMesh.mesh.size() * 4 * 3, &coorWidth, &coorHeight);
	__GetTexSize(newMesh.mesh.size() * 4, &neighbWidth, &neighbHeight);

	this->grids[gridIdx]->volumetricMesh.tetrahedraCount = newMesh.mesh.size();

	GLfloat* rawVertices  = new GLfloat[vertWidth * vertHeight * 3];
	GLfloat* rawNormals	  = new GLfloat[normWidth * normHeight * 4];
	GLfloat* tex		  = new GLfloat[coorWidth * coorHeight * 3];
	GLfloat* rawNeighbors = new GLfloat[neighbWidth * neighbHeight * 3];

	int iNeigh = 0;
    int iPt = 0;
    int iNormal = 0;
    std::vector<std::pair<int, float>> orderedTets;
    if(sort) {
        newMesh.sortTet(this->cameraPosition, orderedTets);
    }

    for (int idx = 0; idx < newMesh.mesh.size(); idx++) {
        int tetIdx = idx;
        if(sort)
            tetIdx = orderedTets[idx].first;
        const Tetrahedron& tet = newMesh.mesh[tetIdx];
        for(int faceIdx = 0; faceIdx < 4; ++faceIdx) {

            if(contain(infoToSend, InfoToSend::NEIGHBORS)) {
                if(sort) {
                    std::vector<std::pair<int, float>>::iterator i = std::find_if(
                        orderedTets.begin(), orderedTets.end(),
                        [&](const std::pair<int, float>& x) { return x.first == tet.neighbors[faceIdx];});
                    rawNeighbors[iNeigh] = static_cast<GLfloat>(i - orderedTets.begin());
                } else {
                    rawNeighbors[iNeigh] = static_cast<GLfloat>(tet.neighbors[faceIdx]);
                }
                iNeigh += 3;
            }

            if(contain(infoToSend, InfoToSend::NORMALS)) {
			    for (int i = 0; i < 4; ++i) {
                    rawNormals[iNormal++] = tet.normals[faceIdx][i];
			    	//rawNormals[iNormal++] = glm::normalize((newMesh.getModelMatrix() * tet.normals[faceIdx]))[i];
			    	//rawNormals[iNormal++] = glm::vec4(1., 0., 0., 1.)[i];
			    }
            }

			for (int k = 0; k < 3; ++k) {
                int ptIndex = tet.getPointIndex(faceIdx, k);
				for (int i = 0; i < 3; ++i) {
                    if(contain(infoToSend, InfoToSend::VERTICES))
                        rawVertices[iPt] = newMesh.getVertice(ptIndex)[i];
                    if(contain(infoToSend, InfoToSend::TEXCOORD))
                        tex[iPt] = newMesh.texCoord[ptIndex][i];
					iPt++;
				}
			}
        }
	}

	// Struct to upload the texture to OpenGL :
	TextureUpload texParams = {};
    texParams.minmag.x = GL_NEAREST;
    texParams.minmag.y = GL_NEAREST;
    texParams.lod.y	   = -1000.f;
    texParams.wrap.s   = GL_CLAMP;
    texParams.wrap.t   = GL_CLAMP;

    if(contain(infoToSend, InfoToSend::VERTICES)) {
        texParams.internalFormat				   = GL_RGB32F;
        texParams.size.x						   = vertWidth;
        texParams.size.y						   = vertHeight;
        texParams.format						   = GL_RGB;
        texParams.type							   = GL_FLOAT;
        texParams.data							   = rawVertices;
        glDeleteTextures(1, &this->grids[gridIdx]->volumetricMesh.vertexPositions);
        this->grids[gridIdx]->volumetricMesh.vertexPositions = this->uploadTexture2D(texParams);
    }

    if(contain(infoToSend, InfoToSend::NORMALS)) {
        texParams.internalFormat			   = GL_RGBA32F;
        texParams.size.x					   = normWidth;
        texParams.size.y					   = normHeight;
        texParams.format					   = GL_RGBA;
        texParams.data						   = rawNormals;
        glDeleteTextures(1, &this->grids[gridIdx]->volumetricMesh.faceNormals);
        this->grids[gridIdx]->volumetricMesh.faceNormals = this->uploadTexture2D(texParams);
    }

    if(contain(infoToSend, InfoToSend::TEXCOORD)) {
        texParams.internalFormat					  = GL_RGB32F;
        texParams.size.x							  = coorWidth;
        texParams.size.y							  = coorHeight;
        texParams.format							  = GL_RGB;
        texParams.data								  = tex;
        glDeleteTextures(1, &this->grids[gridIdx]->volumetricMesh.textureCoordinates);
        this->grids[gridIdx]->volumetricMesh.textureCoordinates = this->uploadTexture2D(texParams);
    }

    if(contain(infoToSend, InfoToSend::NEIGHBORS)) {
        texParams.size.x						= neighbWidth;
        texParams.size.y						= neighbHeight;
        texParams.data							= rawNeighbors;
        glDeleteTextures(1, &this->grids[gridIdx]->volumetricMesh.neighborhood);
        this->grids[gridIdx]->volumetricMesh.neighborhood = this->uploadTexture2D(texParams);
    }

	delete[] tex;
	delete[] rawVertices;
	delete[] rawNormals;
	delete[] rawNeighbors;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Scene::tex3D_buildBuffers(VolMesh& volMesh) {
	// Tetra ///////////////////////////////////////////////////////////////////////
	//    v0----- v
	//   /       /|
	//  v ------v3|
	//  | |     | |
	//  | |v ---|-|v2
	//  |/      |/
	//  v1------v

	float v0[3] = {-1, -1, 1};
	float v1[3] = {-1, 1, -1};
	float v2[3] = {1, -1, -1};
	float v3[3] = {1, 1, 1};

	// vertex coords array
	GLfloat vertices[] = {v3[0], v3[1], v3[2], v1[0], v1[1], v1[2], v2[0], v2[1], v2[2],	// v3-v1-v2
	  v3[0], v3[1], v3[2], v2[0], v2[1], v2[2], v1[0], v1[1], v1[2],	// v3-v2-v1
	  v3[0], v3[1], v3[2], v0[0], v0[1], v0[2], v1[0], v1[1], v1[2],	// v3-v0-v1
	  v2[0], v2[1], v2[2], v1[0], v1[1], v1[2], v0[0], v0[1], v0[2]};	 // v2-v1-v0
	// normal array
	GLfloat normals[] = {v3[0], v3[1], v3[2], v1[0], v1[1], v1[2], v2[0], v2[1], v2[2],	   // v3-v1-v2
	  v3[0], v3[1], v3[2], v2[0], v2[1], v2[2], v1[0], v1[1], v1[2],	// v3-v2-v1
	  v3[0], v3[1], v3[2], v0[0], v0[1], v0[2], v1[0], v1[1], v1[2],	// v3-v0-v1
	  v2[0], v2[1], v2[2], v1[0], v1[1], v1[2], v0[0], v0[1], v0[2]};	 // v2-v1-v0
	// index array of vertex array for glDrawElements()
	// Notice the indices are listed straight from beginning to end as exactly
	// same order of vertex array without hopping, because of different normals at
	// a shared vertex. For this case, glDrawArrays() and glDrawElements() have no
	// difference.
	GLushort indices[] = {0, 1, 2,
	  3, 4, 5,
	  6, 7, 8,
	  9, 10, 11};
	// texture coords :
	GLfloat textureCoords[] = {0., 0., 1., 1., 2., 2.,
	  3., 3., 4., 4., 5., 5.,
	  6., 6., 7., 7., 8., 8.,
	  9., 9., 10., 10., 11., 11.};

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_Texture3D_VertPos);
	glBufferData(GL_ARRAY_BUFFER, 12 * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_Texture3D_VertNorm);
	glBufferData(GL_ARRAY_BUFFER, 12 * 3 * sizeof(GLfloat), normals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_Texture3D_VertTex);
	glBufferData(GL_ARRAY_BUFFER, 12 * 2 * sizeof(GLfloat), textureCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_Texture3D_VertIdx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * sizeof(GLushort), indices, GL_STATIC_DRAW);

	glBindVertexArray(this->vao_VolumetricBuffers);
	this->tex3D_bindVAO();
}

void Scene::tex3D_bindVAO() {
	glBindVertexArray(this->vao_VolumetricBuffers);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_Texture3D_VertPos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_Texture3D_VertNorm);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_Texture3D_VertTex);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
}

void Scene::setPositionResponse(glm::vec4 _pos) {
	this->posRequest = _pos;
	if (this->posFrame == nullptr) {
		std::cerr << "Allocating new frame for scene\n";
		this->posFrame = new qglviewer::Frame();
	}
	this->posFrame->setPosition(_pos.x, _pos.y, _pos.z);
}

void Scene::drawPositionResponse(float radius, bool drawOnTop) {
	if (this->posFrame == nullptr) {
		return;
	}
	if (drawOnTop) {
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	// ddraw the axes :
	glPushMatrix();
	glMultMatrixd(this->posFrame->matrix());
	QGLViewer::drawAxis(radius);
	glPopMatrix();
}

void Scene::resetPositionResponse() {
	delete this->posFrame;
	this->posFrame = nullptr;
}

void Scene::prepareManipulators() {
	this->glMeshManipulator->prepare();
}

/**********************************************************************/
/**********************************************************************/

SceneGL::SceneGL() {
	this->isInitialized = true;
	this->context		= nullptr;
}

SceneGL::~SceneGL(void) {
}

void SceneGL::initGl(QOpenGLContext* _context) {
	// Check if the scene has been initialized, share contexts if it has been :
	this->context = _context;
	if (this->initializeOpenGLFunctions() == false) {
		throw std::runtime_error("Could not initialize OpenGL functions for Scene GL.");
	}
}

GLuint SceneGL::uploadTexture1D(const TextureUpload& tex) {
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

GLuint SceneGL::uploadTexture2D(const TextureUpload& tex) {
	if (this->context != nullptr) {
		if (this->context->isValid() == false) {
			throw std::runtime_error("No associated valid context");
		}
	} else {
		throw std::runtime_error("nullptr as context");
	}

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

/*********************************/

void Scene::launchSaveDialog() {
	if (this->grids.size() == 0) {
		QMessageBox messageBox;
		messageBox.critical(nullptr, "Error", "Cannot save a grid when nothing is loaded !");
		messageBox.setFixedSize(500, 200);
		return;
	}
    //this->grids[this->gridToDraw]->grid->writeDeformedGrid();
	return;
}

/*********************************/
/* Slots */
/*********************************/
void Scene::toggleWireframe() {
	this->glMeshManipulator->toggleDisplayWireframe();
}

void Scene::toggleWireframe(bool value) {
    this->glMeshManipulator->toggleDisplayWireframe(value);
}

void Scene::toggleManipulatorActivation() {
    this->glMeshManipulator->toggleActivation();
}

void Scene::setColorChannel(ColorChannel mode) {
	this->rgbMode = mode;
	std::for_each(this->grids.begin(), this->grids.end(), [this](GridGLView::Ptr& gridView) {
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
	this->shouldUpdateUBOData = true;
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

bool Scene::toggleARAPManipulatorMode() {
    UITool::ARAPManipulator * manipulator = dynamic_cast<UITool::ARAPManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: ARAP manipulator can be used only with the ARAP deformer ! Mode not toggled" << std::endl;
        return false;
    }
    manipulator->toggleMode();
    return true;
}

bool Scene::openMesh(const std::string& name, const std::string& filename, const glm::vec4& color) {
    //this->surfaceMesh = new SurfaceMesh("/home/thomas/data/Data/Mesh/bunny_lowres.off");
    this->meshes.push_back(std::pair<SurfaceMesh*, std::string>(nullptr, name));
    this->drawableMeshes.push_back(std::pair<DrawableMesh*, std::string>(nullptr, name));
    this->meshes.back().first = new SurfaceMesh(filename);
    this->drawableMeshes.back().first = new DrawableMesh();
    this->drawableMeshes.back().first->mesh = this->meshes.back().first;
    this->drawableMeshes.back().first->initialize(this->context, this);
    this->drawableMeshes.back().first->color = color;

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
    glm::vec4 color = this->drawableMeshes[cageIdx].first->color;
    delete this->drawableMeshes[cageIdx].first;

    TetMesh * tetMesh = dynamic_cast<TetMesh*>(meshToDeform);
    if(tetMesh) {
        //this->meshes[cageIdx].first = new CageGreenLRI(this->meshes[cageIdx].first, tetMesh);
    } else {
        if(MVC)
            this->meshes[cageIdx].first = new CageMVC(this->meshes[cageIdx].first, meshToDeform);
        else 
            this->meshes[cageIdx].first = new CageGreen(this->meshes[cageIdx].first, meshToDeform);
    }

    this->drawableMeshes[cageIdx].first = new DrawableMesh();
    this->drawableMeshes[cageIdx].first->mesh = this->meshes[cageIdx].first;
    this->drawableMeshes[cageIdx].first->initialize(this->context, this);
    this->drawableMeshes[cageIdx].first->color = color;

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
    this->meshes.push_back(std::pair<SurfaceMesh*, std::string>(nullptr, name));
    this->drawableMeshes.push_back(std::pair<DrawableMesh*, std::string>(nullptr, name));

    TetMesh * tetMesh = dynamic_cast<TetMesh*>(surfaceMeshToDeform);
    if(MVC) {
        this->meshes.back().first = new CageMVC(filename, surfaceMeshToDeform);
    } else {
        if(tetMesh)
            this->meshes.back().first = new CageGreenLRI(filename, tetMesh);
        else
            this->meshes.back().first = new CageGreen(filename, surfaceMeshToDeform);
    }

    this->drawableMeshes.back().first = new DrawableMesh();
    this->drawableMeshes.back().first->mesh = this->meshes.back().first;
    this->drawableMeshes.back().first->initialize(this->context, this);
    this->drawableMeshes.back().first->color = color;

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

bool Scene::openGrid(const std::string& name, const std::vector<std::string>& imgFilenames, const int subsample, const std::string& transferMeshFileName) {
    int autofitSubsample = this->autofitSubsample(subsample, imgFilenames);
    //TODO: sizeVoxel isn't take into account with loading a custom transferMesh
    Grid * newGrid = new Grid(imgFilenames, autofitSubsample, glm::vec3(0., 0., 0.), transferMeshFileName);
    this->addGridToScene(name, newGrid);
    return true;
}

void Scene::addGridToScene(const std::string& name, Grid * newGrid) {
	GridGLView::Ptr gridView = std::make_shared<GridGLView>(newGrid);
	this->grids.push_back(gridView);

    this->gridToDraw += 1;

    this->addGrid();
    this->grids_name.push_back(name);

    this->updateSceneCenter();
    std::cout << "New grid added with BBox:" << this->grids.back()->grid->bbMax << std::endl;

    Q_EMIT meshAdded(name, true, false);
    this->changeActiveMesh(name);
}

int Scene::autofitSubsample(int initialSubsample, const std::vector<std::string>& imgFilenames) {
    float percentageOfMemory = 0.7;
    int gpuMemoryInGB = 2;
    double gpuMemoryInBytes = double(gpuMemoryInGB) * double(1073741824.);

    int finalSubsample = initialSubsample;
    bool autofitSizeRequired = false;
    bool autofitMemoryRequired = false;

    TIFFReader tiffReader(imgFilenames);
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

DrawableMesh * Scene::getDrawableMesh(const std::string& name) {
    for(int i = 0; i < this->drawableMeshes.size(); ++i) {
        if(this->drawableMeshes[i].second == name) {
            return this->drawableMeshes[i].first;
        }
    }
    std::cout << "Error: wrong drawable name: [" << name << "]" << std::endl;
    return nullptr;
}

void Scene::changeSceneRadius(float sceneRadius) {
    this->glMeshManipulator->updateManipulatorRadius(sceneRadius);
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
         this->openGrid(std::string("atlas"), {std::string("/data/datasets/data/Thomas/data/atlas/atlas.tiff")}, 1, std::string("/data/datasets/data/Thomas/data/atlas/atlas-transfert.mesh"));
         this->openCage(std::string("cage"), std::string("/data/datasets/data/Thomas/data/atlas/atlas-cage-hyperdilated.off"), std::string("atlas"), true);
         this->getCage(std::string("cage"))->setARAPDeformationMethod();
         this->getCage(std::string("cage"))->unbindMovementWithDeformedMesh();
         this->getCage(std::string("cage"))->setOrigin(this->getBaseMesh("atlas")->getOrigin());
         this->getCage(std::string("cage"))->bindMovementWithDeformedMesh();
         this->applyCage(std::string("cage"), std::string("/data/datasets/data/Thomas/data/sourisIGF/atlas-cage-hyperdilated-rigidRegister-lightsheet_2.off"));
         this->changeActiveMesh("cage");
 }

 void Scene::openIRM() {
         this->openGrid(std::string("irm"), {std::string("/home/thomas/data/Data/Demo/IRM/irm.tif")}, 1, glm::vec3(3.9, 3.9, 50));
         this->changeActiveMesh("irm");
 }

void Scene::init() {
    if(this->demos.demo_atlas_to_irm) {
        this->openGrid(std::string("atlas"), {std::string("/data/datasets/data/Thomas/data/atlas/atlas.tiff")}, 1, std::string("/home/thomas/data/Data/teletravail/atlas-transfert.mesh"));
        this->openCage(std::string("cage"), std::string("/data/datasets/data/Thomas/data/atlas/atlas-cage-hyperdilated.off"), std::string("atlas"), true);
        this->getCage(std::string("cage"))->setARAPDeformationMethod();
        this->getCage(std::string("cage"))->unbindMovementWithDeformedMesh();
        this->getCage(std::string("cage"))->setOrigin(this->getBaseMesh("atlas")->getOrigin());
        this->getCage(std::string("cage"))->bindMovementWithDeformedMesh();
        this->applyCage(std::string("cage"), std::string("/home/thomas/data/Data/teletravail/atlas-cage-hyperdilated-rigidRegister-lightsheet_2.off"));

        this->openGrid(std::string("irm"), {std::string("/home/thomas/data/Data/teletravail/irm.tif")}, 1, glm::vec3(3.9, 3.9, 50));
        //this->openGrid(std::string("irm"), {std::string("/home/thomas/data/Data/Demo/IRM/irm.tif")}, 1, glm::vec3(0., 0., 0.));
    }

    if(this->demos.demo_atlas_to_lightsheet) {
        this->openGrid(std::string("atlas"), {std::string("/data/datasets/data/Thomas/data/atlas/atlas.tiff")}, 1, std::string("/data/datasets/data/Thomas/data/atlas/atlas-transfert.mesh"));
        this->openCage(std::string("cage"), std::string("/data/datasets/data/Thomas/data/atlas/atlas-cage-hyperdilated.off"), std::string("atlas"), true);
        this->getCage(std::string("cage"))->setARAPDeformationMethod();
        this->getCage(std::string("cage"))->unbindMovementWithDeformedMesh();
        this->getCage(std::string("cage"))->setOrigin(this->getBaseMesh("atlas")->getOrigin());
        this->getCage(std::string("cage"))->bindMovementWithDeformedMesh();
        this->applyCage(std::string("cage"), std::string("/home/thomas/data/Data/Demo/lightsheet/atlas-cage-registered.off"));

        this->openGrid(std::string("lightsheet"), {std::string("/home/thomas/data/Data/Demo/lightsheet/lighsheet.tiff")}, 1, glm::vec3(1, 1, 1));
    }

    if(this->demos.demo_atlas_registration) {
        this->openMesh(std::string("cage-atlas"), std::string("/data/datasets/data/Thomas/data/sourisIGF/atlas-cage-hyperdilated-rigidRegister-lightsheet.off"));
        this->openGrid(std::string("grid-mouse"), {std::string("/data/datasets/data/Thomas/data/sourisIGF/lighsheet.tiff")}, 1, glm::vec3(1., 1., 1.), glm::vec3(5., 5., 5.));
    }
}

BaseMesh * Scene::getBaseMesh(const std::string& name) {
    SurfaceMesh * surface = nullptr;
    for(int i = 0; i < this->meshes.size(); ++i) {
        if(this->meshes[i].second == name) {
            return this->meshes[i].first;
        }
    }
    for(int i = 0; i < this->grids_name.size(); ++i) {
        if(this->grids_name[i] == name) {
            return this->grids[i]->grid;
        }
    }
    std::cout << "Wrong base mesh name: " << name << std::endl;
    return nullptr;
}

void Scene::updateTools(UITool::MeshManipulatorType tool) {
    this->glMeshManipulator->createNewMeshManipulator(this->getBaseMesh(this->activeMesh), this, tool);

    if(tool == UITool::MeshManipulatorType::NONE || !this->getBaseMesh(this->activeMesh))
        return;

    if(tool == UITool::MeshManipulatorType::DIRECT || 
       tool == UITool::MeshManipulatorType::POSITION ) {
        this->getBaseMesh(this->activeMesh)->setNormalDeformationMethod();
    } else {
        this->getBaseMesh(this->activeMesh)->setARAPDeformationMethod();
    }

    // MeshManipulator->Scene
    if(tool == UITool::MeshManipulatorType::FREE) {
        QObject::connect(this, &Scene::rayIsCasted, this, [this](const glm::vec3& origin, const glm::vec3& direction) { emit dynamic_cast<UITool::FreeManipulator*>(this->glMeshManipulator->meshManipulator)->rayIsCasted(origin, direction, this->getMinTexValue(), this->getMaxTexValue(), this->computePlanePositions());});
    }

    if(tool == UITool::MeshManipulatorType::ARAP) {
        QObject::connect(dynamic_cast<UITool::ARAPManipulator*>(this->glMeshManipulator->meshManipulator), SIGNAL(needPushHandleButton()), this, SIGNAL(needPushHandleButton()));
    }

    if(tool == UITool::MeshManipulatorType::REGISTRATION) {
        QObject::connect(this, &Scene::rayIsCasted, this, [this](const glm::vec3& origin, const glm::vec3& direction) { emit dynamic_cast<UITool::CompManipulator*>(this->glMeshManipulator->meshManipulator)->rayIsCasted(origin, direction, this->getMinTexValue(), this->getMaxTexValue(), this->computePlanePositions());});
        QObject::connect(this, SIGNAL(pointIsClickedInPlanarViewer(const glm::vec3&)), dynamic_cast<UITool::CompManipulator*>(this->glMeshManipulator->meshManipulator), SIGNAL(pointIsClickedInPlanarViewer(const glm::vec3&)));
    }

    if(tool == UITool::MeshManipulatorType::FIXED_REGISTRATION) {
        QObject::connect(this, SIGNAL(pointIsClickedInPlanarViewer(const glm::vec3&)), dynamic_cast<UITool::FixedRegistrationManipulator*>(this->glMeshManipulator->meshManipulator), SIGNAL(pointIsClickedInPlanarViewer(const glm::vec3&)));
        QObject::connect(this, &Scene::rayIsCasted, this, [this](const glm::vec3& origin, const glm::vec3& direction) { emit dynamic_cast<UITool::FixedRegistrationManipulator*>(this->glMeshManipulator->meshManipulator)->rayIsCasted(origin, direction, this->getMinTexValue(), this->getMaxTexValue(), this->computePlanePositions());});
        QObject::connect(dynamic_cast<UITool::FixedRegistrationManipulator*>(this->glMeshManipulator->meshManipulator), SIGNAL(needChangeActivatePreviewPoint(bool)), this, SLOT(setPreviewPointInPlanarView(bool)));
    }
}

void Scene::switchToSelectionModeRegistrationTool() {
    UITool::CompManipulator * manipulator = dynamic_cast<UITool::CompManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->switchToSelectionMode();
}

void Scene::validateRegistrationTool() {
    UITool::CompManipulator * manipulator = dynamic_cast<UITool::CompManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->validate();
}

void Scene::applyRegistrationTool() {
    UITool::CompManipulator * manipulator = dynamic_cast<UITool::CompManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->apply();
    this->sendFirstTetmeshToGPU();
}

void Scene::applyFixedRegistrationTool() {
    UITool::FixedRegistrationManipulator * manipulator = dynamic_cast<UITool::FixedRegistrationManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->apply();
    this->sendFirstTetmeshToGPU();
}

void Scene::clearFixedRegistrationTool() {
    UITool::FixedRegistrationManipulator * manipulator = dynamic_cast<UITool::FixedRegistrationManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->clear();
}

void Scene::undoRegistrationTool() {
    UITool::CompManipulator * manipulator = dynamic_cast<UITool::CompManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
    }
    manipulator->undo();
    // TODO: beurk, improve these interactions
    if(this->glMeshManipulator->persistantRegistrationToolSessions.size() > 0)
        this->glMeshManipulator->persistantRegistrationToolSessions.pop_back();
}

void Scene::clearRegistrationTool() {
    UITool::CompManipulator * manipulator = dynamic_cast<UITool::CompManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->clearSelectedPoints();
}

void Scene::assignMeshToRegisterRegistrationTool(const std::string& name) {
    UITool::CompManipulator * manipulator = dynamic_cast<UITool::CompManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    BaseMesh * meshToRegister = this->getBaseMesh(name);
    if(meshToRegister) {
        manipulator->assignMeshToRegister(meshToRegister);
        // TODO: beurk, improve these interactions
        manipulator->assignPreviousSelectedPoints(this->glMeshManipulator->persistantRegistrationToolSelectedPoints, this->glMeshManipulator->persistantRegistrationToolPreviousPoints, this->glMeshManipulator->persistantRegistrationToolSessions);
    }
}

void Scene::selectSlice(UITool::SliceOrientation sliceOrientation) {
    UITool::SliceManipulator* manipulator = dynamic_cast<UITool::SliceManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->selectSlice(sliceOrientation);
}

void Scene::changeSliceToSelect(UITool::SliceOrientation sliceOrientation) {
    UITool::SliceManipulator* manipulator = dynamic_cast<UITool::SliceManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->updateSliceToSelect(sliceOrientation);
}

void Scene::assignAsHandleSliceTool() {
    UITool::SliceManipulator* manipulator = dynamic_cast<UITool::SliceManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->assignAsHandle();
}

void Scene::removeAllHandlesSliceTool() {
    UITool::SliceManipulator* manipulator = dynamic_cast<UITool::SliceManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->removeAllHandles();
}

void Scene::assignAllHandlesBeforePlaneSliceTool() {
    UITool::SliceManipulator* manipulator = dynamic_cast<UITool::SliceManipulator*>(this->glMeshManipulator->meshManipulator);
    if(!manipulator) {
        std::cout << "WARNING: not the right tool" << std::endl;
        return;
    }
    manipulator->assignAllHandlesBeforePlane();
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

void Scene::saveMesh(const std::string& name, const std::string& filename) {
    this->getMesh(name)->saveOFF(filename.c_str());
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
    this->sendFirstTetmeshToGPU();
}

void Scene::redrawSelection(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec4& color) {
    this->glSelection->setSelectionBB(p0, p1, p2, p3);
    this->glSelection->setColor(color);
    this->glSelection->prepare();
}

void Scene::setLightPosition(const glm::vec3& lighPosition) {
    for(int i = 0; i < this->drawableMeshes.size(); ++i) {
        this->drawableMeshes[i].first->lightPosition = lighPosition;
    }
}

void Scene::changeActiveMesh(const std::string& name) {
    this->activeMesh = name;
    this->updateSceneCenter();
    this->updateSceneRadius();
    if(this->isGrid(activeMesh)) {
        int gridIdx = this->getGridIdx(activeMesh);
        this->gridToDraw = gridIdx;
        this->sendTetmeshToGPU(gridIdx, InfoToSend(InfoToSend::VERTICES | InfoToSend::NORMALS | InfoToSend::TEXCOORD | InfoToSend::NEIGHBORS)); 
    } else if (this->multiGridRendering && this->isCage(activeMesh)) {
        int gridIdx = this->getGridIdxLinkToCage(activeMesh);
        if(gridIdx != -1) {
            this->gridToDraw = gridIdx;
            this->sendTetmeshToGPU(gridIdx, InfoToSend(InfoToSend::VERTICES | InfoToSend::NORMALS | InfoToSend::TEXCOORD | InfoToSend::NEIGHBORS));
        }
    }
    this->changeCurrentTool(this->currentTool);
    this->changeCurrentDeformationMethod(this->currentDeformMethod);
    this->shouldUpdateUserColorScales = true;
}

void Scene::changeCurrentTool(UITool::MeshManipulatorType newTool) {
    this->currentTool = newTool;
    this->updateTools(newTool);
    if(newTool == UITool::MeshManipulatorType::ARAP ||
       newTool == UITool::MeshManipulatorType::FIXED_REGISTRATION) {
        this->changeCurrentDeformationMethod(DeformMethod::ARAP);
    }
    if(newTool == UITool::MeshManipulatorType::DIRECT) {
        this->changeCurrentDeformationMethod(DeformMethod::NORMAL);
    }
}

void Scene::changeCurrentDeformationMethod(DeformMethod newDeformMethod) {
    this->currentDeformMethod = newDeformMethod;
    BaseMesh * mesh = this->getBaseMesh(this->activeMesh);
    if(mesh) {
        switch(newDeformMethod) {
            case DeformMethod::NORMAL:
                mesh->setNormalDeformationMethod();
                break;
            case DeformMethod::ARAP:
                mesh->setARAPDeformationMethod();
                break;
        }
    }
}

void Scene::previewPointInPlanarView(const glm::vec3& positionOfMouse3D) {
    if(this->previewCursorInPlanarView) {
        this->glMeshManipulator->needPreview = true;
        this->glMeshManipulator->previewPosition = positionOfMouse3D;
    } else {
        this->glMeshManipulator->needPreview = false;
    }
}

void Scene::changeSelectedPoint(std::pair<int, glm::vec3> selectedPoint) {
    Q_EMIT selectedPointChanged(selectedPoint);
}

bool Scene::isRightTool(const UITool::MeshManipulatorType& typeToCheck) {
    bool isRightType = (this->glMeshManipulator->meshManipulatorType == typeToCheck);
    if(!isRightType)
        std::cout << "WARNING: not the right tool" << std::endl;
    return isRightType;
}

void Scene::moveTool_toggleEvenMode() { auto * toolPtr = this->getMeshTool<UITool::PositionManipulator>(); if(toolPtr) { toolPtr->toggleEvenMode(); } };

void Scene::ARAPTool_toggleEvenMode() { auto * toolPtr = this->getMeshTool<UITool::ARAPManipulator>(); if(toolPtr) { toolPtr->toggleEvenMode(); } };

void Scene::moveInHistory(bool backward, bool reset) {
    BaseMesh * mesh = this->getBaseMesh(this->activeMesh);
    if(mesh && mesh->history) {
        std::vector<glm::vec3> pointsBefore;
        bool success = false;
        if(reset) {
            success = mesh->history->reset(pointsBefore);
        } else {
            if(backward) {
                success = mesh->history->undo(pointsBefore);
            } else {
                success = mesh->history->redo(pointsBefore);
            }
        }
        if(success) {
            mesh->replacePoints(pointsBefore);
            this->sendFirstTetmeshToGPU();
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

glm::vec3 Scene::getTransformedPoint(const glm::vec3& inputPoint, const std::string& from, const std::string& to) {
    glm::vec3 result = glm::vec3(0., 0., 0.);

    Grid * fromGrid = this->grids[this->getGridIdx(from)]->grid;
    Grid * toGrid = this->grids[this->getGridIdx(to)]->grid;

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

void Scene::getDeformation(const std::string& gridNameValues, const std::string& gridNameSample, std::vector<std::vector<uint16_t>>& data) {
    Grid * grid = this->grids[this->getGridIdx(gridNameSample)]->grid;

    const glm::vec3 bbMin = grid->bbMin;
    const glm::vec3 bbMax = grid->bbMax;
    const glm::vec3 imgSize = grid->getResolution();

    this->grids[this->getGridIdx(gridNameValues)]->grid->sampleGridValues(std::make_pair(grid->bbMin, grid->bbMax), imgSize, data);
}

void Scene::getValues(const std::string& gridName, const std::pair<glm::vec3, glm::vec3>& area, const glm::vec3& resolution, std::vector<std::vector<uint16_t>>& data, Interpolation::Method interpolationMethod) {
    this->grids[this->getGridIdx(gridName)]->grid->sampleGridValues(area, resolution, data, interpolationMethod);
}

void Scene::getValues(const std::string& gridName, const glm::vec3& slice, const std::pair<glm::vec3, glm::vec3>& area, const glm::vec3& resolution, std::vector<uint16_t>& data, Interpolation::Method interpolationMethod) {
    this->grids[this->getGridIdx(gridName)]->grid->sampleSliceGridValues(slice, area, resolution, data, interpolationMethod);
}

void Scene::writeDeformation(const std::string& filename, const std::string& gridNameValues, const std::string& gridNameSample) {
    std::vector<std::vector<uint16_t>> data;
    this->getDeformation(gridNameValues, gridNameSample, data);
    this->writeGreyscaleTIFFImage(filename, this->getGridImgSize(gridNameSample), data);
}

void Scene::writeGreyscaleTIFFImage(const std::string& filename, const glm::vec3& imgDimensions, const std::vector<std::vector<uint16_t>>& data) {
    TinyTIFFWriterFile * tif = TinyTIFFWriter_open(filename.c_str(), 16, TinyTIFFWriter_UInt, 1, imgDimensions[0], imgDimensions[1], TinyTIFFWriter_Greyscale);
    for(int img = 0; img < data.size(); ++img) {
        TinyTIFFWriter_writeImage(tif, data[img].data());
    }
    TinyTIFFWriter_close(tif);
    std::cout << "Save sucessfull" << std::endl;
}

void Scene::writeImageWithPoints(const std::string& filename, const std::string& image, std::vector<glm::vec3>& points) {
    Grid * grid = this->grids[this->getGridIdx(image)]->grid;

    glm::ivec3 imgDimensions = grid->sampler.getImageDimensions();

    TinyTIFFWriterFile * tif = TinyTIFFWriter_open(filename.c_str(), 16, TinyTIFFWriter_UInt, 1, imgDimensions[0], imgDimensions[1], TinyTIFFWriter_Greyscale);
    std::vector<uint16_t> data;
    data.resize(imgDimensions[0] * imgDimensions[1]);

    auto start = std::chrono::steady_clock::now();
    for(int k = 0; k < imgDimensions[2]; ++k) {
        std::cout << "Loading: " << (float(k)/float(imgDimensions[2])) * 100. << "%" << std::endl;
        #pragma omp parallel for
        for(int j = 0; j < imgDimensions[1]; ++j) {
            for(int i = 0; i < imgDimensions[0]; ++i) {
                glm::vec3 p(i, j, k);
                int insertIdx = i + j*imgDimensions[0];
                bool toHighlight = false;
                for(const glm::vec3& pt : points) {
                    if(glm::distance(pt, p) < 5.) {
                        toHighlight = true;
                    }
                }
                if(toHighlight) {
                    data[insertIdx] = 0;
                } else {
                    data[insertIdx] = grid->getValueFromPoint(p);
                }
            }
        }
        TinyTIFFWriter_writeImage(tif, data.data());
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        std::cout << "Duration time: " << elapsed_seconds.count() << "s / " << elapsed_seconds.count()/60. << "m" << std::endl;
    }
    TinyTIFFWriter_close(tif);

    std::cout << "Save sucessfull" << std::endl;
}

void Scene::clear() {
    this->updateTools(UITool::MeshManipulatorType::NONE);
    this->grids_name.clear();
	this->grids.clear();
    this->meshes.clear();
    this->drawableMeshes.clear();
    gridToDraw = -1;
}

glm::vec3 Scene::getGridImgSize(const std::string& name) {
    return this->grids[this->getGridIdx(name)]->grid->getResolution();
}

glm::vec3 Scene::getGridVoxelSize(const std::string& name) {
    return this->grids[this->getGridIdx(name)]->grid->getVoxelSize();
}

template<typename MeshToolType>
MeshToolType* Scene::getMeshTool() { return dynamic_cast<MeshToolType*>(this->glMeshManipulator->meshManipulator); };

void Scene::setGridsToDraw(std::vector<int> indices) {
    this->gridsToDraw = indices;
}

void Scene::setSortingRendering(bool value) {
    this->sortingRendering = value;
}

void Scene::setMultiGridRendering(bool value) {
    this->multiGridRendering = value;
    if(this->isCage(activeMesh)) {
        int gridIdx = this->getGridIdxLinkToCage(activeMesh);
        if(gridIdx != -1) {
            this->gridToDraw = gridIdx;
            this->sendTetmeshToGPU(gridIdx, InfoToSend(InfoToSend::VERTICES | InfoToSend::NORMALS | InfoToSend::TEXCOORD | InfoToSend::NEIGHBORS));
        }
    }
};
