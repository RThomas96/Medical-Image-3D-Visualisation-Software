#include "../include/scene.hpp"
#include "../include/planar_viewer.hpp"
#include "../../qt/include/scene_control.hpp"
#include "../../image/include/reader.hpp"
#include "../../image/include/writer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QOpenGLContext>
#include <QFileDialog>
#include <QMessageBox>
#include <QSurface>

#include <fstream>
#include <iomanip>
#include <type_traits>

inline unsigned int planeHeadingToIndex(planeHeading _heading) {
	switch (_heading) {
		case planeHeading::North : return 1;
		case planeHeading::East  : return 2;
		case planeHeading::South : return 3;
		case planeHeading::West  : return 4;
		default : return 1; // If no value is recognized, return a north heading (default value)
	}
}

inline void __GetTexSize(std::size_t numTexNeeded, std::size_t* opt_width, std::size_t* opt_height) {
	std::size_t iRoot = ( std::size_t )sqrtf(static_cast<float>(numTexNeeded));

	*opt_width = iRoot + 1;
	*opt_height = iRoot;
	if ( ((*opt_width) * (*opt_height)) < numTexNeeded ) { (*opt_height)++; }
	if ( ((*opt_width) * (*opt_height)) < numTexNeeded ) { (*opt_width)++; }
}

Scene::Scene() {
	/** This constructor not only creates the object, but also sets the default values for the Scene in order
	 *  to be drawn, even if it is empty at the time of the first call to a draw function.
	 */
	this->isInitialized = false;
	this->showVAOstate = false;
	this->shouldDeleteGrid = false;

	this->grids.clear();

	this->context = nullptr;
	this->debugLog = nullptr;
	this->glOutput = nullptr;
	this->controlPanel = nullptr;
	this->outputGrid = nullptr;
	this->gridControl = nullptr;
	this->visuBoxController = nullptr;
	this->programStatusBar = nullptr;

	DiscreteGrid::data_t minTexVal = DiscreteGrid::data_t(1);
	DiscreteGrid::data_t maxTexVal = DiscreteGrid::data_t(std::numeric_limits<DiscreteGrid::data_t>::max());
	this->textureBounds0 = GridGLView::data_2(minTexVal, maxTexVal);
	this->textureBounds1 = GridGLView::data_2(minTexVal, maxTexVal);
	this->colorBounds0 = GridGLView::data_2(0, maxTexVal);
	this->colorBounds1 = GridGLView::data_2(0, maxTexVal);
	this->selectedChannel_r = 0; // by default, the R channel
	this->selectedChannel_g = 0; // by default, the R channel

	this->renderSize = 0;

	// Default light positions : at the vertices of a unit cube.
	this->lightPositions = {
		glm::vec3(.0, .0, .0), glm::vec3(1., .0, .0), glm::vec3(.0, 1., .0), glm::vec3(1., 1., .0),
		glm::vec3(.0, .0, 1.), glm::vec3(1., .0, 1.), glm::vec3(.0, 1., 1.), glm::vec3(1., 1., 1.)
	};

	this->planeDirection = glm::vec3(1., 1., 1.);
	this->planeDisplacement = glm::vec3(.0, .0, .0);
	DiscreteGrid::bbox_t::vec min(.0, .0, .0);
	DiscreteGrid::bbox_t::vec max(1., 1., 1.);
	this->sceneBB = DiscreteGrid::bbox_t(min, max);
	this->clipDistanceFromCamera = 5.f;
	this->drawMode = DrawMode::Solid;
	this->rgbMode = RGBMode::RedAndGreen;
	this->channels_r = ColorFunction::SingleChannel;
	this->channels_g = ColorFunction::SingleChannel;

	// Show all planes as default :
	this->planeVisibility = glm::vec<3, bool, glm::defaultp>(true, true, true);

	this->vaoHandle = 0;
	this->vaoHandle_VolumetricBuffers = 0;
	this->vaoHandle_boundingBox = 0;

	this->vboHandle_VertPos = 0;
	this->vboHandle_VertNorm = 0;
	this->vboHandle_VertTex = 0;
	this->vboHandle_Element = 0;
	this->vboHandle_PlaneElement = 0;
	this->vboHandle_SinglePlaneElement = 0;
	this->vboHandle_boundingBoxVertices = 0;
	this->vboHandle_boundingBoxIndices = 0;

	this->programHandle_projectedTex = 0;
	this->programHandle_Plane3D = 0;
	this->programHandle_PlaneViewer = 0;
	this->programHandle_VolumetricViewer = 0;
	this->programHandle_BoundingBox = 0;

	this->texHandle_ColorScaleGrid = 0;
	this->texHandle_ColorScaleGridAlternate = 0;
	this->shouldUpdateVis = false;
	this->volumetricMesh = {};
	this->vboHandle_Texture3D_VertPos = 0;
	this->vboHandle_Texture3D_VertNorm = 0;
	this->vboHandle_Texture3D_VertTex = 0;
	this->vboHandle_Texture3D_VertIdx = 0;

	this->color0 = glm::vec3(1., .0, .0);
	this->color1 = glm::vec3(.0, .0, 1.);
	this->color0_second = glm::vec3(1., .0, .0);
	this->color1_second = glm::vec3(.0, .0, 1.);

	std::cerr << "Allocating " << +std::numeric_limits<DiscreteGrid::data_t>::max() << " elements for vis ...\n";
	this->visibleDomains = new float[std::numeric_limits<DiscreteGrid::data_t>::max()];
	this->visibleDomainsAlternate = new float[std::numeric_limits<DiscreteGrid::data_t>::max()];
	for (std::size_t i = 0; i < std::numeric_limits<DiscreteGrid::data_t>::max(); ++i) {
		this->visibleDomains[i] = .5f;
		this->visibleDomainsAlternate[i] = .5f;
	}
}

Scene::~Scene(void) {
	delete[] this->visibleDomains;
	delete[] this->visibleDomainsAlternate;
}

void Scene::initGl(QOpenGLContext* _context) {
	// Check if the scene has been initialized, share contexts if it has been :
	if (this->isInitialized == true) {
		if (this->context != nullptr && _context != 0 && _context != nullptr) {
			_context->setShareContext(this->context);
			if (_context->create() == false) {
				// throw std::runtime_error("Couldn't re-create context with shared context added\n");
				std::cerr << "Couldn't re-create context with shared context added\n";
			}
		}
		return;
	}
	// If the scene had not yet been initialized, it is now :
	this->isInitialized = true;

	// Set the context for later viewers that want to connect to the scene :
	if (_context == 0) { throw std::runtime_error("Warning : this->context() returned 0 or nullptr !") ; }
	if (_context == nullptr) { std::cerr << "Warning : Initializing a scene without a valid OpenGL context !" << '\n' ; }
	this->context = _context;

	auto maj = this->context->format().majorVersion();
	auto min = this->context->format().minorVersion();
	std::cerr << "OpenGL version " << maj << '.' << min << '\n';

	// Get OpenGL functions from the currently bound context :
	this->initializeOpenGLFunctions();

	std::cerr << "OpenGL vendor string : " << glGetString(GL_VENDOR) << '\n';
	std::cerr << "OpenGL renderer string : " << glGetString(GL_RENDERER) << '\n';

	// Create the debug logger and connect its signals :
	this->setupGLOutput();

	// The default parameters have already been set in the constructor. We
	// need to initialize the OpenGL objects now. Shaders, VAOs, VBOs.

	// Compile the shaders :
	this->recompileShaders(false);

	// Create VAO, VBO elements and populate them in generateSceneData() :
	this->createBuffers();
	this->generateSceneData();

	// Generate visibility array :
	this->texHandle_ColorScaleGrid = 0;
	this->generateColorScale();
	this->uploadColorScale();
}

void Scene::addOpenGLOutput(OpenGLDebugLog* glLog) {
	// if an output is already enabled, stop there
	if (this->glOutput != nullptr) { return; }
	if (glLog == nullptr) { return; }

	// add the glOutput to this class :
	this->glOutput = glLog;

	// if there __is__ a logger in the class (init didn't fail) :
	if (this->debugLog != nullptr) {
		// disconnect the messagelogged signal and re-connect it with glOutput :
		this->debugLog->disconnect(SIGNAL(&QOpenGLDebugLogger::messageLogged));
		QObject::connect(this->debugLog, &QOpenGLDebugLogger::messageLogged, this->glOutput, &OpenGLDebugLog::addOpenGLMessage);
	}
}

void Scene::addStatusBar(QStatusBar *_s) {
	if (this->programStatusBar != nullptr) { return; }

	this->programStatusBar = _s;
}

void Scene::setupGLOutput() {
	// if no context is available, end the func now.
	if (this->context == nullptr) { return; }

	// If the context supports the GL_KHR_debug extension, enable a logger :
	if (this->context->hasExtension(QByteArrayLiteral("GL_KHR_debug"))) {
		this->debugLog = new QOpenGLDebugLogger;
		if (this->debugLog->initialize()) {
			if (this->glOutput != nullptr) {
				// Connect debug logger to gl output class :
				QObject::connect(this->debugLog, &QOpenGLDebugLogger::messageLogged,
						this->glOutput, &OpenGLDebugLog::addOpenGLMessage);
			} else {
				// connect directly to std::cerr if no gl outputs are available
				QObject::connect(this->debugLog, &QOpenGLDebugLogger::messageLogged, [this](QOpenGLDebugMessage _m) {
					this->printOpenGLMessage(_m);
				});
			}
			this->debugLog->startLogging(QOpenGLDebugLogger::LoggingMode::SynchronousLogging);
		} else {
			// Silently ignore the init error, and free the debugLog resource.
			delete this->debugLog;
			this->debugLog = nullptr;
			if (this->glOutput != nullptr) {
				this->glOutput->addErrorMessage("Cannot initialize QOpenGLDebugLogger. No messages will be logged.");
			} else {
				QMessageBox* msgBox = new QMessageBox;
				msgBox->setAttribute(Qt::WA_DeleteOnClose);
				msgBox->critical(nullptr, "QOpenGLDebugLogger Error", "Could not initialize logging for the QOpenGLDebugLogger. No messages will be logged.");
			}
		}
	} else {
		if (this->glOutput != nullptr) {
			this->glOutput->addErrorMessage("Context does not support the <pre>GL_KHR_debug</pre> extension. No mesages <i>can</i> be logged.");
		} else {
			QMessageBox* msgBox = new QMessageBox;
			msgBox->setAttribute(Qt::WA_DeleteOnClose);
			msgBox->critical(nullptr, "QOpenGLDebugLogger Error", "Context does not support the <pre>GL_KHR_debug</pre> extension. No mesages <i>can</i> be logged.");
		}
	}
}

void Scene::printOpenGLMessage(const QOpenGLDebugMessage& message) {
	std::string src = "";
	std::string sev = "";
	std::string typ = "";
	QFlags severity = message.severity();
	QFlags type = message.type();
	QFlags source = message.source();

	//if (severity & QOpenGLDebugMessage::Severity::NotificationSeverity)	{ sev += "{Notif}"; }
	if (severity & QOpenGLDebugMessage::Severity::LowSeverity)		{ sev += "{Low}"; }
	if (severity & QOpenGLDebugMessage::Severity::MediumSeverity)		{ sev += "{Med}"; }
	if (severity & QOpenGLDebugMessage::Severity::HighSeverity)		{ sev += "{High}"; }

	if (type & QOpenGLDebugMessage::Type::ErrorType)			{ typ += "[ERROR]"; }
	if (type & QOpenGLDebugMessage::Type::DeprecatedBehaviorType)		{ typ += "[DEPR. BEHAVIOUR]"; }
	if (type & QOpenGLDebugMessage::Type::UndefinedBehaviorType)		{ typ += "[UNDEF. BEHAVIOUR]"; }
	if (type & QOpenGLDebugMessage::Type::PortabilityType)			{ typ += "[PORTABILITY]"; }
	if (type & QOpenGLDebugMessage::Type::PerformanceType)			{ typ += "[PERF]"; }
	if (type & QOpenGLDebugMessage::Type::OtherType)			{ typ += "[OTHER]"; }
	if (type & QOpenGLDebugMessage::Type::MarkerType)			{ typ += "[MARKER]"; }
	if (type & QOpenGLDebugMessage::Type::GroupPushType)			{ typ += "[GROUP PUSH]"; }
	if (type & QOpenGLDebugMessage::Type::GroupPopType)			{ typ += "[GROUP POP]"; }

	if (source & QOpenGLDebugMessage::Source::APISource)			{ src += "[OpenGL]"; }
	if (source & QOpenGLDebugMessage::Source::WindowSystemSource)		{ src += "[WinSys]"; }
	if (source & QOpenGLDebugMessage::Source::ShaderCompilerSource)		{ src += "[ShaComp]"; }
	if (source & QOpenGLDebugMessage::Source::ThirdPartySource)		{ src += "[3rdParty]"; }
	if (source & QOpenGLDebugMessage::Source::OtherSource)			{ src += "[Other]"; }

	// Currently outputs any message on the GL stack, regardless of severity, type, or source :
	std::cerr << sev << ' ' << typ << ' ' << src << ' ' << +message.id() << " : " << message.message().toStdString() << '\n';
}

void Scene::printGridInfo(const std::shared_ptr<DiscreteGrid>& grid) {
	std::cerr << "[INFO] Information about the grid " << grid->getGridName() << " :\n";
	DiscreteGrid::sizevec3 res = grid->getResolution();
	const glm::vec3& dims = grid->getVoxelDimensions();
	const DiscreteGrid::bbox_t& dataBB = grid->getDataBoundingBox();
	const DiscreteGrid::bbox_t::vec& dataBBm = dataBB.getMin();
	const DiscreteGrid::bbox_t::vec& dataBBM = dataBB.getMax();
	std::cerr << "[INFO]\tResolution : [" << res.x << ", " << res.y << ", " << res.z << "]\n";
	std::cerr << "[INFO]\tVoxel dimensions : [" << dims.x << ", " << dims.y << ", " << dims.z << "]\n";
	std::cerr << "[INFO]\tData Bounding box : [" << dataBBm.x << ", " << dataBBm.y << ", " << dataBBm.z << "] to ["
		  << dataBBM.x << ", " << dataBBM.y << ", " << dataBBM.z << "]\n";
	const DiscreteGrid::bbox_t& bbGS = grid->getBoundingBox();
	const DiscreteGrid::bbox_t::vec& bbGSm = bbGS.getMin();
	const DiscreteGrid::bbox_t::vec& bbGSM = bbGS.getMax();
	std::cerr << "[INFO]\tBounding box GS : [" << bbGSm.x << ", " << bbGSm.y << ", " << bbGSm.z << "] to ["
		  << bbGSM.x << ", " << bbGSM.y << ", " << bbGSM.z << "]\n";
	const DiscreteGrid::bbox_t& bbWS = grid->getBoundingBoxWorldSpace();
	const DiscreteGrid::bbox_t::vec& bbWSm = bbWS.getMin();
	const DiscreteGrid::bbox_t::vec& bbWSM = bbWS.getMax();
	std::cerr << "[INFO]\tBounding box WS : [" << bbWSm.x << ", " << bbWSm.y << ", " << bbWSm.z << "] to ["
		  << bbWSM.x << ", " << bbWSM.y << ", " << bbWSM.z << "]\n";
}

void Scene::createBuffers() {
	/// @b Create a vertex array, bind it and see if it has been succesfully created server-side.
	auto createVAO = [&, this](std::string name) -> GLuint {
		GLuint buf = 0;
		this->glGenVertexArrays(1, &buf);
		this->glBindVertexArray(buf);
		if (this->glIsVertexArray(buf) == GL_FALSE) {
			std::cerr << "[ERROR]["<< __FILE__ << ":" << __LINE__ <<"] : Could not create VAO object " << name << '\n';
		}
		return buf;
	};

	/// @b Create a buffer, bind it and see if it has been succesfully created server-side.
	auto createVBO = [&, this](GLenum bufType, std::string name) -> GLuint {
		GLuint buf = 0;
		this->glGenBuffers(1, &buf);
		this->glBindBuffer(bufType, buf);
		if (this->glIsBuffer(buf) == GL_FALSE) {
			std::cerr << "[ERROR]["<< __FILE__ << ":" << __LINE__ <<"] : Could not create buffer object " << name << '\n';
		}
		return buf;
	};

	// For the default VAO :
	this->vaoHandle = createVAO("vaoHandle");
	this->vboHandle_VertPos = createVBO(GL_ARRAY_BUFFER, "vboHandle_VertPos");
	this->vboHandle_VertNorm= createVBO(GL_ARRAY_BUFFER, "vboHandle_VertNorm");
	this->vboHandle_VertTex = createVBO(GL_ARRAY_BUFFER, "vboHandle_VertTex");
	this->vboHandle_Element = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_Element");
	this->vboHandle_PlaneElement = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_PlaneElement");
	this->vboHandle_SinglePlaneElement = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_SinglePlaneElement");

	// For the texture3D visualization method :
	this->vaoHandle_VolumetricBuffers = createVAO("vaoHandle_VolumetricBuffers");
	this->vboHandle_Texture3D_VertPos = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertPos");
	this->vboHandle_Texture3D_VertNorm = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertNorm");
	this->vboHandle_Texture3D_VertTex = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertTex");
	this->vboHandle_Texture3D_VertIdx = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_Texture3D_VertIdx");

	// For the bounding boxes we have to create/show :
	this->vaoHandle_boundingBox = createVAO("vaoHandle_boundingBox");
	this->vboHandle_boundingBoxVertices = createVBO(GL_ARRAY_BUFFER, "vboHandle_boundingBoxVertices");
	this->vboHandle_boundingBoxIndices = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_boundingBoxIndices");

	return;
}

void Scene::addGrid(const std::shared_ptr<InputGrid> _grid, std::string meshPath) {
	if (this->grids.size() > 0) {
		this->shouldDeleteGrid = true;
		for (std::size_t i = 0; i < this->grids.size(); ++i) { this->delGrid.push_back(i); }
		this->deleteGridNow();
	}
	GridGLView::Ptr gridView = std::make_shared<GridGLView>(std::initializer_list<std::shared_ptr<DiscreteGrid>>({_grid}));

	TextureUpload gridTexture{};
	gridTexture.minmag.x = GL_NEAREST;
	gridTexture.minmag.y = GL_NEAREST;
	gridTexture.lod.y = -1000.f;
	gridTexture.wrap.x = GL_CLAMP_TO_EDGE;
	gridTexture.wrap.y = GL_CLAMP_TO_EDGE;
	gridTexture.wrap.z = GL_CLAMP_TO_EDGE;
	gridTexture.swizzle.r = GL_RED;
	gridTexture.swizzle.g = GL_ZERO;
	gridTexture.swizzle.b = GL_ZERO;
	gridTexture.swizzle.a = GL_ONE;
	gridTexture.alignment.x = 1;
	gridTexture.alignment.y = 1;

	// Tex upload function :
	auto dimensions = _grid->getResolution();
	if (dimensions.x > 0 && dimensions.y > 0 && dimensions.z > 0) {
		gridTexture.level = 0;
		#ifdef VISUALISATION_USE_UINT8
		gridTexture.internalFormat = GL_R8UI;
		#endif
		#ifdef VISUALISATION_USE_UINT16
		gridTexture.internalFormat = GL_R16UI;
		#endif
		gridTexture.size.x = dimensions.x;
		gridTexture.size.y = dimensions.y;
		gridTexture.size.z = dimensions.z;
		gridTexture.format = GL_RED_INTEGER;
		#ifdef VISUALISATION_USE_UINT8
		gridTexture.type = GL_UNSIGNED_BYTE;
		#endif
		#ifdef VISUALISATION_USE_UINT16
		gridTexture.type = GL_UNSIGNED_SHORT;
		#endif
		gridTexture.data = _grid->getDataPtr();
		gridView->gridTexture = this->uploadTexture3D(gridTexture);
	}

	gridView->boundingBoxColor = glm::vec3(.4, .6, .3); // olive-colored by default

	this->tex3D_buildMesh(gridView, meshPath);
	this->tex3D_buildVisTexture(gridView->volumetricMesh);
	this->tex3D_buildBuffers(gridView->volumetricMesh);

	this->grids.push_back(gridView);

	this->updateVis();

	_grid->setOffline(true);

	this->updateBoundingBox();
	this->resetVisuBox();
}

void Scene::addTwoGrids(const std::shared_ptr<InputGrid> _gridR, const std::shared_ptr<InputGrid> _gridB, std::string meshPath) {
	if (this->grids.size() > 0) {
		this->shouldDeleteGrid = true;
		for (std::size_t i = 0; i < this->grids.size(); ++i) { this->delGrid.push_back(i); }
		this->deleteGridNow();
	}
	GridGLView::Ptr gridView = std::make_shared<GridGLView>(std::initializer_list<std::shared_ptr<DiscreteGrid>>({_gridR, _gridB}));

	TextureUpload _gridTex{};
	_gridTex.minmag.x = GL_NEAREST;
	_gridTex.minmag.y = GL_NEAREST;
	_gridTex.lod.y = -1000.f;
	_gridTex.wrap.x = GL_CLAMP_TO_EDGE;
	_gridTex.wrap.y = GL_CLAMP_TO_EDGE;
	_gridTex.wrap.z = GL_CLAMP_TO_EDGE;
	_gridTex.swizzle.r = GL_RED;
	_gridTex.swizzle.g = GL_GREEN;
	_gridTex.swizzle.b = GL_ZERO;
	_gridTex.swizzle.a = GL_ONE;
	_gridTex.alignment.x = 1;
	_gridTex.alignment.y = 2;

	// Tex upload function :
	auto dimensions = _gridR->getResolution();
	if (dimensions.x > 0 && dimensions.y > 0 && dimensions.z > 0) {
		_gridTex.level = 0;
		#ifdef VISUALISATION_USE_UINT8
		_gridTex.internalFormat = GL_RG8UI;
		#endif
		#ifdef VISUALISATION_USE_UINT16
		_gridTex.internalFormat = GL_RG16UI;
		#endif
		_gridTex.size.x = dimensions.x;
		_gridTex.size.y = dimensions.y;
		_gridTex.size.z = dimensions.z;
		_gridTex.format = GL_RG_INTEGER;
		#ifdef VISUALISATION_USE_UINT8
		_gridTex.type = GL_UNSIGNED_BYTE;
		#endif
		#ifdef VISUALISATION_USE_UINT16
		_gridTex.type = GL_UNSIGNED_SHORT;
		#endif

		_gridTex.printInfo();

		gridView->gridTexture = this->uploadTexture3D_iterative(_gridTex, _gridR, _gridB);
	}

	gridView->boundingBoxColor = glm::vec3(.4, .6, .3); // olive-colored by default
	gridView->nbChannels = 2; // loaded 2 channels in the image

	this->tex3D_buildMesh(gridView, meshPath);
	this->tex3D_buildVisTexture(gridView->volumetricMesh);
	this->tex3D_buildBuffers(gridView->volumetricMesh);

	this->grids.push_back(gridView);

	this->updateVis();

	_gridR->setOffline(true);
	_gridB->setOffline(true);

	this->updateBoundingBox();
	this->resetVisuBox();
}

void Scene::updateBoundingBox(void) {
	this->sceneBB = DiscreteGrid::bbox_t();
	this->sceneDataBB = DiscreteGrid::bbox_t();
	for (std::size_t i = 0; i < this->grids.size(); ++i) {
		std::for_each(this->grids[i]->grid.cbegin(), this->grids[i]->grid.cend(),
		[this](const std::shared_ptr<DiscreteGrid>& _g) {
			DiscreteGrid::bbox_t box = _g->getBoundingBoxWorldSpace();
			DiscreteGrid::bbox_t dbox =_g->getDataBoundingBoxWorldSpace();
			this->sceneBB.addPoints(box.getAllCorners());
			this->sceneDataBB.addPoints(dbox.getAllCorners());
		});
	}

	// Update light positions :
	auto corners = this->sceneBB.getAllCorners();
	for (std::size_t i = 0; i < corners.size(); ++i) {
		this->lightPositions[i] = glm::convert_to<float>(corners[i]);
	}

	this->visuBox = this->sceneDataBB;
	if (this->visuBoxController != nullptr) {
		this->visuBoxController->updateValues();
	}

	return;
}

void Scene::recompileShaders(bool verbose) {
	GLuint newProgram = this->compileShaders("../shaders/voxelgrid.vert", "../shaders/voxelgrid.geom", "../shaders/voxelgrid.frag", verbose);
	GLuint newPlaneProgram = this->compileShaders("../shaders/plane.vert", "", "../shaders/plane.frag", verbose);
	GLuint newPlaneViewerProgram = this->compileShaders("../shaders/texture_explorer.vert", "", "../shaders/texture_explorer.frag", verbose);
	GLuint newVolumetricProgram = this->compileShaders("../shaders/transfer_mesh.vert", "../shaders/transfer_mesh.geom", "../shaders/transfer_mesh.frag", verbose);
	GLuint newBoundingBoxProgram = this->compileShaders("../shaders/bounding_box.vert", "", "../shaders/bounding_box.frag", verbose);

	if (newProgram) {
		glDeleteProgram(this->programHandle_projectedTex);
		this->programHandle_projectedTex = newProgram;
	}
	if (newPlaneProgram) {
		glDeleteProgram(this->programHandle_Plane3D);
		this->programHandle_Plane3D = newPlaneProgram;
	}
	if (newPlaneViewerProgram) {
		glDeleteProgram(this->programHandle_PlaneViewer);
		this->programHandle_PlaneViewer = newPlaneViewerProgram;
	}
	if (newVolumetricProgram) {
		glDeleteProgram(this->programHandle_VolumetricViewer);
		this->programHandle_VolumetricViewer = newVolumetricProgram;
	}
	if (newBoundingBoxProgram) {
		glDeleteProgram(this->programHandle_BoundingBox);
		this->programHandle_BoundingBox = newBoundingBoxProgram;
	}
}

GLuint Scene::compileShaders(std::string _vPath, std::string _gPath, std::string _fPath, bool verbose) {
	glUseProgram(0);

	// Checks if we have an available compiler, or exit ! We can't do shit otherwise.
	GLboolean compilerAvailable = GL_FALSE;
	glGetBooleanv(GL_SHADER_COMPILER, &compilerAvailable);
	if (compilerAvailable == GL_FALSE) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] : No shader compiler was available.\nExiting the program.\n";
		exit(EXIT_FAILURE);
	}

	if (verbose) { if (not _vPath.empty()) { std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiling vertex shader \"" << _vPath << "\"...\n"; } }
	GLuint _vSha = this->compileShader(_vPath, GL_VERTEX_SHADER, verbose);

	if (verbose) { if (not _gPath.empty()) { std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiling geometry shader \"" << _gPath << "\"...\n"; } }
	GLuint _gSha = this->compileShader(_gPath, GL_GEOMETRY_SHADER, verbose);

	if (verbose) { if (not _fPath.empty()) { std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiling fragment shader \"" << _fPath << "\"...\n"; } }
	GLuint _fSha = this->compileShader(_fPath, GL_FRAGMENT_SHADER, verbose);

	if (verbose) { std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiling program ...\n"; }
	GLuint _prog = this->compileProgram(_vSha, _gSha, _fSha, verbose);

	return _prog;
}

GLuint Scene::compileShader(const std::string& path, const GLenum shaType, bool verbose) {
	if (path.empty()) { return 0; }
	// Check the given type is accepted :
	if (shaType != GL_VERTEX_SHADER && shaType != GL_GEOMETRY_SHADER && shaType != GL_FRAGMENT_SHADER) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] Error : unrecognized shader type (vertex, geometry and fragment shaders)\n";
		return -1;
	}

	// Create a shader object :
	GLuint _sha = glCreateShader(shaType);

	std::string color_functions = "";
	bool includeColorShader = false;

	if (shaType == GL_FRAGMENT_SHADER) {
		includeColorShader = true;
		std::ifstream colFile = std::ifstream("../shaders/coloring.glsl", std::ios_base::in | std::ios_base::binary);
		if (!colFile.is_open()) {
			std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] Error : could not get the color shader loaded.\n";
			return -1;
		}
		colFile.seekg(0, colFile.end);
		std::size_t colShaSize = static_cast<std::size_t>(colFile.tellg());
		colFile.seekg(0, colFile.beg);
		color_functions.resize(colShaSize);
		colFile.read(color_functions.data(), colShaSize);
	}

	// Open the file :
	std::ifstream shaFile = std::ifstream(path.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!shaFile.is_open()) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] Error : could not get the contents of shader file " << path << '\n';
		return -1;
	}

	// Get the file's length in characters :
	shaFile.seekg(0, shaFile.end);
	std::size_t shaFileSize = static_cast<std::size_t>(shaFile.tellg());
	shaFile.seekg(0, shaFile.beg);

	char* shaSource = nullptr;
	std::string read_buffer;
	std::string shader_contents;

	if (includeColorShader) {
		while (getline(shaFile, read_buffer)) {
			if (read_buffer == "#pragma include_color_shader;") {
				read_buffer = color_functions;
			}
			shader_contents += read_buffer + '\n';
		}
		shaSource = shader_contents.data();
		shaFileSize = shader_contents.size();
	} else {
		// Get the file's contents and null-terminate it :
		shaSource = new char[shaFileSize+1];
		shaFile.read(shaSource, shaFileSize);
		shaSource[shaFileSize] = '\0';
	}

	// Source it into the shader object :
	glShaderSource(_sha, 1, const_cast<const char**>(&shaSource), 0);

	// We can free up the host memory now :
	shaFile.close();

	glCompileShader(_sha);

	GLint shaderInfoLength = 0;
	GLint charsWritten = 0;
	char* shaderInfoLog = nullptr;

	// Get shader information after compilation :
	glGetShaderiv(_sha, GL_INFO_LOG_LENGTH, &shaderInfoLength);
	if (shaderInfoLength > 1) {
		std::cerr << __FILE__ << ":" << __LINE__ << " : start Log ***********************************************" << '\n';

		std::cerr << __FILE__ << ":" << __LINE__ << " : Information about shader " << path << " : " << '\n';
		shaderInfoLog = new char[shaderInfoLength];
		glGetShaderInfoLog(_sha, shaderInfoLength, &charsWritten, shaderInfoLog);
		std::cerr << shaderInfoLog << '\n';
		delete[] shaderInfoLog;

		std::cerr << __FILE__ << ":" << __LINE__ << " : end Log ***********************************************" << '\n';
		#ifdef ENABLE_SHADER_CONTENTS_ON_FAILURE
		std::cerr << "Shader contents :" << '\n';
		std::cerr << "=============================================================================================\n";
		std::cerr << "=============================================================================================\n";
		std::cerr << shaSource << '\n';
		std::cerr << "=============================================================================================\n";
		std::cerr << "=============================================================================================\n";
		std::cerr << "End shader contents" << '\n';
		#endif
	}

	GLint result = GL_FALSE;
	glGetShaderiv(_sha, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] : Could not compile shader.\n";
		return 0;
	}

	return _sha;
}

// line 572 for compileShader
GLuint Scene::compileProgram(const GLuint vSha, const GLuint gSha, const GLuint fSha, bool verbose) {
	// Check any shader was passed to the program :
	if (vSha == 0 && gSha == 0 && fSha == 0) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] : No shader IDs were passed to the function.\n" <<
			"\tArguments : vSha(" << vSha << "), gSha(" << gSha << "), fSha(" << fSha << ")";
	}

	GLuint _prog = glCreateProgram();
	if (vSha != 0) { glAttachShader(_prog, vSha); }
	if (gSha != 0) { glAttachShader(_prog, gSha); }
	if (fSha != 0) { glAttachShader(_prog, fSha); }

	glLinkProgram(_prog);

	int InfoLogLength = 0;
	glGetProgramiv(_prog, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(_prog, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		std::cerr << __FILE__ << ":" << __LINE__ << " : Warning : errors while linking program :" << '\n';
		std::cerr << "------------------------------------------------------------------" << '\n';
		std::cerr << "------------------------------------------------------------------" << '\n';
		std::cerr << ProgramErrorMessage.data() << '\n';
		std::cerr << "------------------------------------------------------------------" << '\n';
		std::cerr << "------------------------------------------------------------------" << '\n';
	}

	GLint Result = 0;
	glGetProgramiv(_prog, GL_LINK_STATUS, &Result);
	if (Result == GL_FALSE) {
		// Return 0 (no program created) :
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] : Could not link shader.\n";
		return 0;
	} else {
		if (verbose) {std::cerr<<"[LOG]["<<__FILE__ << ":" << __LINE__<<"] Compiled program. New name : "<< _prog <<"\n";}
	}

	if (verbose) { std::cerr << '\n'; } // Put an empty line between all program compilations (more readable)

	if (vSha != 0) { glDetachShader(_prog, vSha); }
	if (gSha != 0) { glDetachShader(_prog, gSha); }
	if (fSha != 0) { glDetachShader(_prog, fSha); }
	if (vSha != 0) { glDeleteShader(vSha); }
	if (gSha != 0) { glDeleteShader(gSha); }
	if (fSha != 0) { glDeleteShader(fSha); }

	return _prog;
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

	glTexImage1D(GL_TEXTURE_1D,		// GLenum : Target
		static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
		tex.internalFormat,		// GLint  : Number of color components in the picture.
		tex.size.x,			// GLsizei: Image width
		static_cast<GLint>(0),		// GLint  : Border. This value MUST be 0.
		tex.format,			// GLenum : Format of the pixel data
		tex.type,			// GLenum : Type (the data type as in uchar, uint, float ...)
		tex.data			// void*  : Data to load into the buffer
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

	glTexImage2D(GL_TEXTURE_2D,		// GLenum : Target
		static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
		tex.internalFormat,		// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		tex.size.x,			// GLsizei: Image width
		tex.size.y,			// GLsizei: Image height
		static_cast<GLint>(0),		// GLint  : Border. This value MUST be 0.
		tex.format,			// GLenum : Format of the pixel data
		tex.type,			// GLenum : Type (the data type as in uchar, uint, float ...)
		tex.data			// void*  : Data to load into the buffer
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

	glTexImage3D(GL_TEXTURE_3D,		// GLenum : Target
		static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
		tex.internalFormat,		// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		tex.size.x,			// GLsizei: Image width
		tex.size.y,			// GLsizei: Image height
		tex.size.z,			// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),		// GLint  : Border. This value MUST be 0.
		tex.format,			// GLenum : Format of the pixel data
		tex.type,			// GLenum : Type (the data type as in uchar, uint, float ...)
		tex.data			// void*  : Data to load into the buffer
	);


	return texHandle;
}

GLuint Scene::uploadTexture3D_iterative(const TextureUpload &tex, const std::shared_ptr<DiscreteGrid>& red, const std::shared_ptr<DiscreteGrid>& blue) {
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

	PRINTVAL(tex.size.x)
	PRINTVAL(tex.size.y)
	PRINTVAL(tex.size.z)

	this->debugLog->pushGroup("Texture upload");

	std::cerr << "[LOG] Allocating memory ...";
	glTexImage3D(GL_TEXTURE_3D,		// GLenum : Target
		static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
		tex.internalFormat,		// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		tex.size.x,			// GLsizei: Image width
		tex.size.y,			// GLsizei: Image height
		tex.size.z,			// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),		// GLint  : Border. This value MUST be 0.
		tex.format,			// GLenum : Format of the pixel data
		tex.type,			// GLenum : Type (the data type as in uchar, uint, float ...)
		nullptr		// null for the moment, to allocate data only
	);
	std::cerr << "done.\n";

	// check for error, and throw a std::runtime_error() for now
	GLenum err = glGetError();
	if (err == GL_OUT_OF_MEMORY) {
		std::cerr << "[ERROR] Application could not allocate enough memory.\n";
		return 0;
	}

	std::size_t imgSize = tex.size.x * tex.size.y;
	std::vector<DiscreteGrid::data_t> image(imgSize * 2, DiscreteGrid::data_t(0));
	const DiscreteGrid::data_t* redPtr = red->getDataPtr();
	const DiscreteGrid::data_t* bluePtr = blue->getDataPtr();

	// upload the texture slice by slice :
	for (GLint z = 0; z < tex.size.z; ++z) {
		std::cerr << "[LOG] Uploading slice " << z << " ... \n";
		std::size_t baseIdx = z * imgSize;
		for (std::size_t i = 0; i < imgSize; ++i) {
			image[2*i + 0] = redPtr[baseIdx + i];
			image[2*i + 1] = bluePtr[baseIdx + i];
		}
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, z, tex.size.x, tex.size.y, 1, tex.format, tex.type, image.data());
		err = glGetError();
		if (err == GL_OUT_OF_MEMORY) {
			std::cerr << "[ERROR] Application could not allocate enough memory for slice " << z << "\n";
		}
	}

	this->debugLog->popGroup();

	return texHandle;
}

GLuint Scene::testTextureUpload(GLuint nbTex, DiscreteGrid::sizevec3 dims) {
	GLuint *textures = new GLuint[nbTex];

	glGenTextures(nbTex, textures);

	// connect to cerr output, to have messages on the console
	auto connection = QObject::connect(this->debugLog, &QOpenGLDebugLogger::messageLogged,
	[this](const QOpenGLDebugMessage m) {
			std::cerr << "OpenGL Message logged :";
			this->openGLDebugLogger_inserter(m);
	});

	for (GLuint i = 0; i < nbTex; ++i) {
		std::cerr << "Binding texture " << i << "/" << nbTex << " ... \n";
		// Bind texture to Tex3D binding point :
		glBindTexture(GL_TEXTURE_3D, textures[i]);
		// Allocate texture :
		glTexImage3D(
			GL_TEXTURE_3D,
			0,
			GL_RG16UI,
			dims.x,
			dims.y,
			dims.z,
			0,
			GL_RG_INTEGER,
			GL_UNSIGNED_SHORT,
			nullptr
		);
	}

	// Disconnect the cerr output
	this->debugLog->disconnect(connection);

	glDeleteTextures(nbTex, textures);

	delete[] textures;

	return 0;
}

void Scene::openGLDebugLogger_inserter(const QOpenGLDebugMessage message) {
	QString src = "";
	QString sev = "";
	QString typ = "";
	QFlags severity = message.severity();
	QFlags type = message.type();
	QFlags source = message.source();

	if (severity & QOpenGLDebugMessage::Severity::NotificationSeverity)	{ sev += "{Notif}"; }
	if (severity & QOpenGLDebugMessage::Severity::LowSeverity)			{ sev += "{Low  }"; }
	if (severity & QOpenGLDebugMessage::Severity::MediumSeverity)		{ sev += "{Med  }"; }
	if (severity & QOpenGLDebugMessage::Severity::HighSeverity)			{ sev += "{High }"; }

	if (type & QOpenGLDebugMessage::Type::ErrorType)					{ typ += "[	ERROR     ]"; }
	if (type & QOpenGLDebugMessage::Type::DeprecatedBehaviorType)		{ typ += "[ DEPRE. BEHAV. ]"; }
	if (type & QOpenGLDebugMessage::Type::UndefinedBehaviorType)		{ typ += "[ UNDEF. BEHAV. ]"; }
	if (type & QOpenGLDebugMessage::Type::PortabilityType)				{ typ += "[  PORTABILITY  ]"; }
	if (type & QOpenGLDebugMessage::Type::PerformanceType)				{ typ += "[      PERF     ]"; }
	if (type & QOpenGLDebugMessage::Type::OtherType)					{ typ += "[     OTHER	  ]"; }
	if (type & QOpenGLDebugMessage::Type::MarkerType)					{ typ += "[     MARKER    ]"; }
	if (type & QOpenGLDebugMessage::Type::GroupPushType)				{ typ += "[  GROUP PUSH   ]"; }
	if (type & QOpenGLDebugMessage::Type::GroupPopType)					{ typ += "[   GROUP POP   ]"; }

	if (source & QOpenGLDebugMessage::Source::APISource)				{ src += "[OpenGL  ]"; }
	if (source & QOpenGLDebugMessage::Source::WindowSystemSource)		{ src += "[WinSys  ]"; }
	if (source & QOpenGLDebugMessage::Source::ShaderCompilerSource)		{ src += "[ShaComp ]"; }
	if (source & QOpenGLDebugMessage::Source::ThirdPartySource)			{ src += "[3rdParty]"; }
	if (source & QOpenGLDebugMessage::Source::OtherSource)				{ src += "[Other   ]"; }

	QString glMessage = sev + typ + src + " [ID:" + QString::number(message.id()) + "] : " + message.message() + "\n";

	std::cerr << glMessage.toStdString() << '\n';
}

void Scene::launchSaveDialog() {
	// if no grids are loaded, do nothing !
	if (this->grids.size() == 0) {
		QMessageBox messageBox;
		messageBox.critical(nullptr, "Error","Cannot save a grid when nothing is loaded !");
		messageBox.setFixedSize(500,200);
		return;
	}

	if (this->gridControl != nullptr) {
		std::cerr << "Controller was already added, showing it now ...\n";
		this->gridControl->raise();
		this->gridControl->show();
		return;
	}

	// create an output grid AND a tetmesh to generate it :
	std::shared_ptr<OutputGrid> outputGrid = std::make_shared<OutputGrid>();
	outputGrid->setOffline();

	std::shared_ptr<TetMesh> tetmesh = std::make_shared<TetMesh>();
	// add the grids to the tetmesh
	for (std::size_t i = 0; i < this->grids.size(); ++i) {
		std::for_each(this->grids[i]->grid.cbegin(), this->grids[i]->grid.cend(),
		[&tetmesh](const std::shared_ptr<DiscreteGrid>& _g) {
			tetmesh->addInputGrid(std::dynamic_pointer_cast<InputGrid>(_g));
		});
	}
	// add the output grid
	tetmesh->setOutputGrid(outputGrid);

	// So we can draw it :
	GridGLView::Ptr outputGridView = std::make_shared<GridGLView>(std::initializer_list<std::shared_ptr<DiscreteGrid>>({outputGrid}));
	this->grids.push_back(outputGridView);

	// create a grid controller to generate a new outputgrid
	this->gridControl = new GridControl(outputGrid, tetmesh, this);
	// show it :
	gridControl->show();
	// done
	return;
}

std::vector<std::shared_ptr<DiscreteGrid>> Scene::getInputGrids() const {
	std::vector<std::shared_ptr<DiscreteGrid>> igrids;

	// For each GLView, if the grid is an
	std::for_each(std::cbegin(this->grids), std::cend(this->grids), [&igrids](const GridGLView::Ptr& v) {
		std::for_each(v->grid.cbegin(), v->grid.cend(), [&igrids](const std::shared_ptr<DiscreteGrid>& _g) {
			if (std::dynamic_pointer_cast<InputGrid>(_g) != nullptr) {
				igrids.push_back(_g);
			}
		});
	});

	return igrids;
}

std::size_t Scene::getInputGridCount() const {
	std::size_t cnt = 0;

	// For each GLView :
	std::for_each(std::cbegin(this->grids), std::cend(this->grids), [&cnt](const GridGLView::Ptr& v) {
		// for each grid in it :
		std::for_each(v->grid.cbegin(), v->grid.cend(), [&cnt](const std::shared_ptr<DiscreteGrid>& _g) {
			if (std::dynamic_pointer_cast<InputGrid>(_g) != nullptr) {
				cnt++;
			}
		});
	});

	return cnt;
}

void Scene::removeController() {
	this->gridControl = nullptr;
}

void Scene::deleteGrid(const std::shared_ptr<DiscreteGrid> &_grid) {
	std::cerr << "Deleting a grid" << '\n';
	std::size_t i = 0, j = 0;
	for (i = 0; i < this->grids.size(); ++i) {
		std::cerr << "Checking value " << i << '\n';
		for (j = 0; j < this->grids[i]->grid.size(); ++i) {
			if (this->grids[i]->grid[j] == _grid) {
				std::cerr << "Found at index " << i << '\n';
				break;
			}
		}
	}
	if (i == this->grids.size()) {
		std::cerr << "No grid found !";
		return;
	}

	// Delete the texture and associated meshes on next refresh :
	this->shouldDeleteGrid = true;
	this->delGrid.push_back(i);
}

void Scene::deleteGridNow() {
	this->shouldDeleteGrid = false;
	for (std::size_t g : this->delGrid) {
		// Delete 'raw' grid texture
		glDeleteTextures(1, &this->grids[g]->gridTexture);
		// Delete mesh textures :
		glDeleteTextures(1, &this->grids[g]->volumetricMesh.visibilityMap);
		glDeleteTextures(1, &this->grids[g]->volumetricMesh.vertexPositions);
		glDeleteTextures(1, &this->grids[g]->volumetricMesh.textureCoordinates);
		glDeleteTextures(1, &this->grids[g]->volumetricMesh.neighborhood);
		glDeleteTextures(1, &this->grids[g]->volumetricMesh.faceNormals);
		this->grids[g]->nbChannels = 0;
	}

	// Remove all items from grids which have no channels :
	this->grids.erase(std::remove_if(this->grids.begin(), this->grids.end(), [](const GridGLView::Ptr& v){
		if (v->nbChannels == 0) { return true; }
		return false;
	}), this->grids.end());
	this->delGrid.clear();

	this->updateBoundingBox();
}

void Scene::drawPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading, float zoomRatio, glm::vec2 offset) {
	if (this->grids.size() == 0) { return; }

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	uint min = 0; // min index for drawing commands
	if (_plane == planes::x) { min =  0; }
	if (_plane == planes::y) { min =  6; }
	if (_plane == planes::z) { min = 12; }

	glUseProgram(this->programHandle_PlaneViewer);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_SinglePlaneElement);

	// Do for all grids :
	#warning drawPlaneView() : Only draws the first grid !
	if (this->grids.size() > 0) {
		this->prepareUniforms_PlaneViewer(_plane, _heading, fbDims, zoomRatio, offset, this->grids[0]);

		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*)(min*sizeof(GLuint)));
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	this->showVAOstate = false;
}

void Scene::drawVolumetric(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, const GridGLView::Ptr& grid) {
	if (grid->gridTexture > 0) {
		glUseProgram(this->programHandle_VolumetricViewer);

		/// @b Shortcut for glGetUniform, since this can result in long lines.
		auto getUniform = [&](const char* name) -> GLint {
			GLint g = glGetUniformLocation(this->programHandle_VolumetricViewer, name);
			if (this->showVAOstate) { std::cerr << "[LOG]\tLocation [" << +g << "] for uniform " << name << '\n'; }
			return g;
		};

		if (this->showVAOstate) {
			LOG_ENTER(Scene::drawVolumetric)
			std::cerr << "[LOG] Uniform values for " << __FUNCTION__ << " : \n";
		}

		// Textures :
		GLint location_vertices_translation =	getUniform("vertices_translations");
		GLint location_normals_translation =	getUniform("normals_translations");
		GLint location_visibility_texture =		getUniform("visibility_texture");
		GLint location_texture_coordinates =	getUniform("texture_coordinates");
		GLint location_neighbors =				getUniform("neighbors");
		GLint location_Mask =					getUniform("texData");
		GLint location_visibilityMap =			getUniform("visiblity_map");
		GLint location_visibilityMapAlternate =	getUniform("visiblity_map_alternate");
		// Scalars :
		GLint location_voxelSize =				getUniform("voxelSize");
		GLint location_gridSize =				getUniform("gridSize");
		GLint location_specRef =				getUniform("specRef");
		GLint location_shininess =				getUniform("shininess");
		GLint location_diffuseRef =				getUniform("diffuseRef");
		// Vectors/arrays :
		GLint location_cam =					getUniform("cam");
		GLint location_cut =					getUniform("cut");
		GLint location_cutDirection =			getUniform("cutDirection");
		GLint location_clipDistanceFromCamera = getUniform("clipDistanceFromCamera");
		GLint location_colorBounds =			getUniform("colorBounds");
		GLint location_colorBoundsAlternate =	getUniform("colorBoundsAlternate");
		GLint location_textureBounds =			getUniform("textureBounds");
		GLint location_textureBoundsAlternate =	getUniform("textureBoundsAlternate");
		GLint location_visuBBMin =				getUniform("visuBBMin");
		GLint location_visuBBMax =				getUniform("visuBBMax");
		GLint location_shouldUseBB =			getUniform("shouldUseBB");
		GLint location_volumeEpsilon =			getUniform("volumeEpsilon");
		// Matrices :
		GLint location_mMat =					getUniform("mMat");
		GLint location_vMat =					getUniform("vMat");
		GLint location_pMat =					getUniform("pMat");
		// light positions :
		GLint location_light0 =					getUniform("lightPositions[0]");
		GLint location_light1 =					getUniform("lightPositions[1]");
		GLint location_light2 =					getUniform("lightPositions[2]");
		GLint location_light3 =					getUniform("lightPositions[3]");
		GLint location_light4 =					getUniform("lightPositions[4]");
		GLint location_light5 =					getUniform("lightPositions[5]");
		GLint location_light6 =					getUniform("lightPositions[6]");
		GLint location_light7 =					getUniform("lightPositions[7]");
		// Texture display mode :
		GLint location_rgbMode =				getUniform("rgbMode");

		GLint location_r_channelView =			getUniform("r_channelView");
		GLint location_r_selectedChannel =		getUniform("r_selectedChannel");
		GLint location_r_nbChannels =			getUniform("r_nbChannels");
		GLint location_g_channelView =			getUniform("g_channelView");
		GLint location_g_selectedChannel =		getUniform("g_selectedChannel");
		GLint location_g_nbChannels =			getUniform("g_nbChannels");

		GLint location_color0 =					getUniform("color0");
		GLint location_color1 =					getUniform("color1");
		GLint location_color0Alt =				getUniform("color0Alternate");
		GLint location_color1Alt =				getUniform("color1Alternate");

		glUniform3fv(location_color0, 1, glm::value_ptr(this->color0));
		glUniform3fv(location_color1, 1, glm::value_ptr(this->color1));
		glUniform3fv(location_color0Alt, 1, glm::value_ptr(this->color0_second));
		glUniform3fv(location_color1Alt, 1, glm::value_ptr(this->color1_second));

		if (grid->nbChannels > 1) {
			glUniform1ui(location_r_selectedChannel, this->selectedChannel_r);
			glUniform1ui(location_g_selectedChannel, this->selectedChannel_g);
		} else {
			glUniform1ui(location_r_selectedChannel, 0);
			glUniform1ui(location_g_selectedChannel, 0);
		}

		glUniform1ui(location_rgbMode, this->rgbMode);

		std::size_t tex = 0;
		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.vertexPositions);
		glUniform1i(location_vertices_translation, tex);
		tex++;

		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.faceNormals);
		glUniform1i(location_normals_translation, tex);
		tex++;

		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.visibilityMap);
		glUniform1i(location_visibility_texture, tex);
		tex++;

		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.textureCoordinates);
		glUniform1i(location_texture_coordinates, tex);
		tex++;

		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.neighborhood);
		glUniform1i(location_neighbors, tex);
		tex++;

		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_3D, grid->gridTexture);
		glUniform1i(location_Mask, tex);
		tex++;

		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_2D, this->texHandle_ColorScaleGrid);
		glUniform1i(location_visibilityMap, tex);
		tex++;

		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_2D, this->texHandle_ColorScaleGridAlternate);
		glUniform1i(location_visibilityMapAlternate, tex);
		tex++;

		uint chan_r = this->colorFunctionToUniform(this->channels_r);
		uint chan_g = this->colorFunctionToUniform(this->channels_g);

		glUniform1ui(location_r_channelView, chan_r);
		glUniform1ui(location_g_channelView, chan_g);

		glm::vec3 floatres = glm::convert_to<float>(grid->grid[0]->getResolution());

		glUniform1f(location_diffuseRef, .8f);
		glUniform1f(location_specRef, .8f);
		glUniform1f(location_shininess, .8f);
		glUniform3fv(location_voxelSize, 1, glm::value_ptr(grid->grid[0]->getVoxelDimensions()));
		glUniform3fv(location_gridSize, 1, glm::value_ptr(floatres));
		glUniform1ui(location_r_nbChannels, 1);
		glUniform1ui(location_g_nbChannels, 1);
		glUniform3fv(location_volumeEpsilon, 1, glm::value_ptr(grid->defaultEpsilon));

		glm::vec3 planePos = this->computePlanePositions();

		glUniform3fv(location_cam, 1, glm::value_ptr(camPos));
		glUniform3fv(location_cut, 1, glm::value_ptr(planePos));
		glUniform3fv(location_cutDirection, 1, glm::value_ptr(this->planeDirection));
		glUniform1f(location_clipDistanceFromCamera, this->clipDistanceFromCamera);

		const glm::mat4& gridTransfo = grid->grid[0]->getTransform_GridToWorld();
		glUniformMatrix4fv(location_mMat, 1, GL_FALSE, glm::value_ptr(gridTransfo));
		glUniformMatrix4fv(location_vMat, 1, GL_FALSE, mvMat);
		glUniformMatrix4fv(location_pMat, 1, GL_FALSE, pMat);

		glm::vec2 tb0 = glm::convert_to<float>(this->textureBounds0);
		glm::vec2 tb1 = glm::convert_to<float>(this->textureBounds1);
		glUniform2fv(location_colorBounds, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds0)));
		glUniform2fv(location_colorBoundsAlternate, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds1)));
		glUniform2fv(location_textureBounds, 1, glm::value_ptr(tb0));
		glUniform2fv(location_textureBoundsAlternate, 1, glm::value_ptr(tb1));

		DiscreteGrid::bbox_t::vec min = this->visuBox.getMin();
		DiscreteGrid::bbox_t::vec max = this->visuBox.getMax();
		glUniform3fv(location_visuBBMin, 1, glm::value_ptr(min));
		glUniform3fv(location_visuBBMax, 1, glm::value_ptr(max));
		glUniform1ui(location_shouldUseBB, ((this->drawMode == DrawMode::VolumetricBoxed) ? 1 : 0));

		glUniform3fv(location_light0, 1, glm::value_ptr(this->lightPositions[0]));
		glUniform3fv(location_light1, 1, glm::value_ptr(this->lightPositions[1]));
		glUniform3fv(location_light2, 1, glm::value_ptr(this->lightPositions[2]));
		glUniform3fv(location_light3, 1, glm::value_ptr(this->lightPositions[3]));
		glUniform3fv(location_light4, 1, glm::value_ptr(this->lightPositions[4]));
		glUniform3fv(location_light5, 1, glm::value_ptr(this->lightPositions[5]));
		glUniform3fv(location_light6, 1, glm::value_ptr(this->lightPositions[6]));
		glUniform3fv(location_light7, 1, glm::value_ptr(this->lightPositions[7]));

		// print uniform values :
		if (this->showVAOstate) {
			auto vx = grid->grid[0]->getVoxelDimensions();
			std::cerr << "[LOG] " << __FUNCTION__ << " has uniform values :\n";
			std::cerr << "[LOG]\tMax possible value        : " << static_cast<double>(std::numeric_limits<DiscreteGrid::data_t>::max()) << '\n';
			std::cerr << "[LOG]\tDraw mode                : " << +this->drawMode << '\n';
			std::cerr << "[LOG]\tNumber of channels        : " << +grid->nbChannels << '\n';
			std::cerr << "[LOG]\tClip distance from camera : " << +clipDistanceFromCamera << '\n';
			std::cerr << "[LOG]\tVoxel dimensions          : [" << vx.x << ',' << vx.y << ',' << vx.z << "]\n";
			std::cerr << "[LOG]\tGrid resolution           : [" << floatres.x << ',' << floatres.y << ',' << floatres.z << "]\n";
			std::cerr << "[LOG]\tVolume epsilon            : [" << grid->defaultEpsilon.x << ',' << grid->defaultEpsilon.y << ',' << grid->defaultEpsilon.z << "]\n";
			std::cerr << "[LOG]\tCamera position           : [" << camPos.x << ',' << camPos.y << ',' << camPos.z << "]\n";
			std::cerr << "[LOG]\tPlane positions           : [" << planePos.x << ',' << planePos.y << ',' << planePos.z << "]\n";
			std::cerr << "[LOG]\tPlane directions          : [" << +this->planeDirection.x << ',' << +this->planeDirection.y << ',' << this->planeDirection.z << "]\n";
			std::cerr << "[LOG]\tTexture bounds            : [" << +this->textureBounds0.x << ',' << +this->textureBounds0.y << "]\n";
			std::cerr << "[LOG]\tTexture bounds alt        : [" << +this->textureBounds1.x << ',' << +this->textureBounds1.y << "]\n";
			std::cerr << "[LOG]\tColor bounds              : [" << +this->colorBounds0.x << ',' << +this->colorBounds0.y << "]\n";
			std::cerr << "[LOG]\tColor bounds alt          : [" << +this->colorBounds1.x << ',' << +this->colorBounds1.y << "]\n";
			std::cerr << "[LOG]\tVisu BB min               : [" << +min.x << ',' << +min.y << ',' << +min.z << "]\n";
			std::cerr << "[LOG]\tVisu BB max               : [" << +max.x << ',' << +max.y << ',' << +max.z << "]\n";
			for (std::size_t i = 0; i < this->lightPositions.size(); ++i) {
				std::cerr << "[LOG]\tLight " << i << "                   : [" << +this->lightPositions[i].x << ',' <<
					+this->lightPositions[i].y << ',' << +this->lightPositions[i].z << "]\n";
			}
			LOG_LEAVE(Scene::drawVolumetric)
		}

		this->tex3D_bindVAO();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Texture3D_VertIdx);

		glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, (void*)0, grid->volumetricMesh.tetrahedraCount);

		// Unbind textures :
		for (std::size_t t = tex; t >= 0 && t < tex+1; t--) {
			glActiveTexture(GL_TEXTURE0 + t);
			glBindTexture(GL_TEXTURE_3D, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindTexture(GL_TEXTURE_1D, 0);
		}

		// Unbind program, buffers and VAO :
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}

	// draw grid BB :
	this->drawBoundingBox(grid->grid[0]->getBoundingBoxWorldSpace(), grid->boundingBoxColor, mvMat, pMat);
}

void Scene::drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	#warning drawPlanes() : Only draws the first grid for each plane !

	// Plane X :
	glUseProgram(this->programHandle_Plane3D);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
	if (not this->grids.empty()) {
		this->prepareUniforms_3DPlane(mvMat, pMat, planes::x, this->grids[0], showTexOnPlane);
		this->setupVAOPointers();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, static_cast<GLvoid*>(0));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	// Plane Y :
	glUseProgram(this->programHandle_Plane3D);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
	if (not this->grids.empty()) {
		this->prepareUniforms_3DPlane(mvMat, pMat, planes::y, this->grids[0], showTexOnPlane);
		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*)(6*sizeof(GLuint)));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	// Plane Z :
	glUseProgram(this->programHandle_Plane3D);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
	if (not this->grids.empty()) {
		this->prepareUniforms_3DPlane(mvMat, pMat, planes::z, this->grids[0], showTexOnPlane);
		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*)(12*sizeof(GLuint)));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
}

glm::vec3 Scene::computePlanePositions() {
	DiscreteGrid::bbox_t::vec position = this->sceneBB.getMin();
	DiscreteGrid::bbox_t::vec diagonal = this->sceneBB.getDiagonal();
	glm::vec3 planePos = (position + this->planeDisplacement * diagonal);
	return planePos;
}

void Scene::prepareUniforms_3DSolid(GLfloat *mvMat, GLfloat *pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, const GridGLView::Ptr& gridView) {
	// Get the world to grid transform :
	glm::mat4 transfoMat = baseMatrix * gridView->grid[0]->getTransform_GridToWorld();

	// Get the uniform locations :
	GLint mMatrix_Loc =						glGetUniformLocation(this->programHandle_projectedTex, "mMatrix");
	GLint vMatrix_Loc =						glGetUniformLocation(this->programHandle_projectedTex, "vMatrix");
	GLint pMatrix_Loc =						glGetUniformLocation(this->programHandle_projectedTex, "pMatrix");
	GLint lightPos_Loc =					glGetUniformLocation(this->programHandle_projectedTex, "lightPos");
	GLint voxelGridOrigin_Loc =				glGetUniformLocation(this->programHandle_projectedTex, "voxelGridOrigin");
	GLint voxelGridSize_Loc =				glGetUniformLocation(this->programHandle_projectedTex, "voxelGridSize");
	GLint voxelSize_Loc =					glGetUniformLocation(this->programHandle_projectedTex, "voxelSize");
	GLint drawMode_Loc =					glGetUniformLocation(this->programHandle_projectedTex, "drawMode");
	GLint texDataLoc =						glGetUniformLocation(this->programHandle_projectedTex, "texData");
	GLint planePositionsLoc =				glGetUniformLocation(this->programHandle_projectedTex, "planePositions");
	GLint location_planeDirections =		glGetUniformLocation(this->programHandle_projectedTex, "planeDirections");
	GLint gridPositionLoc =					glGetUniformLocation(this->programHandle_projectedTex, "gridPosition");
	GLint location_colorBounds =			glGetUniformLocation(this->programHandle_projectedTex, "colorBounds");
	GLint location_colorBoundsAlternate =	glGetUniformLocation(this->programHandle_projectedTex, "colorBoundsAlternate");
	GLint location_textureBounds =			glGetUniformLocation(this->programHandle_projectedTex, "texBounds");
	GLint location_textureBoundsAlternate =	glGetUniformLocation(this->programHandle_projectedTex, "texBoundsAlternate");
	GLint location_color0 =					glGetUniformLocation(this->programHandle_projectedTex, "color0");
	GLint location_color1 =					glGetUniformLocation(this->programHandle_projectedTex, "color1");
	GLint location_color0Alt =				glGetUniformLocation(this->programHandle_projectedTex, "color0Alternate");
	GLint location_color1Alt =				glGetUniformLocation(this->programHandle_projectedTex, "color1Alternate");
	GLint location_r_channelView =			glGetUniformLocation(this->programHandle_projectedTex, "r_channelView");
	GLint location_r_selectedChannel =		glGetUniformLocation(this->programHandle_projectedTex, "r_selectedChannel");
	GLint location_r_nbChannels =			glGetUniformLocation(this->programHandle_projectedTex, "r_nbChannels");
	GLint location_g_channelView =			glGetUniformLocation(this->programHandle_projectedTex, "g_channelView");
	GLint location_g_selectedChannel =		glGetUniformLocation(this->programHandle_projectedTex, "g_selectedChannel");
	GLint location_g_nbChannels =			glGetUniformLocation(this->programHandle_projectedTex, "g_nbChannels");

	DiscreteGrid::bbox_t::vec origin = gridView->grid[0]->getBoundingBox().getMin();
	DiscreteGrid::bbox_t::vec originWS = gridView->grid[0]->getBoundingBoxWorldSpace().getMin();
	DiscreteGrid::sizevec3 gridDims = gridView->grid[0]->getResolution();
	glm::vec3 dims = glm::convert_to<float>(gridDims);

	if (showVAOstate) {
		PRINTVAL(gridDims.x);
		PRINTVAL(gridDims.y);
		PRINTVAL(gridDims.z);
	}

	glm::vec2 tb0 = glm::convert_to<float>(this->textureBounds0);
	glm::vec2 tb1 = glm::convert_to<float>(this->textureBounds1);

	glUniform3fv(voxelGridOrigin_Loc, 1, glm::value_ptr(origin));
	glUniform3fv(voxelGridSize_Loc, 1, glm::value_ptr(dims));
	glUniform3fv(voxelSize_Loc, 1, glm::value_ptr(gridView->grid[0]->getVoxelDimensions()));
	glUniform2fv(location_colorBounds, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds0)));
	glUniform2fv(location_colorBoundsAlternate, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds1)));
	glUniform2fv(location_textureBounds, 1, glm::value_ptr(tb0));
	glUniform2fv(location_textureBoundsAlternate, 1, glm::value_ptr(tb1));
	glUniform3fv(location_color0, 1, glm::value_ptr(this->color0));
	glUniform3fv(location_color1, 1, glm::value_ptr(this->color1));
	glUniform3fv(location_color0Alt, 1, glm::value_ptr(this->color0_second));
	glUniform3fv(location_color1Alt, 1, glm::value_ptr(this->color1_second));
	if (gridView->nbChannels > 1) {
		glUniform1ui(location_r_selectedChannel, this->selectedChannel_r);
		glUniform1ui(location_g_selectedChannel, this->selectedChannel_g);
	} else {
		glUniform1ui(location_r_selectedChannel, 0);
		glUniform1ui(location_g_selectedChannel, 0);
	}
	glUniform1ui(drawMode_Loc, this->drawMode);
	glUniform1ui(location_r_nbChannels, 1);
	glUniform1ui(location_g_nbChannels, 1);

	// Textures :
	//if (gridView->grid[0]->hasData() == true || gridView->grid[0]->isGridOffline() == true) {
	glActiveTexture(GL_TEXTURE0 + 0);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, gridView->gridTexture);
	glUniform1i(texDataLoc, 0);
	//}

	uint chan = this->colorFunctionToUniform(this->channels_r);
	uint chan2 = this->colorFunctionToUniform(this->channels_g);
	glUniform1ui(location_r_channelView, chan);
	glUniform1ui(location_g_channelView, chan2);

	glm::vec3 planePos = this->computePlanePositions();

	glUniform3fv(planePositionsLoc, 1, glm::value_ptr(planePos));
	glUniform3fv(location_planeDirections, 1, glm::value_ptr(this->planeDirection));
	glUniform3fv(gridPositionLoc, 1, glm::value_ptr(originWS));

	// Apply the uniforms :
	glUniformMatrix4fv(mMatrix_Loc, 1, GL_FALSE, glm::value_ptr(transfoMat));
	glUniformMatrix4fv(vMatrix_Loc, 1, GL_FALSE, &mvMat[0]);
	glUniformMatrix4fv(pMatrix_Loc, 1, GL_FALSE, &pMat[0]);
	glUniform4fv(lightPos_Loc, 1, glm::value_ptr(lightPos));
}

void Scene::prepareUniforms_3DPlane(GLfloat *mvMat, GLfloat *pMat, planes _plane, const GridGLView::Ptr& grid, bool showTexOnPlane) {
	bool shouldHide = false;
	if (_plane == planes::x) { shouldHide = this->planeVisibility.x; }
	if (_plane == planes::y) { shouldHide = this->planeVisibility.y; }
	if (_plane == planes::z) { shouldHide = this->planeVisibility.z; }

	auto getUniform = [&](const char* name) -> GLint {
		GLint g = glGetUniformLocation(this->programHandle_Plane3D, name);
		if (this->showVAOstate) { std::cerr << "[LOG]\tLocation [" << +g << "] for uniform " << name << '\n'; }
		return g;
	};

	// Get uniform locations for the program :
	GLint location_mMatrix =				getUniform("model_Mat");
	GLint location_vMatrix =				getUniform("view_Mat");
	GLint location_pMatrix =				getUniform("projection_Mat");
	GLint location_gridTransform =			getUniform("gridTransform");
	GLint location_gridSize =				getUniform("gridSize");
	GLint location_gridDimensions =			getUniform("gridDimensions");

	// Shader uniforms :
	GLint location_texData =				getUniform("texData");
	GLint location_planePosition =			getUniform("planePositions");
	GLint location_planeDirection =			getUniform("planeDirections");
	GLint location_sceneBBPosition =		getUniform("sceneBBPosition");
	GLint location_sceneBBDiagonal =		getUniform("sceneBBDiagonal");
	GLint location_colorBounds =			getUniform("colorBounds");
	GLint location_textureBounds =			getUniform("textureBounds");
	GLint location_colorBoundsAlternate =	getUniform("colorBoundsAlternate");
	GLint location_textureBoundsAlternate =	getUniform("textureBoundsAlternate");
	GLint location_currentPlane =			getUniform("currentPlane");
	GLint location_showTex =				getUniform("showTex");
	GLint location_drawOnlyData =			getUniform("drawOnlyData");

	GLint location_color0 =					getUniform("color0");
	GLint location_color1 =					getUniform("color1");
	GLint location_color0Alt =				getUniform("color0Alternate");
	GLint location_color1Alt =				getUniform("color1Alternate");
	GLint location_r_channelView =			getUniform("r_channelView");
	GLint location_r_selectedChannel =		getUniform("r_selectedChannel");
	GLint location_r_nbChannels =			getUniform("r_nbChannels");
	GLint location_g_channelView =			getUniform("g_channelView");
	GLint location_g_selectedChannel =		getUniform("g_selectedChannel");
	GLint location_g_nbChannels =			getUniform("g_nbChannels");

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

	// Generate the data we need :
	glm::mat4 transform = glm::mat4(1.f);
	glm::mat4 gridTransfo = grid->grid[0]->getTransform_GridToWorld();
	DiscreteGrid::bbox_t bbws = grid->grid[0]->getBoundingBoxWorldSpace();
	glm::vec3 dims = glm::convert_to<glm::vec3::value_type>(grid->grid[0]->getResolution()) * grid->grid[0]->getVoxelDimensions();
	glm::vec3 size = bbws.getDiagonal();
	GLint plIdx = (_plane == planes::x) ? 1 : (_plane == planes::y) ? 2 : 3;

	// gridTransfo = glm::mat4(1.);
	gridTransfo = grid->grid[0]->getTransform_GridToWorld();

	DiscreteGrid::bbox_t::vec position = this->sceneBB.getMin();
	DiscreteGrid::bbox_t::vec diagonal = this->sceneBB.getDiagonal();
	glm::vec3 planePos = this->computePlanePositions();

	glUniformMatrix4fv(location_mMatrix, 1, GL_FALSE, glm::value_ptr(transform));
	glUniformMatrix4fv(location_vMatrix, 1, GL_FALSE, mvMat);
	glUniformMatrix4fv(location_pMatrix, 1, GL_FALSE, pMat);
	glUniformMatrix4fv(location_gridTransform, 1, GL_FALSE, glm::value_ptr(gridTransfo));
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
	glUniform2fv(location_textureBounds, 1, glm::value_ptr(tb0));
	glUniform2fv(location_textureBoundsAlternate, 1, glm::value_ptr(tb1));

	glUniform3fv(location_planePosition, 1, glm::value_ptr(planePos));
	glUniform3fv(location_planeDirection, 1, glm::value_ptr(this->planeDirection));
	glUniform1i(location_showTex, showTexOnPlane ? 1 : 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, grid->gridTexture);
	glUniform1i(location_texData, 0);
}

void Scene::prepareUniforms_PlaneViewer(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, glm::vec2 offset, const GridGLView::Ptr& _grid) {
	glUseProgram(this->programHandle_PlaneViewer);
	// The BB used is the scene's bounding box :
	const DiscreteGrid::bbox_t::vec& bbox = this->sceneBB.getDiagonal();
	const DiscreteGrid::bbox_t::vec& posBox = this->sceneBB.getMin();

	glm::vec2 gridBBDims;
	if (_plane == planes::x) { gridBBDims.x = bbox.y; gridBBDims.y = bbox.z; }
	if (_plane == planes::y) { gridBBDims.x = bbox.x; gridBBDims.y = bbox.z; }
	if (_plane == planes::z) { gridBBDims.x = bbox.x; gridBBDims.y = bbox.y; }

	uint plane_heading = planeHeadingToIndex(_heading);

	// Grid transform :
	glm::mat4 gridTransform = _grid->grid[0]->getTransform_WorldToGrid();
	// Grid dimensions :
	glm::vec3 gridDimensions = glm::convert_to<glm::vec3::value_type>(_grid->grid[0]->getBoundingBox().getDiagonal());

	// Depth of the plane :
	glm::vec3 planePos = this->computePlanePositions();

	// Uniform locations :
	// VShader :
	GLint location_fbDims =					glGetUniformLocation(this->programHandle_PlaneViewer, "fbDims");
	GLint location_bbDims =					glGetUniformLocation(this->programHandle_PlaneViewer, "bbDims");
	GLint location_planeIndex =				glGetUniformLocation(this->programHandle_PlaneViewer, "planeIndex");
	GLint location_gridTransform =			glGetUniformLocation(this->programHandle_PlaneViewer, "gridTransform");
	GLint location_gridDimensions =			glGetUniformLocation(this->programHandle_PlaneViewer, "gridDimensions");
	GLint location_gridBBDiagonal =			glGetUniformLocation(this->programHandle_PlaneViewer, "sceneBBDiagonal");
	GLint location_gridBBPosition =			glGetUniformLocation(this->programHandle_PlaneViewer, "sceneBBPosition");
	GLint location_planePositions =			glGetUniformLocation(this->programHandle_PlaneViewer, "planePositions");
	GLint location_heading =				glGetUniformLocation(this->programHandle_PlaneViewer, "heading");
	GLint location_zoom =					glGetUniformLocation(this->programHandle_PlaneViewer, "zoom");
	GLint location_offset =					glGetUniformLocation(this->programHandle_PlaneViewer, "offset");
	// FShader :
	GLint location_texData =				glGetUniformLocation(this->programHandle_PlaneViewer, "texData");
	GLint location_colorBounds =			glGetUniformLocation(this->programHandle_PlaneViewer, "colorBounds");
	GLint location_colorBoundsAlternate =	glGetUniformLocation(this->programHandle_PlaneViewer, "colorBoundsAlternate");
	GLint location_textureBounds =			glGetUniformLocation(this->programHandle_PlaneViewer, "textureBounds");
	GLint location_textureBoundsAlternate = glGetUniformLocation(this->programHandle_PlaneViewer, "textureBoundsAlternate");
	GLint location_color0 =					glGetUniformLocation(this->programHandle_PlaneViewer, "color0");
	GLint location_color1 =					glGetUniformLocation(this->programHandle_PlaneViewer, "color1");
	GLint location_color0Alt =				glGetUniformLocation(this->programHandle_PlaneViewer, "color0Alternate");
	GLint location_color1Alt =				glGetUniformLocation(this->programHandle_PlaneViewer, "color1Alternate");
	GLint location_r_channelView =			glGetUniformLocation(this->programHandle_PlaneViewer, "r_channelView");
	GLint location_r_selectedChannel =		glGetUniformLocation(this->programHandle_PlaneViewer, "r_selectedChannel");
	GLint location_r_nbChannels =			glGetUniformLocation(this->programHandle_PlaneViewer, "r_nbChannels");
	GLint location_g_channelView =			glGetUniformLocation(this->programHandle_PlaneViewer, "g_channelView");
	GLint location_g_selectedChannel =		glGetUniformLocation(this->programHandle_PlaneViewer, "g_selectedChannel");
	GLint location_g_nbChannels =			glGetUniformLocation(this->programHandle_PlaneViewer, "g_nbChannels");

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
	glUniformMatrix4fv(location_gridTransform, 1, GL_FALSE, glm::value_ptr(gridTransform));
	glUniform4f(location_gridDimensions, gridDimensions.x, gridDimensions.y, gridDimensions.z, 1.f);
	glUniform4f(location_gridBBDiagonal, bbox.x, bbox.y, bbox.z, 1.f);
	glUniform4f(location_gridBBPosition, posBox.x, posBox.y, posBox.z, .0f);
	glUniform3fv(location_planePositions, 1, glm::value_ptr(planePos));
	glUniform2fv(location_colorBounds, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds0)));
	glUniform2fv(location_colorBoundsAlternate, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds1)));
	glUniform2fv(location_textureBounds, 1, glm::value_ptr(tb0));
	glUniform2fv(location_textureBoundsAlternate, 1, glm::value_ptr(tb1));
	glUniform1ui(location_heading, plane_heading);
	glUniform1f(location_zoom, zoomRatio);
	glUniform1ui(location_r_nbChannels, 1);
	glUniform1ui(location_g_nbChannels, 1);
	glm::vec2 realOffset = offset;
	realOffset.y = -realOffset.y;
	glUniform2fv(location_offset, 1, glm::value_ptr(realOffset));

	// Uniform samplers :
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, _grid->gridTexture);
	glUniform1i(location_texData, 0);
}

void Scene::drawGrid(GLfloat *mvMat, GLfloat *pMat, glm::mat4 baseMatrix, const GridGLView::Ptr& grid) {
	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	// If the grid has been uploaded, show it :
	if (grid->gridTexture > 0) {
		glUseProgram(this->programHandle_projectedTex);
		glBindVertexArray(this->vaoHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Element);

		this->prepareUniforms_3DSolid(mvMat, pMat, lightPos, baseMatrix, grid);
		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}

	// No matter the grid's state, print the bounding box :
	this->drawBoundingBox(grid->grid[0]->getBoundingBoxWorldSpace(), grid->boundingBoxColor, mvMat, pMat);
}

void Scene::draw3DView(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, bool showTexOnPlane) {
	if (this->shouldUpdateVis) {
		this->shouldUpdateVis = false;
		this->generateColorScale();
		this->uploadColorScale();
	}
	if (this->shouldDeleteGrid) {
		this->deleteGridNow();
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_3D);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 transfoMat = glm::mat4(1.f);

	if (this->drawMode == DrawMode::Solid) {
		for (std::size_t i = 0; i < this->grids.size(); ++i) {
			this->drawGrid(mvMat, pMat, transfoMat, this->grids[i]);
		}
	} else if (this->drawMode == DrawMode::Volumetric || this->drawMode == DrawMode::VolumetricBoxed) {
		for (std::size_t i = 0; i < this->grids.size(); ++i) {
			this->drawVolumetric(mvMat, pMat, camPos, this->grids[i]);
		}

		if (this->drawMode == DrawMode::VolumetricBoxed) {
			this->drawBoundingBox(this->visuBox, glm::vec3(1., .0, .0), mvMat, pMat);
		}
	}

	this->drawPlanes(mvMat, pMat, this->drawMode == DrawMode::Solid);
	this->drawBoundingBox(this->sceneBB, glm::vec4(.5, .5, .0, 1.), mvMat, pMat);
	this->showVAOstate = false;
}

void Scene::generateSceneData() {
	Mesh mesh{};
	this->generateTexCube(mesh);
	this->renderSize = mesh.indices.size();
	this->generatePlanesArray(mesh);
	this->setupVBOData(mesh);
	this->createBoundingBoxBuffers();
}

void Scene::generatePlanesArray(Mesh& _mesh) {
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
	glm::vec4 apos=glm::vec4(.0, .0, .0, 1.); glm::vec4 anorm=apos-center;
	glm::vec4 bpos=glm::vec4(1., .0, .0, 1.); glm::vec4 bnorm=bpos-center;
	glm::vec4 cpos=glm::vec4(.0, 1., .0, 1.); glm::vec4 cnorm=cpos-center;
	glm::vec4 dpos=glm::vec4(1., 1., .0, 1.); glm::vec4 dnorm=dpos-center;
	glm::vec4 epos=glm::vec4(.0, .0, 1., 1.); glm::vec4 enorm=epos-center;
	glm::vec4 fpos=glm::vec4(1., .0, 1., 1.); glm::vec4 fnorm=fpos-center;
	glm::vec4 gpos=glm::vec4(.0, 1., 1., 1.); glm::vec4 gnorm=gpos-center;
	// Create texture coordinates serving as framebuffer positions once all is done :
	glm::vec3 ll = glm::vec3(-1., -1., .0);	// lower-left
	glm::vec3 lr = glm::vec3(-1., 1., .0);	// lower-right
	glm::vec3 hl = glm::vec3(1., -1., .0);	// higher-left
	glm::vec3 hr = glm::vec3(1., 1., .0);	// higher-right

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

	// Push back enough indices to draw the planes all at once :
	for (unsigned int i = 0; i < _mesh.positions.size()-base; ++i) { _mesh.planar_view.push_back(base + i); }

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
	glm::vec3 ll = glm::vec3(-1., -1., .0);	// lower-left
	glm::vec3 lr = glm::vec3(-1., 1., .0);	// lower-right
	glm::vec3 hl = glm::vec3(1., -1., .0);	// higher-left
	glm::vec3 hr = glm::vec3(1., 1., .0);	// higher-right

	// Create arrays of 'positions' depending on the heading :
	std::vector<glm::vec3> north{ll, lr, hl, hl, lr, hr}; // default orientation
	std::vector<glm::vec3> east {lr, hr, ll, ll, hr, hl};
	std::vector<glm::vec3> west {hl, ll, hr, hr, ll, lr};
	std::vector<glm::vec3> south{hr, hl, lr, lr, hl, ll};

	// Get the offset into the buffer to read from :
	GLsizeiptr planeBegin = 0;
	QString planeName;
	switch (_plane) {
		case planes::x : planeBegin = 0; planeName = "X"; break;
		case planes::y : planeBegin = 6; planeName = "Y"; break;
		case planes::z : planeBegin =12; planeName = "Z"; break;
	}

	// Add 8 here because the 8 vertices of the cube will be placed
	// before the tex coordinates we're interested in :
	GLsizeiptr begin = (planeBegin + 8)*sizeof(glm::vec3);

	QString h;
	// Select the vertices' tex coordinates in order to draw it the 'right' way up :
	void* data = nullptr;
	switch(_heading) {
		case North: data = (void*) north.data(); h = "N"; break;
		case East : data = (void*) east.data();  h = "E"; break;
		case West : data = (void*) west.data();  h = "W"; break;
		case South: data = (void*) south.data(); h = "S"; break;
	}

	// Bind vertex texture coordinate buffer :
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertTex);
	// Modify a part of the data :
	glBufferSubData(GL_ARRAY_BUFFER, begin, 6*sizeof(glm::vec3), data);

	if (this->programStatusBar != nullptr) {
		QString message = "Plane " + planeName + " is now oriented " + h;
		this->programStatusBar->showMessage(message, 5000);
	}

	return;
}

void Scene::generateTexCube(Mesh& _mesh) {
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
	// Build position, normals, and tex coordinates all in one line for each vertex
	glm::vec4 apos=glm::vec4(.0, .0, .0, 1.); glm::vec4 anorm=apos-center; glm::vec3 atex = glm::vec3(.0, .0, .0);
	glm::vec4 bpos=glm::vec4(1., .0, .0, 1.); glm::vec4 bnorm=bpos-center; glm::vec3 btex = glm::vec3(1., .0, .0);
	glm::vec4 cpos=glm::vec4(.0, 1., .0, 1.); glm::vec4 cnorm=cpos-center; glm::vec3 ctex = glm::vec3(.0, 1., .0);
	glm::vec4 dpos=glm::vec4(1., 1., .0, 1.); glm::vec4 dnorm=dpos-center; glm::vec3 dtex = glm::vec3(1., 1., .0);
	glm::vec4 epos=glm::vec4(.0, .0, 1., 1.); glm::vec4 enorm=epos-center; glm::vec3 etex = glm::vec3(.0, .0, 1.);
	glm::vec4 fpos=glm::vec4(1., .0, 1., 1.); glm::vec4 fnorm=fpos-center; glm::vec3 ftex = glm::vec3(1., .0, 1.);
	glm::vec4 gpos=glm::vec4(.0, 1., 1., 1.); glm::vec4 gnorm=gpos-center; glm::vec3 gtex = glm::vec3(.0, 1., 1.);
	glm::vec4 hpos=glm::vec4(1., 1., 1., 1.); glm::vec4 hnorm=hpos-center; glm::vec3 htex = glm::vec3(1., 1., 1.);
	_mesh.positions.push_back(apos); _mesh.normals.push_back(anorm); _mesh.texture.push_back(atex);
	_mesh.positions.push_back(bpos); _mesh.normals.push_back(bnorm); _mesh.texture.push_back(btex);
	_mesh.positions.push_back(cpos); _mesh.normals.push_back(cnorm); _mesh.texture.push_back(ctex);
	_mesh.positions.push_back(dpos); _mesh.normals.push_back(dnorm); _mesh.texture.push_back(dtex);
	_mesh.positions.push_back(epos); _mesh.normals.push_back(enorm); _mesh.texture.push_back(etex);
	_mesh.positions.push_back(fpos); _mesh.normals.push_back(fnorm); _mesh.texture.push_back(ftex);
	_mesh.positions.push_back(gpos); _mesh.normals.push_back(gnorm); _mesh.texture.push_back(gtex);
	_mesh.positions.push_back(hpos); _mesh.normals.push_back(hnorm); _mesh.texture.push_back(htex);
	unsigned int a = 0; unsigned int b = 1; unsigned int c = 2; unsigned int d = 3;
	unsigned int e = 4; unsigned int f = 5; unsigned int g = 6; unsigned int h = 7;
	unsigned int faceIdx1[]	= {a, d, c, a, b, d, e, b, a, e, f, b, g, e, a, g, a, c};
	unsigned int faceIdx2[] = {h, e, g, h, f, e, h, g, c, h, c, d, h, d, b, h, f, b};
	_mesh.indices.insert(_mesh.indices.end(), faceIdx1, faceIdx1+18);
	_mesh.indices.insert(_mesh.indices.end(), faceIdx2, faceIdx2+18);
}

void Scene::setupVBOData(const Mesh& _mesh) {
	if (glIsVertexArray(this->vaoHandle) == GL_FALSE) { throw std::runtime_error("The vao handle generated by OpenGL has been invalidated !"); }

	// UPLOAD DATA (VBOs have previously been created in createBuffers() :

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertPos);
	glBufferData(GL_ARRAY_BUFFER, _mesh.positions.size()*sizeof(glm::vec4), _mesh.positions.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertNorm);
	glBufferData(GL_ARRAY_BUFFER, _mesh.normals.size()*sizeof(glm::vec4), _mesh.normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertTex);
	glBufferData(GL_ARRAY_BUFFER, _mesh.texture.size()*sizeof(glm::vec3), _mesh.texture.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size()*sizeof(unsigned int), _mesh.indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.cutting_planes.size()*sizeof(unsigned int), _mesh.cutting_planes.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_SinglePlaneElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.planar_view.size() * sizeof(unsigned int), _mesh.planar_view.data(), GL_STATIC_DRAW);

	this->setupVAOPointers();
}

void Scene::setupVAOPointers() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertPos);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertNorm);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertTex);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

glm::vec3 Scene::getSceneBoundaries() const {
	return this->sceneBB.getDiagonal();
}

void Scene::printProgramUniforms(const GLuint _pid) {
	const GLsizei bufSize = 256; // maximum name length
	GLchar name[bufSize]; // variable name in GLSL
	GLsizei length = 0; // name length
	GLint attrCount = 0; // the count of attributes/uniforms
	GLenum type = GL_NONE; // type of the variable (float, vec3 or mat4, etc)
	GLint size = 0; // size of the variable

	std::cerr << "[TRACE] Printing attributes of the program " << +_pid << '\n';
	glUseProgram(_pid);

	std::cerr << "[TRACE]\t\tActive attributes :\n";
	glGetProgramiv(_pid, GL_ACTIVE_ATTRIBUTES, &attrCount);
	for (GLint i = 0; i < attrCount; ++i ) {
		//std::cerr << "[TRACE]\t" << ;
		glGetActiveAttrib(_pid, (GLuint)i, bufSize, &length, &size, &type, name);
		fprintf(stderr, "[TRACE]\t\t\tAttribute #%d Type: %u Name: %s\n", i, type, name);
	}

	std::cerr << "[TRACE]\t\tActive uniforms :\n";
	glGetProgramiv(_pid, GL_ACTIVE_UNIFORMS, &attrCount);
	for (GLint i = 0; i < attrCount; ++i ) {
		glGetActiveUniform(_pid, (GLuint)i, bufSize, &length, &size, &type, name);
		fprintf(stderr, "[TRACE]\t\t\tUniform #%d Type: %u Name: %s\n", i, type, name);
	}
}

glm::mat4 Scene::computeTransformationMatrix(const std::shared_ptr<DiscreteGrid>& _grid) const {
	glm::mat4 transfoMat = glm::mat4(1.0);

	double angleDeg = -45.;
	double angleRad = (angleDeg * M_PI) / 180.;

	if (angleDeg < 0.) {
		DiscreteGrid::sizevec3 d = _grid->getResolution();
		// compute translation along Z :
		float w = static_cast<float>(d.x) * .39;
		float displacement = w * std::abs(std::sin(angleRad));
		transfoMat = glm::translate(transfoMat, glm::vec3(.0, .0, displacement));
	}

	transfoMat[0][0] = 0.39 * std::cos(angleRad);
	transfoMat[0][2] = 0.39 * std::sin(angleRad);
	transfoMat[1][1] = 0.39;
	transfoMat[2][2] = 1.927 * std::cos(angleRad);

	/*
	std::cerr << "Matrix : " << '\n';
	for (int i = 0; i < 4; ++i) {
		std::cerr << '{' << transfoMat[i][0] << ", " << transfoMat[i][1] << ", " << transfoMat[i][2] << ", " << transfoMat[i][3] << "}\n";
	}
	std::cerr << '\n';
	*/

	return transfoMat;
	// return glm::mat4(1.f);
	// return glm::translate(glm::mat4(1.f), glm::vec3(.0, .0, 20.));
}

void Scene::generateColorScale() {
	for (DiscreteGrid::data_t i = 0; i < this->textureBounds0.x; ++i) { this->visibleDomains[i] = 0.f; }
	for (DiscreteGrid::data_t i = this->textureBounds0.x; i < this->textureBounds0.y; ++i) {
		this->visibleDomains[i] = 1.f;
	}
	for (DiscreteGrid::data_t i = this->textureBounds0.y; i < std::numeric_limits<DiscreteGrid::data_t>::max(); ++i) {
		this->visibleDomains[i] = 0.f;
	}

	for (DiscreteGrid::data_t i = 0; i < this->textureBounds1.x; ++i) { this->visibleDomainsAlternate[i] = 0.f; }
	for (DiscreteGrid::data_t i = this->textureBounds1.x; i < this->textureBounds1.y; ++i) {
		this->visibleDomainsAlternate[i] = 1.f;
	}
	for (DiscreteGrid::data_t i = this->textureBounds1.y; i < std::numeric_limits<DiscreteGrid::data_t>::max(); ++i) {
		this->visibleDomainsAlternate[i] = 0.f;
	}
}

void Scene::uploadColorScale() {
	TextureUpload texParams = {};

	if (this->texHandle_ColorScaleGrid != 0) {
		if (glIsTexture(this->texHandle_ColorScaleGrid) == GL_TRUE) {
			glDeleteTextures(1, &this->texHandle_ColorScaleGrid);
		} else {
			std::cerr << "[LOG] Texture handle for visibility was not 0, but not a texture either.\n";
		}
		this->texHandle_ColorScaleGrid = 0;
	}
	if (this->texHandle_ColorScaleGridAlternate != 0) {
		if (glIsTexture(this->texHandle_ColorScaleGridAlternate) == GL_TRUE) {
			glDeleteTextures(1, &this->texHandle_ColorScaleGridAlternate);
		} else {
			std::cerr << "[LOG] Texture handle for visibility was not 0, but not a texture either.\n";
		}
		this->texHandle_ColorScaleGridAlternate = 0;
	}

	// Vertex positions :
	texParams.minmag.x = GL_NEAREST;
	texParams.minmag.y = GL_NEAREST;
	texParams.lod.y = -1000.f;
	texParams.wrap.s = GL_CLAMP_TO_EDGE;
	texParams.wrap.t = GL_CLAMP_TO_EDGE;
	texParams.swizzle.y = GL_ZERO;
	texParams.swizzle.z = GL_ZERO;
	texParams.swizzle.a = GL_ONE;
	// Swizzle and alignment unchanged, not present in Texture3D
	texParams.internalFormat = GL_R32F;
	texParams.size.x = 256;
	#ifdef VISUALISATION_USE_UINT16
	texParams.size.y = 256;
	#else
	texParams.size.y = 1;
	#endif
	texParams.format = GL_RED;
	texParams.type = GL_FLOAT;
	texParams.data = this->visibleDomains;
	this->texHandle_ColorScaleGrid = this->uploadTexture2D(texParams);
	texParams.data = this->visibleDomainsAlternate;
	this->texHandle_ColorScaleGridAlternate = this->uploadTexture2D(texParams);
}

void Scene::updateVis() {
	// will update on the next draw call, where the countext is sure to be bound
	this->shouldUpdateVis = true;
}

void Scene::createBoundingBoxBuffers() {
	/**
	 * The bounding boxes will be drawn using GL_LINES. As such, we have to get all the positions of the corners of
	 * the bounding boxes uploaded to OpenGL, and then use an index buffer to make the lines to be drawn. (12 lines
	 * in total to draw a 'cube').
	 */
	// Create a basic bounding box :
	DiscreteGrid::bbox_t defaultBB = this->sceneBB;
	// Get all corners and put them sequentially in an array :
	std::vector<DiscreteGrid::bbox_t::vec> corners = defaultBB.getAllCorners();
	GLfloat* rawVertices = new GLfloat[corners.size()*3];
	for (std::size_t i = 0; i < corners.size(); ++i) {
		rawVertices[3*i + 0] = corners[i].x;
		rawVertices[3*i + 1] = corners[i].y;
		rawVertices[3*i + 2] = corners[i].z;
	}

	/**
	 * The corners of a BB are given in a special order (see BoundingBox_General::getAllCorners())
	 * The lines are constructed from the indices
	 */
	GLuint a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6, h = 7;
	GLuint rawIndices[] = {a, b, b, d, d, c, c, a, e, f, f, h, h, g, g, e, a, e, b, f, c, g, d, h};

	// The VAO and VBOs have been created previously in createBuffers(). No need to create them again here.

	// Bind the vertex buffer :
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_boundingBoxVertices);
	// Upload data to OpenGL :
	glBufferData(GL_ARRAY_BUFFER, corners.size()*3*sizeof(GLfloat), rawVertices, GL_STATIC_DRAW);

	// Bind the index buffer :
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_boundingBoxIndices);
	// Upload data to OpenGL :
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24*sizeof(GLuint), &(rawIndices[0]), GL_STATIC_DRAW);
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
	glBindVertexArray(this->vaoHandle_boundingBox);

	// Bind vertex buffer :
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_boundingBoxVertices);
	// Enable VAO pointer 0 : vertices
	glEnableVertexAttribArray(0);
	// Enable VAO pointer for 3-component vectors, non-normalized starting at index 0 :
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

void Scene::drawBoundingBox(const DiscreteGrid::bbox_t& _box, glm::vec3 color, GLfloat* vMat, GLfloat* pMat) {
	glUseProgram(this->programHandle_BoundingBox);

	glLineWidth(2.0f);

	// Locate uniforms :
	GLint location_pMat = glGetUniformLocation(this->programHandle_BoundingBox, "pMat");
	GLint location_vMat = glGetUniformLocation(this->programHandle_BoundingBox, "vMat");
	GLint location_bbColor = glGetUniformLocation(this->programHandle_BoundingBox, "bbColor");
	GLint location_bbSize = glGetUniformLocation(this->programHandle_BoundingBox, "bbSize");
	GLint location_bbPos = glGetUniformLocation(this->programHandle_BoundingBox, "bbPos");

	DiscreteGrid::bbox_t::vec min = _box.getMin();
	DiscreteGrid::bbox_t::vec diag = _box.getDiagonal();

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

	glBindVertexArray(this->vaoHandle_boundingBox);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_boundingBoxIndices);
	this->setupVAOBoundingBox();

	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

void Scene::showVisuBoxController() {
	if (this->visuBoxController == nullptr) {
		this->visuBoxController = new VisuBoxController(this);
	}
	this->visuBoxController->show();
}

void Scene::removeVisuBoxController() {
	this->visuBoxController = nullptr;
}

DiscreteGrid::bbox_t Scene::getVisuBox() {
	glm::vec3 min = this->computePlanePositions();
	glm::vec3 max = this->visuBox.getMax();
	this->visuBox = DiscreteGrid::bbox_t();
	this->visuBox.addPoint(min);
	this->visuBox.addPoint(max);
	return this->visuBox;
}

void Scene::setVisuBox(DiscreteGrid::bbox_t box) {
	this->visuBox = box;
	return;
}

void Scene::resetVisuBox() {
	this->visuBox = this->sceneDataBB;
	return;
}

float Scene::getSceneRadius() {
	// in case of box visu, it's easy :
	if (this->drawMode == VolumetricBoxed) { return glm::length(this->visuBox.getDiagonal()); }
	// if draw solid or volumetric, focus on the part of the scene's BB we want to see
	// need to compute plane positions, and get the god min/max points according to it
	if (this->drawMode == Solid || this->drawMode == Volumetric) {
		// get plane positions :
		glm::vec3 bbmin = this->sceneBB.getMin();
		glm::vec3 bbmax = this->sceneBB.getMax();
		glm::vec3 planePos = (this->sceneBB.getMin() + this->planeDisplacement * this->sceneBB.getDiagonal());
		glm::vec3 min = glm::vec3(), max = glm::vec3();

		if (this->planeDirection.x < 0) { min.x = bbmin.x; max.x = planePos.x; } else { min.x = planePos.x; max.x = bbmax.x; }
		if (this->planeDirection.y < 0) { min.y = bbmin.y; max.y = planePos.y; } else { min.y = planePos.y; max.y = bbmax.y; }
		if (this->planeDirection.z < 0) { min.z = bbmin.z; max.z = planePos.z; } else { min.z = planePos.z; max.z = bbmax.z; }

		glm::vec3 diag = max - min;
		return glm::length(diag);
	}
	//default value when no things are loaded
	return 1.f;
}

glm::vec3 Scene::getSceneCenter() {
	// in case of box visu, it's easy :
	if (this->drawMode == VolumetricBoxed) { return this->visuBox.getMin() + (this->visuBox.getDiagonal()/2.f); }
	// if draw solid, or volumetric focus on the part of the scene's BB we want to see
	// need to compute plane positions, and get the god min/max points according to it
	if (this->drawMode == Solid || this->drawMode == Volumetric) {
		// get plane positions :
		glm::vec3 bbmin = this->sceneBB.getMin();
		glm::vec3 bbmax = this->sceneBB.getMax();
		glm::vec3 planePos = (this->sceneBB.getMin() + this->planeDisplacement * this->sceneBB.getDiagonal());
		glm::vec3 min = glm::vec3(), max = glm::vec3();

		if (this->planeDirection.x < 0) { min.x = bbmin.x; max.x = planePos.x; } else { min.x = planePos.x; max.x = bbmax.x; }
		if (this->planeDirection.y < 0) { min.y = bbmin.y; max.y = planePos.y; } else { min.y = planePos.y; max.y = bbmax.y; }
		if (this->planeDirection.z < 0) { min.z = bbmin.z; max.z = planePos.z; } else { min.z = planePos.z; max.z = bbmax.z; }

		glm::vec3 diag = max - min;
		return min + diag/2.f;
	}
	// default value, for no
	return glm::vec3(.5, .5, .5);
}

uint Scene::colorFunctionToUniform(ColorFunction _c) {
	switch (_c) {
		case ColorFunction::SingleChannel: return 1;
		case ColorFunction::HistologyHandE: return 2;
		case ColorFunction::HSV2RGB: return 3;
		case ColorFunction::ColorMagnitude: return 4;
	}
	return 0;
}

void Scene::setColorFunction_r(ColorFunction _c) {
	if (_c == ColorFunction::SingleChannel && this->channels_r != ColorFunction::SingleChannel) {
		this->selectedChannel_r = 0;
	}
	if (_c == ColorFunction::SingleChannel && this->channels_r == ColorFunction::SingleChannel) {
		this->selectedChannel_r = (this->selectedChannel_r+1)%2;
	}
	this->channels_r = _c;
}

void Scene::setColorFunction_g(ColorFunction _c) {
	if (_c == ColorFunction::SingleChannel && this->channels_g != ColorFunction::SingleChannel) {
		this->selectedChannel_g = 0;
	}
	if (_c == ColorFunction::SingleChannel && this->channels_g == ColorFunction::SingleChannel) {
		this->selectedChannel_g = (this->selectedChannel_g+1)%2;
	}
	this->channels_g = _c;
}

void Scene::setRGBMode(RGBMode _mode) {
	this->rgbMode = _mode;
	switch (_mode) {
		case RGBMode::None : std::cerr << "Set mode to none" << '\n'; break;
		case RGBMode::RedOnly : std::cerr << "Set mode to red" << '\n'; break;
		case RGBMode::GreenOnly : std::cerr << "Set mode to green" << '\n'; break;
		case RGBMode::RedAndGreen: std::cerr << "Set mode to both channels" << '\n'; break;
		default: std::cerr << "Cannot set unknown mode\n"; break;
	}
}

DiscreteGrid::bbox_t Scene::getSceneBoundingBox() const { return this->sceneBB; };

void Scene::slotSetPlaneDisplacementX(float scalar) { this->planeDisplacement.x = scalar; }
void Scene::slotSetPlaneDisplacementY(float scalar) { this->planeDisplacement.y = scalar; }
void Scene::slotSetPlaneDisplacementZ(float scalar) { this->planeDisplacement.z = scalar; }

void Scene::slotTogglePlaneDirectionX() { this->planeDirection.x = - this->planeDirection.x; }
void Scene::slotTogglePlaneDirectionY() { this->planeDirection.y = - this->planeDirection.y; }
void Scene::slotTogglePlaneDirectionZ() { this->planeDirection.z = - this->planeDirection.z; }
void Scene::toggleAllPlaneDirections() { this->planeDirection = - this->planeDirection; }

void Scene::slotSetMinTexValue(DiscreteGrid::data_t val) { this->textureBounds0.x = val; this->updateVis(); }
void Scene::slotSetMaxTexValue(DiscreteGrid::data_t val) { this->textureBounds0.y = val; this->updateVis(); }
void Scene::slotSetMinTexValueAlternate(DiscreteGrid::data_t val) { this->textureBounds1.x = val; this->updateVis(); }
void Scene::slotSetMaxTexValueAlternate(DiscreteGrid::data_t val) { this->textureBounds1.y = val; this->updateVis(); }

void Scene::slotSetMinColorValue(DiscreteGrid::data_t val) { this->colorBounds0.x = val; }
void Scene::slotSetMaxColorValue(DiscreteGrid::data_t val) { this->colorBounds0.y = val; }
void Scene::slotSetMinColorValueAlternate(DiscreteGrid::data_t val) { this->colorBounds1.x = val; }
void Scene::slotSetMaxColorValueAlternate(DiscreteGrid::data_t val) { this->colorBounds1.y = val; }

void Scene::setColor0(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r,g,b);
	this->color0 = glm::convert_to<float>(qtcolor);
	return;
}

void Scene::setColor1(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r,g,b);
	this->color1 = glm::convert_to<float>(qtcolor);
	return;
}

void Scene::setColor0Alternate(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r,g,b);
	this->color0_second = glm::convert_to<float>(qtcolor);
	return;
}

void Scene::setColor1Alternate(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r,g,b);
	this->color1_second = glm::convert_to<float>(qtcolor);
	return;
}

void Scene::setDrawMode(DrawMode _mode) {
	this->drawMode = _mode;
	if (this->programStatusBar != nullptr) {
		switch (_mode) {
			case DrawMode::Solid :
				this->programStatusBar->showMessage("Set draw mode to Solid.\n", 5000);
			break;
			case DrawMode::Volumetric :
				this->programStatusBar->showMessage("Set draw mode to Volumetric.\n", 5000);
			break;
			case DrawMode::VolumetricBoxed :
				this->programStatusBar->showMessage("Set draw mode to VolumetricBoxed.\n", 5000);
				this->showVisuBoxController();
			break;
		}
	}
}

void Scene::togglePlaneVisibility(planes _plane) {
	if (_plane == planes::x) { this->planeVisibility.x = not this->planeVisibility.x; }
	if (_plane == planes::y) { this->planeVisibility.y = not this->planeVisibility.y; }
	if (_plane == planes::z) { this->planeVisibility.z = not this->planeVisibility.z; }
	std::cerr << "Visibilities : {" << std::boolalpha << this->planeVisibility.x << ',' <<
		     this->planeVisibility.y << ',' << this->planeVisibility.z << "}\n";
	return;
}

void Scene::toggleAllPlaneVisibilities() {
	this->planeVisibility.x = not this->planeVisibility.x;
	this->planeVisibility.y = not this->planeVisibility.y;
	this->planeVisibility.z = not this->planeVisibility.z;
}

void Scene::tex3D_buildMesh(GridGLView::Ptr& grid, const std::string path) {
	/* Here, we'll build the tetrahedral mesh used to visualize the 3D structure of the acquisition in real-time. */
	/* Textures built : index of vertices, index of per-face normals for tetrahedra, index of per-face neighbors
	 * for tetrehedra */

	VolMeshData mesh{};

	if (path.empty()) {
		this->tex3D_generateMESH(grid, mesh);
	} else {
		this->tex3D_loadMESHFile(path, grid, mesh);
	}

	// Hard-coded indices smh, got to move to a more portable (and readable) solution :
	std::size_t indices[4][3];
	indices[0][0] = 3; indices[0][1] = 1; indices[0][2] = 2;
	indices[1][0] = 3; indices[1][1] = 2; indices[1][2] = 0;
	indices[2][0] = 3; indices[2][1] = 0; indices[2][2] = 1;
	indices[3][0] = 2; indices[3][1] = 1; indices[3][2] = 0;

	// INIT NEIGHBORS :
	mesh.neighbors.clear();
	mesh.neighbors.resize(mesh.tetrahedra.size());
	for(std::size_t i = 0 ; i < mesh.tetrahedra.size() ; i++ ){
		mesh.neighbors[i].resize(4, -1);
		mesh.normals.push_back(std::array<glm::vec4, 4>{glm::vec4(.0f), glm::vec4(.0f), glm::vec4(.0f), glm::vec4(.0f)});
	}

	// Adjacency map :
	std::map< Face , std::pair<int, int> > adjacent_faces;
	// generate the correspondance by looking at which faces are similar : [MEDIUM/HEAVY COMPUTATION]
	for(std::size_t i = 0 ; i < mesh.tetrahedra.size() ; i ++ ){
		for( int v = 0 ; v < 4 ; v++ ){
			// find similar face in other tetrahedrons :
			Face face = Face(mesh.tetrahedra[i][indices[v][0]],
					mesh.tetrahedra[i][indices[v][1]],
					mesh.tetrahedra[i][indices[v][2]]);
			std::map< Face , std::pair<int, int> >::iterator it = adjacent_faces.find(face);
			if( it == adjacent_faces.end() ){
				adjacent_faces[face] = std::make_pair(static_cast<int>(i),v);
			} else {
				mesh.neighbors[i][v] = it->second.first;
				mesh.neighbors[it->second.first][it->second.second] = i;
			}
		}
	}

	// determine the size of the texture, depending on the # of tetrahedron neighbors :
	std::size_t vertWidth = 0, vertHeight = 0;
	std::size_t normWidth = 0, normHeight = 0;
	std::size_t coorWidth = 0, coorHeight = 0;
	std::size_t neighbWidth = 0, neighbHeight = 0;

	__GetTexSize(mesh.tetrahedra.size()*4*3, &vertWidth, &vertHeight);
	__GetTexSize(mesh.tetrahedra.size()*4, &normWidth, &normHeight);
	__GetTexSize(mesh.tetrahedra.size()*4*3, &coorWidth, &coorHeight);
	__GetTexSize(mesh.tetrahedra.size()*4, &neighbWidth, &neighbHeight);

	grid->volumetricMesh.tetrahedraCount = mesh.tetrahedra.size();

	#ifdef DIRECT_FROM_VOLMESH_TO_ARRAY_SIZE
	GLfloat* rawVertices = new GLfloat[grid.volumetricMesh.tetrahedraCount*4*3*3];
	GLfloat* rawNormals = new GLfloat[grid.volumetricMesh.tetrahedraCount*4*4];
	GLfloat* tex = new GLfloat[grid.volumetricMesh.tetrahedraCount*4*3*3];
	GLfloat* rawNeighbors = new GLfloat[grid.volumetricMesh.tetrahedraCount*4*3];
	#else
	GLfloat* rawVertices = new GLfloat[vertWidth * vertHeight *3];
	GLfloat* rawNormals = new GLfloat[normWidth * normHeight *4];
	GLfloat* tex = new GLfloat[coorWidth * coorHeight *3];
	GLfloat* rawNeighbors = new GLfloat[neighbWidth * neighbHeight *3];
	#endif

	/*
	Warning : the loop that follows is horribly bad : it duplicates every texture coordinate by the number of times
	the vertex is used within the mesh. It MUST be improved once the method is working, to save up on video memory.
	It does the same for the vertex positions.
	*/
	std::size_t iter = 0;
	std::size_t texcnt = 0;
	std::size_t ncount = 0;
	for(std::size_t t = 0; t < mesh.tetrahedra.size(); ++t) {
		// For each tet :
		const std::array<std::size_t, 4>& tetrahedron = mesh.tetrahedra[t];
		// For each tet's face :
		for (std::size_t i = 0; i < 4; ++i) {
			// For all vertices within that face :
			for (std::size_t j = 0; j < 3; ++j) {
				// Get the vertex's position and tex coord :
				const glm::vec4& position = mesh.positions[tetrahedron[indices[i][j]]];
				const glm::vec3& tetCoord = mesh.texture[tetrahedron[indices[i][j]]];
				// And put it in the array :
				for (std::size_t k = 0; k < 3; ++k) {
					rawVertices[texcnt] = position[k];
					tex[texcnt] = tetCoord[k];
					texcnt++;
				}
			}

			// Compute this face's normal :
			glm::vec4 n1 = mesh.positions[tetrahedron[indices[i][1]]] - mesh.positions[tetrahedron[indices[i][0]]];
			glm::vec4 n2 = mesh.positions[tetrahedron[indices[i][2]]] - mesh.positions[tetrahedron[indices[i][0]]];
			glm::vec4 norm = glm::normalize(glm::cross(n1, n2));
			// Put inverse of dot with opposing vertex in norm.w :
			glm::vec4 v1 = mesh.positions[tetrahedron[i]] - mesh.positions[tetrahedron[(i+1)%4]];
			glm::vec4::value_type val = 1. / glm::dot(v1, norm);
			norm.w = val;
			// Put it in the array :
			for (std::size_t n = 0; n < 4; ++n) {
				rawNormals[ncount++] = norm[n];
			}

			rawNeighbors[iter] = static_cast<GLfloat>(mesh.neighbors[t][i]);
			iter+=3;
		}
	}

	// Struct to upload the texture to OpenGL :
	TextureUpload texParams = {};

	// Vertex positions :
	texParams.minmag.x = GL_NEAREST;
	texParams.minmag.y = GL_NEAREST;
	texParams.lod.y = -1000.f;
	texParams.wrap.s = GL_CLAMP;
	texParams.wrap.t = GL_CLAMP;
	// Swizzle and alignment unchanged, not present in Texture3D
	texParams.internalFormat = GL_RGB32F;
	texParams.size.x = vertWidth;
	texParams.size.y = vertHeight;
	texParams.format = GL_RGB;
	texParams.type = GL_FLOAT;
	texParams.data = rawVertices;
	grid->volumetricMesh.vertexPositions = this->uploadTexture2D(texParams);

	// Face normals :
	texParams.internalFormat = GL_RGBA32F;
	texParams.size.x = normWidth;
	texParams.size.y = normHeight;
	texParams.format = GL_RGBA;
	texParams.data = rawNormals;
	grid->volumetricMesh.faceNormals = this->uploadTexture2D(texParams);

	// Texture coordinates :
	texParams.internalFormat = GL_RGB32F;
	texParams.size.x = coorWidth;
	texParams.size.y = coorHeight;
	texParams.format = GL_RGB;
	texParams.data = tex;
	grid->volumetricMesh.textureCoordinates = this->uploadTexture2D(texParams);

	// Neighborhood information :
	texParams.size.x = neighbWidth;
	texParams.size.y = neighbHeight;
	texParams.data = rawNeighbors;
	grid->volumetricMesh.neighborhood = this->uploadTexture2D(texParams);

	delete[] tex;
	delete[] rawVertices;
	delete[] rawNormals;
	delete[] rawNeighbors;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Scene::tex3D_loadMESHFile(const std::string file, const GridGLView::Ptr& grid, VolMeshData& mesh) {
	DiscreteGrid::bbox_t box = grid->grid[0]->getBoundingBox();

	std::ifstream myfile(file.c_str());

	if (not myfile.is_open()) {
		throw std::runtime_error("could not open mesh file");
	}

	std::string meshString;
	std::size_t sizeV, sizeT, sizeTet, dimension;

	myfile >> meshString;
	while (meshString.find("Dimension") == std::string::npos)
		myfile >> meshString;

	myfile>> dimension;
	while (meshString.find("Vertices") == std::string::npos)
		myfile >> meshString;

	myfile >> sizeV;

	float maxX = 0, maxY = 0, maxZ = 0;

	std::vector<glm::vec4> rawVert;
	DiscreteGrid::bbox_t normalBox;
	DiscreteGrid::bbox_t deformedBox;

	int s;
	for (unsigned int i = 0; i < sizeV; i++) {
		float p[3];
		for (unsigned int j = 0; j < 3; j++)
			myfile >> p[j];

		if (p[0] > maxX) { maxX = p[0]; }
		if (p[1] > maxY) { maxY = p[1]; }
		if (p[2] > maxZ) { maxZ = p[2]; }
		glm::vec4 position = glm::vec4(p[0], p[1], p[2], 1.);
		normalBox.addPoint(DiscreteGrid::bbox_t::vec(p[0],p[1],p[2]));
		rawVert.push_back(position);
		position = grid->grid[0]->getTransform_GridToWorld() * position;
		deformedBox.addPoint(DiscreteGrid::bbox_t::vec(position.x, position.y, position.z));
		mesh.positions.push_back(position);
/*
		texCoords.push_back(glm::vec3(
			std::min(1.f,(p[0] - bmin.x)/diag.x),
			std::min(1.f,(p[1] - bmin.y)/diag.y),
			std::min(1.f,(p[2] - bmin.z)/diag.z)
		));
*/
		myfile >> s;
	}

	// Current mesh size :
	glm::vec3 size{maxX, maxY, maxZ};
	// Grid size divided by mesh size gives a scalar to apply to the mesh to make it grid-sized :
	glm::vec3 gridsize = glm::convert_to<float>(grid->grid[0]->getResolution()) * grid->grid[0]->getVoxelDimensions();
	glm::vec4 scale = glm::vec4(gridsize / size, 1.);

	std::cerr << "[LOG][" << __FILE__ << ':' << __LINE__ << "] Max dimensions of the mesh : [" << maxX << ',' << maxY << ',' << maxZ << "]\n";
	for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
		//mesh.positions[i] *= scale;
		mesh.texture.push_back(glm::vec3(
			std::min(1.f,rawVert[i].x/size.x),
			std::min(1.f,rawVert[i].y/size.y),
			std::min(1.f,rawVert[i].z/size.z)
		));
	}
	std::cerr << "done.\n";

	rawVert.clear();

	while (meshString.find("Triangles") == std::string::npos)
		myfile >> meshString;

	myfile >> sizeT;
	unsigned int v[3];
	for (unsigned int i = 0; i < sizeT; i++) {
		for (unsigned int j = 0; j < 3; j++)
			myfile >> v[j];
		myfile >> s;
	}

	if( dimension == 3 ){
		while (meshString.find("Tetrahedra") == std::string::npos)
			myfile >> meshString;

		myfile >> sizeTet;
		unsigned int t[4];
		for (unsigned int i = 0; i < sizeTet; i++) {
			for (unsigned int j = 0; j < 4; j++) {
				myfile >> t[j];
			}
			myfile >> s;
			if( s > 0 ) {
				mesh.tetrahedra.push_back({t[0]-1, t[1]-1, t[2]-1, t[3]-1});
			}
		}
	}
	std::cerr << "loadMesh() done\n";

	myfile.close ();
}

void Scene::tex3D_generateMESH(GridGLView::Ptr& grid, VolMeshData& mesh) {
	// typedef for bounding box's vector type :
	using vec_t = typename DiscreteGrid::bbox_t::vec;

	//Min and diagonal of the bounding box (used for position computation) :
	const vec_t min = grid->grid[0]->getBoundingBox().getMin();
	const vec_t diag = grid->grid[0]->getBoundingBox().getDiagonal();
	// Dimensions, subject to change :
	std::size_t xv = 12 ; glm::vec4::value_type xs = diag.x / static_cast<glm::vec4::value_type>(xv);
	std::size_t yv = 12 ; glm::vec4::value_type ys = diag.y / static_cast<glm::vec4::value_type>(yv);
	std::size_t zv = 12 ; glm::vec4::value_type zs = diag.z / static_cast<glm::vec4::value_type>(zv);

	// Size of tetrahedra, to compute epsilon :
	glm::vec3 epsVolu = glm::vec3(xs, ys, zs);
	grid->defaultEpsilon = epsVolu * .9f;

	// Containers for the computation of positions and texture coordinates :
	glm::vec4 pos = glm::vec4();
	glm::vec3 tex = glm::vec3();
	// Transformation to apply to the mesh :
	glm::mat4 transfo = grid->grid[0]->getTransform_GridToWorld();

	// Create vertices along with their texture coordinates. We
	// need to go one after because we want _n_ tetrahedra, and
	// thus must finish the last 'row'/'column' of tetrahedra :
	for (std::size_t k = 0; k <= zv; ++k) {
		for (std::size_t j = 0; j <= yv; ++j) {
			for (std::size_t i = 0; i <= xv; ++i) {
				pos = transfo * glm::vec4(
					min.x + static_cast<vec_t::value_type>(i)*xs,
					min.y + static_cast<vec_t::value_type>(j)*ys,
					min.z + static_cast<vec_t::value_type>(k)*zs,
					1.
				);
				tex = glm::vec3(
					static_cast<vec_t::value_type>(i)/static_cast<vec_t::value_type>(xv),
					static_cast<vec_t::value_type>(j)/static_cast<vec_t::value_type>(yv),
					static_cast<vec_t::value_type>(k)/static_cast<vec_t::value_type>(zv)
				);

				mesh.positions.push_back(pos);
				mesh.texture.push_back(tex);
			}
		}
	}

	// Return position for vertex generated at indices I, J, K :
	std::size_t xt = xv+1; std::size_t yt = yv+1;
	auto getIndice = [&, xt, yt](std::size_t i, std::size_t j, std::size_t k) -> std::size_t {
		return i + j * xt + k * xt * yt;
	};

	// Create tetrahedra :
	for (std::size_t i = 0; i < xv; ++i) {
		for (std::size_t j = 0; j < yv; ++j) {
			for (std::size_t k = 0; k < zv; ++k) {
				mesh.tetrahedra.push_back({getIndice(i+1, j  , k  ), getIndice(i+1, j+1, k  ), getIndice(i  , j+1, k  ), getIndice(i+1, j+1, k+1)});
				mesh.tetrahedra.push_back({getIndice(i  , j  , k+1), getIndice(i  , j  , k  ), getIndice(i  , j+1, k+1), getIndice(i+1, j  , k+1)});
				mesh.tetrahedra.push_back({getIndice(i  , j+1, k+1), getIndice(i+1, j  , k  ), getIndice(i+1, j+1, k+1), getIndice(i+1, j  , k+1)});
				mesh.tetrahedra.push_back({getIndice(i  , j  , k  ), getIndice(i+1, j  , k  ), getIndice(i  , j+1, k+1), getIndice(i+1, j  , k+1)});
				mesh.tetrahedra.push_back({getIndice(i  , j  , k  ), getIndice(i+1, j  , k  ), getIndice(i  , j+1, k  ), getIndice(i  , j+1, k+1)});
				mesh.tetrahedra.push_back({getIndice(i  , j+1, k  ), getIndice(i+1, j  , k  ), getIndice(i+1, j+1, k+1), getIndice(i  , j+1, k+1)});

			}
		}
	}

	/* Mesh is now built, and should conform to the grid well enough.
	 * Some artefacts might appear at some orientations, due to floating
	 * point imprecision in the DDA tracing. They will be along the edge
	 * of the mesh's tetrahedra. */

	return;
}

void Scene::tex3D_buildVisTexture(VolMesh& volMesh) {
	std::size_t visWidth = 0, visHeight = 0;
	__GetTexSize(256, &visWidth, &visHeight);

	GLfloat* rawVisibility = new GLfloat[256*3];

	for (std::size_t i = 0; i < 256; ++i) {
		rawVisibility[i*3 + 0] = 100.f;
		rawVisibility[i*3 + 1] = 100.f;
		rawVisibility[i*3 + 2] = 100.f;
	}

	// Struct to upload the texture to OpenGL :
	TextureUpload texParams = {};

	// Vertex positions :
	texParams.minmag.x = GL_NEAREST;
	texParams.minmag.y = GL_NEAREST;
	texParams.lod.y = -1000.f;
	texParams.wrap.s = GL_CLAMP;
	texParams.wrap.t = GL_CLAMP;
	// Swizzle and alignment unchanged, not present in Texture3D
	texParams.internalFormat = GL_RGB32F;
	texParams.size.x = visWidth;
	texParams.size.y = visHeight;
	texParams.format = GL_RGB;
	texParams.type = GL_FLOAT;
	texParams.data = rawVisibility;
	volMesh.visibilityMap = this->uploadTexture2D(texParams);

	delete[] rawVisibility;
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

	float v0 [3] = { -1, -1,  1 };
	float v1 [3] = { -1,  1, -1 };
	float v2 [3] = {  1, -1, -1 };
	float v3 [3] = {  1,  1,  1 };

	// vertex coords array
	GLfloat vertices[] = {v3[0],v3[1],v3[2],  v1[0],v1[1],v1[2],  v2[0],v2[1],v2[2],        // v3-v1-v2
			      v3[0],v3[1],v3[2],  v2[0],v2[1],v2[2],  v1[0],v1[1],v1[2],        // v3-v2-v1
			      v3[0],v3[1],v3[2],  v0[0],v0[1],v0[2],  v1[0],v1[1],v1[2],        // v3-v0-v1
			      v2[0],v2[1],v2[2],  v1[0],v1[1],v1[2],  v0[0],v0[1],v0[2]};       // v2-v1-v0
	// normal array
	GLfloat normals[] = {v3[0],v3[1],v3[2],  v1[0],v1[1],v1[2],  v2[0],v2[1],v2[2],        // v3-v1-v2
			     v3[0],v3[1],v3[2],  v2[0],v2[1],v2[2],  v1[0],v1[1],v1[2],        // v3-v2-v1
			     v3[0],v3[1],v3[2],  v0[0],v0[1],v0[2],  v1[0],v1[1],v1[2],        // v3-v0-v1
			     v2[0],v2[1],v2[2],  v1[0],v1[1],v1[2],  v0[0],v0[1],v0[2]};       // v2-v1-v0
	// index array of vertex array for glDrawElements()
	// Notice the indices are listed straight from beginning to end as exactly
	// same order of vertex array without hopping, because of different normals at
	// a shared vertex. For this case, glDrawArrays() and glDrawElements() have no
	// difference.
	GLushort indices[] = {0,1,2,
			      3,4,5,
			      6,7,8,
			      9,10,11};
	// texture coords :
	GLfloat textureCoords[] = {0.,0.,1. ,1. ,2. ,2. ,
				   3.,3.,4. ,4. ,5. ,5. ,
				   6.,6.,7. ,7. ,8. ,8. ,
				   9.,9.,10.,10.,11.,11.};

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertPos);
	glBufferData(GL_ARRAY_BUFFER, 12*3*sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertNorm);
	glBufferData(GL_ARRAY_BUFFER, 12*3*sizeof(GLfloat), normals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertTex);
	glBufferData(GL_ARRAY_BUFFER, 12*2*sizeof(GLfloat), textureCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Texture3D_VertIdx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*sizeof(GLushort), indices, GL_STATIC_DRAW);

	glBindVertexArray(this->vaoHandle_VolumetricBuffers);
	this->tex3D_bindVAO();
}

void Scene::tex3D_bindVAO() {
	glBindVertexArray(this->vaoHandle_VolumetricBuffers);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertPos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertNorm);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertTex);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

