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

template<class T>
std::remove_reference_t<T> const& as_const(T&&t){return t;}

inline int __GetOpenGLError ( char* szFile, int iLine )
{
	int    retCode = 0;
	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		std::cerr << "GLError @ " << szFile << ":" << iLine << " : ";
		switch (glErr) {
			case GL_INVALID_ENUM:
				std::cerr << "Invalid enum";
			break;
			case GL_INVALID_VALUE:
				std::cerr << "Invalid value";
			break;
			case GL_INVALID_OPERATION:
				std::cerr << "Invalid operation";
			break;
			case GL_OUT_OF_MEMORY:
				std::cerr << "out of memory";
			break;
			case GL_STACK_OVERFLOW:
				std::cerr << "out of memory";
			break;
			case GL_STACK_UNDERFLOW:
				std::cerr << "out of memory";
			break;
			default:
				std::cerr << "(unknown error code)";
			break;
		}
		std::cerr << '\n';
		glErr = glGetError();
		retCode = 1;
	}
//	if (retCode) {
//		exit(EXIT_FAILURE);
//	}
	return retCode;
}

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

Scene::Scene(GridControl* const gc) {
	/** This constructor not only creates the object, but also sets the default values for the Scene in order
	 *  to be drawn, even if it is empty at the time of the first call to a draw function.
	 */

	this->isInitialized = false;
	this->inputGridVisible = false;
	this->outputGridVisible = false;
	this->colorOrTexture = true; // by default, the cube is shown !
	this->showVAOstate = false;

	this->grids.clear();

	this->context = nullptr;
	this->debugLog = nullptr;
	this->controlPanel = nullptr;
	#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
	this->inputGrid_Blue = nullptr;
	this->inputGrid_Red = nullptr;
	#else
	this->inputGrid = nullptr;
	#endif
	this->outputGrid = nullptr;
	this->mesh = nullptr;
	this->gridControl = gc;

	this->minTexVal = uchar(0);
	this->maxTexVal = uchar(255);
	this->minColorVal = uchar(1);
	this->maxColorVal = uchar(255);
	this->renderSize = 0;

	// Default light positions : at the vertices of a unit cube.
	this->lightPositions = {
		glm::vec3(.0, .0, .0), glm::vec3(1., .0, .0), glm::vec3(.0, 1., .0), glm::vec3(1., 1., .0),
		glm::vec3(.0, .0, 1.), glm::vec3(1., .0, 1.), glm::vec3(.0, 1., 1.), glm::vec3(1., 1., 1.)
	};

	this->planePosition = glm::vec3(.0, .0, .0);
	this->planeDirection = glm::vec3(1., 1., 1.);
	this->planeDisplacement = glm::vec3(.0, .0, .0);
	DiscreteGrid::bbox_t::vec min(.0, .0, .0);
	DiscreteGrid::bbox_t::vec max(1., 1., 1.);
	this->sceneBB = DiscreteGrid::bbox_t(min, max);
	this->clipDistanceFromCamera = 5.f;
	this->drawMode = DrawMode::Solid;

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
	this->volumetricMesh = {};
	this->vboHandle_Texture3D_VertPos = 0;
	this->vboHandle_Texture3D_VertNorm = 0;
	this->vboHandle_Texture3D_VertTex = 0;
	this->vboHandle_Texture3D_VertIdx = 0;

	this->visibleDomains = new unsigned int[256];
	for (std::size_t i = 0; i < 256; ++i) {
		this->visibleDomains[i] = 1;
	}
}

Scene::~Scene(void) {
	glDeleteTextures(1, &this->texHandle_ColorScaleGrid);
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

	// Get OpenGL functions from the currently bound context :
	this->initializeOpenGLFunctions();

	// If the context supports the GL_KHR_debug extension, enable a logger :
	if (_context->hasExtension(QByteArrayLiteral("GL_KHR_debug"))) {
		this->debugLog = new QOpenGLDebugLogger;
		this->debugLog->initialize();
	} else {
		this->debugLog = nullptr;
		std::cerr << "OpenGL logging not enabled" << '\n';
	}

	// The default parameters have already been set in the constructor. We
	// need to initialize the OpenGL objects now. Shaders, VAOs, VBOs.

	// Compile the shaders :
	this->recompileShaders(false);

	// Create VAO, VBO elements and populate them in generateSceneData() :
	this->createBuffers();
	this->generateSceneData();

	std::vector<float> colorScale = this->generateColorScale(0, 255);
	this->uploadColorScale(colorScale);

	if (this->controlPanel) {
		this->controlPanel->activatePanels();
	}

	std::cerr << "Finished initializing scene" << '\n';
}

void Scene::printOpenGLMessage(const QOpenGLDebugMessage& message) {
	if ((message.severity() & QOpenGLDebugMessage::Severity::HighSeverity) != 0) {
		std::cerr << message.message().toStdString() << '\n';
	} else {
		std::cerr << "Message id " << message.id() << " ignored.\n";
	}
}

void Scene::printGridInfo(const std::shared_ptr<DiscreteGrid>& grid) {
	std::cerr << "[INFO] Information about the grid " << grid->getGridName() << " :\n";
	DiscreteGrid::sizevec3 res = grid->getGridDimensions();
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
		GetOpenGLError();
		this->glBindVertexArray(buf);
		GetOpenGLError();
		if (this->glIsVertexArray(buf) == GL_FALSE) {
			std::cerr << "[ERROR]["<< __FILE__ << ":" << __LINE__ <<"] : Could not create VAO object " << name << '\n';
		} else {
			std::cerr << "[LOG]["<< __FILE__ << ":" << __LINE__ <<"] : Created VAO object " << name << '\n';
		}
		return buf;
	};

	/// @b Create a buffer, bind it and see if it has been succesfully created server-side.
	auto createVBO = [&, this](GLenum bufType, std::string name) -> GLuint {
		GLuint buf = 0;
		this->glGenBuffers(1, &buf);
		GetOpenGLError();
		this->glBindBuffer(bufType, buf);
		GetOpenGLError();
		if (this->glIsBuffer(buf) == GL_FALSE) {
			std::cerr << "[ERROR]["<< __FILE__ << ":" << __LINE__ <<"] : Could not create buffer object " << name << '\n';
		} else {
			std::cerr << "[LOG]["<< __FILE__ << ":" << __LINE__ <<"] : Created buffer object " << name << '\n';
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
	GridGLView gridView(_grid);

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
	auto dimensions = _grid->getGridDimensions();
	gridTexture.level = 0;
	gridTexture.internalFormat = GL_R8UI;
	gridTexture.size.x = dimensions.x;
	gridTexture.size.y = dimensions.y;
	gridTexture.size.z = dimensions.z;
	gridTexture.format = GL_RED_INTEGER;
	gridTexture.type = GL_UNSIGNED_BYTE;
	gridTexture.data = _grid->getData().data();

	gridView.gridTexture = this->uploadTexture3D(gridTexture);

	gridView.boundingBoxColor = glm::vec3(.4, .6, .3); // olive-colored

	this->tex3D_buildTexture();
	this->tex3D_buildMesh(gridView, meshPath);
	this->tex3D_buildVisTexture(gridView.volumetricMesh);
	this->tex3D_buildBuffers(gridView.volumetricMesh);

	this->grids.push_back(gridView);

	this->updateVis();

	this->updateBoundingBox();
}

void Scene::updateBoundingBox(void) {
	this->sceneBB = DiscreteGrid::bbox_t();
	for (std::size_t i = 0; i < this->grids.size(); ++i) {
		this->sceneBB.addPoints(this->grids[i].grid->getBoundingBoxWorldSpace().getAllCorners());
	}

	// Update light positions :
	auto corners = this->sceneBB.getAllCorners();
	for (std::size_t i = 0; i < corners.size(); ++i) {
		this->lightPositions[i] = glm::convert_to<float>(corners[i]);
	}

	return;
}

void Scene::recompileShaders(bool verbose) {
	GLuint newProgram = this->compileShaders("./shaders/voxelgrid.vert", "./shaders/voxelgrid.geom", "./shaders/voxelgrid.frag", verbose);
	GLuint newPlaneProgram = this->compileShaders("./shaders/plane.vert", "", "./shaders/plane.frag", verbose);
	GLuint newPlaneViewerProgram = this->compileShaders("./shaders/texture_explorer.vert", "", "./shaders/texture_explorer.frag", verbose);
	GLuint newVolumetricProgram = this->compileShaders("./shaders/transfer_mesh.vert", "./shaders/transfer_mesh.geom", "./shaders/transfer_mesh.frag", verbose);
	GLuint newBoundingBoxProgram = this->compileShaders("./shaders/bounding_box.vert", "", "./shaders/bounding_box.frag", verbose);

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
	GetOpenGLError();
	GetOpenGLError();

	// Create a shader object :
	GLuint _sha = glCreateShader(shaType);
	GetOpenGLError();

	// Open the file :
	std::ifstream shaFile = std::ifstream(path.c_str(), std::ios_base::in);
	if (!shaFile.is_open()) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] Error : could not get the contents of shader file " << path << '\n';
		return -1;
	}

	// Get the file's length in characters :
	shaFile.seekg(0, shaFile.end);
	std::size_t shaFileSize = static_cast<std::size_t>(shaFile.tellg());
	shaFile.seekg(0, shaFile.beg);

	// Get the file's contents and null-terminate it :
	char* shaSource = new char[shaFileSize+1];
	shaFile.read(shaSource, shaFileSize);
	shaSource[shaFileSize] = '\0';

	// Source it into the shader object :
	glShaderSource(_sha, 1, const_cast<const char**>(&shaSource), 0);

	// We can free up the host memory now :
	delete[] shaSource;
	shaFile.close();

	glCompileShader(_sha);
	GetOpenGLError();

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
		GetOpenGLError();
		std::cerr << shaderInfoLog << '\n';
		delete[] shaderInfoLog;

		std::cerr << __FILE__ << ":" << __LINE__ << " : end Log ***********************************************" << '\n';
	}

	GLint result = GL_FALSE;
	glGetShaderiv(_sha, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] : Could not compile shader.\n";
		return 0;
	}

	return _sha;
}

GLuint Scene::compileProgram(const GLuint vSha, const GLuint gSha, const GLuint fSha, bool verbose) {
	// Check any shader was passed to the program :
	if (vSha == 0 && gSha == 0 && fSha == 0) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] : No shader IDs were passed to the function.\n" <<
			"\tArguments : vSha(" << vSha << "), gSha(" << gSha << "), fSha(" << fSha << ")";
	}

	GLuint _prog = glCreateProgram();
	GetOpenGLError();
	if (vSha != 0) { glAttachShader(_prog, vSha); GetOpenGLError(); }
	if (gSha != 0) { glAttachShader(_prog, gSha); GetOpenGLError(); }
	if (fSha != 0) { glAttachShader(_prog, fSha); GetOpenGLError(); }
	GetOpenGLError();

	glLinkProgram(_prog);
	GetOpenGLError();

	int InfoLogLength = 0;
	glGetProgramiv(_prog, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(_prog, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		GetOpenGLError();
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
	GetOpenGLError();

	GLuint texHandle = 0;
	glGenTextures(1, &texHandle);
	GetOpenGLError();
	glBindTexture(GL_TEXTURE_1D, texHandle);
	GetOpenGLError();

	// Min and mag filters :
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, tex.minmag.x);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, tex.minmag.y);
	GetOpenGLError();

	// Set the min and max LOD values :
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_LOD, tex.lod.x);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAX_LOD, tex.lod.y);
	GetOpenGLError();

	// Set the wrap parameters :
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, tex.wrap.x);
	GetOpenGLError();

	// Set the swizzle the user wants :
	glTexParameteriv(GL_TEXTURE_1D, GL_TEXTURE_SWIZZLE_RGBA, glm::value_ptr(tex.swizzle));
	GetOpenGLError();

	// Set the pixel alignment :
	glPixelStorei(GL_PACK_ALIGNMENT, tex.alignment.x);
	glPixelStorei(GL_UNPACK_ALIGNMENT, tex.alignment.y);
	GetOpenGLError();

	glTexImage1D(GL_TEXTURE_1D,		// GLenum : Target
		static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
		tex.internalFormat,		// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		tex.size.x,			// GLsizei: Image width
		static_cast<GLint>(0),		// GLint  : Border. This value MUST be 0.
		tex.format,			// GLenum : Format of the pixel data
		tex.type,			// GLenum : Type (the data type as in uchar, uint, float ...)
		tex.data			// void*  : Data to load into the buffer
	);
	GetOpenGLError();

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
	GetOpenGLError();

	GLuint texHandle = 0;
	glGenTextures(1, &texHandle);
	GetOpenGLError();
	glBindTexture(GL_TEXTURE_2D, texHandle);
	GetOpenGLError();

	// Min and mag filters :
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex.minmag.x);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex.minmag.y);
	GetOpenGLError();

	// Set the min and max LOD values :
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, tex.lod.x);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, tex.lod.y);
	GetOpenGLError();

	// Set the wrap parameters :
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex.wrap.x);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex.wrap.y);
	GetOpenGLError();

	// Set the swizzle the user wants :
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, glm::value_ptr(tex.swizzle));
	GetOpenGLError();

	// Set the pixel alignment :
	glPixelStorei(GL_PACK_ALIGNMENT, tex.alignment.x);
	glPixelStorei(GL_UNPACK_ALIGNMENT, tex.alignment.y);
	GetOpenGLError();

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
	GetOpenGLError();

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
	GetOpenGLError();

	GLuint texHandle = 0;
	glGenTextures(1, &texHandle);
	GetOpenGLError();
	glBindTexture(GL_TEXTURE_3D, texHandle);
	GetOpenGLError();

	// Min and mag filters :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, tex.minmag.x);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, tex.minmag.y);
	GetOpenGLError();

	// Set the min and max LOD values :
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_LOD, tex.lod.x);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, tex.lod.y);
	GetOpenGLError();

	// Set the wrap parameters :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, tex.wrap.x);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, tex.wrap.y);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, tex.wrap.z);
	GetOpenGLError();

	// Set the swizzle the user wants :
	glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, glm::value_ptr(tex.swizzle));
	GetOpenGLError();

	// Set the pixel alignment :
	glPixelStorei(GL_PACK_ALIGNMENT, tex.alignment.x);
	glPixelStorei(GL_UNPACK_ALIGNMENT, tex.alignment.y);
	GetOpenGLError();

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
	GetOpenGLError();

	return texHandle;
}

void Scene::fillNearestNeighbor() {
/*
	InterpolationMethods method = InterpolationMethods::NearestNeighbor;
	using timepoint = std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<double, std::ratio<1,1>>>;
	if (this->mesh != nullptr && this->outputGrid != nullptr) {
		timepoint start_point = std::chrono::high_resolution_clock::now();
		this->mesh->populateOutputGrid(method);
		timepoint end_point = std::chrono::high_resolution_clock::now();
		std::cerr << "To fill the grid, it took " << (end_point - start_point).count() << " seconds" << '\n';
	}
	this->loadVoxelGrid();
*/
	std::cerr << "[ERROR] Not implemented anymore ! Use Ctrl+S to save a grid of your choosing.\n";
	return;
}

void Scene::fillTrilinear() {
/*
	InterpolationMethods method = InterpolationMethods::TriLinear;
	using timepoint = std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<double, std::ratio<1,1>>>;
	if (this->mesh != nullptr && this->outputGrid != nullptr) {
		timepoint start_point = std::chrono::high_resolution_clock::now();
		this->mesh->populateOutputGrid(method);
		timepoint end_point = std::chrono::high_resolution_clock::now();
		std::cerr << "To fill the grid, it took " << (end_point - start_point).count() << " seconds" << '\n';
	}
	this->loadVoxelGrid();
*/
	std::cerr << "[ERROR] Not implemented anymore ! Use Ctrl+S to save a grid of your choosing.\n";
	return;
}

void Scene::drawPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading, float zoomRatio, GLfloat* vMat, GLfloat* pMat) {
	if (this->grids.size() == 0) { return; }

	glEnable(GL_DEPTH_TEST);
	GetOpenGLError();

	uint min = 0; // min index for drawing commands
	if (_plane == planes::x) { min =  0; }
	if (_plane == planes::y) { min =  6; }
	if (_plane == planes::z) { min = 12; }

	glUseProgram(this->programHandle_PlaneViewer);
	GetOpenGLError();
	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_SinglePlaneElement);

	// Do for all grids :
	if (this->grids.size() > 0) {
		this->prepPlane_SingleUniforms(_plane, _heading, fbDims, zoomRatio, this->grids[0]);

		GLint locV = glGetUniformLocation(this->programHandle_PlaneViewer, "vMat");
		GLint locP = glGetUniformLocation(this->programHandle_PlaneViewer, "pMat");
		glUniformMatrix4fv(locV, 1, GL_FALSE, vMat);
		glUniformMatrix4fv(locP, 1, GL_FALSE, pMat);

		this->setupVAOPointers();
		GetOpenGLError();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*)(min*sizeof(GLuint)));
		GetOpenGLError();
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GetOpenGLError();
	glBindVertexArray(0);
	GetOpenGLError();
	glUseProgram(0);
	GetOpenGLError();
	this->showVAOstate = false;
}

void Scene::drawVolumetric(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, const GridGLView& grid) {
	glUseProgram(this->programHandle_VolumetricViewer);
	GetOpenGLError();

	/// @b Shortcut for glGetUniform, since this can result in long lines.
	auto getUniform = [&](const char* name) -> GLint {
		GLint g = glGetUniformLocation(this->programHandle_VolumetricViewer, name);
		// if (g < 0) { std::cerr << "Cannot find uniform " << name << '\n'; }
		return g;
	};

	// Textures :
	GLint location_vertices_translation = getUniform("vertices_translations");
	GLint location_normals_translation = getUniform("normals_translations");
	GLint location_visibility_texture = getUniform("visibility_texture");
	GLint location_texture_coordinates = getUniform("texture_coordinates");
	GLint location_neighbors = getUniform("neighbors");
	GLint location_Mask = getUniform("Mask");
	GLint location_color_texture = getUniform("color_texture");
	GetOpenGLError();
	// Scalars :
	GLint location_voxelSize = getUniform("voxelSize");
	GLint location_specRef = getUniform("specRef");
	GLint location_shininess = getUniform("shininess");
	GLint location_diffuseRef = getUniform("diffuseRef");
	GetOpenGLError();
	// Vectors/arrays :
	GLint location_cam = getUniform("cam");
	GLint location_cut = getUniform("cut");
	GLint location_cutDirection = getUniform("cutDirection");
	GLint location_clipDistanceFromCamera = getUniform("clipDistanceFromCamera");
	GLint location_visibilityMap = getUniform("visiblity_map");
	GLint location_colorBounds = getUniform("colorBounds");
	GLint location_textureBounds = getUniform("textureBounds");
	GetOpenGLError();
	// Matrices :
	GLint location_mMat = getUniform("mMat");
	GLint location_vMat = getUniform("vMat");
	GLint location_pMat = getUniform("pMat");
	GetOpenGLError();
	GLint location_light0 = getUniform("lightPositions[0]");
	GLint location_light1 = getUniform("lightPositions[1]");
	GLint location_light2 = getUniform("lightPositions[2]");
	GLint location_light3 = getUniform("lightPositions[3]");
	GLint location_light4 = getUniform("lightPositions[4]");
	GLint location_light5 = getUniform("lightPositions[5]");
	GLint location_light6 = getUniform("lightPositions[6]");
	GLint location_light7 = getUniform("lightPositions[7]");
	GetOpenGLError();

	std::size_t tex = 0;
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, grid.volumetricMesh.vertexPositions);
	glUniform1i(location_vertices_translation, tex);
	tex++;
	GetOpenGLError();
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, grid.volumetricMesh.faceNormals);
	glUniform1i(location_normals_translation, tex);
	tex++;
	GetOpenGLError();
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, grid.volumetricMesh.visibilityMap);
	glUniform1i(location_visibility_texture, tex);
	tex++;
	GetOpenGLError();
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, grid.volumetricMesh.textureCoordinates);
	glUniform1i(location_texture_coordinates, tex);
	tex++;
	GetOpenGLError();
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, grid.volumetricMesh.neighborhood);
	glUniform1i(location_neighbors, tex);
	tex++;
	GetOpenGLError();
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_3D, grid.gridTexture);
	glUniform1i(location_Mask, tex);
	tex++;
	GetOpenGLError();
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_ColorScaleGrid);
	glUniform1i(location_color_texture, tex);
	tex++;
	GetOpenGLError();

	glUniform1f(location_diffuseRef, .8f);
	glUniform1f(location_specRef, .8f);
	glUniform1f(location_shininess, .8f);
	#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
	// same for both grids, we can leave it at that
	glUniform3fv(location_voxelSize, 1, glm::value_ptr(this->inputGrid_Blue->getVoxelDimensions()));
	#else
	glUniform3fv(location_voxelSize, 1, glm::value_ptr(grid.grid->getVoxelDimensions()));
	#endif
	glUniform1uiv(location_visibilityMap, 256, this->visibleDomains);
	GetOpenGLError();

	DiscreteGrid::bbox_t::vec position = this->sceneBB.getMin();
	DiscreteGrid::bbox_t::vec diagonal = this->sceneBB.getDiagonal();
	#ifndef PLANE_POS_FLOOR
	glm::vec3 planePos = glm::round(position + this->planeDisplacement * diagonal); // PLANE POSITIONS
	#else
	glm::vec3 planePos = glm::floor(position + this->planeDisplacement * diagonal); // PLANE POSITIONS
	#endif

	glUniform3fv(location_cam, 1, glm::value_ptr(camPos));
	// glUniform3fv(location_cut, 1, glm::value_ptr(this->planePosition));
	glUniform3fv(location_cut, 1, glm::value_ptr(planePos));	// PLANE POSITIONS
	glUniform3fv(location_cutDirection, 1, glm::value_ptr(this->planeDirection));
	glUniform1f(location_clipDistanceFromCamera, this->clipDistanceFromCamera);
	GetOpenGLError();

	#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
	const glm::mat4& gridTransfo = this->inputGrid_Blue->getTransform_GridToWorld();
	#else
	const glm::mat4& gridTransfo = grid.grid->getTransform_GridToWorld();
	#endif
	glUniformMatrix4fv(location_mMat, 1, GL_FALSE, glm::value_ptr(gridTransfo));
	glUniformMatrix4fv(location_vMat, 1, GL_FALSE, mvMat);
	glUniformMatrix4fv(location_pMat, 1, GL_FALSE, pMat);
	GetOpenGLError();

	glm::vec2 colorbounds{static_cast<float>(this->minColorVal), static_cast<float>(this->maxColorVal)};
	glm::vec2 texbounds{static_cast<float>(this->minTexVal), static_cast<float>(this->maxTexVal)};
	glUniform2fv(location_colorBounds, 1, glm::value_ptr(colorbounds));
	glUniform2fv(location_textureBounds, 1, glm::value_ptr(texbounds));
	GetOpenGLError();

	glUniform3fv(location_light0, 1, glm::value_ptr(this->lightPositions[0]));
	glUniform3fv(location_light1, 1, glm::value_ptr(this->lightPositions[1]));
	glUniform3fv(location_light2, 1, glm::value_ptr(this->lightPositions[2]));
	glUniform3fv(location_light3, 1, glm::value_ptr(this->lightPositions[3]));
	glUniform3fv(location_light4, 1, glm::value_ptr(this->lightPositions[4]));
	glUniform3fv(location_light5, 1, glm::value_ptr(this->lightPositions[5]));
	glUniform3fv(location_light6, 1, glm::value_ptr(this->lightPositions[6]));
	glUniform3fv(location_light7, 1, glm::value_ptr(this->lightPositions[7]));
	GetOpenGLError();

	glBindVertexArray(this->vaoHandle_VolumetricBuffers);
	this->tex3D_bindVAO();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Texture3D_VertIdx);
	GetOpenGLError();

	glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, (void*)0, grid.volumetricMesh.tetrahedraCount);
	GetOpenGLError();

	// Unbind textures :
	for (std::size_t t = tex; t >= 0 && t < tex+1; t--) {
		glActiveTexture(GL_TEXTURE0 + t);
		glBindTexture(GL_TEXTURE_3D, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_1D, 0);
		GetOpenGLError();
	}

	// Unbind program, buffers and VAO :
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	GetOpenGLError();

	// draw grid BB :
	this->drawBoundingBox(grid.grid->getBoundingBoxWorldSpace(), grid.boundingBoxColor, mvMat, pMat);
}

void Scene::drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	// Plane X :
	if (this->planeVisibility.x == true) {
		glUseProgram(this->programHandle_Plane3D);
		glBindVertexArray(this->vaoHandle);
		GetOpenGLError();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
		GetOpenGLError();

		for (std::size_t i = 0; i < this->grids.size(); ++i) {
			this->prepPlaneUniforms(mvMat, pMat, planes::x, this->grids[i], showTexOnPlane);
			this->setupVAOPointers();
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, static_cast<GLvoid*>(0));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}

	// Plane Y :
	if (this->planeVisibility.y == true) {
		glUseProgram(this->programHandle_Plane3D);
		glBindVertexArray(this->vaoHandle);
		GetOpenGLError();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
		GetOpenGLError();
		for (std::size_t i = 0; i < this->grids.size(); ++i) {
			this->prepPlaneUniforms(mvMat, pMat, planes::y, this->grids[i], showTexOnPlane);
			this->setupVAOPointers();

			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*)(6*sizeof(GLuint)));
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}

	// Plane Z :
	if (this->planeVisibility.z == true) {
		glUseProgram(this->programHandle_Plane3D);
		glBindVertexArray(this->vaoHandle);
		GetOpenGLError();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
		GetOpenGLError();

		for (std::size_t i = 0; i < this->grids.size(); ++i) {
			this->prepPlaneUniforms(mvMat, pMat, planes::z, this->grids[i], showTexOnPlane);
			this->setupVAOPointers();

			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*)(12*sizeof(GLuint)));
			GetOpenGLError();
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}

	glDisable(GL_BLEND);
}

void Scene::drawWithPlanes(GLfloat mvMat[], GLfloat pMat[]) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	GetOpenGLError();

	glm::mat4 transfoMat = glm::mat4(1.f);

	for (std::size_t i = 0; i < this->grids.size(); ++i) {
		#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
		this->drawGrid_Generic(mvMat, pMat, transfoMat, this->texHandle_InputGrid, this->inputGrid_Blue);
		#else
		this->drawGrid(mvMat, pMat, transfoMat, this->grids[i]);
		#endif
		this->drawBoundingBox(this->sceneBB, this->grids[i].boundingBoxColor, mvMat, pMat);
	}

	this->drawPlanes(mvMat, pMat);

	this->showVAOstate = false;
}

void Scene::prepGridUniforms(GLfloat *mvMat, GLfloat *pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid>& grid) {
	// Get the world to grid transform :
	glm::mat4 transfoMat = baseMatrix * grid->getTransform_GridToWorld();

	// Get the uniform locations :
	GLint mMatrix_Loc = glGetUniformLocation(this->programHandle_projectedTex, "mMatrix");
	GLint vMatrix_Loc = glGetUniformLocation(this->programHandle_projectedTex, "vMatrix");
	GLint pMatrix_Loc = glGetUniformLocation(this->programHandle_projectedTex, "pMatrix");
	GLint lightPos_Loc = glGetUniformLocation(this->programHandle_projectedTex, "lightPos");
	GLint voxelGridOrigin_Loc = glGetUniformLocation(this->programHandle_projectedTex, "voxelGridOrigin");
	GLint voxelGridSize_Loc = glGetUniformLocation(this->programHandle_projectedTex, "voxelGridSize");
	GLint voxelSize_Loc = glGetUniformLocation(this->programHandle_projectedTex, "voxelSize");
	GLint location_texBounds = glGetUniformLocation(this->programHandle_projectedTex, "texBounds");
	GLint location_colorBounds = glGetUniformLocation(this->programHandle_projectedTex, "colorBounds");
	GLint drawMode_Loc = glGetUniformLocation(this->programHandle_projectedTex, "drawMode");
	GLint colorOrTexture_Loc = glGetUniformLocation(this->programHandle_projectedTex, "colorOrTexture");
	GLint texDataLoc = glGetUniformLocation(this->programHandle_projectedTex, "texData");
	GLint colorScaleLoc = glGetUniformLocation(this->programHandle_projectedTex, "colorScale");
	GLint planePositionsLoc = glGetUniformLocation(this->programHandle_projectedTex, "planePositions");
	GLint location_planeDirections = glGetUniformLocation(this->programHandle_projectedTex, "planeDirections");
	GLint gridPositionLoc = glGetUniformLocation(this->programHandle_projectedTex, "gridPosition");
	GLint location_textureBounds = glGetUniformLocation(this->programHandle_projectedTex, "textureBounds");

	GetOpenGLError();

	DiscreteGrid::bbox_t::vec origin = grid->getBoundingBox().getMin();
	DiscreteGrid::bbox_t::vec originWS = grid->getBoundingBoxWorldSpace().getMin();
	DiscreteGrid::sizevec3 gridDims = grid->getGridDimensions();
	glm::vec3 dims = glm::vec3(static_cast<float>(gridDims.x), static_cast<float>(gridDims.y), static_cast<float>(gridDims.z));

	glm::vec2 texBounds{static_cast<float>(this->minTexVal), static_cast<float>(this->maxTexVal)};
	glm::vec2 colorBounds{static_cast<float>(this->minColorVal), static_cast<float>(this->maxColorVal)};

	glUniform3fv(voxelGridOrigin_Loc, 1, glm::value_ptr(origin));
	glUniform3fv(voxelGridSize_Loc, 1, glm::value_ptr(dims));
	glUniform3fv(voxelSize_Loc, 1, glm::value_ptr(grid->getVoxelDimensions()));
	glUniform2fv(location_texBounds, 1, glm::value_ptr(texBounds));
	glUniform2fv(location_colorBounds, 1, glm::value_ptr(colorBounds));
	glm::vec2 texbounds{static_cast<float>(this->minColorVal), static_cast<float>(this->maxColorVal)};
	glUniform2fv(location_textureBounds, 1, glm::value_ptr(texbounds));
	glUniform1ui(colorOrTexture_Loc, this->colorOrTexture ? 1 : 0);
	if (grid->getData().size() == 0) {
		glUniform1ui(drawMode_Loc, 2);
	} else {
		glUniform1ui(drawMode_Loc, this->drawMode);
	}
	GetOpenGLError();

	// Textures :
	if (grid->getData().size() != 0) {
		glActiveTexture(GL_TEXTURE0 + 0);
		GetOpenGLError();
		glEnable(GL_TEXTURE_3D);
		GetOpenGLError();
		glBindTexture(GL_TEXTURE_3D, texHandle);
		GetOpenGLError();
		glUniform1i(texDataLoc, 0);
		GetOpenGLError();
	}
	glActiveTexture(GL_TEXTURE0 + 1);
	GetOpenGLError();
	glEnable(GL_TEXTURE_1D);
	GetOpenGLError();
	glBindTexture(GL_TEXTURE_1D, this->texHandle_ColorScaleGrid);
	GetOpenGLError();
	glUniform1i(colorScaleLoc, 1);
	GetOpenGLError();

	DiscreteGrid::bbox_t::vec position = this->sceneBB.getMin();
	DiscreteGrid::bbox_t::vec diagonal = this->sceneBB.getDiagonal();
	#ifndef PLANE_POS_FLOOR
	glm::vec3 planePos = glm::round(position + this->planeDisplacement * diagonal); // PLANE POSITIONS
	#else
	glm::vec3 planePos = glm::floor(position + this->planeDisplacement * diagonal); // PLANE POSITIONS
	#endif

	glUniform3fv(planePositionsLoc, 1, glm::value_ptr(planePos));
	glUniform3fv(location_planeDirections, 1, glm::value_ptr(this->planeDirection));
	glUniform3fv(gridPositionLoc, 1, glm::value_ptr(originWS));

	// Apply the uniforms :
	glUniformMatrix4fv(mMatrix_Loc, 1, GL_FALSE, glm::value_ptr(transfoMat));
	glUniformMatrix4fv(vMatrix_Loc, 1, GL_FALSE, &mvMat[0]);
	glUniformMatrix4fv(pMatrix_Loc, 1, GL_FALSE, &pMat[0]);
	glUniform4fv(lightPos_Loc, 1, glm::value_ptr(lightPos));
	GetOpenGLError();
}

void Scene::prepPlaneUniforms(GLfloat *mvMat, GLfloat *pMat, planes _plane, const GridGLView& grid, bool showTexOnPlane) {
	GetOpenGLError();
	// lambda function to check uniform location :
	auto checkUniformLocation = [](const GLint id, const char* name = nullptr) -> bool {
		if (id == -1) {
			/*
			std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Could not " <<
				"find uniform \"" << name << "\" in program.\n";
			*/
			return false;
		}
		return true;
	};

	// quick alias :
	using boxvec_t = typename DiscreteGrid::bbox_t::vec;

	// Get uniform locations for the program :
	GLint location_mMatrix = glGetUniformLocation(this->programHandle_Plane3D, "model_Mat");
	GLint location_vMatrix = glGetUniformLocation(this->programHandle_Plane3D, "view_Mat");
	GLint location_pMatrix = glGetUniformLocation(this->programHandle_Plane3D, "projection_Mat");
	GLint location_gridTransform = glGetUniformLocation(this->programHandle_Plane3D, "gridTransform");
	GLint location_sceneBBPosition = glGetUniformLocation(this->programHandle_Plane3D, "sceneBBPosition");
	GLint location_sceneBBDiagonal = glGetUniformLocation(this->programHandle_Plane3D, "sceneBBDiagonal");
	GLint location_gridSize = glGetUniformLocation(this->programHandle_Plane3D, "gridSize");
	GLint location_gridDimensions = glGetUniformLocation(this->programHandle_Plane3D, "gridDimensions");
	GLint location_currentPlane = glGetUniformLocation(this->programHandle_Plane3D, "currentPlane");
	GLint location_planePosition = glGetUniformLocation(this->programHandle_Plane3D, "planePositions");
	GLint location_planeDirection = glGetUniformLocation(this->programHandle_Plane3D, "planeDirections");
	GLint location_texData = glGetUniformLocation(this->programHandle_Plane3D, "texData");
	GLint location_colorScale = glGetUniformLocation(this->programHandle_Plane3D, "colorScale");
	GLint location_colorBounds = glGetUniformLocation(this->programHandle_Plane3D, "colorBounds");
	GLint location_textureBounds = glGetUniformLocation(this->programHandle_Plane3D, "textureBounds");
	GLint location_showTex = glGetUniformLocation(this->programHandle_Plane3D, "showTex");
	GetOpenGLError();

	// Check the location values :
	checkUniformLocation(location_mMatrix, "mMatrix");
	checkUniformLocation(location_vMatrix, "vMatrix");
	checkUniformLocation(location_pMatrix, "pMatrix");
	checkUniformLocation(location_gridTransform, "gridTransform");
	checkUniformLocation(location_sceneBBPosition, "sceneBBPosition");
	checkUniformLocation(location_sceneBBDiagonal, "sceneBBDiagonal");
	checkUniformLocation(location_gridSize, "gridSize");
	checkUniformLocation(location_gridDimensions, "gridDimensions");
	checkUniformLocation(location_currentPlane, "currentPlane");
	checkUniformLocation(location_planePosition, "planePositions");
	checkUniformLocation(location_texData, "texData");
	checkUniformLocation(location_colorScale, "colorScale");
	GetOpenGLError();

	// Generate the data we need :
	glm::mat4 transform = glm::mat4(1.f);
	#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
	// Data here is the same for both grids :
	glm::mat4 gridTransfo = this->inputGrid_Blue->getTransform_GridToWorld();
	DiscreteGrid::bbox_t bbws = this->inputGrid_Blue->getBoundingBoxWorldSpace();
	glm::vec3 dims = glm::convert_to<glm::vec3::value_type>(this->inputGrid_Blue->getGridDimensions());
	#else
	glm::mat4 gridTransfo = grid.grid->getTransform_GridToWorld();
	DiscreteGrid::bbox_t bbws = grid.grid->getBoundingBoxWorldSpace();
	glm::vec3 dims = glm::convert_to<glm::vec3::value_type>(grid.grid->getGridDimensions());
	#endif
	glm::vec3 size = bbws.getDiagonal();
	GLint plIdx = (_plane == planes::x) ? 1 : (_plane == planes::y) ? 2 : 3;
	GetOpenGLError();

	// gridTransfo = glm::mat4(1.);
	gridTransfo = grid.grid->getTransform_GridToWorld();

	DiscreteGrid::bbox_t::vec position = this->sceneBB.getMin();
	DiscreteGrid::bbox_t::vec diagonal = this->sceneBB.getDiagonal();
	#ifndef PLANE_POS_FLOOR
	glm::vec3 planePos = glm::round(position + this->planeDisplacement * diagonal); // PLANE POSITIONS
	#else
	glm::vec3 planePos = glm::floor(position + this->planeDisplacement * diagonal); // PLANE POSITIONS
	#endif

	glUniformMatrix4fv(location_mMatrix, 1, GL_FALSE, glm::value_ptr(transform));
	glUniformMatrix4fv(location_vMatrix, 1, GL_FALSE, mvMat);
	glUniformMatrix4fv(location_pMatrix, 1, GL_FALSE, pMat);
	glUniformMatrix4fv(location_gridTransform, 1, GL_FALSE, glm::value_ptr(gridTransfo));
	glUniform3fv(location_sceneBBPosition, 1, glm::value_ptr(position));
	glUniform3fv(location_sceneBBDiagonal, 1, glm::value_ptr(diagonal));
	glUniform3fv(location_gridSize, 1, glm::value_ptr(size));
	glUniform3fv(location_gridDimensions, 1, glm::value_ptr(dims));
	glUniform1i(location_currentPlane, plIdx);

	glm::vec2 colorBounds{static_cast<float>(this->minColorVal), static_cast<float>(this->maxColorVal)};
	glm::vec2 textureBounds{static_cast<float>(this->minTexVal), static_cast<float>(this->maxTexVal)};
	glUniform2fv(location_colorBounds, 1, glm::value_ptr(colorBounds));
	glUniform2fv(location_textureBounds, 1, glm::value_ptr(textureBounds));

	glUniform3fv(location_planePosition, 1, glm::value_ptr(planePos)); // PLANE POSITIONS
	glUniform3fv(location_planeDirection, 1, glm::value_ptr(this->planeDirection));
	glUniform1i(location_showTex, showTexOnPlane ? 1 : 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, grid.gridTexture);
	glUniform1i(location_texData, 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	glEnable(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_ColorScaleGrid);
	glUniform1i(location_colorScale, 1);
	GetOpenGLError();
}

void Scene::prepPlane_SingleUniforms(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, const GridGLView& _grid) {
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
	glm::mat4 gridTransform = _grid.grid->getTransform_WorldToGrid();
	// Grid dimensions :
	glm::vec3 gridDimensions = glm::convert_to<glm::vec3::value_type>(_grid.grid->getGridDimensions());

	// Depth of the plane :
	DiscreteGrid::bbox_t::vec position = this->sceneBB.getMin();
	DiscreteGrid::bbox_t::vec diagonal = this->sceneBB.getDiagonal();
	#ifndef PLANE_POS_FLOOR
	glm::vec3 planePos = glm::round(position + this->planeDisplacement * diagonal); // PLANE POSITIONS
	#else
	glm::vec3 planePos = glm::floor(position + this->planeDisplacement * diagonal); // PLANE POSITIONS
	#endif

	// Color and texture bounds :
	glm::vec2 colorBounds{static_cast<float>(this->minColorVal), static_cast<float>(this->maxColorVal)};
	glm::vec2 textureBounds{static_cast<float>(this->minTexVal), static_cast<float>(this->maxTexVal)};

	// Uniform locations :
	// VShader :
	GLint location_fbDims = glGetUniformLocation(this->programHandle_PlaneViewer, "fbDims");
	GLint location_bbDims = glGetUniformLocation(this->programHandle_PlaneViewer, "bbDims");
	GLint location_planeIndex = glGetUniformLocation(this->programHandle_PlaneViewer, "planeIndex");
	GLint location_gridTransform = glGetUniformLocation(this->programHandle_PlaneViewer, "gridTransform");
	GLint location_gridDimensions = glGetUniformLocation(this->programHandle_PlaneViewer, "gridDimensions");
	GLint location_gridBBDiagonal = glGetUniformLocation(this->programHandle_PlaneViewer, "sceneBBDiagonal");
	GLint location_gridBBPosition = glGetUniformLocation(this->programHandle_PlaneViewer, "sceneBBPosition");
	GLint location_planePositions = glGetUniformLocation(this->programHandle_PlaneViewer, "planePositions");
	GLint location_heading = glGetUniformLocation(this->programHandle_PlaneViewer, "heading");
	GLint location_zoom = glGetUniformLocation(this->programHandle_PlaneViewer, "zoom");
	// FShader :
	GLint location_texData = glGetUniformLocation(this->programHandle_PlaneViewer, "texData");
	GLint location_colorScale = glGetUniformLocation(this->programHandle_PlaneViewer, "colorScale");
	GLint location_colorBounds = glGetUniformLocation(this->programHandle_PlaneViewer, "colorBounds");
	GLint location_textureBounds = glGetUniformLocation(this->programHandle_PlaneViewer, "textureBounds");
	GetOpenGLError();

	if (this->showVAOstate) {
		std::cerr << "[TRACE] Program uniforms :" << '\n';
		this->printProgramUniforms(this->programHandle_PlaneViewer);
		std::cerr << "[TRACE][Shader variables] " << "fbDims : " << +location_fbDims << '\n';
		std::cerr << "[TRACE][Shader variables] " << "bbDims : " << +location_bbDims << '\n';
		std::cerr << "[TRACE][Shader variables] " << "planeIndex : " << +location_planeIndex << '\n';
		std::cerr << "[TRACE][Shader variables] " << "gridTransform : " << +location_gridTransform << '\n';
		std::cerr << "[TRACE][Shader variables] " << "gridDimensions : " << +location_gridDimensions << '\n';
		std::cerr << "[TRACE][Shader variables] " << "gridBBDiagonal : " << +location_gridBBDiagonal << '\n';
		std::cerr << "[TRACE][Shader variables] " << "gridBBPosition : " << +location_gridBBPosition << '\n';
		std::cerr << "[TRACE][Shader variables] " << "depth : " << +location_planePositions << '\n';
		std::cerr << "[TRACE][Shader variables] " << "texData : " << +location_texData << '\n';
		std::cerr << "[TRACE][Shader variables] " << "colorScale : " << +location_colorScale << '\n';
	}

	// Uniform variables :
	glUniform2fv(location_fbDims, 1, glm::value_ptr(fbDims));
	glUniform2fv(location_bbDims, 1, glm::value_ptr(gridBBDims));
	glUniform1ui(location_planeIndex, (_plane == planes::x) ? 1 : (_plane == planes::y) ? 2 : 3);
	glUniformMatrix4fv(location_gridTransform, 1, GL_FALSE, glm::value_ptr(gridTransform));
	glUniform4f(location_gridDimensions, gridDimensions.x, gridDimensions.y, gridDimensions.z, 1.f);
	glUniform4f(location_gridBBDiagonal, bbox.x, bbox.y, bbox.z, 1.f);
	glUniform4f(location_gridBBPosition, posBox.x, posBox.y, posBox.z, .0f);
	glUniform3fv(location_planePositions, 1, glm::value_ptr(planePos));
	glUniform2fv(location_colorBounds, 1, glm::value_ptr(colorBounds));
	glUniform2fv(location_textureBounds, 1, glm::value_ptr(textureBounds));
	glUniform1ui(location_heading, plane_heading);
	glUniform1f(location_zoom, zoomRatio);
	GetOpenGLError();

	// Uniform samplers :
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, _grid.gridTexture);
	glUniform1i(location_texData, 0);
	GetOpenGLError();

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_1D, this->texHandle_ColorScaleGrid);
	glUniform1i(location_colorScale, 1);
	GetOpenGLError();
}

void Scene::drawGrid(GLfloat *mvMat, GLfloat *pMat, glm::mat4 baseMatrix, const GridGLView& grid) {
	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	// If the grid has data, show it :
	if (grid.grid->getData().size() > 0) {
		glUseProgram(this->programHandle_projectedTex);
		glBindVertexArray(this->vaoHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Element);

		this->prepGridUniforms(mvMat, pMat, lightPos, baseMatrix, grid.gridTexture, grid.grid);
		this->setupVAOPointers();

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);
		GetOpenGLError();
	}

	// No matter the grid's state, print the bounding box :
	this->drawBoundingBox(grid.grid->getBoundingBoxWorldSpace(), grid.boundingBoxColor, mvMat, pMat);
}

void Scene::draw3DView(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, bool showTexOnPlane) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_3D);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	GetOpenGLError();

	glm::mat4 transfoMat = glm::mat4(1.f);

	if (this->drawMode == DrawMode::Solid) {
		for (std::size_t i = 0; i < this->grids.size(); ++i) {
			this->drawGrid(mvMat, pMat, transfoMat, this->grids[i]);
		}
	} else if (this->drawMode == DrawMode::Volumetric) {
		for (std::size_t i = 0; i < this->grids.size(); ++i) {
			this->drawVolumetric(mvMat, pMat, camPos, this->grids[i]);
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
	if (_plane == planes::y) { planeBegin = 6; }
	if (_plane == planes::z) { planeBegin = 12; }

	// Add 8 here because the 8 vertices of the cube will be placed
	// before the tex coordinates we're interested in :
	GLsizeiptr begin = (planeBegin + 8)*sizeof(glm::vec3);

	// Select the vertices' tex coordinates in order to draw it the 'right' way up :
	void* data = nullptr;
	switch(_heading) {
		case North: data = (void*) north.data(); break;
		case East : data = (void*) east.data();  break;
		case West : data = (void*) west.data();  break;
		case South: data = (void*) south.data(); break;
	}

	// Bind vertex texture coordinate buffer :
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertTex);
	GetOpenGLError();
	// Modify a part of the data :
	glBufferSubData(GL_ARRAY_BUFFER, begin, 6*sizeof(glm::vec3), data);
	GetOpenGLError();

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
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, _mesh.positions.size()*sizeof(glm::vec4), _mesh.positions.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertNorm);
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, _mesh.normals.size()*sizeof(glm::vec4), _mesh.normals.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertTex);
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, _mesh.texture.size()*sizeof(glm::vec3), _mesh.texture.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Element);
	GetOpenGLError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size()*sizeof(unsigned int), _mesh.indices.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_PlaneElement);
	GetOpenGLError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.cutting_planes.size()*sizeof(unsigned int), _mesh.cutting_planes.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_SinglePlaneElement);
	GetOpenGLError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.planar_view.size() * sizeof(unsigned int), _mesh.planar_view.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	this->setupVAOPointers();
}

void Scene::setupVAOPointers() {
	glEnableVertexAttribArray(0);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertPos);
	GetOpenGLError();
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();

	glEnableVertexAttribArray(1);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertNorm);
	GetOpenGLError();
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();

	glEnableVertexAttribArray(2);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_VertTex);
	GetOpenGLError();
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();
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
		DiscreteGrid::sizevec3 d = _grid->getGridDimensions();
		// compute translation along Z :
		float w = static_cast<float>(d.x) * .39;
		float displacement = w * std::abs(std::sin(angleRad));
		transfoMat = glm::translate(transfoMat, glm::vec3(.0, .0, -displacement));
	}

	transfoMat[0][0] = 0.39 * std::cos(angleRad);
	transfoMat[0][2] = 0.39 * std::sin(angleRad);
	transfoMat[1][1] = 0.39;
	transfoMat[2][2] = 1.927 * std::cos(angleRad);

	std::cerr << "Matrix : " << '\n';
	for (int i = 0; i < 4; ++i) {
		std::cerr << '{' << transfoMat[i][0] << ", " << transfoMat[i][1] << ", " << transfoMat[i][2] << ", " << transfoMat[i][3] << "}\n";
	}
	std::cerr << '\n';

	return transfoMat;
	// return glm::mat4(1.f);
	// return glm::translate(glm::mat4(1.f), glm::vec3(.0, .0, 20.));
}

std::vector<float> Scene::generateColorScale(std::size_t minVal, std::size_t maxVal) {
	std::vector<float> raw_colors;
	raw_colors.resize(3*256);

	for (std::size_t i = 0; i < 256; ++i) {
		// Apply the RGB2HSV conversion (adapted from the shaders) :
		glm::vec3 color = glm::vec3(.0f, .0f, .0f);
		if (i >= minVal && i <= maxVal) {
			float a = static_cast<float>(minVal) / 255.f;
			float b = static_cast<float>(maxVal) / 255.f;
			#ifdef COLOR_SCALE_RELATIVE_COLORS
			float c	= .2f * b;
			float d	= .7f * b;
			#else
			float c = 50.f / 255.f;
			float d = 200.f / 255.f;
			#endif
			float r = 1.f - ((b - a) / (d - c)) * ((static_cast<float>(i)/255.f)-c)+a;
			glm::vec4 k = glm::vec4(1.f, 2.f/3.f, 1.f/3.f, 3.f);
			glm::vec3 p = glm::abs(glm::fract(glm::vec3(r,r,r) + glm::vec3(k.x,k.y,k.z)) * 6.f - glm::vec3(k.w));
			color = glm::mix(glm::vec3(k.x, k.x, k.x), glm::clamp(p-glm::vec3(k.x, k.x, k.x), .1f, .7f), r);
		} else if (i > maxVal) {
			color.r = 1.f;
			color.g = 1.f;
			color.b = 1.f;
		}
		raw_colors[3u*i+0] = color.r;
		raw_colors[3u*i+1] = color.g;
		raw_colors[3u*i+2] = color.b;
		/*
		std::cerr << "[TRACE][" << __PRETTY_FUNCTION__ << "] Color index : " << std::setw(3) << i << " : { x = "
			<< raw_colors[3u*i+0] << ", y = " << raw_colors[3u*i+1] << ", z = " << raw_colors[3u*i+2] << " }\n";
		*/
	}

	return raw_colors;
}

void Scene::uploadColorScale(const std::vector<float>& colorScale) {
	TextureUpload texParams = {};

	// Vertex positions :
	texParams.minmag.x = GL_NEAREST;
	texParams.minmag.y = GL_NEAREST;
	texParams.wrap.s = GL_CLAMP_TO_EDGE;
	// Swizzle and alignment unchanged, not present in Texture3D
	texParams.internalFormat = GL_RGB;
	texParams.size.x = 256;
	texParams.format = GL_RGB;
	texParams.type = GL_FLOAT;
	texParams.data = colorScale.data();

	/*
	glGenTextures(1, &this->texHandle_ColorScaleGrid);
	GetOpenGLError();

	glBindTexture(GL_TEXTURE_1D, this->texHandle_ColorScaleGrid);
	GetOpenGLError();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GetOpenGLError();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GetOpenGLError();

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_FLOAT, colorScale.data());
	GetOpenGLError();
	*/
	this->texHandle_ColorScaleGrid = this->uploadTexture1D(texParams);
}

void Scene::updateVis() {
	for (uint i = 0; i < this->minTexVal; ++i) {
		this->visibleDomains[i] = 0;
	}
	for (uint i = this->minTexVal; i < this->maxTexVal; ++i) {
		this->visibleDomains[i] = 1;
	}
	for (uint i = this->maxTexVal; i < 256; ++i) {
		this->visibleDomains[i] = 0;
	}
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
	GetOpenGLError();
	// Upload data to OpenGL :
	glBufferData(GL_ARRAY_BUFFER, corners.size()*3*sizeof(GLfloat), rawVertices, GL_STATIC_DRAW);
	GetOpenGLError();

	// Bind the index buffer :
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_boundingBoxIndices);
	GetOpenGLError();
	// Upload data to OpenGL :
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24*sizeof(GLuint), &(rawIndices[0]), GL_STATIC_DRAW);
	GetOpenGLError();
	// Unbind the buffer :
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GetOpenGLError();

	// Setup VAO pointer for BB drawing :
	this->setupVAOBoundingBox();

	// Unbind the buffer :
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GetOpenGLError();
	// Unbind VAO :
	glBindVertexArray(0);
	GetOpenGLError();

	// Free memory :
	delete[] rawVertices;
}

void Scene::setupVAOBoundingBox() {
	glBindVertexArray(this->vaoHandle_boundingBox);
	GetOpenGLError();
	// Bind vertex buffer :
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_boundingBoxVertices);
	GetOpenGLError();
	// Enable VAO pointer 0 : vertices
	glEnableVertexAttribArray(0);
	GetOpenGLError();
	// Enable VAO pointer for 3-component vectors, non-normalized starting at index 0 :
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();
}

void Scene::drawBoundingBox(const DiscreteGrid::bbox_t& _box, glm::vec3 color, GLfloat* vMat, GLfloat* pMat) {
	glUseProgram(this->programHandle_BoundingBox);
	GetOpenGLError();

	glLineWidth(2.0f);
	GetOpenGLError();

	// Locate uniforms :
	GLint location_pMat = glGetUniformLocation(this->programHandle_BoundingBox, "pMat");
	GLint location_vMat = glGetUniformLocation(this->programHandle_BoundingBox, "vMat");
	GLint location_bbColor = glGetUniformLocation(this->programHandle_BoundingBox, "bbColor");
	GLint location_bbSize = glGetUniformLocation(this->programHandle_BoundingBox, "bbSize");
	GLint location_bbPos = glGetUniformLocation(this->programHandle_BoundingBox, "bbPos");
	GetOpenGLError();

	DiscreteGrid::bbox_t::vec min = _box.getMin();
	DiscreteGrid::bbox_t::vec diag = _box.getDiagonal();

	// Set uniforms :
	glUniformMatrix4fv(location_pMat, 1, GL_FALSE, pMat);
	glUniformMatrix4fv(location_vMat, 1, GL_FALSE, vMat);
	glUniform3fv(location_bbColor, 1, glm::value_ptr(color));
	glUniform3fv(location_bbSize, 1, glm::value_ptr(diag));
	glUniform3fv(location_bbPos, 1, glm::value_ptr(min));
	GetOpenGLError();

	if (this->showVAOstate) {
		std::cerr << "[LOG] Uniform locations : \n";
		std::cerr << "[LOG]\tpMat : " << +location_pMat << '\n';
		std::cerr << "[LOG]\tvMat : " << +location_vMat << '\n';
		std::cerr << "[LOG]\tbbColor : " << +location_bbColor << '\n';
		std::cerr << "[LOG]\tbbSize : " << +location_bbSize << '\n';
		std::cerr << "[LOG]\tbbPos : " << +location_bbPos << '\n';
		DiscreteGrid::bbox_t::vec max = _box.getMax();
		std::cerr << "[LOG] BB dimensions and position :\n";
		std::cerr << "[LOG]\tMin : {" << min.x << ", " << min.y << ", " << min.z << "}\n";
		std::cerr << "[LOG]\tMax : {" << max.x << ", " << max.y << ", " << max.z << "}\n";
		std::cerr << "[LOG]\tDiag : {" << diag.x << ", " << diag.y << ", " << diag.z << "}\n";
	}

	glBindVertexArray(this->vaoHandle_boundingBox);
	GetOpenGLError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_boundingBoxIndices);
	GetOpenGLError();
	this->setupVAOBoundingBox();

	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void*)0);
	GetOpenGLError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
	GetOpenGLError();
}

void Scene::slotToggleShowTextureCube(bool show) { this->inputGridVisible = show; }

void Scene::slotSetPlaneDisplacementX(float scalar) { this->planeDisplacement.x = scalar; }
void Scene::slotSetPlaneDisplacementY(float scalar) { this->planeDisplacement.y = scalar; }
void Scene::slotSetPlaneDisplacementZ(float scalar) { this->planeDisplacement.z = scalar; }

void Scene::slotTogglePlaneDirectionX() { this->planeDirection.x = - this->planeDirection.x; }
void Scene::slotTogglePlaneDirectionY() { this->planeDirection.y = - this->planeDirection.y; }
void Scene::slotTogglePlaneDirectionZ() { this->planeDirection.z = - this->planeDirection.z; }

void Scene::slotSetMinTexValue(uchar val) { this->minTexVal = val; this->updateVis(); }
void Scene::slotSetMaxTexValue(uchar val) { this->maxTexVal = val; this->updateVis(); }

void Scene::slotSetMinColorValue(uchar val) { this->minColorVal = val; }
void Scene::slotSetMaxColorValue(uchar val) { this->maxColorVal = val; }

void Scene::slotSetPlanePositionX(float coord) { this->planePosition.x = coord; }
void Scene::slotSetPlanePositionY(float coord) { this->planePosition.y = coord; }
void Scene::slotSetPlanePositionZ(float coord) { this->planePosition.z = coord; }

void Scene::setDrawMode(DrawMode _mode) { this->drawMode = _mode; }

void Scene::togglePlaneVisibility(planes _plane) {
	if (_plane == planes::x) { this->planeVisibility.x = not this->planeVisibility.x; }
	if (_plane == planes::y) { this->planeVisibility.y = not this->planeVisibility.y; }
	if (_plane == planes::z) { this->planeVisibility.z = not this->planeVisibility.z; }
	return;
}

void Scene::writeGridDIM(const std::string name) {
	IO::Writer::DIM* writer = new IO::Writer::DIM(name);
	writer->write(this->outputGrid);
	return;
}

void Scene::draft_writeRawGridPortion(DiscreteGrid::sizevec3 begin, DiscreteGrid::sizevec3 size, std::string name) {
	std::shared_ptr<OutputGrid> rawGrid = std::make_shared<OutputGrid>();
	//fetch data from input grid :
	std::vector<DiscreteGrid::DataType> data(size.x * size.y * size.z, uchar(0));
	#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
	const std::vector<DiscreteGrid::DataType>& src = this->inputGrid_Blue->getData();
	const DiscreteGrid::sizevec3 dims = this->inputGrid_Blue->getGridDimensions();
	#else
	const std::vector<DiscreteGrid::DataType>& src = this->inputGrid->getData();
	const DiscreteGrid::sizevec3 dims = this->inputGrid->getGridDimensions();
	#endif

	if ((begin.x + size.x) > dims.x || (begin.y + size.y) > dims.y || (begin.z + size.z) > dims.z) { return; }
	std::cerr << "Writing data ...";
	std::size_t x = 0, y = 0, z = 0;
	for (std::size_t i = begin.x; i < begin.x + size.x; ++i) {
		y = 0;
		for (std::size_t j = begin.y; j < begin.y + size.y; ++j) {
			z = 0;
			for (std::size_t k = begin.z; k < begin.z + size.z; ++k) {
				// this is beautiful, fucking hell
				data[x + y * size.x + z * size.x * size.y] = src[i + j *  dims.x + k * dims.x * dims.y];
				z++;
			}
			y++;
		}
		x++;
	}
	std::cerr << " done\n";
	std::cerr << "Writing with sizes : " << size.x << ',' << size.y << ',' << size.z << "\n";
	DiscreteGrid::bbox_t box(glm::vec3(.0, .0, .0), glm::convert_to<float>(size));
	DiscreteGrid::bbox_t::vec mi = box.getMin();
	DiscreteGrid::bbox_t::vec ma = box.getMax();
	std::cerr << "Writing with box : {" << mi.x << ',' << mi.y << ',' << mi.z << "}, {" << ma.x << ',' << ma.y << ',' << ma.z << "}\n";
	rawGrid->setBoundingBox(box);
	rawGrid->setResolution(size);
	rawGrid->setData(data);
	IO::Writer::DIM* writer = new IO::Writer::DIM(name);
	writer->write(rawGrid);
	return;
}

void Scene::tex3D_buildTexture() {
	/* By default, this step should have been done in the Scene::loadImage() function. */
}

void Scene::tex3D_buildMesh(GridGLView& grid, const std::string path) {
	/* Here, we'll build the tetrahedral mesh used to visualize the 3D structure of the acquisition in real-time. */
	/* Textures built : index of vertices, index of per-face normals for tetrahedra, index of per-face neighbors
	 * for tetrehedra */

	VolMeshData mesh{};

	std::string meshpath = path;
	if (path.empty() == true) {
		QString filename = QFileDialog::getOpenFileName(nullptr, "Open MESH file", "../../", "MESH Files (*.MESH, *.mesh)");
		meshpath = filename.toStdString();
	}
	this->tex3D_loadMESHFile(meshpath, grid, mesh);

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

	grid.volumetricMesh.tetrahedraCount = mesh.tetrahedra.size();

	GLfloat* rawVertices = new GLfloat[grid.volumetricMesh.tetrahedraCount*4*3*3];
	GLfloat* rawNormals = new GLfloat[grid.volumetricMesh.tetrahedraCount*4*4];
	GLfloat* tex = new GLfloat[grid.volumetricMesh.tetrahedraCount*4*3*3];
	GLfloat* rawNeighbors = new GLfloat[grid.volumetricMesh.tetrahedraCount*4*3];

	/*
	Warning : the loop that follows is horribly bad : it duplicates every texture coordinate by the number of times
	the vertex is used within the mesh. It MUST be improved once the method is working, to save up on video memory.
	It does the same for the vertex positions.
	*/
	std::size_t cnt = 0;
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
	grid.volumetricMesh.vertexPositions = this->uploadTexture2D(texParams);

	// Face normals :
	texParams.internalFormat = GL_RGBA32F;
	texParams.size.x = normWidth;
	texParams.size.y = normHeight;
	texParams.format = GL_RGBA;
	texParams.data = rawNormals;
	grid.volumetricMesh.faceNormals = this->uploadTexture2D(texParams);

	// Texture coordinates :
	texParams.internalFormat = GL_RGB32F;
	texParams.size.x = coorWidth;
	texParams.size.y = coorWidth;
	texParams.format = GL_RGB;
	texParams.data = tex;
	grid.volumetricMesh.textureCoordinates = this->uploadTexture2D(texParams);

	// Neighborhood information :
	texParams.size.x = neighbWidth;
	texParams.size.y = neighbHeight;
	texParams.data = rawNeighbors;
	grid.volumetricMesh.neighborhood = this->uploadTexture2D(texParams);

	delete[] tex;
	delete[] rawVertices;
	delete[] rawNormals;
	delete[] rawNeighbors;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Scene::tex3D_loadMESHFile(const std::string file, const GridGLView& grid, VolMeshData& mesh) {
	#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
	DiscreteGrid::sizevec3 dims = this->inputGrid_Blue->getGridDimensions();
	DiscreteGrid::bbox_t box = this->inputGrid_Blue->getBoundingBox();
	#else
	DiscreteGrid::sizevec3 dims = grid.grid->getGridDimensions();
	DiscreteGrid::bbox_t box = grid.grid->getBoundingBox();
	#endif
	glm::vec3 bmin = glm::convert_to<float>(box.getMin());
	glm::vec3 bmax = glm::convert_to<float>(box.getMax());
	glm::vec3 diag = glm::abs(bmax - bmin);

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
		#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
		position = this->inputGrid_Blue->getTransform_GridToWorld() * position;
		#else
		position = grid.grid->getTransform_GridToWorld() * position;
		#endif
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

/*
	*/
	// Current mesh size :
	glm::vec3 size{maxX, maxY, maxZ};
	// Grid size divided by mesh size gives a scalar to apply to the mesh to make it grid-sized :
	#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
	glm::vec3 grid = glm::convert_to<float>(this->inputGrid_Blue->getGridDimensions());
	#else
	glm::vec3 gridsize = glm::convert_to<float>(grid.grid->getGridDimensions());
	#endif
	glm::vec4 scale = glm::vec4(gridsize / size, 1.);

	std::cerr << "[LOG][" << __FILE__ << ':' << __LINE__ << "] Max dimensions of the mesh : [" << maxX << ',' << maxY << ',' << maxZ << "]\n";
	for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
		mesh.positions[i] *= scale;
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

	myfile.close ();

	/*
	std::cerr << "=============== loadMESHFile() ===============\n";
	std::cerr << "Bounding box is :\n";
	normalBox.printInfo("Bounding box info is :");
	deformedBox.printInfo("Deformed box is : ");
	normalBox.transformTo(this->inputGrid->getTransform_GridToWorld()).printInfo("Transformed box is :");
	std::cerr << "===============<loadMESHFile()>===============\n";
	*/
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
	GetOpenGLError();

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
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, 12*3*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertNorm);
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, 12*3*sizeof(GLfloat), normals, GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertTex);
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, 12*2*sizeof(GLfloat), textureCoords, GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboHandle_Texture3D_VertIdx);
	GetOpenGLError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*sizeof(GLushort), indices, GL_STATIC_DRAW);
	GetOpenGLError();

	glBindVertexArray(this->vaoHandle_VolumetricBuffers);
	GetOpenGLError();
	this->tex3D_bindVAO();
}

void Scene::tex3D_bindVAO() {
	glBindVertexArray(this->vaoHandle_VolumetricBuffers);
	GetOpenGLError();
	glEnableVertexAttribArray(0);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertPos);
	GetOpenGLError();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();

	glEnableVertexAttribArray(1);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertNorm);
	GetOpenGLError();
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();

	glEnableVertexAttribArray(2);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_Texture3D_VertTex);
	GetOpenGLError();
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();
}

