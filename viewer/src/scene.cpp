#include "../include/scene.hpp"
#include "../../image/tiff/include/tiff_writer.hpp"
#include "../../image/tiff/include/tiff_writer_templated.hpp"
#include "../../new_grid/include/grid_writer.hpp"
#include "../include/planar_viewer.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include <QMessageBox>
#include <QFileDialog>
#include <QOpenGLContext>
#include <QSurface>

#include <fstream>
#include <type_traits>

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
Scene::Scene() {
	this->isInitialized	   = false;
	this->showVAOstate	   = false;
	this->shouldDeleteGrid = false;

	//this->grids.clear();

	this->context			= nullptr;
	this->debugLog			= nullptr;
	this->glOutput			= nullptr;
	this->controlPanel		= nullptr;
	this->gridControl		= nullptr;
	this->visuBoxController = nullptr;
	this->programStatusBar	= nullptr;

	double minTexVal = 1;
	double maxTexVal = std::numeric_limits<int>::max();
	this->textureBounds0		   = NewAPI_GridGLView::data_2(minTexVal, maxTexVal);
	this->textureBounds1		   = NewAPI_GridGLView::data_2(minTexVal, maxTexVal);
	this->colorBounds0			   = NewAPI_GridGLView::data_2(0, maxTexVal);
	this->colorBounds1			   = NewAPI_GridGLView::data_2(0, maxTexVal);
	this->selectedChannel_r		   = 0;	   // by default, the R channel
	this->selectedChannel_g		   = 0;	   // by default, the R channel

	this->renderSize = 0;

	// Default light positions : at the vertices of a unit cube.
	this->lightPositions = {
	  glm::vec3(.0, .0, .0), glm::vec3(1., .0, .0), glm::vec3(.0, 1., .0), glm::vec3(1., 1., .0),
	  glm::vec3(.0, .0, 1.), glm::vec3(1., .0, 1.), glm::vec3(.0, 1., 1.), glm::vec3(1., 1., 1.)};

	this->planeDirection	= glm::vec3(1., 1., 1.);
	this->planeDisplacement = glm::vec3(.0, .0, .0);
	Image::bbox_t::vec min(.0, .0, .0);
	Image::bbox_t::vec max(1., 1., 1.);
	this->sceneBB				 = Image::bbox_t(min, max);
	this->clipDistanceFromCamera = 5.f;
	this->drawMode				 = DrawMode::Solid;
	this->rgbMode				 = RGBMode::HandEColouring;
	this->channels_r			 = ColorFunction::SingleChannel;
	this->channels_g			 = ColorFunction::SingleChannel;

	// Show all planes as default :
	this->planeVisibility = glm::vec<3, bool, glm::defaultp>(true, true, true);

	this->vaoHandle					  = 0;
	this->vaoHandle_VolumetricBuffers = 0;
	this->vaoHandle_boundingBox		  = 0;

	this->vboHandle_VertPos				= 0;
	this->vboHandle_VertNorm			= 0;
	this->vboHandle_VertTex				= 0;
	this->vboHandle_Element				= 0;
	this->vboHandle_PlaneElement		= 0;
	this->vboHandle_SinglePlaneElement	= 0;
	this->vboHandle_boundingBoxVertices = 0;
	this->vboHandle_boundingBoxIndices	= 0;

	this->programHandle_projectedTex	 = 0;
	this->programHandle_Plane3D			 = 0;
	this->programHandle_PlaneViewer		 = 0;
	this->programHandle_VolumetricViewer = 0;
	this->programHandle_BoundingBox		 = 0;

	this->texHandle_ColorScaleGrid			= 0;
	this->texHandle_ColorScaleGridAlternate = 0;
	this->shouldUpdateVis					= false;
	this->volumetricMesh					= {};
	this->vboHandle_Texture3D_VertPos		= 0;
	this->vboHandle_Texture3D_VertNorm		= 0;
	this->vboHandle_Texture3D_VertTex		= 0;
	this->vboHandle_Texture3D_VertIdx		= 0;

	this->color0		= glm::vec3(1., .0, .0);
	this->color1		= glm::vec3(.0, .0, 1.);
	this->color0_second = glm::vec3(1., .0, .0);
	this->color1_second = glm::vec3(.0, .0, 1.);

	std::cerr << "Allocating " << +std::numeric_limits<NewAPI_GridGLView::data_t>::max() << " elements for vis ...\n";

	this->pb_loadProgress			  = nullptr;
	this->timer_refreshProgress		  = nullptr;
	this->isFinishedLoading			  = false;
	this->shouldUpdateUserColorScales = false;
	this->shouldUpdateUBOData		  = false;

	this->posFrame = nullptr;

	this->visibleDomains = nullptr;
	this->visibleDomainsAlternate = nullptr;

	this->meshes.clear();
	this->drawables.clear();
}

Scene::~Scene(void) {
	delete[] this->visibleDomains;
	delete[] this->visibleDomainsAlternate;
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

	auto maj = this->context->format().majorVersion();
	auto min = this->context->format().minorVersion();
	std::cerr << "OpenGL version " << maj << '.' << min << '\n';

	std::cerr << "OpenGL vendor string : " << glGetString(GL_VENDOR) << '\n';
	std::cerr << "OpenGL renderer string : " << glGetString(GL_RENDERER) << '\n';

	// Create the debug logger and connect its signals :
	this->setupGLOutput();

	// The default parameters have already been set in the constructor. We
	// need to initialize the OpenGL objects now. Shaders, VAOs, VBOs.

	this->initialize_limits();
	this->newSHADERS_generateColorScales();

	this->shaderCompiler = std::make_unique<ShaderCompiler>(this);

	// Compile the shaders :
	this->recompileShaders(false);

	// Create VAO, VBO elements and populate them in generateSceneData() :
	this->createBuffers();
	this->generateSceneData();

	// Generate visibility array :
	this->texHandle_ColorScaleGrid = 0;
}

void Scene::initialize_limits() {
	this->gl_limit_max_texture_size = 0;

	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &gl_limit_max_texture_size);
	std::cerr << "Info : max texture size was set to : " << this->gl_limit_max_texture_size << "\n";

	return;
}

void Scene::generateColorScales() {
	TextureUpload colorScaleUploadParameters;

	std::size_t textureSize = this->gl_limit_max_texture_size / 2u;
	float textureSize_f		= static_cast<float>(this->gl_limit_max_texture_size / 2u);

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
	this->texHandle_colorScale_greyscale	  = this->uploadTexture1D(colorScaleUploadParameters);

	colorScaleUploadParameters.data	   = colorScaleData_hsv2rgb.data();
	this->texHandle_colorScale_hsv2rgb = this->uploadTexture1D(colorScaleUploadParameters);
}

void Scene::addOpenGLOutput(OpenGLDebugLog* glLog) {
	// if an output is already enabled, stop there
	if (this->glOutput != nullptr) {
		return;
	}
	if (glLog == nullptr) {
		return;
	}

	// add the glOutput to this class :
	this->glOutput = glLog;

	// if there __is__ a logger in the class (init didn't fail) :
	if (this->debugLog != nullptr) {
		// disconnect the messagelogged signal and re-connect it with glOutput :
		this->debugLog->disconnect(SIGNAL(&QOpenGLDebugLogger::messageLogged));
		QObject::connect(this->debugLog, &QOpenGLDebugLogger::messageLogged, this->glOutput, &OpenGLDebugLog::addOpenGLMessage);
	}
}

void Scene::addStatusBar(QStatusBar* _s) {
	if (this->programStatusBar != nullptr) {
		return;
	}

	this->programStatusBar = _s;
}

void Scene::setupGLOutput() {
	// if no context is available, end the func now.
	if (this->context == nullptr) {
		return;
	}

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
	QFlags type		= message.type();
	QFlags source	= message.source();

	//if (severity & QOpenGLDebugMessage::Severity::NotificationSeverity)	{ sev += "{Notif}"; }
	if (severity & QOpenGLDebugMessage::Severity::LowSeverity) {
		sev += "{Low}";
	}
	if (severity & QOpenGLDebugMessage::Severity::MediumSeverity) {
		sev += "{Med}";
	}
	if (severity & QOpenGLDebugMessage::Severity::HighSeverity) {
		sev += "{High}";
	}

	if (type & QOpenGLDebugMessage::Type::ErrorType) {
		typ += "[ERROR]";
	}
	if (type & QOpenGLDebugMessage::Type::DeprecatedBehaviorType) {
		typ += "[DEPR. BEHAVIOUR]";
	}
	if (type & QOpenGLDebugMessage::Type::UndefinedBehaviorType) {
		typ += "[UNDEF. BEHAVIOUR]";
	}
	if (type & QOpenGLDebugMessage::Type::PortabilityType) {
		typ += "[PORTABILITY]";
	}
	if (type & QOpenGLDebugMessage::Type::PerformanceType) {
		typ += "[PERF]";
	}
	if (type & QOpenGLDebugMessage::Type::OtherType) {
		typ += "[OTHER]";
	}
	if (type & QOpenGLDebugMessage::Type::MarkerType) {
		typ += "[MARKER]";
	}
	if (type & QOpenGLDebugMessage::Type::GroupPushType) {
		typ += "[GROUP PUSH]";
	}
	if (type & QOpenGLDebugMessage::Type::GroupPopType) {
		typ += "[GROUP POP]";
	}

	if (source & QOpenGLDebugMessage::Source::APISource) {
		src += "[OpenGL]";
	}
	if (source & QOpenGLDebugMessage::Source::WindowSystemSource) {
		src += "[WinSys]";
	}
	if (source & QOpenGLDebugMessage::Source::ShaderCompilerSource) {
		src += "[ShaComp]";
	}
	if (source & QOpenGLDebugMessage::Source::ThirdPartySource) {
		src += "[3rdParty]";
	}
	if (source & QOpenGLDebugMessage::Source::OtherSource) {
		src += "[Other]";
	}

	// Currently outputs any message on the GL stack, regardless of severity, type, or source :
	std::cerr << sev << ' ' << typ << ' ' << src << ' ' << +message.id() << " : " << message.message().toStdString() << '\n';
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
	this->vaoHandle					   = createVAO("vaoHandle");
	this->vboHandle_VertPos			   = createVBO(GL_ARRAY_BUFFER, "vboHandle_VertPos");
	this->vboHandle_VertNorm		   = createVBO(GL_ARRAY_BUFFER, "vboHandle_VertNorm");
	this->vboHandle_VertTex			   = createVBO(GL_ARRAY_BUFFER, "vboHandle_VertTex");
	this->vboHandle_Element			   = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_Element");
	this->vboHandle_PlaneElement	   = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_PlaneElement");
	this->vboHandle_SinglePlaneElement = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_SinglePlaneElement");

	// For the texture3D visualization method :
	this->vaoHandle_VolumetricBuffers  = createVAO("vaoHandle_VolumetricBuffers");
	this->vboHandle_Texture3D_VertPos  = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertPos");
	this->vboHandle_Texture3D_VertNorm = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertNorm");
	this->vboHandle_Texture3D_VertTex  = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertTex");
	this->vboHandle_Texture3D_VertIdx  = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_Texture3D_VertIdx");

	// For the bounding boxes we have to create/show :
	this->vaoHandle_boundingBox			= createVAO("vaoHandle_boundingBox");
	this->vboHandle_boundingBoxVertices = createVBO(GL_ARRAY_BUFFER, "vboHandle_boundingBoxVertices");
	this->vboHandle_boundingBoxIndices	= createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_boundingBoxIndices");

	return;
}

void Scene::loadGridROI() {
	LOG_ENTER(Scene::loadGridROI)
	// It will load the grid, based on the visu box coordinates !
	// The context will be made current at this stage, no need to worry :)
    
    std::cerr << "Error: visu box controller is broken for now due to new API update" << std::endl;

	// if (this->grids.size() == 0) {
	// 	return;
	// }
	// if (this->grids[0]->grid.size() == 0) {
	// 	return;
	// }

	// // Warn the user we will load the image in high resolution :
	// QMessageBox* msgBox = new QMessageBox;
	// msgBox->setWindowTitle("Warning : loading high-resolution grid");
	// msgBox->setText("This grid will be loaded in high resolution, and no checks will be done to see if it can fit in memory. Do you want to continue ?");
	// QPushButton* deny_button = msgBox->addButton("Cancel", QMessageBox::ButtonRole::RejectRole);
	// msgBox->addButton("Ok", QMessageBox::ButtonRole::AcceptRole);
	// msgBox->exec();
	// if (msgBox->clickedButton() == deny_button) {
	// 	return;
	// }

	// // Iterate over the grids loaded, and generate them with the coordinates given.
	// // Copy the populate grid function to load the current grid
	// // Have a vector of std::thread(s) each with the populate function running, and reporting their progress on this
	// // thread.
	// // At the end, insert the two grids with addGrid() or addTwoGrids() :)

	// std::for_each(this->grids[0]->grid.cbegin(), this->grids[0]->grid.cend(), [this](const std::shared_ptr<DiscreteGrid>& grid) -> void {
	// 	// Create a thread, in a shared ptr to add it to the list of running threads
	// 	std::shared_ptr<std::thread> localThread = std::make_shared<std::thread>([this, &grid](void) -> void {
	// 		{
	// 			std::lock_guard(this->mutexout);
	// 			std::cerr << "Launching thread " << std::this_thread::get_id() << '\n';
	// 		}

	// 		// Get the original voxel dimensions :
	// 		glm::vec3 voxelDims = grid->getVoxelDimensions();
	// 		if (grid->getGridReader() != nullptr) {
	// 			voxelDims = grid->getGridReader()->getOriginalVoxelDimensions();
	// 		}
	// 		// Compute the positions for the bounding box from the given coordinates :
	// 		Image::bbox_t renderBox = this->visuBox;
	// 		// The resolution is going to be defined so the voxel dimensions are the original voxel dimensions :
	// 		DiscreteGrid::sizevec3 dims = glm::convert_to<std::size_t>(this->visuBox.getDiagonal() / voxelDims);

	// 		// Create a tetrahedral mesh :
	// 		std::unique_ptr<InterpolationMesh> tetMesh = std::make_unique<InterpolationMesh>();
	// 		// Add the unique "input" grid to it :
	// 		tetMesh->addInputGrid(std::dynamic_pointer_cast<InputGrid>(grid));

	// 		// Create a new output grid :
	// 		std::shared_ptr<DiscreteGrid> outputGrid = std::make_shared<DiscreteGrid>(dims, renderBox);
	// 		// Specifies the grid size, and bounding box of the grid
	// 		outputGrid->setOffline(false);	  // we want to keep the grid in memory

	// 		tetMesh->setOutputGrid_raw(outputGrid);

	// 		{
	// 			std::lock_guard(this->mutexout);
	// 			std::cerr << "TetMesh info : " << '\n';
	// 			tetMesh->printInfo();
	// 		}

	// 		// Create a new task to keep track of this thread's progress
	// 		IO::ThreadedTask::Ptr task = std::make_shared<IO::ThreadedTask>();

	// 		// Add the task to the scene
	// 		{
	// 			std::lock_guard<std::mutex> locking(this->mutexadd);
	// 			this->tasks.push_back(task);
	// 		}

	// 		// Launch grid fill :
	// 		tetMesh->populateOutputGrid_threaded(InterpolationMethods::NearestNeighbor, task);

	// 		// Add the grid to the list of grids to be added later
	// 		{
	// 			std::lock_guard<std::mutex> locking(this->mutexadd);
    //             // TODO: new API
	// 			//this->gridsToAdd.push_back(outputGrid);
	// 		}
	// 	});

	// 	// Add it to the list of running joinable threads :
	// 	this->runningThreads.push_back(localThread);
	// });

	// if (this->programStatusBar != nullptr) {
	// 	// Add a progress bar and update it :
	// 	this->pb_loadProgress = new QProgressBar;
	// 	this->programStatusBar->addWidget(this->pb_loadProgress);
	// 	this->timer_refreshProgress = new QTimer;
	// 	this->timer_refreshProgress->setSingleShot(false);
	// 	this->timer_refreshProgress->setInterval(50);
	// 	QObject::connect(this->timer_refreshProgress, &QTimer::timeout, [this](void) {
	// 		this->updateProgressBar();
	// 	});
	// 	this->timer_refreshProgress->start();
	// }

	// LOG_LEAVE(Scene::loadGridROI)
}

void Scene::updateProgressBar() {
	if (this->tasks.size() == 0) {
		return;
	}

	// Track progress of tasks :
	std::size_t current	 = 0;
	std::size_t maxSteps = 0;
	std::for_each(this->tasks.cbegin(), this->tasks.cend(), [&current, &maxSteps](const IO::ThreadedTask::Ptr& task) {
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

void Scene::newAPI_addGrid(Image::Grid::Ptr gridLoaded) {
	glm::vec<4, std::size_t, glm::defaultp> dimensions{gridLoaded->getResolution(), gridLoaded->getVoxelDimensionality()};

	NewAPI_GridGLView::Ptr gridView = std::make_shared<NewAPI_GridGLView>(gridLoaded);

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

	std::vector<std::uint16_t> slices(dimensions.x * dimensions.y * dimensions.a);
	gridView->gridTexture = this->newAPI_uploadTexture3D_allocateonly(_gridTex);

	for (std::size_t s = 0; s < dimensions.z; ++s) {
		if (gridLoaded->readSlice(s, slices)) {
			this->newAPI_uploadTexture3D(gridView->gridTexture, _gridTex, s, slices);
		} else {
			std::cerr << "Scene texture upload : Could not read the data at index " << s << " !\n";
		}
	}

	gridView->boundingBoxColor = glm::vec3(.4, .6, .3);	   // olive-colored by default
	gridView->nbChannels	   = 2;	   // loaded 2 channels in the image

	// Create the uniform buffer :
	auto mainColorChannel				= gridView->mainColorChannelAttributes();
	gridView->uboHandle_colorAttributes = this->createUniformBuffer(4 * sizeof(colorChannelAttributes_GL), GL_STATIC_DRAW);
	this->setUniformBufferData(gridView->uboHandle_colorAttributes, 0, 32, &mainColorChannel);
	this->setUniformBufferData(gridView->uboHandle_colorAttributes, 32, 32, &gridView->colorChannelAttributes[0]);
	this->setUniformBufferData(gridView->uboHandle_colorAttributes, 64, 32, &gridView->colorChannelAttributes[1]);
	this->setUniformBufferData(gridView->uboHandle_colorAttributes, 96, 32, &gridView->colorChannelAttributes[2]);

	this->newAPI_tex3D_buildMesh(gridView, "");
	this->tex3D_buildVisTexture(gridView->volumetricMesh);
	this->tex3D_buildBuffers(gridView->volumetricMesh);

	this->newGrids.push_back(gridView);

	this->updateVis();
	this->updateBoundingBox();
	this->setVisuBoxMinCoord(glm::uvec3());
	this->setVisuBoxMaxCoord(gridLoaded->getResolution());
	this->resetVisuBox();
}

void Scene::updateBoundingBox(void) {
    std::cerr << "Error: updateBoundingBox brocken due to new API update" << std::endl;
	this->sceneBB	  = Image::bbox_t();
	this->sceneDataBB = Image::bbox_t();
	// for (std::size_t i = 0; i < this->grids.size(); ++i) {
	// 	std::for_each(this->grids[i]->grid.cbegin(), this->grids[i]->grid.cend(),
	// 	  [this](const std::shared_ptr<DiscreteGrid>& _g) {
	// 		  Image::bbox_t box	= _g->getBoundingBoxWorldSpace();
	// 		  Image::bbox_t dbox = _g->getDataBoundingBoxWorldSpace();
	// 		  this->sceneBB.addPoints(box.getAllCorners());
	// 		  this->sceneDataBB.addPoints(dbox.getAllCorners());
	// 	  });
	// }
	for (std::size_t i = 0; i < this->newGrids.size(); ++i) {
		const Image::Grid::Ptr _g = this->newGrids[i]->grid;
		Image::bbox_t box  = _g->getBoundingBox();
		Image::bbox_t dbox = _g->getBoundingBox();
		this->sceneBB.addPoints(box.getAllCorners());
		this->sceneDataBB.addPoints(dbox.getAllCorners());
	}

	// Take into account drawables :
	for (auto drawable : this->drawables) {
		if (drawable->isInitialized()) {
			auto drawable_bb = drawable->getBoundingBox();
			this->sceneBB.addPoint(drawable_bb.first);
			this->sceneBB.addPoint(drawable_bb.second);
			this->sceneDataBB.addPoint(drawable_bb.first);
			this->sceneDataBB.addPoint(drawable_bb.second);
		}
	}

	// Update light positions :
	auto corners = this->sceneBB.getAllCorners();
	for (std::size_t i = 0; i < corners.size(); ++i) {
		this->lightPositions[i] = glm::convert_to<float>(corners[i]);
	}

	return;
}

void Scene::recompileShaders(bool verbose) {
	GLuint newProgram			 = this->compileShaders("../new_shaders/voxelgrid.vert", "../new_shaders/voxelgrid.geom", "../new_shaders/voxelgrid.frag", verbose);
	GLuint newPlaneProgram		 = this->compileShaders("../new_shaders/plane.vert", "", "../new_shaders/plane.frag", verbose);
	GLuint newPlaneViewerProgram = this->compileShaders("../new_shaders/texture_explorer.vert", "", "../new_shaders/texture_explorer.frag", verbose);
	GLuint newVolumetricProgram	 = this->compileShaders("../new_shaders/transfer_mesh.vert", "../new_shaders/transfer_mesh.geom", "../new_shaders/transfer_mesh.frag", verbose);
	GLuint newBoundingBoxProgram = this->compileShaders("../new_shaders/bounding_box.vert", "", "../new_shaders/bounding_box.frag", verbose);

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
	this->shaderCompiler->reset();
	this->shaderCompiler->pragmaReplacement_file("include_color_shader", "../new_shaders/colorize_new_flow.glsl");
	this->shaderCompiler->vertexShader_file(_vPath).geometryShader_file(_gPath).fragmentShader_file(_fPath);
	if (this->shaderCompiler->compileShaders()) {
		return this->shaderCompiler->programName();
	}
	std::cerr << this->shaderCompiler->errorString() << '\n';
	return 0;
}

GLuint Scene::compileShader(const std::string& path, const GLenum shaType, bool verbose) {
	if (path.empty()) {
		return 0;
	}
	// Check the given type is accepted :
	if (shaType != GL_VERTEX_SHADER && shaType != GL_GEOMETRY_SHADER && shaType != GL_FRAGMENT_SHADER) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] Error : unrecognized shader type (vertex, geometry and fragment shaders)\n";
		return -1;
	}

	// Create a shader object :
	GLuint _sha = glCreateShader(shaType);

	std::string color_functions = "";
	bool includeColorShader		= false;

	if (shaType == GL_FRAGMENT_SHADER) {
		includeColorShader	  = true;
		std::ifstream colFile = std::ifstream("../new_shaders/colorize_new_flow.glsl", std::ios_base::in | std::ios_base::binary);
		if (! colFile.is_open()) {
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
	if (! shaFile.is_open()) {
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
#if defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32)
			if (read_buffer.find("#pragma include_color_shader;") != std::string::npos) {
#else
			if (read_buffer == "#pragma include_color_shader;") {
#endif
				read_buffer = color_functions;
			}
			shader_contents += read_buffer + '\n';
		}
		shaSource	= shader_contents.data();
		shaFileSize = shader_contents.size();
	} else {
		// Get the file's contents and null-terminate it :
		shaSource = new char[shaFileSize + 1];
		shaFile.read(shaSource, shaFileSize);
		shaSource[shaFileSize] = '\0';
	}

	// Source it into the shader object :
	glShaderSource(_sha, 1, const_cast<const char**>(&shaSource), 0);

	// We can free up the host memory now :
	shaFile.close();

	glCompileShader(_sha);

	GLint shaderInfoLength = 0;
	GLint charsWritten	   = 0;
	char* shaderInfoLog	   = nullptr;

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
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] : No shader IDs were passed to the function.\n"
				  << "\tArguments : vSha(" << vSha << "), gSha(" << gSha << "), fSha(" << fSha << ")";
	}

	GLuint _prog = glCreateProgram();
	if (vSha != 0) {
		glAttachShader(_prog, vSha);
	}
	if (gSha != 0) {
		glAttachShader(_prog, gSha);
	}
	if (fSha != 0) {
		glAttachShader(_prog, fSha);
	}

	glLinkProgram(_prog);

	int InfoLogLength = 0;
	glGetProgramiv(_prog, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
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
		if (verbose) {
			std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiled program. New name : " << _prog << "\n";
		}
	}

	if (verbose) {
		std::cerr << '\n';
	}	 // Put an empty line between all program compilations (more readable)

	if (vSha != 0) {
		glDetachShader(_prog, vSha);
	}
	if (gSha != 0) {
		glDetachShader(_prog, gSha);
	}
	if (fSha != 0) {
		glDetachShader(_prog, fSha);
	}
	if (vSha != 0) {
		glDeleteShader(vSha);
	}
	if (gSha != 0) {
		glDeleteShader(gSha);
	}
	if (fSha != 0) {
		glDeleteShader(fSha);
	}

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

void Scene::openGLDebugLogger_inserter(const QOpenGLDebugMessage message) {
	QString src		= "";
	QString sev		= "";
	QString typ		= "";
	QFlags severity = message.severity();
	QFlags type		= message.type();
	QFlags source	= message.source();

	if (severity & QOpenGLDebugMessage::Severity::NotificationSeverity) {
		sev += "{Notif}";
	}
	if (severity & QOpenGLDebugMessage::Severity::LowSeverity) {
		sev += "{Low  }";
	}
	if (severity & QOpenGLDebugMessage::Severity::MediumSeverity) {
		sev += "{Med  }";
	}
	if (severity & QOpenGLDebugMessage::Severity::HighSeverity) {
		sev += "{High }";
	}

	if (type & QOpenGLDebugMessage::Type::ErrorType) {
		typ += "[	ERROR     ]";
	}
	if (type & QOpenGLDebugMessage::Type::DeprecatedBehaviorType) {
		typ += "[ DEPRE. BEHAV. ]";
	}
	if (type & QOpenGLDebugMessage::Type::UndefinedBehaviorType) {
		typ += "[ UNDEF. BEHAV. ]";
	}
	if (type & QOpenGLDebugMessage::Type::PortabilityType) {
		typ += "[  PORTABILITY  ]";
	}
	if (type & QOpenGLDebugMessage::Type::PerformanceType) {
		typ += "[      PERF     ]";
	}
	if (type & QOpenGLDebugMessage::Type::OtherType) {
		typ += "[     OTHER	  ]";
	}
	if (type & QOpenGLDebugMessage::Type::MarkerType) {
		typ += "[     MARKER    ]";
	}
	if (type & QOpenGLDebugMessage::Type::GroupPushType) {
		typ += "[  GROUP PUSH   ]";
	}
	if (type & QOpenGLDebugMessage::Type::GroupPopType) {
		typ += "[   GROUP POP   ]";
	}

	if (source & QOpenGLDebugMessage::Source::APISource) {
		src += "[OpenGL  ]";
	}
	if (source & QOpenGLDebugMessage::Source::WindowSystemSource) {
		src += "[WinSys  ]";
	}
	if (source & QOpenGLDebugMessage::Source::ShaderCompilerSource) {
		src += "[ShaComp ]";
	}
	if (source & QOpenGLDebugMessage::Source::ThirdPartySource) {
		src += "[3rdParty]";
	}
	if (source & QOpenGLDebugMessage::Source::OtherSource) {
		src += "[Other   ]";
	}

	QString glMessage = sev + typ + src + " [ID:" + QString::number(message.id()) + "] : " + message.message() + "\n";

	std::cerr << glMessage.toStdString() << '\n';
}

void Scene::loadMesh() {
	// Launch a file picker to get the name of an OFF file :
	QString file_name = QFileDialog::getOpenFileName(nullptr, "Open a Mesh file (OFF)", QString(), "OFF files (*.off)");
	if (file_name.isEmpty() || not QFileInfo::exists(file_name)) {
		std::cerr << "Error : nothing to open.\nFile path given : \"" << file_name.toStdString() << "\"\n";
		return;
	}

	// Create a mesh structure :
	auto mesh_to_load = std::make_shared<Mesh>();
	auto& vertices = mesh_to_load->getVertices();
	auto& normals = mesh_to_load->getNormals();
	// Load that OFF file :
	FileIO::openOFF(file_name.toStdString(), mesh_to_load->getVertices(), mesh_to_load->getTriangles());
	mesh_to_load->update();
	this->meshes.emplace_back(mesh_to_load);

	auto mesh_drawable = std::make_shared<DrawableMesh>(mesh_to_load);

	// Insert it into the meshes to initialize :
	this->to_init.emplace(mesh_drawable);
}

void Scene::launchSaveDialog() {
	// if no grids are loaded, do nothing !
	if (this->newGrids.size() == 0) {
		QMessageBox messageBox;
		messageBox.critical(nullptr, "Error", "Cannot save a grid when nothing is loaded !");
		messageBox.setFixedSize(500, 200);
		return;
	}

	if (this->gridControl != nullptr) {
		std::cerr << "Controller was already added, showing it now ...\n";
		this->gridControl->raise();
		this->gridControl->show();
		return;
	}

    // TODO 
    // Add calls to tetmesh and grid control when they will be compatible with new API 

	//// create an output grid AND a tetmesh to generate it :
	//std::shared_ptr<OutputGrid> outputGrid = std::make_shared<OutputGrid>();
	//outputGrid->setOffline();

	//InterpolationMesh::Ptr tetmesh = std::make_shared<InterpolationMesh>();
	//// add the grids to the tetmesh
	//for (std::size_t i = 0; i < this->grids.size(); ++i) {
	//	std::for_each(this->grids[i]->grid.cbegin(), this->grids[i]->grid.cend(),
	//	  [&tetmesh](const std::shared_ptr<DiscreteGrid>& _g) {
	//		  tetmesh->addInputGrid(std::dynamic_pointer_cast<InputGrid>(_g));
	//	  });
	//}
	//// add the output grid
	//tetmesh->setOutputGrid(outputGrid);

	//// So we can draw it :
	//GridGLView::Ptr outputGridView = std::make_shared<GridGLView>(std::initializer_list<std::shared_ptr<DiscreteGrid>>({outputGrid}));
	//this->grids.push_back(outputGridView);

	//// create a grid controller to generate a new outputgrid
	//this->gridControl = new GridControl(outputGrid, tetmesh, this);
	//// show it :
	//gridControl->show();
	//// done
	return;
}

void Scene::removeController() {
	this->gridControl = nullptr;
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

void Scene::drawPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading, float zoomRatio, glm::vec2 offset) {
	if (this->newGrids.empty()) {
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

	glUseProgram(this->programHandle_PlaneViewer);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_SinglePlaneElement);

// Do for all grids :
#warning drawPlaneView() : Only draws the first grid !
	if (not this->newGrids.empty()) {
		this->newAPI_prepareUniforms_PlaneViewer(_plane, _heading, fbDims, zoomRatio, offset, this->newGrids[0]);

		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*) (min * sizeof(GLuint)));
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	this->showVAOstate = false;

	return;
}

void Scene::newAPI_drawVolumetric(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, const NewAPI_GridGLView::Ptr& grid) {
	if (grid->gridTexture > 0) {
		glUseProgram(this->programHandle_VolumetricViewer);

		this->newAPI_prepareUniforms_Volumetric(mvMat, pMat, camPos, grid);

		this->tex3D_bindVAO();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Texture3D_VertIdx);

		glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, (void*) 0, grid->volumetricMesh.tetrahedraCount);

		// Unbind program, buffers and VAO :
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}

	// draw grid BB :
	this->drawBoundingBox(grid->grid->getBoundingBox(), grid->boundingBoxColor, mvMat, pMat);
}

void Scene::newAPI_drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

#warning drawPlanes() : Only draws the first grid for each plane !

	// Plane X :
	glUseProgram(this->programHandle_Plane3D);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
	if (not this->newGrids.empty()) {
		this->newAPI_prepareUniforms_3DPlane(mvMat, pMat, planes::x, this->newGrids[0], showTexOnPlane);
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
	if (not this->newGrids.empty()) {
		this->newAPI_prepareUniforms_3DPlane(mvMat, pMat, planes::y, this->newGrids[0], showTexOnPlane);
		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*) (6 * sizeof(GLuint)));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	// Plane Z :
	glUseProgram(this->programHandle_Plane3D);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
	if (not this->newGrids.empty()) {
		this->newAPI_prepareUniforms_3DPlane(mvMat, pMat, planes::z, this->newGrids[0], showTexOnPlane);
		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*) (12 * sizeof(GLuint)));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
}

glm::vec3 Scene::computePlanePositions() {
	Image::bbox_t::vec position = this->sceneBB.getMin();
	Image::bbox_t::vec diagonal = this->sceneBB.getDiagonal();
	glm::vec3 planePos				   = (position + this->planeDisplacement * diagonal);
	return planePos;
}

void Scene::newAPI_prepareUniforms_3DSolid(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, const NewAPI_GridGLView::Ptr& gridView) {
	// Get the world to grid transform :
	MatrixTransform::Ptr grid_transform_pointer = std::dynamic_pointer_cast<MatrixTransform>(gridView->grid->getPrecomputedMatrix());
	glm::mat4 transfoMat						= baseMatrix * grid_transform_pointer->matrix();
#warning Transform API is still in-progress.

	auto getUniform = [&](const char* name) -> GLint {
		GLint g = glGetUniformLocation(this->programHandle_projectedTex, name);
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
		this->newSHADERS_print_all_uniforms(this->programHandle_projectedTex);
		glUseProgram(this->programHandle_projectedTex);
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

	Image::bbox_t::vec origin   = gridView->grid->getBoundingBox().getMin();
	Image::bbox_t::vec originWS = gridView->grid->getBoundingBox().getMin();
	Image::sizevec3 gridDims	   = gridView->grid->getResolution();
	glm::vec3 dims					   = glm::convert_to<float>(gridDims);

	if (showVAOstate) {
		PRINTVAL(gridDims.x);
		PRINTVAL(gridDims.y);
		PRINTVAL(gridDims.z);
	}

	glUniform3fv(voxelGridOrigin_Loc, 1, glm::value_ptr(origin));
	glUniform3fv(voxelGridSize_Loc, 1, glm::value_ptr(dims));
	glUniform3fv(voxelSize_Loc, 1, glm::value_ptr(gridView->grid->getVoxelDimensions()));
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
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_greyscale);
	glUniform1i(location_colorScales0, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glEnable(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_hsv2rgb);
	glUniform1i(location_colorScales1, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_user0);
	glUniform1i(location_colorScales2, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_user1);
	glUniform1i(location_colorScales3, enabled_textures);
	enabled_textures++;

	/**
	 * NOTE : assumes the main color channel is always green, a.k.a. colorChannels[1]
	 * Load all the available color channels in the shader program, using the discrete identifiers :
	 */
	const GLchar uniform_block_name[] = "ColorBlock";
	GLuint colorBlock_index			  = glGetUniformBlockIndex(this->programHandle_projectedTex, uniform_block_name);
	glUniformBlockBinding(this->programHandle_projectedTex, colorBlock_index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, gridView->uboHandle_colorAttributes);

	if (this->showVAOstate) {
		this->printAllUniforms(this->programHandle_projectedTex);
	}
}

glm::vec3 Scene::getPlaneDirections() const {
	return this->planeDirection;
}

glm::tvec4<GLuint> Scene::draft_getGeneratedColorScales() {
	return glm::tvec4<GLuint>(
	  this->texHandle_colorScale_greyscale, this->texHandle_colorScale_hsv2rgb,
	  this->texHandle_colorScale_user0, this->texHandle_colorScale_user1);
}

void Scene::draft_tryAndSaveFirstGrid() {
	if (this->newGrids.size() == 0) {
		std::cerr << "Error : no new grids loaded.\n";
		return;
	}

	Image::ThreadedTask::Ptr task = std::make_shared<Image::ThreadedTask>();
	// get the grid
	Image::Grid::Ptr gridToSave = this->newGrids[0]->grid;
	// Create a writer backend :
	Image::GridWriter::Ptr gridWriter = nullptr;
	std::cerr << "Creating grid writer ...\n";
	try {
		gridWriter = std::make_shared<Image::GridWriter>(
		  Image::Tiff::TIFFWriterTemplated<std::uint32_t>::createBackend("tiff_grid", "/home/thibault/Pictures/tiff"));
		if (not gridWriter->writeGrid(gridToSave, task)) {
			std::cerr << "Error : cannot write grid.\n";
		}
	} catch (std::exception& _e) {
		std::cerr << "Error : exception caught !\n";
		std::cerr << "Error message : " << _e.what() << '\n';
	}
	std::cerr << "Grid writer finished writing.\n";
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

void Scene::newAPI_prepareUniforms_3DPlane(GLfloat* mvMat, GLfloat* pMat, planes _plane, const NewAPI_GridGLView::Ptr& grid, bool showTexOnPlane) {
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
		GLint g = glGetUniformLocation(this->programHandle_Plane3D, name);
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
	MatrixTransform::Ptr grid_transform_pointer = std::dynamic_pointer_cast<MatrixTransform>(grid->grid->getPrecomputedMatrix());
	glm::mat4 transform							= glm::mat4(1.f);
	glm::mat4 gridTransfo						= grid_transform_pointer->matrix();	   //grid->grid->getTransform_GridToWorld();
#warning Transform API is still in-progress.
	Image::bbox_t bbws = grid->grid->getBoundingBox();
	glm::vec3 dims			  = glm::convert_to<glm::vec3::value_type>(grid->grid->getResolution()) * grid->grid->getVoxelDimensions();
	glm::vec3 size			  = bbws.getDiagonal();
	GLint plIdx				  = (_plane == planes::x) ? 1 : (_plane == planes::y) ? 2 : 3;

	Image::bbox_t::vec position = this->sceneBB.getMin();
	Image::bbox_t::vec diagonal = this->sceneBB.getDiagonal();
	glm::vec3 planePos				   = this->computePlanePositions();

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
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_greyscale);
	glUniform1i(location_colorScales0, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_hsv2rgb);
	glUniform1i(location_colorScales1, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_user0);
	glUniform1i(location_colorScales2, enabled_textures);
	enabled_textures++;

	glActiveTexture(GL_TEXTURE0 + enabled_textures);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_user1);
	glUniform1i(location_colorScales3, enabled_textures);
	enabled_textures++;

	const GLchar uniform_block_name[] = "ColorBlock";
	GLuint colorBlock_index			  = glGetUniformBlockIndex(this->programHandle_projectedTex, uniform_block_name);
	glUniformBlockBinding(this->programHandle_projectedTex, colorBlock_index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, grid->uboHandle_colorAttributes);
}

void Scene::newAPI_prepareUniforms_PlaneViewer(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, glm::vec2 offset, const NewAPI_GridGLView::Ptr& _grid) {
	glUseProgram(this->programHandle_PlaneViewer);
	// The BB used is the scene's bounding box :
	const Image::bbox_t::vec& bbox	= this->sceneBB.getDiagonal();
	const Image::bbox_t::vec& posBox = this->sceneBB.getMin();

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
	MatrixTransform::Ptr grid_transform_pointer = std::dynamic_pointer_cast<MatrixTransform>(_grid->grid->getPrecomputedMatrix());
	// Grid transform :
	glm::mat4 gridTransform = grid_transform_pointer->matrix();	   //_grid->grid->getTransform_WorldToGrid();
#warning Transform API is still in-progress.
	// Grid dimensions :
	glm::vec3 gridDimensions = glm::convert_to<glm::vec3::value_type>(_grid->grid->getBoundingBox().getDiagonal());
	// Depth of the plane :
	glm::vec3 planePos = this->computePlanePositions();

	auto getUniform = [&](const char* name) -> GLint {
		GLint g = glGetUniformLocation(this->programHandle_PlaneViewer, name);
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
	glUniformMatrix4fv(location_gridTransform, 1, GL_FALSE, glm::value_ptr(gridTransform));
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
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_greyscale);
	glUniform1i(location_colorScales0, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_hsv2rgb);
	glUniform1i(location_colorScales1, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_user0);
	glUniform1i(location_colorScales2, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_user1);
	glUniform1i(location_colorScales3, tex);
	tex++;

	const GLchar uniform_block_name[] = "ColorBlock";
	GLuint colorBlock_index			  = glGetUniformBlockIndex(this->programHandle_projectedTex, uniform_block_name);
	glUniformBlockBinding(this->programHandle_projectedTex, colorBlock_index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, _grid->uboHandle_colorAttributes);
}

void Scene::newAPI_prepareUniforms_Volumetric(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, const NewAPI_GridGLView::Ptr& _grid) {
	// We assume the right program has been bound.

	/// @brief Shortcut for glGetUniform, since this can result in long lines.
	auto getUniform = [&](const char* name) -> GLint {
		GLint g = glGetUniformLocation(this->programHandle_VolumetricViewer, name);
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
	glBindTexture(GL_TEXTURE_2D, this->texHandle_ColorScaleGrid);
	glUniform1i(location_visibilityMap, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, this->texHandle_ColorScaleGridAlternate);
	glUniform1i(location_visibilityMapAlternate, tex);
	tex++;

	// Scalars :
	GLint location_voxelSize = getUniform("voxelSize");
	GLint location_gridSize	 = getUniform("gridSize");

	glm::vec3 floatres = glm::convert_to<float>(_grid->grid->getResolution());

	glUniform3fv(location_voxelSize, 1, glm::value_ptr(_grid->grid->getVoxelDimensions()));
	glUniform3fv(location_gridSize, 1, glm::value_ptr(floatres));

	// Vectors/arrays :
	GLint location_cam					  = getUniform("cam");
	GLint location_cut					  = getUniform("cut");
	GLint location_cutDirection			  = getUniform("cutDirection");
	GLint location_clipDistanceFromCamera = getUniform("clipDistanceFromCamera");
	GLint location_visuBBMin			  = getUniform("visuBBMin");
	GLint location_visuBBMax			  = getUniform("visuBBMax");
	GLint location_shouldUseBB			  = getUniform("shouldUseBB");
	GLint location_volumeEpsilon		  = getUniform("volumeEpsilon");

	glm::vec3 planePos			  = this->computePlanePositions();
	Image::bbox_t::vec min = this->visuBox.getMin();
	Image::bbox_t::vec max = this->visuBox.getMax();

	glUniform3fv(location_cam, 1, glm::value_ptr(camPos));
	glUniform3fv(location_cut, 1, glm::value_ptr(planePos));
	glUniform3fv(location_cutDirection, 1, glm::value_ptr(this->planeDirection));
	glUniform1f(location_clipDistanceFromCamera, this->clipDistanceFromCamera);
	glUniform3fv(location_visuBBMin, 1, glm::value_ptr(min));
	glUniform3fv(location_visuBBMax, 1, glm::value_ptr(max));
	glUniform1ui(location_shouldUseBB, ((this->drawMode == DrawMode::VolumetricBoxed) ? 1 : 0));
	glUniform3fv(location_volumeEpsilon, 1, glm::value_ptr(_grid->defaultEpsilon));

	// Matrices :
	GLint location_mMat = getUniform("mMat");
	GLint location_vMat = getUniform("vMat");
	GLint location_pMat = getUniform("pMat");

//const glm::mat4& gridTransfo = _grid->grid->getTransform_GridToWorld();
#warning Transform API is still in-progress.
	MatrixTransform::Ptr grid_transform_pointer = std::dynamic_pointer_cast<MatrixTransform>(_grid->grid->getPrecomputedMatrix());
	const glm::mat4 gridTransfo					= grid_transform_pointer->matrix();
	glUniformMatrix4fv(location_mMat, 1, GL_FALSE, glm::value_ptr(gridTransfo));
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

	//	// Limits :
	//	GLint location_colorBounds =			getUniform("colorBounds");
	//	GLint location_colorBoundsAlternate =	getUniform("colorBoundsAlternate");
	//	GLint location_textureBounds =			getUniform("textureBounds");
	//	GLint location_textureBoundsAlternate =	getUniform("textureBoundsAlternate");

	//	glUniform2fv(location_colorBounds, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds0)));
	//	glUniform2fv(location_colorBoundsAlternate, 1, glm::value_ptr(glm::convert_to<float>(this->colorBounds1)));
	//	glUniform2fv(location_textureBounds, 1, glm::value_ptr(glm::convert_to<float>(this->textureBounds0)));
	//	glUniform2fv(location_textureBoundsAlternate, 1, glm::value_ptr(glm::convert_to<float>(this->textureBounds1)));

	// Color and shading parameters :
	GLint location_specRef	  = getUniform("specRef");
	GLint location_shininess  = getUniform("shininess");
	GLint location_diffuseRef = getUniform("diffuseRef");
	//	GLint location_rgbMode =			getUniform("rgbMode");
	//	GLint location_r_channelView =		getUniform("r_channelView");
	//	GLint location_r_selectedChannel =	getUniform("r_selectedChannel");
	//	GLint location_r_nbChannels =		getUniform("r_nbChannels");
	//	GLint location_g_channelView =		getUniform("g_channelView");
	//	GLint location_g_selectedChannel =	getUniform("g_selectedChannel");
	//	GLint location_g_nbChannels =		getUniform("g_nbChannels");
	//	uint chan_r = this->colorFunctionToUniform(this->channels_r);
	//	uint chan_g = this->colorFunctionToUniform(this->channels_g);
	glUniform1f(location_specRef, .8f);
	glUniform1f(location_shininess, .8f);
	glUniform1f(location_diffuseRef, .8f);
	//	glUniform1ui(location_rgbMode, this->rgbMode);
	//	glUniform1ui(location_r_channelView, chan_r);
	//	glUniform1ui(location_g_channelView, chan_g);
	//	glUniform1ui(location_r_nbChannels, 1);
	//	glUniform1ui(location_g_nbChannels, (_grid->grid->getVoxelDimensionality() > 1) ? 1 : 0); // only set this if there's two grids
	//	if (_grid->nbChannels > 1) {
	//		glUniform1ui(location_r_selectedChannel, this->selectedChannel_r);
	//		glUniform1ui(location_g_selectedChannel, this->selectedChannel_g);
	//	} else {
	//		glUniform1ui(location_r_selectedChannel, 0);
	//		glUniform1ui(location_g_selectedChannel, 0);
	//	}

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
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_greyscale);
	glUniform1i(location_colorScales0, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_hsv2rgb);
	glUniform1i(location_colorScales1, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_user0);
	glUniform1i(location_colorScales2, tex);
	tex++;

	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_colorScale_user1);
	glUniform1i(location_colorScales3, tex);
	tex++;

	const GLchar uniform_block_name[] = "ColorBlock";
	GLuint colorBlock_index			  = glGetUniformBlockIndex(this->programHandle_projectedTex, uniform_block_name);
	glUniformBlockBinding(this->programHandle_projectedTex, colorBlock_index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, _grid->uboHandle_colorAttributes);

	// print uniform values :
	if (this->showVAOstate) {
		auto vx = _grid->grid->getVoxelDimensions();
		std::cerr << "[LOG] " << __FUNCTION__ << " has uniform values :\n";
		//std::cerr << "[LOG]\tMax possible value        : " << static_cast<double>(std::numeric_limits<DiscreteGrid::data_t>::max()) << '\n';
		std::cerr << "[LOG]\tDraw mode                : " << +this->drawMode << '\n';
		std::cerr << "[LOG]\tNumber of channels        : " << +_grid->nbChannels << '\n';
		std::cerr << "[LOG]\tClip distance from camera : " << +clipDistanceFromCamera << '\n';
		std::cerr << "[LOG]\tVoxel dimensions          : [" << vx.x << ',' << vx.y << ',' << vx.z << "]\n";
		std::cerr << "[LOG]\tGrid resolution           : [" << floatres.x << ',' << floatres.y << ',' << floatres.z << "]\n";
		std::cerr << "[LOG]\tVolume epsilon            : [" << _grid->defaultEpsilon.x << ',' << _grid->defaultEpsilon.y << ',' << _grid->defaultEpsilon.z << "]\n";
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
			std::cerr << "[LOG]\tLight " << i << "                   : [" << +this->lightPositions[i].x << ',' << +this->lightPositions[i].y << ',' << +this->lightPositions[i].z << "]\n";
		}
		LOG_LEAVE(Scene::drawVolumetric)
	}
}

GLuint Scene::getVolumetricProgram() {
	return this->programHandle_VolumetricViewer;
}

void Scene::useProgram(GLuint _program_handle) {
	glUseProgram(_program_handle);
}

GLint Scene::findUniform(GLuint _program_handle, const char* _uniform_name) {
	return glGetUniformLocation(_program_handle, _uniform_name);
}

void Scene::newAPI_drawGrid(GLfloat* mvMat, GLfloat* pMat, glm::mat4 baseMatrix, const NewAPI_GridGLView::Ptr& grid) {
	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	// If the grid has been uploaded, show it :
	if (grid->gridTexture > 0) {
		glUseProgram(this->programHandle_projectedTex);
		glBindVertexArray(this->vaoHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Element);

		this->newAPI_prepareUniforms_3DSolid(mvMat, pMat, lightPos, baseMatrix, grid);
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
	if (this->shouldUpdateVis) {
		this->shouldUpdateVis = false;
	}
	if (this->shouldDeleteGrid) {
		this->deleteGridNow();
	}
	if (this->shouldUpdateUserColorScales) {
		this->newSHADERS_updateUserColorScales();
	}
	if (this->shouldUpdateUBOData) {
		this->newSHADERS_updateUBOData();
	}
	if (not this->to_init.empty()) {
		// Initialize all meshes to load into the scene :
		while (not this->to_init.empty()) {
			auto to_initialize = this->to_init.back();
			to_initialize->initialize(this->context, this);
			this->drawables.emplace_back(to_initialize);
			this->to_init.pop();
		}
		this->updateBoundingBox();
	}

	glEnable(GL_DEPTH_TEST);
	glEnablei(GL_BLEND, 0);
	glEnable(GL_TEXTURE_3D);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 transfoMat = glm::mat4(1.f);

	if (this->drawMode == DrawMode::Solid) {
		for (std::size_t i = 0; i < this->newGrids.size(); ++i) {
			this->newAPI_drawGrid(mvMat, pMat, transfoMat, this->newGrids[i]);
		}
	} else if (this->drawMode == DrawMode::Volumetric || this->drawMode == DrawMode::VolumetricBoxed) {
		for (std::size_t i = 0; i < this->newGrids.size(); ++i) {
			this->newAPI_drawVolumetric(mvMat, pMat, camPos, this->newGrids[i]);
		}

		if (this->drawMode == DrawMode::VolumetricBoxed) {
			this->drawBoundingBox(this->visuBox, glm::vec3(1., .0, .0), mvMat, pMat);
		}
	}

	for (auto& drawable : this->drawables) {
		drawable->draw(pMat, mvMat, glm::vec4{camPos, 1.f});
	}

	if (not this->newGrids.empty()) {
		this->newAPI_drawPlanes(mvMat, pMat, this->drawMode == DrawMode::Solid);
	} 
	this->drawBoundingBox(this->sceneBB, glm::vec4(.5, .5, .0, 1.), mvMat, pMat);
	this->showVAOstate = false;
}

void Scene::newSHADERS_updateUBOData() {
	this->shouldUpdateUBOData = false;
	for (const auto& grid : this->newGrids) {
		this->setUniformBufferData(grid->uboHandle_colorAttributes, 0, 32, &grid->mainColorChannelAttributes());
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

	// Push them into the vectors, so they can be drawn without indices :
	// Plane X :
	_mesh.positions.push_back(apos);
	_mesh.normals.push_back(anorm);
	_mesh.texture.push_back(ll);
	_mesh.positions.push_back(cpos);
	_mesh.normals.push_back(cnorm);
	_mesh.texture.push_back(lr);
	_mesh.positions.push_back(epos);
	_mesh.normals.push_back(enorm);
	_mesh.texture.push_back(hl);
	_mesh.positions.push_back(epos);
	_mesh.normals.push_back(enorm);
	_mesh.texture.push_back(hl);
	_mesh.positions.push_back(cpos);
	_mesh.normals.push_back(cnorm);
	_mesh.texture.push_back(lr);
	_mesh.positions.push_back(gpos);
	_mesh.normals.push_back(gnorm);
	_mesh.texture.push_back(hr);
	// Plane Y :
	_mesh.positions.push_back(apos);
	_mesh.normals.push_back(anorm);
	_mesh.texture.push_back(ll);
	_mesh.positions.push_back(bpos);
	_mesh.normals.push_back(bnorm);
	_mesh.texture.push_back(lr);
	_mesh.positions.push_back(epos);
	_mesh.normals.push_back(enorm);
	_mesh.texture.push_back(hl);
	_mesh.positions.push_back(epos);
	_mesh.normals.push_back(enorm);
	_mesh.texture.push_back(hl);
	_mesh.positions.push_back(bpos);
	_mesh.normals.push_back(bnorm);
	_mesh.texture.push_back(lr);
	_mesh.positions.push_back(fpos);
	_mesh.normals.push_back(fnorm);
	_mesh.texture.push_back(hr);
	// Plane Z :
	_mesh.positions.push_back(apos);
	_mesh.normals.push_back(anorm);
	_mesh.texture.push_back(ll);
	_mesh.positions.push_back(bpos);
	_mesh.normals.push_back(bnorm);
	_mesh.texture.push_back(lr);
	_mesh.positions.push_back(cpos);
	_mesh.normals.push_back(cnorm);
	_mesh.texture.push_back(hl);
	_mesh.positions.push_back(cpos);
	_mesh.normals.push_back(cnorm);
	_mesh.texture.push_back(hl);
	_mesh.positions.push_back(bpos);
	_mesh.normals.push_back(bnorm);
	_mesh.texture.push_back(lr);
	_mesh.positions.push_back(dpos);
	_mesh.normals.push_back(dnorm);
	_mesh.texture.push_back(hr);

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
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertTex);
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
	// Build position, normals, and tex coordinates all in one line for each vertex
	glm::vec4 apos	= glm::vec4(.0, .0, .0, 1.);
	glm::vec4 anorm = apos - center;
	glm::vec3 atex	= glm::vec3(.0, .0, .0);
	glm::vec4 bpos	= glm::vec4(1., .0, .0, 1.);
	glm::vec4 bnorm = bpos - center;
	glm::vec3 btex	= glm::vec3(1., .0, .0);
	glm::vec4 cpos	= glm::vec4(.0, 1., .0, 1.);
	glm::vec4 cnorm = cpos - center;
	glm::vec3 ctex	= glm::vec3(.0, 1., .0);
	glm::vec4 dpos	= glm::vec4(1., 1., .0, 1.);
	glm::vec4 dnorm = dpos - center;
	glm::vec3 dtex	= glm::vec3(1., 1., .0);
	glm::vec4 epos	= glm::vec4(.0, .0, 1., 1.);
	glm::vec4 enorm = epos - center;
	glm::vec3 etex	= glm::vec3(.0, .0, 1.);
	glm::vec4 fpos	= glm::vec4(1., .0, 1., 1.);
	glm::vec4 fnorm = fpos - center;
	glm::vec3 ftex	= glm::vec3(1., .0, 1.);
	glm::vec4 gpos	= glm::vec4(.0, 1., 1., 1.);
	glm::vec4 gnorm = gpos - center;
	glm::vec3 gtex	= glm::vec3(.0, 1., 1.);
	glm::vec4 hpos	= glm::vec4(1., 1., 1., 1.);
	glm::vec4 hnorm = hpos - center;
	glm::vec3 htex	= glm::vec3(1., 1., 1.);
	_mesh.positions.push_back(apos);
	_mesh.normals.push_back(anorm);
	_mesh.texture.push_back(atex);
	_mesh.positions.push_back(bpos);
	_mesh.normals.push_back(bnorm);
	_mesh.texture.push_back(btex);
	_mesh.positions.push_back(cpos);
	_mesh.normals.push_back(cnorm);
	_mesh.texture.push_back(ctex);
	_mesh.positions.push_back(dpos);
	_mesh.normals.push_back(dnorm);
	_mesh.texture.push_back(dtex);
	_mesh.positions.push_back(epos);
	_mesh.normals.push_back(enorm);
	_mesh.texture.push_back(etex);
	_mesh.positions.push_back(fpos);
	_mesh.normals.push_back(fnorm);
	_mesh.texture.push_back(ftex);
	_mesh.positions.push_back(gpos);
	_mesh.normals.push_back(gnorm);
	_mesh.texture.push_back(gtex);
	_mesh.positions.push_back(hpos);
	_mesh.normals.push_back(hnorm);
	_mesh.texture.push_back(htex);
	unsigned int a			= 0;
	unsigned int b			= 1;
	unsigned int c			= 2;
	unsigned int d			= 3;
	unsigned int e			= 4;
	unsigned int f			= 5;
	unsigned int g			= 6;
	unsigned int h			= 7;
	unsigned int faceIdx1[] = {a, d, c, a, b, d, e, b, a, e, f, b, g, e, a, g, a, c};
	unsigned int faceIdx2[] = {h, e, g, h, f, e, h, g, c, h, c, d, h, d, b, h, f, b};
	_mesh.indices.insert(_mesh.indices.end(), faceIdx1, faceIdx1 + 18);
	_mesh.indices.insert(_mesh.indices.end(), faceIdx2, faceIdx2 + 18);
}

void Scene::setupVBOData(const SimpleVolMesh& _mesh) {
	if (glIsVertexArray(this->vaoHandle) == GL_FALSE) {
		throw std::runtime_error("The vao handle generated by OpenGL has been invalidated !");
	}

	// UPLOAD DATA (VBOs have previously been created in createBuffers() :

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertPos);
	glBufferData(GL_ARRAY_BUFFER, _mesh.positions.size() * sizeof(glm::vec4), _mesh.positions.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertNorm);
	glBufferData(GL_ARRAY_BUFFER, _mesh.normals.size() * sizeof(glm::vec4), _mesh.normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertTex);
	glBufferData(GL_ARRAY_BUFFER, _mesh.texture.size() * sizeof(glm::vec3), _mesh.texture.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(unsigned int), _mesh.indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.cutting_planes.size() * sizeof(unsigned int), _mesh.cutting_planes.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_SinglePlaneElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.planar_view.size() * sizeof(unsigned int), _mesh.planar_view.data(), GL_STATIC_DRAW);

	this->setupVAOPointers();
}

void Scene::setupVAOPointers() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertPos);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertNorm);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertTex);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
}

glm::vec3 Scene::getSceneBoundaries() const {
	return this->sceneBB.getDiagonal();
}

void Scene::printProgramUniforms(const GLuint _pid) {
	const GLsizei bufSize = 256;	// maximum name length
	GLchar name[bufSize];	 // variable name in GLSL
	GLsizei length	= 0;	// name length
	GLint attrCount = 0;	// the count of attributes/uniforms
	GLenum type		= GL_NONE;	  // type of the variable (float, vec3 or mat4, etc)
	GLint size		= 0;	// size of the variable

	std::cerr << "[TRACE] Printing attributes of the program " << +_pid << '\n';
	glUseProgram(_pid);

	std::cerr << "[TRACE]\t\tActive attributes :\n";
	glGetProgramiv(_pid, GL_ACTIVE_ATTRIBUTES, &attrCount);
	for (GLint i = 0; i < attrCount; ++i) {
		//std::cerr << "[TRACE]\t" << ;
		glGetActiveAttrib(_pid, (GLuint) i, bufSize, &length, &size, &type, name);
		fprintf(stderr, "[TRACE]\t\t\tAttribute #%d Type: %u Name: %s\n", i, type, name);
	}

	std::cerr << "[TRACE]\t\tActive uniforms :\n";
	glGetProgramiv(_pid, GL_ACTIVE_UNIFORMS, &attrCount);
	for (GLint i = 0; i < attrCount; ++i) {
		glGetActiveUniform(_pid, (GLuint) i, bufSize, &length, &size, &type, name);
		fprintf(stderr, "[TRACE]\t\t\tUniform #%d Type: %u Name: %s\n", i, type, name);
	}
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
	Image::bbox_t defaultBB = this->sceneBB;
	// Get all corners and put them sequentially in an array :
	std::vector<Image::bbox_t::vec> corners = defaultBB.getAllCorners();
	GLfloat* rawVertices						   = new GLfloat[corners.size() * 3];
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
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_boundingBoxVertices);
	// Upload data to OpenGL :
	glBufferData(GL_ARRAY_BUFFER, corners.size() * 3 * sizeof(GLfloat), rawVertices, GL_STATIC_DRAW);

	// Bind the index buffer :
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_boundingBoxIndices);
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
	glBindVertexArray(this->vaoHandle_boundingBox);

	// Bind vertex buffer :
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_boundingBoxVertices);
	// Enable VAO pointer 0 : vertices
	glEnableVertexAttribArray(0);
	// Enable VAO pointer for 3-component vectors, non-normalized starting at index 0 :
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
}

void Scene::drawBoundingBox(const Image::bbox_t& _box, glm::vec3 color, GLfloat* vMat, GLfloat* pMat) {
	glUseProgram(this->programHandle_BoundingBox);

	glLineWidth(2.0f);

	// Locate uniforms :
	GLint location_pMat	   = glGetUniformLocation(this->programHandle_BoundingBox, "pMat");
	GLint location_vMat	   = glGetUniformLocation(this->programHandle_BoundingBox, "vMat");
	GLint location_bbColor = glGetUniformLocation(this->programHandle_BoundingBox, "bbColor");
	GLint location_bbSize  = glGetUniformLocation(this->programHandle_BoundingBox, "bbSize");
	GLint location_bbPos   = glGetUniformLocation(this->programHandle_BoundingBox, "bbPos");

	Image::bbox_t::vec min  = _box.getMin();
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

	glBindVertexArray(this->vaoHandle_boundingBox);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_boundingBoxIndices);
	this->setupVAOBoundingBox();

	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void*) 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

void Scene::showVisuBoxController(VisuBoxController* _controller) {
	if (this->visuBoxController != nullptr) {
		std::cerr << "Warning : rewriting visu box controller.\n";
	}
	this->visuBoxController = _controller;
	this->visuBoxController->raise();
	this->visuBoxController->show();
}

void Scene::removeVisuBoxController() {
	this->visuBoxController = nullptr;
}

std::pair<glm::uvec3, glm::uvec3> Scene::getVisuBoxCoordinates() {
    std::cerr << "WARNING: trying to call Scene::getVisuBoxCoordinates() on old grid API" << std::endl;
    // TODO: port this function to new API
	// if (this->grids.size() == 0) {
	// 	return std::make_pair<glm::uvec3, glm::uvec3>(glm::uvec3(), glm::uvec3());
	// }
	// if (this->grids[0]->grid.size() == 0) {
	// 	return std::make_pair<glm::uvec3, glm::uvec3>(glm::uvec3(), glm::uvec3());
	// }

	// std::pair<glm::uvec3, glm::uvec3> result;

	// auto mi		   = this->computePlanePositions();
	// glm::uvec3 min = this->grids[0]->grid[0]->worldPositionToIndex(glm::vec4(mi, 1.));
	// result.first   = min;

	// // The second coordinate can be directly taken from the current value (automatically max of scene)
	// result.second = this->visuMax;
	// return result;
	return std::make_pair<glm::uvec3, glm::uvec3>(glm::uvec3(), glm::uvec3());
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
	if (this->newGrids.size() == 0) {
		return;
	}

	// Reset the visu box :
	using vec	  = Image::bbox_t::vec;
	this->visuBox = Image::bbox_t();

	if (this->newGrids.size()) {
		Image::Grid::Ptr g	 = this->newGrids[0]->grid;
		auto min			 = glm::vec3(.0);
		auto max			 = glm::convert_to<float>(g->getResolution());
		Image::bbox_t imgBox = Image::bbox_t(min, max);
		this->visuBox.addPoints(imgBox.getAllCorners());
	}

	return;
}

void Scene::resetVisuBox() {
	this->visuBox = this->sceneDataBB;

	if (this->newGrids.size()) {
		this->visuMin	 = glm::uvec3(0, 0, 0);
		Image::svec3 max = this->newGrids[0]->grid->getResolution();
		this->visuMax	 = glm::uvec3(max.x, max.y, max.z);
		this->updateVisuBoxCoordinates();
	}
}

float Scene::getSceneRadius() {
	// in case of box visu, it's easy :
	if (this->drawMode == VolumetricBoxed) {
		return glm::length(this->visuBox.getDiagonal());
	}
	// if draw solid or volumetric, focus on the part of the scene's BB we want to see
	// need to compute plane positions, and get the god min/max points according to it
	if (this->drawMode == Solid || this->drawMode == Volumetric) {
		// get plane positions :
		glm::vec3 bbmin	   = this->sceneBB.getMin();
		glm::vec3 bbmax	   = this->sceneBB.getMax();
		glm::vec3 planePos = (this->sceneBB.getMin() + this->planeDisplacement * this->sceneBB.getDiagonal());
		glm::vec3 min = glm::vec3(), max = glm::vec3();

		if (this->planeDirection.x < 0) {
			min.x = bbmin.x;
			max.x = planePos.x;
		} else {
			min.x = planePos.x;
			max.x = bbmax.x;
		}
		if (this->planeDirection.y < 0) {
			min.y = bbmin.y;
			max.y = planePos.y;
		} else {
			min.y = planePos.y;
			max.y = bbmax.y;
		}
		if (this->planeDirection.z < 0) {
			min.z = bbmin.z;
			max.z = planePos.z;
		} else {
			min.z = planePos.z;
			max.z = bbmax.z;
		}

		glm::vec3 diag = max - min;
		return glm::length(diag);
	}
	//default value when no things are loaded
	return 1.f;
}

glm::vec3 Scene::getSceneCenter() {
	// in case of box visu, it's easy :
	if (this->drawMode == VolumetricBoxed) {
		return this->visuBox.getMin() + (this->visuBox.getDiagonal() / 2.f);
	}
	// if draw solid, or volumetric focus on the part of the scene's BB we want to see
	// need to compute plane positions, and get the god min/max points according to it
	if (this->drawMode == Solid || this->drawMode == Volumetric) {
		// get plane positions :
		glm::vec3 bbmin	   = this->sceneBB.getMin();
		glm::vec3 bbmax	   = this->sceneBB.getMax();
		glm::vec3 planePos = (this->sceneBB.getMin() + this->planeDisplacement * this->sceneBB.getDiagonal());
		glm::vec3 min = glm::vec3(), max = glm::vec3();

		if (this->planeDirection.x < 0) {
			min.x = bbmin.x;
			max.x = planePos.x;
		} else {
			min.x = planePos.x;
			max.x = bbmax.x;
		}
		if (this->planeDirection.y < 0) {
			min.y = bbmin.y;
			max.y = planePos.y;
		} else {
			min.y = planePos.y;
			max.y = bbmax.y;
		}
		if (this->planeDirection.z < 0) {
			min.z = bbmin.z;
			max.z = planePos.z;
		} else {
			min.z = planePos.z;
			max.z = bbmax.z;
		}

		glm::vec3 diag = max - min;
		return min + diag / 2.f;
	}
	// default value, for no
	return glm::vec3(.5, .5, .5);
}

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
	for (auto& grid : newGrids) {
		grid->colorChannelAttributes[0].setColorScale(static_cast<int>(_c));
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
	for (auto& grid : newGrids) {
		grid->colorChannelAttributes[1].setColorScale(static_cast<int>(_c));
	}
	this->shouldUpdateUBOData = true;
}

void Scene::setRGBMode(RGBMode _mode) {
	this->rgbMode = _mode;
	std::for_each(this->newGrids.begin(), this->newGrids.end(), [this](NewAPI_GridGLView::Ptr& gridView) {
		switch (this->rgbMode) {
			case RGBMode::None:
				gridView->colorChannelAttributes[0].setHidden();
				gridView->colorChannelAttributes[1].setHidden();
				break;
			case RGBMode::RedOnly:
				gridView->colorChannelAttributes[0].setHidden(false);
				gridView->colorChannelAttributes[1].setHidden();
				break;
			case RGBMode::GreenOnly:
				gridView->colorChannelAttributes[0].setHidden();
				gridView->colorChannelAttributes[1].setHidden(false);
				break;
			case RGBMode::RedAndGreen:
				gridView->colorChannelAttributes[0].setHidden(false);
				gridView->colorChannelAttributes[1].setHidden(false);
				break;
			default:
				// do nothing
				break;
		}
	});
	this->shouldUpdateUBOData = true;
	switch (_mode) {
		case RGBMode::None:
			std::cerr << "Set mode to none" << '\n';
			break;
		case RGBMode::RedOnly:
			std::cerr << "Set mode to red" << '\n';
			break;
		case RGBMode::GreenOnly:
			std::cerr << "Set mode to green" << '\n';
			break;
		case RGBMode::RedAndGreen:
			std::cerr << "Set mode to both channels" << '\n';
			break;
		case RGBMode::HandEColouring:
			std::cerr << "Set mode to both channels" << '\n';
			break;
		default:
			std::cerr << "Cannot set unknown mode\n";
			break;
	}
}

void Scene::slotSetPlaneDisplacementX(float scalar) {
	this->planeDisplacement.x = scalar;
}
void Scene::slotSetPlaneDisplacementY(float scalar) {
	this->planeDisplacement.y = scalar;
}
void Scene::slotSetPlaneDisplacementZ(float scalar) {
	this->planeDisplacement.z = scalar;
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

void Scene::slotSetMinTexValue(double val) {
	this->textureBounds0.x = val;
	this->updateVis();
	this->updateCVR();
}
void Scene::slotSetMaxTexValue(double val) {
	this->textureBounds0.y = val;
	this->updateVis();
	this->updateCVR();
}
void Scene::slotSetMinTexValueAlternate(double val) {
	this->textureBounds1.x = val;
	this->updateVis();
	this->updateCVR();
}
void Scene::slotSetMaxTexValueAlternate(double val) {
	this->textureBounds1.y = val;
	this->updateVis();
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
	return;
}

void Scene::setColor1(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r, g, b);
	this->color1 = glm::convert_to<float>(qtcolor);
	this->signal_updateUserColorScales();
	return;
}

void Scene::setColor0Alternate(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r, g, b);
	this->color0_second = glm::convert_to<float>(qtcolor);
	this->signal_updateUserColorScales();
	return;
}

void Scene::setColor1Alternate(qreal r, qreal g, qreal b) {
	glm::vec<3, qreal, glm::highp> qtcolor(r, g, b);
	this->color1_second = glm::convert_to<float>(qtcolor);
	this->signal_updateUserColorScales();
	return;
}

void Scene::signal_updateUserColorScales() {
	this->shouldUpdateUserColorScales = true;
}

void Scene::newSHADERS_updateUserColorScales() {
	this->shouldUpdateUserColorScales = false;
	TextureUpload colorScaleUploadParameters;
	std::size_t textureSize = this->gl_limit_max_texture_size / 2u;
	float textureSize_f		= static_cast<float>(this->gl_limit_max_texture_size / 2u);
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
	this->texHandle_colorScale_user0		  = this->uploadTexture1D(colorScaleUploadParameters);

	colorScaleUploadParameters.data	 = colorScaleData_user1.data();
	this->texHandle_colorScale_user1 = this->uploadTexture1D(colorScaleUploadParameters);
}

void Scene::updateCVR() {
	// For all grids, signal they need to be
	for (const auto& grid : this->newGrids) {
		grid->colorChannelAttributes[0].setMinVisible(this->textureBounds0.x);
		grid->colorChannelAttributes[0].setMaxVisible(this->textureBounds0.y);
		grid->colorChannelAttributes[0].setMinColorScale(this->colorBounds0.x);
		grid->colorChannelAttributes[0].setMaxColorScale(this->colorBounds0.y);
		grid->colorChannelAttributes[1].setMinVisible(this->textureBounds1.x);
		grid->colorChannelAttributes[1].setMaxVisible(this->textureBounds1.y);
		grid->colorChannelAttributes[1].setMinColorScale(this->colorBounds1.x);
		grid->colorChannelAttributes[1].setMaxColorScale(this->colorBounds1.y);
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

void Scene::newAPI_tex3D_buildMesh(NewAPI_GridGLView::Ptr& grid, const std::string path) {
	/* Here, we'll build the tetrahedral mesh used to visualize the 3D structure of the acquisition in real-time. */
	/* Textures built : index of vertices, index of per-face normals for tetrahedra, index of per-face neighbors
	 * for tetrehedra */

	VolMeshData mesh{};

	if (path.empty()) {
		this->newAPI_tex3D_generateMESH(grid, mesh);
	} else {
		this->newAPI_tex3D_loadMESHFile(path, grid, mesh);
	}

	// Hard-coded indices smh, got to move to a more portable (and readable) solution :
	std::size_t indices[4][3];
	indices[0][0] = 3;
	indices[0][1] = 1;
	indices[0][2] = 2;
	indices[1][0] = 3;
	indices[1][1] = 2;
	indices[1][2] = 0;
	indices[2][0] = 3;
	indices[2][1] = 0;
	indices[2][2] = 1;
	indices[3][0] = 2;
	indices[3][1] = 1;
	indices[3][2] = 0;

	// INIT NEIGHBORS :
	mesh.neighbors.clear();
	mesh.neighbors.resize(mesh.tetrahedra.size());
	for (std::size_t i = 0; i < mesh.tetrahedra.size(); i++) {
		mesh.neighbors[i].resize(4, -1);
		mesh.normals.push_back(std::array<glm::vec4, 4>{glm::vec4(.0f), glm::vec4(.0f), glm::vec4(.0f), glm::vec4(.0f)});
	}

	// Adjacency map :
	std::map<Face, std::pair<int, int>> adjacent_faces;
	// generate the correspondance by looking at which faces are similar : [MEDIUM/HEAVY COMPUTATION]
	for (std::size_t i = 0; i < mesh.tetrahedra.size(); i++) {
		for (int v = 0; v < 4; v++) {
			// find similar face in other tetrahedrons :
			Face face										 = Face(mesh.tetrahedra[i][indices[v][0]],
				 mesh.tetrahedra[i][indices[v][1]],
				 mesh.tetrahedra[i][indices[v][2]]);
			std::map<Face, std::pair<int, int>>::iterator it = adjacent_faces.find(face);
			if (it == adjacent_faces.end()) {
				adjacent_faces[face] = std::make_pair(static_cast<int>(i), v);
			} else {
				mesh.neighbors[i][v]								= it->second.first;
				mesh.neighbors[it->second.first][it->second.second] = i;
			}
		}
	}

	// determine the size of the texture, depending on the # of tetrahedron neighbors :
	std::size_t vertWidth = 0, vertHeight = 0;
	std::size_t normWidth = 0, normHeight = 0;
	std::size_t coorWidth = 0, coorHeight = 0;
	std::size_t neighbWidth = 0, neighbHeight = 0;

	__GetTexSize(mesh.tetrahedra.size() * 4 * 3, &vertWidth, &vertHeight);
	__GetTexSize(mesh.tetrahedra.size() * 4, &normWidth, &normHeight);
	__GetTexSize(mesh.tetrahedra.size() * 4 * 3, &coorWidth, &coorHeight);
	__GetTexSize(mesh.tetrahedra.size() * 4, &neighbWidth, &neighbHeight);

	grid->volumetricMesh.tetrahedraCount = mesh.tetrahedra.size();

#ifdef DIRECT_FROM_VOLMESH_TO_ARRAY_SIZE
	GLfloat* rawVertices  = new GLfloat[grid.volumetricMesh.tetrahedraCount * 4 * 3 * 3];
	GLfloat* rawNormals	  = new GLfloat[grid.volumetricMesh.tetrahedraCount * 4 * 4];
	GLfloat* tex		  = new GLfloat[grid.volumetricMesh.tetrahedraCount * 4 * 3 * 3];
	GLfloat* rawNeighbors = new GLfloat[grid.volumetricMesh.tetrahedraCount * 4 * 3];
#else
	GLfloat* rawVertices = new GLfloat[vertWidth * vertHeight * 3];
	GLfloat* rawNormals = new GLfloat[normWidth * normHeight * 4];
	GLfloat* tex = new GLfloat[coorWidth * coorHeight * 3];
	GLfloat* rawNeighbors = new GLfloat[neighbWidth * neighbHeight * 3];
#endif

	/*
	Warning : the loop that follows is horribly bad : it duplicates every texture coordinate by the number of times
	the vertex is used within the mesh. It MUST be improved once the method is working, to save up on video memory.
	It does the same for the vertex positions.
	*/
	std::size_t iter   = 0;
	std::size_t texcnt = 0;
	std::size_t ncount = 0;
	for (std::size_t t = 0; t < mesh.tetrahedra.size(); ++t) {
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
					tex[texcnt]			= tetCoord[k];
					texcnt++;
				}
			}

			// Compute this face's normal :
			glm::vec4 n1   = mesh.positions[tetrahedron[indices[i][1]]] - mesh.positions[tetrahedron[indices[i][0]]];
			glm::vec4 n2   = mesh.positions[tetrahedron[indices[i][2]]] - mesh.positions[tetrahedron[indices[i][0]]];
			glm::vec4 norm = glm::normalize(glm::cross(n1, n2));
			// Put inverse of dot with opposing vertex in norm.w :
			glm::vec4 v1			  = mesh.positions[tetrahedron[i]] - mesh.positions[tetrahedron[(i + 1) % 4]];
			glm::vec4::value_type val = 1. / glm::dot(v1, norm);
			norm.w					  = val;
			// Put it in the array :
			for (std::size_t n = 0; n < 4; ++n) {
				rawNormals[ncount++] = norm[n];
			}

			rawNeighbors[iter] = static_cast<GLfloat>(mesh.neighbors[t][i]);
			iter += 3;
		}
	}

	// Struct to upload the texture to OpenGL :
	TextureUpload texParams = {};

	// Vertex positions :
	texParams.minmag.x = GL_NEAREST;
	texParams.minmag.y = GL_NEAREST;
	texParams.lod.y	   = -1000.f;
	texParams.wrap.s   = GL_CLAMP;
	texParams.wrap.t   = GL_CLAMP;
	// Swizzle and alignment unchanged, not present in Texture3D
	texParams.internalFormat			 = GL_RGB32F;
	texParams.size.x					 = vertWidth;
	texParams.size.y					 = vertHeight;
	texParams.format					 = GL_RGB;
	texParams.type						 = GL_FLOAT;
	texParams.data						 = rawVertices;
	grid->volumetricMesh.vertexPositions = this->uploadTexture2D(texParams);

	// Face normals :
	texParams.internalFormat		 = GL_RGBA32F;
	texParams.size.x				 = normWidth;
	texParams.size.y				 = normHeight;
	texParams.format				 = GL_RGBA;
	texParams.data					 = rawNormals;
	grid->volumetricMesh.faceNormals = this->uploadTexture2D(texParams);

	// Texture coordinates :
	texParams.internalFormat				= GL_RGB32F;
	texParams.size.x						= coorWidth;
	texParams.size.y						= coorHeight;
	texParams.format						= GL_RGB;
	texParams.data							= tex;
	grid->volumetricMesh.textureCoordinates = this->uploadTexture2D(texParams);

	// Neighborhood information :
	texParams.size.x				  = neighbWidth;
	texParams.size.y				  = neighbHeight;
	texParams.data					  = rawNeighbors;
	grid->volumetricMesh.neighborhood = this->uploadTexture2D(texParams);

	delete[] tex;
	delete[] rawVertices;
	delete[] rawNormals;
	delete[] rawNeighbors;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Scene::newAPI_tex3D_loadMESHFile(const std::string file, const NewAPI_GridGLView::Ptr& grid, VolMeshData& mesh) {
	Image::bbox_t box = grid->grid->getBoundingBox();

	std::ifstream myfile(file.c_str());

	if (not myfile.is_open()) {
		throw std::runtime_error("could not open mesh file");
	}

	std::string meshString;
	std::size_t sizeV, sizeT, sizeTet, dimension;

	myfile >> meshString;
	while (meshString.find("Dimension") == std::string::npos)
		myfile >> meshString;

	myfile >> dimension;
	while (meshString.find("Vertices") == std::string::npos)
		myfile >> meshString;

	myfile >> sizeV;

	float maxX = 0, maxY = 0, maxZ = 0;

	std::vector<glm::vec4> rawVert;
	Image::bbox_t normalBox;
	Image::bbox_t deformedBox;

	int s;
	for (unsigned int i = 0; i < sizeV; i++) {
		float p[3];
		for (unsigned int j = 0; j < 3; j++)
			myfile >> p[j];

		if (p[0] > maxX) {
			maxX = p[0];
		}
		if (p[1] > maxY) {
			maxY = p[1];
		}
		if (p[2] > maxZ) {
			maxZ = p[2];
		}
		glm::vec4 position = glm::vec4(p[0], p[1], p[2], 1.);
		normalBox.addPoint(Image::bbox_t::vec(p[0], p[1], p[2]));
		rawVert.push_back(position);
#warning Transform API is still in-progress.
		deformedBox.addPoint(Image::bbox_t::vec(position.x, position.y, position.z));
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
	glm::vec3 gridsize = glm::convert_to<float>(grid->grid->getResolution()) * grid->grid->getVoxelDimensions();
	glm::vec4 scale	   = glm::vec4(gridsize / size, 1.);

	std::cerr << "[LOG][" << __FILE__ << ':' << __LINE__ << "] Max dimensions of the mesh : [" << maxX << ',' << maxY << ',' << maxZ << "]\n";
	for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
		//mesh.positions[i] *= scale;
		mesh.texture.push_back(glm::vec3(
		  std::min(1.f, rawVert[i].x / size.x),
		  std::min(1.f, rawVert[i].y / size.y),
		  std::min(1.f, rawVert[i].z / size.z)));
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

	if (dimension == 3) {
		while (meshString.find("Tetrahedra") == std::string::npos)
			myfile >> meshString;

		myfile >> sizeTet;
		unsigned int t[4];
		for (unsigned int i = 0; i < sizeTet; i++) {
			for (unsigned int j = 0; j < 4; j++) {
				myfile >> t[j];
			}
			myfile >> s;
			if (s > 0) {
				mesh.tetrahedra.push_back({t[0] - 1, t[1] - 1, t[2] - 1, t[3] - 1});
			}
		}
	}
	std::cerr << "loadMesh() done\n";

	myfile.close();
}

void Scene::newAPI_tex3D_generateMESH(NewAPI_GridGLView::Ptr& grid, VolMeshData& mesh) {
	// typedef for bounding box's vector type :
	using vec_t = typename Image::bbox_t::vec;

	//Min and diagonal of the bounding box (used for position computation) :
	const vec_t min	 = grid->grid->getBoundingBox().getMin();
	const vec_t diag = grid->grid->getBoundingBox().getDiagonal();
	// Dimensions, subject to change :
	std::size_t xv			 = 10;
	glm::vec4::value_type xs = diag.x / static_cast<glm::vec4::value_type>(xv);
	std::size_t yv			 = 10;
	glm::vec4::value_type ys = diag.y / static_cast<glm::vec4::value_type>(yv);
	std::size_t zv			 = 10;
	glm::vec4::value_type zs = diag.z / static_cast<glm::vec4::value_type>(zv);

	// Size of tetrahedra, to compute epsilon :
	glm::vec3 epsVolu	 = glm::vec3(xs, ys, zs);
	grid->defaultEpsilon = epsVolu;

	// Containers for the computation of positions and texture coordinates :
	glm::vec4 pos = glm::vec4();
	glm::vec3 tex = glm::vec3();
	// Transformation to apply to the mesh :
	MatrixTransform::Ptr grid_transform_pointer = std::dynamic_pointer_cast<MatrixTransform>(grid->grid->getPrecomputedMatrix());
	glm::mat4 transfo							= grid_transform_pointer->matrix();
#warning Transform API is still in-progress.

	// Create vertices along with their texture coordinates. We
	// need to go one after because we want _n_ tetrahedra, and
	// thus must finish the last 'row'/'column' of tetrahedra :
	for (std::size_t k = 0; k <= zv; ++k) {
		for (std::size_t j = 0; j <= yv; ++j) {
			for (std::size_t i = 0; i <= xv; ++i) {
				pos = transfo * glm::vec4(
								  min.x + static_cast<vec_t::value_type>(i) * xs,
								  min.y + static_cast<vec_t::value_type>(j) * ys,
								  min.z + static_cast<vec_t::value_type>(k) * zs,
								  1.);
				tex = glm::vec3(
				  static_cast<vec_t::value_type>(i) / static_cast<vec_t::value_type>(xv),
				  static_cast<vec_t::value_type>(j) / static_cast<vec_t::value_type>(yv),
				  static_cast<vec_t::value_type>(k) / static_cast<vec_t::value_type>(zv));

				mesh.positions.push_back(pos);
				mesh.texture.push_back(tex);
			}
		}
	}

	// Return position for vertex generated at indices I, J, K :
	std::size_t xt = xv + 1;
	std::size_t yt = yv + 1;
	auto getIndice = [&, xt, yt](std::size_t i, std::size_t j, std::size_t k) -> std::size_t {
		return i + j * xt + k * xt * yt;
	};

	// Create tetrahedra :
	for (std::size_t k = 0; k < zv; ++k) {
		for (std::size_t j = 0; j < yv; ++j) {
			for (std::size_t i = 0; i < xv; ++i) {
				mesh.tetrahedra.push_back({getIndice(i + 1, j, k), getIndice(i + 1, j + 1, k), getIndice(i, j + 1, k), getIndice(i + 1, j + 1, k + 1)});
				mesh.tetrahedra.push_back({getIndice(i, j, k + 1), getIndice(i, j, k), getIndice(i, j + 1, k + 1), getIndice(i + 1, j, k + 1)});
				mesh.tetrahedra.push_back({getIndice(i, j + 1, k + 1), getIndice(i + 1, j, k), getIndice(i + 1, j + 1, k + 1), getIndice(i + 1, j, k + 1)});
				mesh.tetrahedra.push_back({getIndice(i, j, k), getIndice(i + 1, j, k), getIndice(i, j + 1, k + 1), getIndice(i + 1, j, k + 1)});
				mesh.tetrahedra.push_back({getIndice(i, j, k), getIndice(i + 1, j, k), getIndice(i, j + 1, k), getIndice(i, j + 1, k + 1)});
				mesh.tetrahedra.push_back({getIndice(i, j + 1, k), getIndice(i + 1, j, k), getIndice(i + 1, j + 1, k + 1), getIndice(i, j + 1, k + 1)});
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

	GLfloat* rawVisibility = new GLfloat[256 * 3];

	for (std::size_t i = 0; i < 256; ++i) {
		rawVisibility[i * 3 + 0] = 100.f;
		rawVisibility[i * 3 + 1] = 100.f;
		rawVisibility[i * 3 + 2] = 100.f;
	}

	// Struct to upload the texture to OpenGL :
	TextureUpload texParams = {};

	// Vertex positions :
	texParams.minmag.x = GL_NEAREST;
	texParams.minmag.y = GL_NEAREST;
	texParams.lod.y	   = -1000.f;
	texParams.wrap.s   = GL_CLAMP;
	texParams.wrap.t   = GL_CLAMP;
	// Swizzle and alignment unchanged, not present in Texture3D
	texParams.internalFormat = GL_RGB32F;
	texParams.size.x		 = visWidth;
	texParams.size.y		 = visHeight;
	texParams.format		 = GL_RGB;
	texParams.type			 = GL_FLOAT;
	texParams.data			 = rawVisibility;
	volMesh.visibilityMap	 = this->uploadTexture2D(texParams);

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

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertPos);
	glBufferData(GL_ARRAY_BUFFER, 12 * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertNorm);
	glBufferData(GL_ARRAY_BUFFER, 12 * 3 * sizeof(GLfloat), normals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertTex);
	glBufferData(GL_ARRAY_BUFFER, 12 * 2 * sizeof(GLfloat), textureCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Texture3D_VertIdx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * sizeof(GLushort), indices, GL_STATIC_DRAW);

	glBindVertexArray(this->vaoHandle_VolumetricBuffers);
	this->tex3D_bindVAO();
}

void Scene::tex3D_bindVAO() {
	glBindVertexArray(this->vaoHandle_VolumetricBuffers);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertPos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertNorm);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertTex);
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
