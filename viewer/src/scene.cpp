#include "../include/scene.hpp"
#include "../include/planar_viewer.hpp"
#include "../../qt/include/scene_control.hpp"
#include "../../image/include/reader.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QOpenGLContext>
#include <QFileDialog>
#include <QMessageBox>
#include <QSurface>

#include <fstream>
#include <iomanip>
#include <type_traits>

#warning Check the signification of the drawWithPlanes method, and other plane-drawing methods.
#warning One draws all planes but still takes a plane to draw, and other inconsistencies.

template<class T>
std::remove_reference_t<T> const& as_const(T&&t){return t;}

inline int __GetOpenGLError ( char* szFile, int iLine )
{
	int    retCode = 0;
	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		std::cerr << "GLError in file " << szFile << " @ line " << iLine << " : ";
		switch (glErr) {
			case GL_INVALID_ENUM:
				std::cerr << "was an invalid enum";
			break;
			case GL_INVALID_VALUE:
				std::cerr << "was an invalid value";
			break;
			case GL_INVALID_OPERATION:
				std::cerr << "was an invalid operation";
			break;
			default:
				std::cerr << "was another error";
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

Scene::Scene(GridControl* const gc) {
	this->controlPanel = nullptr;
	this->texStorage = nullptr;
	this->mesh = nullptr;
	this->voxelGrid = nullptr;
	this->gridControl = gc;

	this->gridWidth = 0;
	this->gridHeight = 0;
	this->gridDepth = 0;

	this->inputGridVisible = false;
	this->colorOrTexture = true; // by default, the cube is shown !
	this->renderSize = 0;

	this->inputGridVisible = false;
	this->outputGridVisible = false;

	this->isInitialized = false;

	this->textureHandle = 0;
	this->colorScaleHandle = 0;

	this->planePosition = glm::vec3();
	this->planeDepths = glm::vec3();
	this->sceneBBDiag = glm::vec3();
	this->sceneBBPosition = glm::vec3();

	this->vboVertPosHandle = 0;
	this->vboElementHandle = 0;
	this->vaoHandle = 0;
	this->programHandle = 0;
	this->planeProgramHandle = 0;
	this->planeViewerProgramHandle = 0;
	this->showVAOstate = false;

	this->minTexVal = uchar(0);
	this->maxTexVal = uchar(255);
}

Scene::~Scene(void) {
	glDeleteTextures(1, &this->textureHandle);
	glDeleteTextures(1, &this->colorScaleHandle);
	glDeleteTextures(1, &this->voxelGridTexHandle);
}

void Scene::initGl(QOpenGLContext* _context) {
	if (this->isInitialized == true) { return; }
	this->isInitialized = true;
	std::cerr << "Initializing scene ..." << '\n';

	if (_context == 0) { throw std::runtime_error("Warning : this->context() returned 0 or nullptr !") ; }
	if (_context == nullptr) { std::cerr << "Warning : Initializing a scene without a valid OpenGL context !" << '\n' ; }

	this->initializeOpenGLFunctions();

	IO::GenericGridReader* reader = nullptr;
	IO::GenericGridReader::data_t threshold = IO::GenericGridReader::data_t(6);

#ifdef USER_DEFINED_IMAGE_LOADING
	QMessageBox* msgBox = new QMessageBox();
	msgBox->setText("Choose your input data type");
	QPushButton* dimButton = msgBox->addButton("DIM", QMessageBox::ActionRole);
	QPushButton* tiffButton = msgBox->addButton("TIFF", QMessageBox::ActionRole);

	msgBox->exec();

	if (msgBox->clickedButton() == dimButton) {
		reader = new IO::DIMReader(threshold);
		QString filename = QFileDialog::getOpenFileName(nullptr, "Open a DIM/IMA image", "../../", "BrainVISA DIM Files (*.dim)");
		std::vector<std::string> f;
		f.push_back(filename.toStdString());
		reader->setFilenames(f);
	} else if (msgBox->clickedButton() == tiffButton) {
		// do nothing :
		reader = new IO::Reader::TIFF(threshold);
		QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Open multiple TIFF images","../../", "TIFF Files (*.tiff, *.tif)");
		std::vector<std::string> f;
		for (const QString& fn : as_const(filenames)) {
			f.push_back(fn.toStdString());
		}
		reader->setFilenames(f);
	} else {
		std::cerr << "No button was pressed." << '\n';
		throw std::runtime_error("error : no button pressed");
	}
#else
	reader = new IO::Reader::TIFF(threshold);
	std::vector<std::string> filenames = {
		"/home/thibault/git/datasets/Blue/Blue_P5B-A2_2500.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2501.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2502.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2503.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2504.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2505.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2506.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2507.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2508.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2509.tif",
		"/home/thibault/git/datasets/Blue/Blue_P5B-A2_2510.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2511.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2512.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2513.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2514.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2515.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2516.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2517.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2518.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2519.tif",
		"/home/thibault/git/datasets/Blue/Blue_P5B-A2_2520.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2521.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2522.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2523.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2524.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2525.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2526.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2527.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2528.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2529.tif",
		"/home/thibault/git/datasets/Blue/Blue_P5B-A2_2530.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2531.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2532.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2533.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2534.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2535.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2536.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2537.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2538.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2539.tif",
		"/home/thibault/git/datasets/Blue/Blue_P5B-A2_2540.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2541.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2542.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2543.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2544.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2545.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2546.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2547.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2548.tif","/home/thibault/git/datasets/Blue/Blue_P5B-A2_2549.tif",
	};
	reader->setFilenames(filenames);
#endif
	// Set reader properties :
	reader->setDataThreshold(threshold);
	// Load the data :
	reader->loadImage();

	// Update data from the grid reader :
	this->texStorage = std::make_shared<InputGrid>();
	this->texStorage->fromGridReader(*reader);
	this->texStorage->setTransform_GridToWorld(this->computeTransformationMatrix());

	// free up the reader's resources :
	delete reader;

	this->voxelGrid	= std::make_shared<OutputGrid>();
	if (this->gridControl) {
		this->gridControl->setVoxelGrid(this->voxelGrid);
	}

	this->mesh = std::make_shared<TetMesh>();
	this->mesh->addInputGrid(this->texStorage).setOutputGrid(this->voxelGrid);

	///////////////////////////
	/// CREATE VAO :
	///////////////////////////
	glGenVertexArrays(1, &this->vaoHandle);
	GetOpenGLError();
	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	this->programHandle = this->compileShaders("./shaders/voxelgrid.vert", "./shaders/voxelgrid.geom", "./shaders/voxelgrid.frag");
	this->planeProgramHandle = this->compileShaders("./shaders/plane.vert", "", "./shaders/plane.frag");
	this->planeViewerProgramHandle = this->compileShaders("./shaders/texture_explorer.vert", "", "./shaders/texture_explorer.frag");
	if (this->programHandle == 0) { throw std::runtime_error("[ERROR] Program did not compile"); }
	if (this->planeProgramHandle == 0) { throw std::runtime_error("[ERROR] 'Planes' Program did not compile"); }
	if (this->planeViewerProgramHandle == 0) { throw std::runtime_error("[ERROR] 'PlaneViewer' Program did not compile"); }

	this->loadImage();

	this->generateGrid();

	std::vector<float> colorScale = this->generateColorScale(0, 255);
	this->uploadColorScale(colorScale);

	// Get the bounding box of the input grid :
	DiscreteGrid::bbox_t bb_ws = this->texStorage->getBoundingBoxWorldSpace();
	// Add the bounding box of the output grid to it :
	bb_ws.addPoints(this->voxelGrid->getBoundingBox().getAllCorners());
	this->sceneBBPosition = bb_ws.getMin();
	this->sceneBBDiag = bb_ws.getDiagonal();
	// Set the plane position to the min point of the BB :
	this->planePosition = glm::convert_to<glm::vec3::value_type>(bb_ws.getMin());

	this->minTexVal = 5;
	this->maxTexVal = 255;

	if (this->controlPanel) {
		this->controlPanel->activatePanels();
	}

	glUseProgram(this->programHandle);
	GetOpenGLError();
	std::cerr << "Finished initializing scene" << '\n';
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
#ifdef ENABLE_BASIC_BB
	const DiscreteGrid::bbox_t& bbGS = grid->getBoundingBox();
	const DiscreteGrid::bbox_t::vec& bbGSm = bbGS.getMin();
	const DiscreteGrid::bbox_t::vec& bbGSM = bbGS.getMax();
	std::cerr << "[INFO]\tBounding box GS : [" << bbGSm.x << ", " << bbGSm.y << ", " << bbGSm.z << "] to ["
		  << bbGSM.x << ", " << bbGSM.y << ", " << bbGSM.z << "]\n";
#ifdef ENABLE_BB_TRANSFORM
	const DiscreteGrid::bbox_t& bbWS = grid->getBoundingBoxWorldSpace();
	const DiscreteGrid::bbox_t::vec& bbWSm = bbWS.getMin();
	const DiscreteGrid::bbox_t::vec& bbWSM = bbWS.getMax();
	std::cerr << "[INFO]\tBounding box WS : [" << bbWSm.x << ", " << bbWSm.y << ", " << bbWSm.z << "] to ["
		  << bbWSM.x << ", " << bbWSM.y << ", " << bbWSM.z << "]\n";
#endif
#endif
}

void Scene::recompileShaders() {
	GLuint newProgram = this->compileShaders("./shaders/voxelgrid.vert", "./shaders/voxelgrid.geom", "./shaders/voxelgrid.frag", true);
	GLuint newPlaneProgram = this->compileShaders("./shaders/plane.vert", "", "./shaders/plane.frag", true);
	GLuint newPlaneViewerProgram = this->compileShaders("./shaders/texture_explorer.vert", "", "./shaders/texture_explorer.frag", true);

	if (newProgram) {
		glDeleteProgram(this->programHandle);
		this->programHandle = newProgram;
	}
	if (newPlaneProgram) {
		glDeleteProgram(this->planeProgramHandle);
		this->planeProgramHandle = newPlaneProgram;
	}
	if (newPlaneViewerProgram) {
		glDeleteProgram(this->planeViewerProgramHandle);
		this->planeViewerProgramHandle = newPlaneViewerProgram;
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

	if (verbose) { std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiling vertex shader \"" << _vPath << "\"...\n"; }
	GLuint _vSha = this->compileShader(_vPath, GL_VERTEX_SHADER, verbose);

	if (verbose) { std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiling geometry shader \"" << _gPath << "\"...\n"; }
	GLuint _gSha = this->compileShader(_gPath, GL_GEOMETRY_SHADER, verbose);

	if (verbose) { std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiling fragment shader \"" << _fPath << "\"...\n"; }
	GLuint _fSha = this->compileShader(_fPath, GL_FRAGMENT_SHADER, verbose);

	if (verbose) { std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiling program ...\n"; }
	GLuint _prog = this->compileProgram(_vSha, _gSha, _fSha, verbose);

	if (_prog != 0) {
		if (verbose) { std::cerr<<"[LOG]["<<__FILE__ << ":" << __LINE__<<"] Compiled program at index "<<_prog<<"\n"; }
	}

	return _prog;
}

GLuint Scene::compileShader(const std::string& path, const GLenum shaType, bool verbose) {
	if (path.empty()) { return -1; }
	// Check the given type is accepted :
	if (shaType != GL_VERTEX_SHADER && shaType != GL_GEOMETRY_SHADER && shaType != GL_FRAGMENT_SHADER) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] Error : unrecognized shader type (vertex, geometry and fragment shaders)\n";
		return -1;
	}

	// Create a shader object :
	GLuint _sha = glCreateShader(shaType);
	if (verbose) {std::cerr<<"[LOG]["<<__FILE__ << ":" << __LINE__<<"] Created shader at index" << _sha << ".\n";}
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
		std::cerr << __FILE__ << ":" << __LINE__ << " : Shader was a vertex shader ";
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
	if (verbose) { std::cerr << "[LOG][" << __FILE__ << ":" << __LINE__ << "] Compiled shader successfully.\n"; }

	return _sha;
}

GLuint Scene::compileProgram(const GLuint vSha, const GLuint gSha, const GLuint fSha, bool verbose) {
	// Check any shader was passed to the program :
	if (vSha == 0 && gSha == 0 && fSha == 0) {
		std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] : No shader IDs were passed to the function.\n" <<
			"\tArguments : vSha(" << vSha << "), gSha(" << gSha << "), fSha(" << fSha << ")";
	}

	GLuint _prog = glCreateProgram();
	if (verbose) {std::cerr<<"[LOG]["<<__FILE__ << ":" << __LINE__<<"] Created new program at index"<<_prog<<"\n";}
	GetOpenGLError();
	if (vSha != 0) { glAttachShader(_prog, vSha); GetOpenGLError(); }
	if (gSha != 0) { glAttachShader(_prog, gSha); GetOpenGLError(); }
	if (fSha != 0) { glAttachShader(_prog, fSha); GetOpenGLError(); }
	GetOpenGLError();
	if (verbose) {std::cerr<<"[LOG]["<<__FILE__ << ":" << __LINE__<<"] Attached shaders.\n";}

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
	}
	if (verbose) {std::cerr<<"[LOG]["<<__FILE__ << ":" << __LINE__<<"] Program linked.\n";}

	glDetachShader(_prog, vSha);
	glDetachShader(_prog, gSha);
	glDetachShader(_prog, fSha);
	glDeleteShader(vSha);
	glDeleteShader(gSha);
	glDeleteShader(fSha);

	return _prog;
}

void Scene::loadImage() {
	DiscreteGrid::sizevec3 d = this->texStorage->getGridDimensions();
	this->gridWidth = d.x;
	this->gridHeight= d.y;
	this->gridDepth = d.z;

	glEnable(GL_TEXTURE_3D);
	GetOpenGLError();
	glDeleteTextures(1, &this->textureHandle); // just in case one was allocated before
	GetOpenGLError();

	if (this->textureHandle == 0) {
		glGenTextures(1, &this->textureHandle);
		GetOpenGLError();
	}
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
	GetOpenGLError();

	// Set nearest neighbor :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Set the texture upload to not generate mimaps :
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, static_cast<GLfloat>(-1000.f));
	// Stop once UV > 1 or < 0
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Swizzle G/B to R value, to save data upload
	GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
	glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	glTexImage3D(
		GL_TEXTURE_3D,		// GLenum : Target
		static_cast<GLint>(0),	// GLint  : Level of detail of the current texture (0 = original)
		GL_R8UI,		// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		static_cast<GLsizei>(d.x), // GLsizei: Image width
		static_cast<GLsizei>(d.y), // GLsizei: Image height
		static_cast<GLsizei>(d.z), // GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
		GL_RED_INTEGER,		// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,	// GLenum : Type (the data type as in uchar, uint, float ...)
		this->texStorage->getData().data() // void*  : Data to load into the buffer
	);
	GetOpenGLError();

	if (this->gridControl) {
		this->gridControl->updateGridDimensions();
	}
}

void Scene::loadVoxelGrid() {
	if (this->voxelGrid == nullptr) {
		// Sanity check. This function can *now* be called from the voxel grid only,
		// but just in case we call it from elsewhere, all bases are covered.
		std::cerr << "Voxel grid in scene was not created ! (voxelGrid == nullptr) => true" << '\n';
		return;
	}

	DiscreteGrid::sizevec3 size = this->voxelGrid->getGridDimensions();

	glEnable(GL_TEXTURE_3D);

	if (this->voxelGridTexHandle != 0) {
		// Texture has already been created, destroy it:
		glDeleteTextures(1, &this->voxelGridTexHandle);
		this->voxelGridTexHandle = 0;
	}
	if (this->voxelGridTexHandle == 0) {
		// Create texture handle
		glGenTextures(1, &this->voxelGridTexHandle);
		GetOpenGLError();
	}
	std::cerr << "Voxel grid texture was generated at index " << this->voxelGridTexHandle << "\n";
	glBindTexture(GL_TEXTURE_3D, this->voxelGridTexHandle);
	GetOpenGLError();

	// Set nearest neighbor :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Set the texture upload to not generate mimaps :
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, static_cast<GLfloat>(-1000.f));
	// Stop once UV > 1 or < 0
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// Swizzle G/B to R value, to save data upload
	GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
	glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	// Fix for a stupid fucking bug : OpenGL expects textures with dimensions which are a multiple of 4. However,
	// this will only be the case 25% of the time, so we need to manually pad the texture before sending it to
	// OpenGL for displaying. Might shift the data on very small datasets (resolution < 50).
	const auto& data = this->voxelGrid->getData();
	DiscreteGrid::sizevec3 sane = size;
	if (sane.x%4u != 0) { sane.x += (4-size.x%4u); }
	if (sane.y%4u != 0) { sane.y += (4-size.y%4u); }
	//if (sane.z%4u != 0) { sane.z += (4-size.z%4u); }
	std::vector<DiscreteGrid::DataType> texData;
	texData.resize(sane.x*sane.y*sane.z);
	DiscreteGrid::DataType d = 0;
	for (std::size_t k = 0; k < sane.z; ++k) {
		for (std::size_t j = 0; j < sane.y; ++j) {
			for (std::size_t i = 0; i < sane.x; ++i) {
				if (i >= size.x || j >= size.y || k >= size.z) {
					d = 0;
				} else {
					d = data[i+j*size.x+k*size.x*size.y];
				}
				texData[i+j*sane.x+k*sane.x*sane.y] = d;
			}
		}
	}

	glTexImage3D(
		GL_TEXTURE_3D,			// GLenum : Target
		static_cast<GLint>(0),		// GLint  : Level of detail of the current texture (0 = original)
		GL_R8UI,			// GLint  : Number of color components in the picture. Here grayscale in uchar so GL_R8UI
		static_cast<GLsizei>(sane.x),	// GLsizei: Image width
		static_cast<GLsizei>(sane.y),	// GLsizei: Image height
		static_cast<GLsizei>(sane.z),	// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),		// GLint  : Border. This value MUST be 0.
		GL_RED_INTEGER,			// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,		// GLenum : Type (the data type as in uchar, uint, float ...)
		texData.data() // void*  : Data to load into the buffer
	);
	GetOpenGLError();
}

void Scene::fillNearestNeighbor() {
	InterpolationMethods method = InterpolationMethods::NearestNeighbor;
	using timepoint = std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<double, std::ratio<1,1>>>;
	if (this->mesh != nullptr && this->voxelGrid != nullptr) {
		timepoint start_point = std::chrono::high_resolution_clock::now();
		this->mesh->populateOutputGrid(method);
		timepoint end_point = std::chrono::high_resolution_clock::now();
		std::cerr << "To fill the grid, it took " << (end_point - start_point).count() << " seconds" << '\n';
	}
	this->loadVoxelGrid();
}

void Scene::fillTrilinear() {
	InterpolationMethods method = InterpolationMethods::TriLinear;
	using timepoint = std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<double, std::ratio<1,1>>>;
	if (this->mesh != nullptr && this->voxelGrid != nullptr) {
		timepoint start_point = std::chrono::high_resolution_clock::now();
		this->mesh->populateOutputGrid(method);
		timepoint end_point = std::chrono::high_resolution_clock::now();
		std::cerr << "To fill the grid, it took " << (end_point - start_point).count() << " seconds" << '\n';
	}
	this->loadVoxelGrid();
}

void Scene::drawPlaneView(glm::vec2 fbDims, planes _plane) {
	if (this->texStorage == nullptr) { return; }
	glEnable(GL_DEPTH_TEST);

	uint min = 0; // min index for drawing commands
	if (_plane == planes::x) { min =  0; }
	if (_plane == planes::y) { min =  6; }
	if (_plane == planes::z) { min = 12; }

	glUseProgram(this->planeViewerProgramHandle);
	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboPlaneElementHandle);
	this->prepPlane_SingleUniforms(_plane, fbDims, this->texStorage);
	this->setupVAOPointers();

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*)(min*sizeof(GLuint)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	this->showVAOstate = false;
}

void Scene::drawPlanes(GLfloat mvMat[], GLfloat pMat[]) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	// Plane X :
	glUseProgram(this->planeProgramHandle);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboPlaneElementHandle);
	this->prepPlaneUniforms(mvMat, pMat, planes::x);
	this->setupVAOPointers();

	// glDrawRangeElements(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(6), GL_UNSIGNED_INT, static_cast<GLvoid*>(0));
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, static_cast<GLvoid*>(0));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	// Plane Y :
	glUseProgram(this->planeProgramHandle);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboPlaneElementHandle);
	this->prepPlaneUniforms(mvMat, pMat, planes::y);
	this->setupVAOPointers();

	// glDrawRangeElements(GL_TRIANGLES, 6, 12, static_cast<GLsizei>(6), GL_UNSIGNED_INT, static_cast<GLvoid*>(0));
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*)(6*sizeof(GLuint)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	// Plane Z :
	glUseProgram(this->planeProgramHandle);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboPlaneElementHandle);
	this->prepPlaneUniforms(mvMat, pMat, planes::z);
	this->setupVAOPointers();

	// glDrawRangeElements(GL_TRIANGLES, 12, 18, static_cast<GLsizei>(6), GL_UNSIGNED_INT, static_cast<GLvoid*>(0));
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, (GLvoid*)(12*sizeof(GLuint)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void Scene::drawGridOnly(GLfloat mvMat[], GLfloat pMat[]) {
	glEnable(GL_DEPTH_TEST);
	GetOpenGLError();

	glm::mat4 transfoMat = glm::mat4(1.f);

	if (this->inputGridVisible) {
		this->drawGrid_Generic(mvMat, pMat, transfoMat, this->textureHandle, this->texStorage);
	}
	if (this->outputGridVisible) {
		this->drawGrid_Generic(mvMat, pMat, transfoMat, this->voxelGridTexHandle, this->voxelGrid);
	}
}

void Scene::drawWithPlanes(GLfloat mvMat[], GLfloat pMat[]) {
	glEnable(GL_DEPTH_TEST);
	GetOpenGLError();

	glm::mat4 transfoMat = glm::mat4(1.f);

	if (this->inputGridVisible) {
		this->drawGrid_Generic(mvMat, pMat, transfoMat, this->textureHandle, this->texStorage);
	}
	if (this->outputGridVisible) {
		this->drawGrid_Generic(mvMat, pMat, transfoMat, this->voxelGridTexHandle, this->voxelGrid);
	}

	this->drawPlanes(mvMat, pMat);

	this->showVAOstate = false;
}

void Scene::prepGridUniforms(GLfloat *mvMat, GLfloat *pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid>& grid) {
	// Get the world to grid transform :
	glm::mat4 transfoMat = baseMatrix * grid->getTransform_GridToWorld();

	// Get the uniform locations :
	GLint mMatrix_Loc = glGetUniformLocation(this->programHandle, "mMatrix");
	GLint vMatrix_Loc = glGetUniformLocation(this->programHandle, "vMatrix");
	GLint pMatrix_Loc = glGetUniformLocation(this->programHandle, "pMatrix");
	GLint lightPos_Loc = glGetUniformLocation(this->programHandle, "lightPos");
	GLint voxelGridOrigin_Loc = glGetUniformLocation(this->programHandle, "voxelGridOrigin");
	GLint voxelGridSize_Loc = glGetUniformLocation(this->programHandle, "voxelGridSize");
	GLint voxelSize_Loc = glGetUniformLocation(this->programHandle, "voxelSize");
	GLint minTexVal_Loc = glGetUniformLocation(this->programHandle, "minTexVal");
	GLint maxTexVal_Loc = glGetUniformLocation(this->programHandle, "maxTexVal");
	GLint drawMode_Loc = glGetUniformLocation(this->programHandle, "drawMode");
	GLint colorOrTexture_Loc = glGetUniformLocation(this->programHandle, "colorOrTexture");
	GLint texDataLoc = glGetUniformLocation(this->programHandle, "texData");
	GLint colorScaleLoc = glGetUniformLocation(this->programHandle, "colorScale");
	GLint planePositionsLoc = glGetUniformLocation(this->programHandle, "planePositions");
	GLint gridPositionLoc = glGetUniformLocation(this->programHandle, "gridPosition");
	GetOpenGLError();

	DiscreteGrid::bbox_t::vec origin = grid->getBoundingBox().getMin();
	DiscreteGrid::bbox_t::vec originWS = grid->getBoundingBoxWorldSpace().getMin();
	DiscreteGrid::sizevec3 gridDims = grid->getGridDimensions();
	glm::vec3 dims = glm::vec3(static_cast<float>(gridDims.x), static_cast<float>(gridDims.y), static_cast<float>(gridDims.z));

	glUniform3fv(voxelGridOrigin_Loc, 1, glm::value_ptr(origin));
	glUniform3fv(voxelGridSize_Loc, 1, glm::value_ptr(dims));
	glUniform3fv(voxelSize_Loc, 1, glm::value_ptr(grid->getVoxelDimensions()));
	glUniform1ui(minTexVal_Loc, this->minTexVal);
	glUniform1ui(maxTexVal_Loc, this->maxTexVal);
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
	glBindTexture(GL_TEXTURE_1D, this->colorScaleHandle);
	GetOpenGLError();
	glUniform1i(colorScaleLoc, 1);
	GetOpenGLError();

	glUniform3fv(planePositionsLoc, 1, glm::value_ptr(this->planePosition));
	glUniform3fv(gridPositionLoc, 1, glm::value_ptr(originWS));

	// Apply the uniforms :
	glUniformMatrix4fv(mMatrix_Loc, 1, GL_FALSE, glm::value_ptr(transfoMat));
	glUniformMatrix4fv(vMatrix_Loc, 1, GL_FALSE, &mvMat[0]);
	glUniformMatrix4fv(pMatrix_Loc, 1, GL_FALSE, &pMat[0]);
	glUniform4fv(lightPos_Loc, 1, glm::value_ptr(lightPos));
	GetOpenGLError();
}

void Scene::prepPlaneUniforms(GLfloat *mvMat, GLfloat *pMat, planes _plane) {
	// lambda function to check uniform location :
	auto checkUniformLocation = [](const GLint id, const char* name) -> bool {
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
	GLint location_mMatrix = glGetUniformLocation(this->planeProgramHandle, "model_Mat");
	GLint location_vMatrix = glGetUniformLocation(this->planeProgramHandle, "view_Mat");
	GLint location_pMatrix = glGetUniformLocation(this->planeProgramHandle, "projection_Mat");
	GLint location_gridTransform = glGetUniformLocation(this->planeProgramHandle, "gridTransform");
	GLint location_sceneBBPosition = glGetUniformLocation(this->planeProgramHandle, "sceneBBPosition");
	GLint location_sceneBBDiagonal = glGetUniformLocation(this->planeProgramHandle, "sceneBBDiagonal");
	GLint location_gridSize = glGetUniformLocation(this->planeProgramHandle, "gridSize");
	GLint location_gridDimensions = glGetUniformLocation(this->planeProgramHandle, "gridDimensions");
	GLint location_currentPlane = glGetUniformLocation(this->planeProgramHandle, "currentPlane");
	GLint location_planePosition = glGetUniformLocation(this->planeProgramHandle, "planePosition");
	GLint location_texData = glGetUniformLocation(this->planeProgramHandle, "texData");
	GLint location_colorScale = glGetUniformLocation(this->planeProgramHandle, "colorScale");
	GLint location_minTexVal = glGetUniformLocation(this->planeProgramHandle, "minTexVal");
	GLint location_maxTexVal = glGetUniformLocation(this->planeProgramHandle, "maxTexVal");

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
	checkUniformLocation(location_planePosition, "planePosition");
	checkUniformLocation(location_texData, "texData");
	checkUniformLocation(location_colorScale, "colorScale");

	// Generate the data we need :
	glm::mat4 transform = glm::mat4(1.f);
	glm::mat4 gridTransfo = this->texStorage->getTransform_GridToWorld();
	DiscreteGrid::bbox_t bbws = this->texStorage->getBoundingBoxWorldSpace();
	boxvec_t min = bbws.getMin();
	glm::vec3 size = bbws.getDiagonal();
	glm::vec3 dims = glm::convert_to<glm::vec3::value_type>(this->texStorage->getGridDimensions());
	GLint plIdx = (_plane == planes::x) ? 1 : (_plane == planes::y) ? 2 : 3;

	glUniformMatrix4fv(location_mMatrix, 1, GL_FALSE, glm::value_ptr(transform));
	glUniformMatrix4fv(location_vMatrix, 1, GL_FALSE, mvMat);
	glUniformMatrix4fv(location_pMatrix, 1, GL_FALSE, pMat);
	glUniformMatrix4fv(location_gridTransform, 1, GL_FALSE, glm::value_ptr(gridTransfo));
	glUniform3fv(location_sceneBBPosition, 1, glm::value_ptr(this->sceneBBPosition));
	glUniform3fv(location_sceneBBDiagonal, 1, glm::value_ptr(this->sceneBBDiag));
	glUniform3fv(location_gridSize, 1, glm::value_ptr(size));
	glUniform3fv(location_gridDimensions, 1, glm::value_ptr(dims));
	glUniform1i(location_currentPlane, plIdx);
	glUniform1ui(location_minTexVal, this->minTexVal);
	glUniform1ui(location_maxTexVal, this->maxTexVal);
	glUniform3fv(location_planePosition, 1, glm::value_ptr(this->planePosition));
	glActiveTexture(GL_TEXTURE0 + 0);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
	glUniform1i(location_texData, 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	glEnable(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, this->colorScaleHandle);
	glUniform1i(location_colorScale, 1);
	GetOpenGLError();
}

void Scene::prepPlane_SingleUniforms(planes _plane, glm::vec2 fbDims, const std::shared_ptr<DiscreteGrid> _grid) {
	glUseProgram(this->planeViewerProgramHandle);
	// Info about the grid's BB :
	DiscreteGrid::bbox_t ws = _grid->getBoundingBoxWorldSpace();
	DiscreteGrid::bbox_t::vec bbox = ws.getDiagonal();
	DiscreteGrid::bbox_t::vec posBox = ws.getMin();
	if (this->showVAOstate) {
		std::cerr << "[TRACE] BBox diagonal : {" << bbox.x << ", " << bbox.y << ", " << bbox.z << "}\n";
		std::cerr << "[TRACE] BBox position : {" << posBox.x << ", " << posBox.y << ", " << posBox.z << "}\n";
		std::cerr << "[TRACE] Program uniforms :" << '\n';
		this->printProgramUniforms(this->planeViewerProgramHandle);
	}
	glm::vec2 gridBBDims;
	if (_plane == planes::x) { gridBBDims.x = bbox.y; gridBBDims.y = bbox.z; }
	if (_plane == planes::y) { gridBBDims.x = bbox.x; gridBBDims.y = bbox.z; }
	if (_plane == planes::z) { gridBBDims.x = bbox.x; gridBBDims.y = bbox.y; }
	// Grid transform :
	glm::mat4 gridTransform = _grid->getTransform_WorldToGrid();
	// Grid dimensions :
	glm::vec3 gridDimensions = glm::convert_to<float>(_grid->getGridDimensions());
	// Depth of the plane :
	float planeDepth = (_plane == planes::x) ? this->planeDepths.x :
			   (_plane == planes::y) ? this->planeDepths.y : this->planeDepths.z;

	// Uniform locations :
	// VShader :
	GLint location_fbDims = glGetUniformLocation(this->planeViewerProgramHandle, "fbDims");
	GLint location_bbDims = glGetUniformLocation(this->planeViewerProgramHandle, "bbDims");
	GLint location_planeIndex = glGetUniformLocation(this->planeViewerProgramHandle, "planeIndex");
	GLint location_gridTransform = glGetUniformLocation(this->planeViewerProgramHandle, "gridTransform");
	GLint location_gridDimensions = glGetUniformLocation(this->planeViewerProgramHandle, "gridDimensions");
	GLint location_gridBBDiagonal = glGetUniformLocation(this->planeViewerProgramHandle, "gridBBDiagonal");
	GLint location_gridBBPosition = glGetUniformLocation(this->planeViewerProgramHandle, "gridBBPosition");
	GLint location_depth = glGetUniformLocation(this->planeViewerProgramHandle, "depth");
	// FShader :
	GLint location_texData = glGetUniformLocation(this->planeViewerProgramHandle, "texData");
	GLint location_colorScale = glGetUniformLocation(this->planeViewerProgramHandle, "colorScale");
	GLint location_colorBounds = glGetUniformLocation(this->planeViewerProgramHandle, "colorBounds");
	GetOpenGLError();

	if (this->showVAOstate) {
		std::cerr << "[TRACE][Shader variables] " << "fbDims : " << +location_fbDims << '\n';
		std::cerr << "[TRACE][Shader variables] " << "bbDims : " << +location_bbDims << '\n';
		std::cerr << "[TRACE][Shader variables] " << "planeIndex : " << +location_planeIndex << '\n';
		std::cerr << "[TRACE][Shader variables] " << "gridTransform : " << +location_gridTransform << '\n';
		std::cerr << "[TRACE][Shader variables] " << "gridDimensions : " << +location_gridDimensions << '\n';
		std::cerr << "[TRACE][Shader variables] " << "gridBBDiagonal : " << +location_gridBBDiagonal << '\n';
		std::cerr << "[TRACE][Shader variables] " << "gridBBPosition : " << +location_gridBBPosition << '\n';
		std::cerr << "[TRACE][Shader variables] " << "depth : " << +location_depth << '\n';
		std::cerr << "[TRACE][Shader variables] " << "texData : " << +location_texData << '\n';
		std::cerr << "[TRACE][Shader variables] " << "colorScale : " << +location_colorScale << '\n';
	}

	// Uniform settings :
	glUniform2fv(location_fbDims, 1, glm::value_ptr(fbDims));
	glUniform2fv(location_bbDims, 1, glm::value_ptr(gridBBDims));
	glUniform1ui(location_planeIndex, (_plane == planes::x) ? 1 : (_plane == planes::y) ? 2 : 3);
	glUniformMatrix4fv(location_gridTransform, 1, GL_FALSE, glm::value_ptr(gridTransform));
	glUniform4f(location_gridDimensions, gridDimensions.x, gridDimensions.y, gridDimensions.z, 1.f);
	glUniform4f(location_gridBBDiagonal, bbox.x, bbox.y, bbox.z, 1.f);
	glUniform4f(location_gridBBPosition, posBox.x, posBox.y, posBox.z, .0f);
	glUniform1f(location_depth, planeDepth);
	glUniform2ui(location_colorBounds, this->minTexVal, this->maxTexVal);
	GetOpenGLError();

	// Uniform samplers :
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
	glUniform1i(location_texData, 0);
	GetOpenGLError();

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_1D, this->colorScaleHandle);
	glUniform1i(location_colorScale, 1);
	GetOpenGLError();
}

void Scene::drawGrid_Generic(GLfloat *mvMat, GLfloat *pMat, glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid> &grid) {
	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);
	glEnable(GL_TEXTURE_3D);
	GetOpenGLError();

	glUseProgram(this->programHandle);
	glBindVertexArray(this->vaoHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);

	this->prepGridUniforms(mvMat, pMat, lightPos, baseMatrix, texHandle, grid);
	this->setupVAOPointers();

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	GetOpenGLError();
}

void Scene::generateGrid() {
	std::vector<glm::vec4> vertPos;
	std::vector<glm::vec4> vertNorm;
	std::vector<glm::vec3> vertTex;
	std::vector<unsigned int> vertIdx;
	std::vector<unsigned int> vertIdx_plane;
	this->generateTexCube(vertPos, vertNorm, vertTex, vertIdx);
	this->generatePlanesArray(vertIdx_plane);

	this->renderSize = vertIdx.size();

	this->setupVBOData(vertPos, vertNorm, vertTex, vertIdx, vertIdx_plane);
}

void Scene::generatePlanesArray(std::vector<unsigned int>& idx) {
	idx.clear();
	// Refer to the generateTexCube() function to see which
	// way the cube's vertices are inserted into the array.

	unsigned int a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6;
	// Create the element array for each plane :
	unsigned int planeZ[] = {a, b, c, c, b, d};
	unsigned int planeY[] = {a, e, b, b, e, f};
	unsigned int planeX[] = {a, g, e, g, a, c};
	idx.insert(idx.end(), planeX, planeX+6);
	idx.insert(idx.end(), planeY, planeY+6);
	idx.insert(idx.end(), planeZ, planeZ+6);
}

void Scene::generateTexCube(std::vector<glm::vec4>& vertPos, std::vector<glm::vec4>& vertNorm, std::vector<glm::vec3>& vertTex, std::vector<unsigned int>& vertIdx) {
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
	vertPos.push_back(apos); vertNorm.push_back(anorm); vertTex.push_back(atex);
	vertPos.push_back(bpos); vertNorm.push_back(bnorm); vertTex.push_back(btex);
	vertPos.push_back(cpos); vertNorm.push_back(cnorm); vertTex.push_back(ctex);
	vertPos.push_back(dpos); vertNorm.push_back(dnorm); vertTex.push_back(dtex);
	vertPos.push_back(epos); vertNorm.push_back(enorm); vertTex.push_back(etex);
	vertPos.push_back(fpos); vertNorm.push_back(fnorm); vertTex.push_back(ftex);
	vertPos.push_back(gpos); vertNorm.push_back(gnorm); vertTex.push_back(gtex);
	vertPos.push_back(hpos); vertNorm.push_back(hnorm); vertTex.push_back(htex);
	unsigned int a = 0; unsigned int b = 1; unsigned int c = 2; unsigned int d = 3;
	unsigned int e = 4; unsigned int f = 5; unsigned int g = 6; unsigned int h = 7;
	unsigned int faceIdx1[]	= {a, d, c, a, b, d, e, b, a, e, f, b, g, e, a, g, a, c};
	unsigned int faceIdx2[] = {h, e, g, h, f, e, h, g, c, h, c, d, h, d, b, h, f, b};
	vertIdx.insert(vertIdx.end(), faceIdx1, faceIdx1+18);
	vertIdx.insert(vertIdx.end(), faceIdx2, faceIdx2+18);
}

void Scene::setupVBOData(const std::vector<glm::vec4>& vertPos, const std::vector<glm::vec4>& vertNorm, const std::vector<glm::vec3>& vertTex,
			const std::vector<unsigned int>& vertIdx, const std::vector<unsigned int>& vertIdx_plane) {

	if (glIsVertexArray(this->vaoHandle) == GL_FALSE) { throw std::runtime_error("The vao handle generated by OpenGL has been invalidated !"); }
	///////////////////////////////////////////////
	/// CREATE VBO AND UPLOAD DATA
	///////////////////////////////////////////////
	glGenBuffers(1, &this->vboVertPosHandle);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertPosHandle);
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, vertPos.size()*sizeof(glm::vec4), vertPos.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboVertNormHandle);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertNormHandle);
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, vertNorm.size()*sizeof(glm::vec4), vertNorm.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboVertTexHandle);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertTexHandle);
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, vertTex.size()*sizeof(glm::vec3), vertTex.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboElementHandle);
	GetOpenGLError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	GetOpenGLError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertIdx.size()*sizeof(unsigned int), vertIdx.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboPlaneElementHandle);
	GetOpenGLError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboPlaneElementHandle);
	GetOpenGLError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertIdx_plane.size()*sizeof(unsigned int), vertIdx_plane.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	this->setupVAOPointers();
}

void Scene::setupVAOPointers() {
	glEnableVertexAttribArray(0);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertPosHandle);
	GetOpenGLError();
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();

	glEnableVertexAttribArray(1);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertNormHandle);
	GetOpenGLError();
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();

	glEnableVertexAttribArray(2);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertTexHandle);
	GetOpenGLError();
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();
}

glm::vec3 Scene::getSceneBoundaries(bool realSpace) const {
	glm::vec3 baseVtx = glm::vec3(static_cast<float>(this->gridWidth), static_cast<float>(this->gridHeight), static_cast<float>(this->gridDepth));
	if (realSpace) {
		glm::mat3 transfoMat = glm::mat3(this->computeTransformationMatrix());
		return transfoMat * baseVtx;
	} else {
		return baseVtx;
	}
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

	std::cerr << "[TRACE]\t\tPlane program active attributes :\n";
	glGetProgramiv(_pid, GL_ACTIVE_ATTRIBUTES, &attrCount);
	for (GLint i = 0; i < attrCount; ++i ) {
		//std::cerr << "[TRACE]\t" << ;
		glGetActiveAttrib(_pid, (GLuint)i, bufSize, &length, &size, &type, name);
		fprintf(stderr, "[TRACE]\t\t\tAttribute #%d Type: %u Name: %s\n", i, type, name);
	}

	std::cerr << "[TRACE]\t\tProgram active attributes :\n";
	glGetProgramiv(_pid, GL_ACTIVE_UNIFORMS, &attrCount);
	for (GLint i = 0; i < attrCount; ++i ) {
		glGetActiveUniform(_pid, (GLuint)i, bufSize, &length, &size, &type, name);
		fprintf(stderr, "[TRACE]\t\t\tUniform #%d Type: %u Name: %s\n", i, type, name);
	}
}

glm::mat4 Scene::computeTransformationMatrix() const {
	glm::mat4 transfoMat = glm::mat4(1.0);

	double angleDeg = -45.;
	double angleRad = (angleDeg * M_PI) / 180.;

	transfoMat[0][0] = 0.39 * std::cos(angleRad);
	transfoMat[0][2] = 0.39 * std::sin(angleRad);
	transfoMat[1][1] = 0.39;
	transfoMat[2][2] = 1.927 * std::cos(angleRad);

	if (angleDeg < 0.) {
		// compute translation along Z :
		float w = static_cast<float>(this->gridWidth) *	.39;
		transfoMat[3][2] = w * std::abs(std::sin(angleRad));
	}

	return transfoMat;
	//return glm::mat4(1.f);
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
	if (this->colorScaleHandle != 0) {
		glDeleteTextures(1, &this->colorScaleHandle);
		GetOpenGLError();
		this->colorScaleHandle = 0;
	}

	glGenTextures(1, &this->colorScaleHandle);
	GetOpenGLError();

	glBindTexture(GL_TEXTURE_1D, this->colorScaleHandle);
	GetOpenGLError();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GetOpenGLError();
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GetOpenGLError();

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_FLOAT, colorScale.data());
	GetOpenGLError();
}

void Scene::slotToggleShowTextureCube(bool show) { this->inputGridVisible = show; }

void Scene::slotSetPlaneDepthX(float newXCoord) { this->planeDepths.x = newXCoord; }

void Scene::slotSetPlaneDepthY(float newYCoord) { this->planeDepths.y = newYCoord; }

void Scene::slotSetPlaneDepthZ(float newZCoord) { this->planeDepths.z = newZCoord; }

void Scene::slotSetMinTexValue(uchar val) { this->minTexVal = val; }

void Scene::slotSetMaxTexValue(uchar val) { this->maxTexVal = val; }

void Scene::slotSetPlanePositionX(float coord) { this->planePosition.x = coord; }

void Scene::slotSetPlanePositionY(float coord) { this->planePosition.y = coord; }

void Scene::slotSetPlanePositionZ(float coord) { this->planePosition.z = coord; }
