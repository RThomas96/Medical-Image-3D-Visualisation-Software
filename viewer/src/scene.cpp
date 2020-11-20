#include "../include/scene.hpp"
#include "../../qt/include/scene_control.hpp"
#include "../../image/include/ima_loader.hpp"
#include "../../image/include/reader.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QOpenGLContext>
#include <QFileDialog>
#include <QMessageBox>
#include <QSurface>

#include <fstream>
#include <type_traits>

template<class T>
std::remove_reference_t<T> const& as_const(T&&t){return t;}

Scene::Scene(GridControl* const gc) {
	this->controlPanel = nullptr;
	this->texStorage = nullptr;
	this->mesh = nullptr;
	this->voxelGrid = nullptr;
	this->gridControl = gc;

	this->gridWidth = 0;
	this->gridHeight = 0;
	this->gridDepth = 0;
	this->neighborWidth = 0;
	this->neighborHeight = 0;
	this->neighborDepth = 0;

	this->showTextureCube = false;
	this->cubeShown = true; // by default, the cube is shown !
	this->renderSize = 0;

	this->drawCalls = 0;

	this->isInitialized = false;

	this->textureHandle = 0;

	this->vboVertPosHandle = 0;
	this->vboElementHandle = 0;
	this->vaoHandle = 0;
	this->programHandle = 0;
	this->programHandle_VG = 0;

	this->neighborOffset = glm::vec3(0, 0, 0);
	this->neighborPos = uvec3(0, 0, 0);

	// Uniform locations :
	this->mMatrixLocation = -1;
	this->vMatrixLocation = -1;
	this->pMatrixLocation = -1;
	this->lightPosLocation = -1;
	this->neighborOffsetLocation = -1;
	this->scaledCubes = 1;

	this->minTexVal = uchar(0);
	this->maxTexVal = uchar(255);

	this->cutPlaneMin = glm::vec3(.0f, .0f, .0f);
	this->cutPlaneMax = glm::vec3(1.f, 1.f, 1.f);
}

Scene::~Scene(void) {
	glDeleteTextures(1, &this->textureHandle);
	glDeleteTextures(1, &this->voxelGridTexHandle);
}

void Scene::initGl(QOpenGLContext* _context, std::size_t _x, std::size_t _y, std::size_t _z) {
	if (this->isInitialized == true) { return; }
	this->isInitialized = true;

	if (_context == 0) { throw std::runtime_error("Warning : this->context() returned 0 or nullptr !") ; }
	if (_context == nullptr) { std::cerr << "Warning : Initializing a scene without a valid OpenGL context !" << '\n' ; }

	this->initializeOpenGLFunctions();

	QMessageBox* msgBox = new QMessageBox();
	msgBox->setText("Choose your input data type");
	QPushButton* dimButton = msgBox->addButton("DIM", QMessageBox::ActionRole);
	QPushButton* tiffButton = msgBox->addButton("TIFF", QMessageBox::ActionRole);

	msgBox->exec();

	IO::GenericGridReader* reader = nullptr;
	IO::GenericGridReader::data_t threshold = IO::GenericGridReader::data_t(6);

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
	// Set reader properties :
	reader->setDataThreshold(threshold);
	// Load the data :
	reader->loadImage();

	// Update data from the grid reader :
	this->texStorage = std::make_shared<InputGrid>();
	this->texStorage->fromGridReader(*reader);
	this->texStorage->setTransform_GridToWorld(this->computeTransformationMatrix());
	this->texStorage->printInfo("Once the data is loaded :", "[DEBUG-I]");

	// free up the reader's resources :
	delete reader;

	this->voxelGrid	= std::make_shared<OutputGrid>();
	this->gridControl->setVoxelGrid(this->voxelGrid);

	this->mesh = std::make_shared<TetMesh>();
	this->mesh->addInputGrid(this->texStorage).setOutputGrid(this->voxelGrid);
	this->voxelGrid->printInfo("Output grid after being set : ", "[DEBUG-O]");

	///////////////////////////
	/// CREATE VAO :
	///////////////////////////
	glGenVertexArrays(1, &this->vaoHandle);
	GetOpenGLError();
	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	this->recompileShaders();

	this->loadImage();

	this->generateGrid(_x, _y, _z);

	int bounds[] = {
		0, static_cast<int>(this->gridWidth - this->neighborWidth),
		0, static_cast<int>(this->gridHeight - this->neighborHeight),
		0, static_cast<int>(this->gridDepth - this->neighborDepth)
	};
	if (this->gridWidth <= this->neighborWidth ||
	    this->gridHeight <= this->neighborHeight ||
	    this->gridDepth <= this->neighborDepth) {
		bounds[1] = 0;
		bounds[3] = 0;
		bounds[5] = 0;
	}

	if (this->controlPanel) {
		this->controlPanel->setImageBoundaries(bounds);
		this->controlPanel->activatePanels();
	}

	glUseProgram(this->programHandle);
	GetOpenGLError();

	this->mMatrixLocation = glGetUniformLocation(this->programHandle, "mMatrix");
	this->vMatrixLocation = glGetUniformLocation(this->programHandle, "vMatrix");
	this->pMatrixLocation = glGetUniformLocation(this->programHandle, "pMatrix");
	this->texDataLocation = glGetUniformLocation(this->programHandle, "texData");
	this->lightPosLocation = glGetUniformLocation(this->programHandle, "lightPos");
	this->neighborOffsetLocation = glGetUniformLocation(this->programHandle, "neighborOffset");
	if (this->neighborOffsetLocation < 0) {
		std::cerr << "Cannot find " << "neighborOffset" << " uniform" << '\n';
	}
	GetOpenGLError();
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
	this->programHandle = this->compileShaders("./shaders/base.vert", "./shaders/base.geom", "./shaders/base.frag");
	this->programHandle_VG = this->compileShaders("./shaders/voxelgrid.vert", "./shaders/voxelgrid.geom", "./shaders/voxelgrid.frag");
}

GLuint Scene::compileShaders(std::string _vPath, std::string _gPath, std::string _fPath) {
	glUseProgram(0);

	GLuint _vSha;
	GLuint _gSha;
	GLuint _fSha;
	GLuint _prog;

	///////////////////////////
	/// CREATE SHADERS AND COMPILE :
	///////////////////////////
	_vSha = glCreateShader(GL_VERTEX_SHADER);
	GetOpenGLError();
	_gSha= glCreateShader(GL_GEOMETRY_SHADER);
	GetOpenGLError();
	_fSha= glCreateShader(GL_FRAGMENT_SHADER);
	GetOpenGLError();

	// Open shader file for reading :
	std::ifstream vShaFile = std::ifstream(_vPath, std::ios_base::in);
	std::ifstream gShaFile = std::ifstream(_gPath, std::ios_base::in);
	std::ifstream fShaFile = std::ifstream(_fPath, std::ios_base::in);

	if (!vShaFile.is_open()) {
		std::cerr << "Error : could not get the contents of shader file " << _vPath << '\n';
		exit(EXIT_FAILURE);
	}
	if (!gShaFile.is_open()) {
		vShaFile.close();
		std::cerr << "Error : could not get the contents of shader file " << _gPath << '\n';
		exit(EXIT_FAILURE);
	}
	if (!fShaFile.is_open()) {
		vShaFile.close();
		gShaFile.close();
		std::cerr << "Error : could not get the contents of shader file " << _fPath << '\n';
		exit(EXIT_FAILURE);
	}

	// Get file size by seeking end and rewinding :
	vShaFile.seekg(0, vShaFile.end);
	std::size_t vShaFileSize = static_cast<std::size_t>(vShaFile.tellg());
	vShaFile.seekg(0, vShaFile.beg);
	gShaFile.seekg(0, gShaFile.end);
	std::size_t gShaFileSize = static_cast<std::size_t>(gShaFile.tellg());
	gShaFile.seekg(0, gShaFile.beg);
	fShaFile.seekg(0, fShaFile.end);
	std::size_t fShaFileSize = static_cast<std::size_t>(fShaFile.tellg());
	fShaFile.seekg(0, fShaFile.beg);

	// Get shader file contents :
	char* vShaSource = new char[vShaFileSize+1];
	vShaFile.read(vShaSource, vShaFileSize);
	vShaSource[vShaFileSize] = '\0';
	char* gShaSource = new char[gShaFileSize+1];
	gShaFile.read(gShaSource, gShaFileSize);
	gShaSource[gShaFileSize] = '\0';
	char* fShaSource = new char[fShaFileSize+1];
	fShaFile.read(fShaSource, fShaFileSize);
	fShaSource[fShaFileSize] = '\0';

	glShaderSource(_vSha, 1, const_cast<const char**>(&vShaSource), 0);
	glShaderSource(_gSha, 1, const_cast<const char**>(&gShaSource), 0);
	glShaderSource(_fSha, 1, const_cast<const char**>(&fShaSource), 0);

	delete[] vShaSource;
	delete[] gShaSource;
	delete[] fShaSource;

	vShaFile.close();
	gShaFile.close();
	fShaFile.close();

	glCompileShader(_vSha);
	GetOpenGLError();

	{
		GLint shaderInfoLength = 0;
		GLint charsWritten = 0;
		char* shaderInfoLog = nullptr;

		glGetShaderiv(_vSha, GL_INFO_LOG_LENGTH, &shaderInfoLength);
		if (shaderInfoLength > 1) {
			std::cerr << __PRETTY_FUNCTION__ << " : start Log ***********************************************" << '\n';

			std::cerr << __FUNCTION__ << " : Information about shader " << _vPath << " : " << '\n';
			std::cerr << __FUNCTION__ << " : Shader was a vertex shader ";
			shaderInfoLog = new char[shaderInfoLength];
			glGetShaderInfoLog(_vSha, shaderInfoLength, &charsWritten, shaderInfoLog);
			GetOpenGLError();
			std::cerr << shaderInfoLog << '\n';
			delete[] shaderInfoLog;

			std::cerr << __PRETTY_FUNCTION__ << " : end Log ***********************************************" << '\n';
		} else {
			//std::cerr << "No more info about shader " << _vPath << '\n';
		}
	}

	glCompileShader(_gSha);
	GetOpenGLError();

	{
		GLint shaderInfoLength = 0;
		GLint charsWritten = 0;
		char* shaderInfoLog = nullptr;

		glGetShaderiv(_gSha, GL_INFO_LOG_LENGTH, &shaderInfoLength);
		if (shaderInfoLength > 1) {
			std::cerr << __PRETTY_FUNCTION__ << " : start Log ***********************************************" << '\n';

			std::cerr << __FUNCTION__ << " : Information about shader " << _gPath << " : " << '\n';
			std::cerr << __FUNCTION__ << " : Shader was a geometry shader ";
			shaderInfoLog = new char[shaderInfoLength];
			glGetShaderInfoLog(_gSha, shaderInfoLength, &charsWritten, shaderInfoLog);
			GetOpenGLError();
			std::cerr << shaderInfoLog << '\n';
			delete[] shaderInfoLog;

			std::cerr << __PRETTY_FUNCTION__ << " : end Log ***********************************************" << '\n';
		} else {
			//std::cerr << "No more info about shader " << _gPath << '\n';
		}
	}

	glCompileShader(_fSha);
	GetOpenGLError();

	{
		GLint shaderInfoLength = 0;
		GLint charsWritten = 0;
		char* shaderInfoLog = nullptr;

		glGetShaderiv(_fSha, GL_INFO_LOG_LENGTH, &shaderInfoLength);
		if (shaderInfoLength > 1) {
			std::cerr << __PRETTY_FUNCTION__ << " : start Log ***********************************************" << '\n';

			std::cerr << __FUNCTION__ << " : Information about shader " << _fPath << " : " << '\n';
			std::cerr << __FUNCTION__ << " : Shader was a fragment shader ";
			shaderInfoLog = new char[shaderInfoLength];
			glGetShaderInfoLog(_fSha, shaderInfoLength, &charsWritten, shaderInfoLog);
			GetOpenGLError();
			std::cerr << shaderInfoLog << '\n';
			delete[] shaderInfoLog;

			std::cerr << __PRETTY_FUNCTION__ << " : end Log ***********************************************" << '\n';
		} else {
			//std::cerr << "No more info about shader " << _fPath << '\n';
		}
	}

	///////////////////////////
	/// CREATE PROGRAM AND LINK :
	///////////////////////////
	_prog = glCreateProgram();
	GetOpenGLError();
	glAttachShader(_prog, _vSha);
	glAttachShader(_prog, _gSha);
	glAttachShader(_prog, _fSha);

	glLinkProgram(_prog);
	GetOpenGLError();

	{
		GLint Result = 0;
		int InfoLogLength = 0;
		glGetProgramiv(_prog, GL_LINK_STATUS, &Result);
		glGetProgramiv(_prog, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if ( InfoLogLength > 0 ){
			std::vector<char> ProgramErrorMessage(InfoLogLength+1);
			glGetProgramInfoLog(_prog, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			GetOpenGLError();
			std::cerr << __FUNCTION__ << " : Warning : errors while linking program :" << '\n';
			std::cerr << "------------------------------------------------------------------" << '\n';
			std::cerr << "------------------------------------------------------------------" << '\n';
			std::cerr << ProgramErrorMessage.data() << '\n';
			std::cerr << "------------------------------------------------------------------" << '\n';
			std::cerr << "------------------------------------------------------------------" << '\n';
		} else {
			//std::cerr << "Linking of program happened just fine." << '\n';
		}

	}

	glDetachShader(_prog, _vSha);
	glDetachShader(_prog, _gSha);
	glDetachShader(_prog, _fSha);
	GetOpenGLError();

	glDeleteShader(_vSha);
	glDeleteShader(_gSha);
	glDeleteShader(_fSha);

	return _prog;
}

void Scene::loadImage() {
	DiscreteGrid::sizevec3 d = this->texStorage->getGridDimensions();
	this->gridWidth = d.x;
	this->gridHeight= d.y;
	this->gridDepth = d.z;

	glEnable(GL_TEXTURE_3D);
	glDeleteTextures(1, &this->textureHandle); // just in case one was allocated before

	if (this->textureHandle == 0) {
		glGenTextures(1, &this->textureHandle);
		GetOpenGLError();
	}
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
	GetOpenGLError();

	// Set nearest neighbor :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

	this->gridControl->updateGridDimensions();
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

	glTexImage3D(
		GL_TEXTURE_3D,			// GLenum : Target
		static_cast<GLint>(0),		// GLint  : Level of detail of the current texture (0 = original)
		GL_R8UI,			// GLint  : Number of color components in the picture. Here grayscale in uchar so GL_R8UI
		static_cast<GLsizei>(size.x),	// GLsizei: Image width
		static_cast<GLsizei>(size.y),	// GLsizei: Image height
		static_cast<GLsizei>(size.z),	// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),		// GLint  : Border. This value MUST be 0.
		GL_RED_INTEGER,			// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,		// GLenum : Type (the data type as in uchar, uint, float ...)
		this->voxelGrid->getData().data() // void*  : Data to load into the buffer
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

void Scene::drawRealSpace(GLfloat mvMat[], GLfloat pMat[]) {
	glEnable(GL_DEPTH_TEST);
	GetOpenGLError();

//	glm::vec4 n = glm::vec4(this->neighborOffset.x, this->neighborOffset.y, this->neighborOffset.z, 1.);
//	this->neighborPos = this->t->convertRealSpaceToVoxelIndex(n);

	glm::mat4 transfoMat = glm::mat4(1.f); //this->computeTransformationMatrix();

	//this->draw(mvMat, pMat, transfoMat, voxelMat);
	if (this->showTextureCube) {
		this->drawGrid_Generic(mvMat, pMat, transfoMat, this->textureHandle, this->texStorage);
	}
	this->drawGrid_Generic(mvMat, pMat, transfoMat, this->voxelGridTexHandle, this->voxelGrid);
}

void Scene::drawInitialSpace(GLfloat mvMat[], GLfloat pMat[]) {

	//this->neighborPos = this->t->convertRealSpaceToVoxelIndex(glm::vec4(this->neighborOffset.x, this->neighborOffset.y, this->neighborOffset.z, 1.));
	//this->neighborPos -= glm::uvec3(1, 1, 1);

	glm::mat4 transfoMat = this->texStorage->getTransform_WorldToGrid(); // glm::mat4(1.f);
	if (this->showTextureCube) {
		this->drawGrid_Generic(mvMat, pMat, transfoMat, this->textureHandle, this->texStorage);
	}
	this->drawGrid_Generic(mvMat, pMat, transfoMat, this->voxelGridTexHandle, this->voxelGrid);

/*
	glm::mat4 transfoMat = glm::mat4(1.f);
	glm::mat4 voxelMat = glm::inverse(this->computeTransformationMatrix());
	// In initial space, the voxel grid needs to be deformed inverse to the
	// transformation applied when saving them to disk. So, computeMatrix^-1
	this->draw(mvMat, pMat, transfoMat, voxelMat);
*/
}

void Scene::prepUniforms(glm::mat4 transfoMat, GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
	glUniform1i(this->texDataLocation, 0);
	GetOpenGLError();

	GLint imgLoc = glGetUniformLocation(this->programHandle, "imageSize");
	if (imgLoc < 0) {
		std::cerr << "cannot find imgsize" << '\n';
	}

	glUniform3ui(imgLoc, gridWidth, gridHeight, gridDepth);
	glUniform3ui(this->neighborOffsetLocation, this->neighborPos.x, this->neighborPos.y, this->neighborPos.z);

	GLint modeLoc = glGetUniformLocation(this->programHandle, "drawMode");
	if (modeLoc < 0) {
		std::cerr << "Could not find draw mode uniform !" << '\n';
	} else {
		if (this->drawMode == DrawMode::Solid) {
			glUniform1ui(modeLoc, 0);
		} else if (this->drawMode == DrawMode::SolidAndWireframe) {
			glUniform1ui(modeLoc, 1);
		} else {
			glUniform1ui(modeLoc, 2);
		}
	}

	GLint minTexVal_Loc = glGetUniformLocation(this->programHandle, "minTexVal");
	glUniform1ui(minTexVal_Loc, static_cast<GLuint>(this->minTexVal));
	GLint maxTexVal_Loc = glGetUniformLocation(this->programHandle, "maxTexVal");
	glUniform1ui(maxTexVal_Loc, static_cast<GLuint>(this->maxTexVal));

	GLint scaledLoc = glGetUniformLocation(this->programHandle, "scaledCubes");
	if (scaledLoc < 0) {
		std::cerr << "Cannot find scaled cubes !" << '\n';
	} else {
		glUniform1ui(scaledLoc, this->scaledCubes);
	}
	GetOpenGLError();
	glUniformMatrix4fv(this->mMatrixLocation, 1, GL_FALSE, glm::value_ptr(transfoMat));
	glUniformMatrix4fv(this->vMatrixLocation, 1, GL_FALSE, &mvMat[0]);
	glUniformMatrix4fv(this->pMatrixLocation, 1, GL_FALSE, &pMat[0]);
	glUniform4fv(this->lightPosLocation, 1, glm::value_ptr(lightPos));

	// Cutting planes :
	GLint cutPlaneMin_Loc = glGetUniformLocation(this->programHandle_VG, "cutPlaneMin");
	glUniform3fv(cutPlaneMin_Loc, 1, glm::value_ptr(this->cutPlaneMin));
	GetOpenGLError();
	GLint cutPlaneMax_Loc = glGetUniformLocation(this->programHandle_VG, "cutPlaneMax");
	glUniform3fv(cutPlaneMax_Loc, 1, glm::value_ptr(this->cutPlaneMax));
	GetOpenGLError();
}

void Scene::draw(GLfloat mvMat[], GLfloat pMat[], glm::mat4 transfoMat, glm::mat4 voxelGridMat) {
	glEnable(GL_DEPTH_TEST);
	GetOpenGLError();

	glUseProgram(this->programHandle);
	GetOpenGLError();

	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	this->prepUniforms(transfoMat, mvMat, pMat, lightPos);
	GetOpenGLError();

	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	this->setupVAOPointers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);

	if (this->showTextureCube) {
		if (this->cubeShown == false) { this->showTexCubeVBO(); }
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(this->drawCalls));
	} else {
		if (this->cubeShown) { this->hideTexCubeVBO(); }
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(this->drawCalls));
	}
	GetOpenGLError();

	this->drawVoxelGrid(mvMat, pMat, voxelGridMat);

	glUseProgram(0);
	glBindVertexArray(0);
}

void Scene::drawVoxelGrid(GLfloat mvMat[], GLfloat pMat[], glm::mat4 transfoMat) {
	// Each uniform location is separated in its own variable in order to see them in debug view. Should be optimized away in release mode.

	// The VAO should still be in use, so :
	glUseProgram(this->programHandle_VG); // use the voxel grid shader program

	// Light position world space :
	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	// Compute grid dimensions as float, not size_t for GLSL (to make sure no overflow errors can happen while uplodaing data) :
	DiscreteGrid::sizevec3 dims = this->voxelGrid->getGridDimensions();
	glm::vec3 d = glm::vec3(static_cast<float>(dims.x), static_cast<float>(dims.y), static_cast<float>(dims.z));

	if (this->voxelGrid->getData().size() != 0u) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, this->voxelGridTexHandle);
		GLint texData_Loc = glGetUniformLocation(this->programHandle_VG, "texData");
		glUniform1i(texData_Loc, 0);
		GetOpenGLError();
	}

	// Min/max values for the texture data :
	GLint minTexVal_Loc = glGetUniformLocation(this->programHandle_VG, "minTexVal");
	glUniform1ui(minTexVal_Loc, static_cast<GLuint>(this->minTexVal));
	GetOpenGLError();
	GLint maxTexVal_Loc = glGetUniformLocation(this->programHandle_VG, "maxTexVal");
	glUniform1ui(maxTexVal_Loc, static_cast<GLuint>(this->maxTexVal));
	GetOpenGLError();

	// Voxel grid data such as size, position, and voxel dimensions :
	GLint voxelSize_Loc = glGetUniformLocation(this->programHandle_VG, "voxelSize");
	glUniform3fv(voxelSize_Loc, 1, glm::value_ptr(this->voxelGrid->getVoxelDimensions()));
	GetOpenGLError();
	GLint voxelGridSize_Loc = glGetUniformLocation(this->programHandle_VG, "voxelGridSize");
	glUniform3fv(voxelGridSize_Loc, 1, glm::value_ptr(d));
	GetOpenGLError();
	GLint voxelGridOrigin_Loc = glGetUniformLocation(this->programHandle_VG, "voxelGridOrigin");
#ifdef ENABLE_BASIC_BB
	glUniform3fv(voxelGridOrigin_Loc, 1, glm::value_ptr(this->voxelGrid->getBoundingBox().getMin()));
#else
	glm::vec3 o = glm::vec3(.0f);
	glUniform3fv(voxelGridOrigin_Loc, 1, glm::value_ptr(o));
#endif
	GetOpenGLError();

	// Cutting planes :
	GLint cutPlaneMin_Loc = glGetUniformLocation(this->programHandle_VG, "cutPlaneMin");
	glUniform3fv(cutPlaneMin_Loc, 1, glm::value_ptr(this->cutPlaneMin));
	GetOpenGLError();
	GLint cutPlaneMax_Loc = glGetUniformLocation(this->programHandle_VG, "cutPlaneMax");
	glUniform3fv(cutPlaneMax_Loc, 1, glm::value_ptr(this->cutPlaneMax));
	GetOpenGLError();

	GLint modeLoc = glGetUniformLocation(this->programHandle_VG, "drawMode");
	if (this->voxelGrid->getData().size() == 0) {
		glUniform1ui(modeLoc, 2); // force wireframe only if grid not generated yet
	} else {
		if (this->drawMode == DrawMode::Solid) {
			glUniform1ui(modeLoc, 0);
		} else if (this->drawMode == DrawMode::SolidAndWireframe) {
			glUniform1ui(modeLoc, 1);
		} else {
			glUniform1ui(modeLoc, 2);
		}
	}
	GetOpenGLError();

	GLint mMatrix_Loc = glGetUniformLocation(this->programHandle_VG, "mMatrix");
	glUniformMatrix4fv(mMatrix_Loc, 1, GL_FALSE, glm::value_ptr(transfoMat));
	GetOpenGLError();

	GLint vMatrix_Loc = glGetUniformLocation(this->programHandle_VG, "vMatrix");
	glUniformMatrix4fv(vMatrix_Loc, 1, GL_FALSE, &mvMat[0]);
	GetOpenGLError();

	GLint pMatrix_Loc = glGetUniformLocation(this->programHandle_VG, "pMatrix");
	glUniformMatrix4fv(pMatrix_Loc, 1, GL_FALSE, &pMat[0]);
	GetOpenGLError();

	GLint lightPos_Loc = glGetUniformLocation(this->programHandle_VG, "lightPos");
	glUniform4fv(lightPos_Loc, 1, glm::value_ptr(lightPos));
	GetOpenGLError();

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0);
	GetOpenGLError();
}

void Scene::prepGridUniforms(GLfloat *mvMat, GLfloat *pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid>& grid) {
	// Get the world to grid transform :
	glm::mat4 transfoMat = baseMatrix * grid->getTransform_GridToWorld();

	// Get the uniform locations :
	GLint mMatrix_Loc = glGetUniformLocation(this->programHandle_VG, "mMatrix");
	GLint vMatrix_Loc = glGetUniformLocation(this->programHandle_VG, "vMatrix");
	GLint pMatrix_Loc = glGetUniformLocation(this->programHandle_VG, "pMatrix");
	GLint lightPos_Loc = glGetUniformLocation(this->programHandle_VG, "lightPos");
	GLint voxelGridOrigin_Loc = glGetUniformLocation(this->programHandle_VG, "voxelGridOrigin");
	GLint voxelGridSize_Loc = glGetUniformLocation(this->programHandle_VG, "voxelGridSize");
	GLint voxelSize_Loc = glGetUniformLocation(this->programHandle_VG, "voxelSize");
	GLint cutPlaneMin_Loc = glGetUniformLocation(this->programHandle_VG, "cutPlaneMin");
	GLint cutPlaneMax_Loc = glGetUniformLocation(this->programHandle_VG, "cutPlaneMax");
	GLint minTexVal_Loc = glGetUniformLocation(this->programHandle_VG, "minTexVal");
	GLint maxTexVal_Loc = glGetUniformLocation(this->programHandle_VG, "maxTexVal");
	GLint drawMode_Loc = glGetUniformLocation(this->programHandle_VG, "drawMode");
	GLint texData_Loc = glGetUniformLocation(this->programHandle_VG, "texData");

#ifdef ENABLE_BASIC_BB
	DiscreteGrid::bbox_t::vec origin = grid->getBoundingBox().getMin();
#else
	DiscreteGrid::bbox_t::vec origin = glm::vec3(.0f);
#endif
	glUniform3fv(voxelGridOrigin_Loc, 1, glm::value_ptr(origin));
	DiscreteGrid::sizevec3 gridDims = grid->getGridDimensions();
	glm::vec3 dims = glm::vec3(static_cast<float>(gridDims.x), static_cast<float>(gridDims.y), static_cast<float>(gridDims.z));
	glUniform3fv(voxelGridSize_Loc, 1, glm::value_ptr(dims));
	glUniform3fv(voxelSize_Loc, 1, glm::value_ptr(grid->getVoxelDimensions()));
	glUniform3fv(cutPlaneMin_Loc, 1, glm::value_ptr(this->cutPlaneMin));
	glUniform3fv(cutPlaneMax_Loc, 1, glm::value_ptr(this->cutPlaneMax));
	glUniform1ui(minTexVal_Loc, this->minTexVal);
	glUniform1ui(maxTexVal_Loc, this->maxTexVal);
	if (grid->getData().size() == 0) {
		glUniform1ui(drawMode_Loc, 2);
	} else {
		glUniform1ui(drawMode_Loc, this->drawMode);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texHandle);
	glUniform1ui(texData_Loc, 0);

	// Apply the uniforms :
	glUniformMatrix4fv(mMatrix_Loc, 1, GL_FALSE, glm::value_ptr(transfoMat));
	GetOpenGLError();
	glUniformMatrix4fv(vMatrix_Loc, 1, GL_FALSE, &mvMat[0]);
	GetOpenGLError();
	glUniformMatrix4fv(pMatrix_Loc, 1, GL_FALSE, &pMat[0]);
	GetOpenGLError();
	glUniform4fv(lightPos_Loc, 1, glm::value_ptr(lightPos));
	GetOpenGLError();
}

void Scene::drawGrid_Generic(GLfloat *mvMat, GLfloat *pMat, glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid> &grid) {
	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);
	glUseProgram(this->programHandle_VG);
	this->prepGridUniforms(mvMat, pMat, lightPos, baseMatrix, texHandle, grid);
	GetOpenGLError();

	glBindVertexArray(this->vaoHandle);
	this->setupVAOPointers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);

	glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(this->drawCalls));

	glUseProgram(0);
	glBindVertexArray(0);
}

void Scene::generateGrid(std::size_t _x, std::size_t _y, std::size_t _z) {
	this->vertPos.clear();
	this->vertNorm.clear();
	this->vertIdx.clear();
	this->vertIdxDraw.clear();

	this->generateNeighborGrid(_x,_y,_z);
	this->generateTexCube();

	this->renderSize = this->vertIdx.size();
	this->drawCalls	= this->vertIdxDraw.size();

	this->neighborOffset = glm::vec3(0, 0, 0);
	this->controlPanel->setXCoord(0);
	this->controlPanel->setYCoord(0);
	this->controlPanel->setZCoord(0);

	this->setupVBOData();
}

void Scene::populateGrid() {
	if (this->voxelGrid == nullptr) {
		std::cerr << "Could not populate grid, was not allocated !" << '\n';
		return;
	}

	std::cerr << "Could not populate grid from scene !\n";

	//this->voxelGrid->populateGrid(InterpolationMethods::NearestNeighbor);
}

void Scene::generateTexCube() {
	// Generate statically a cube of dimension gridSize :
	glm::vec4 center = glm::vec4(.5f, .5f, .5f, 1.f);
	/**
	 * Name references :
	 *     -i
	 *     A
	 *     a-------b-->+k
	 *    /|      /|
	 *   c-------d |
	 *  /| |     | |
	 *+j | |e----|-|f
	 *   |/      |/
	 *   g-------h
	 */
	// Build position, normals, and tex coordinates all in one line for each vertex
	glm::vec4 apos = glm::vec4(.0, .0, .0, 1.); glm::vec4 anorm = apos - center;
	glm::vec4 bpos = glm::vec4(1., .0, .0, 1.); glm::vec4 bnorm = bpos - center;
	glm::vec4 cpos = glm::vec4(.0, 1., .0, 1.); glm::vec4 cnorm = cpos - center;
	glm::vec4 dpos = glm::vec4(1., 1., .0, 1.); glm::vec4 dnorm = dpos - center;
	glm::vec4 epos = glm::vec4(.0, .0, 1., 1.); glm::vec4 enorm = epos - center;
	glm::vec4 fpos = glm::vec4(1., .0, 1., 1.); glm::vec4 fnorm = fpos - center;
	glm::vec4 gpos = glm::vec4(.0, 1., 1., 1.); glm::vec4 gnorm = gpos - center;
	glm::vec4 hpos = glm::vec4(1., 1., 1., 1.); glm::vec4 hnorm = hpos - center;
	this->vertPos.push_back(apos); this->vertNorm.push_back(anorm);
	this->vertPos.push_back(bpos); this->vertNorm.push_back(bnorm);
	this->vertPos.push_back(cpos); this->vertNorm.push_back(cnorm);
	this->vertPos.push_back(dpos); this->vertNorm.push_back(dnorm);
	this->vertPos.push_back(epos); this->vertNorm.push_back(enorm);
	this->vertPos.push_back(fpos); this->vertNorm.push_back(fnorm);
	this->vertPos.push_back(gpos); this->vertNorm.push_back(gnorm);
	this->vertPos.push_back(hpos); this->vertNorm.push_back(hnorm);
	unsigned int a = 0; unsigned int b = 1; unsigned int c = 2; unsigned int d = 3;
	unsigned int e = 4; unsigned int f = 5; unsigned int g = 6; unsigned int h = 7;
	unsigned int faceIdx1[]	= {a, d, c, a, b, d, e, b, a, e, f, b, g, e, a, g, a, c};
	unsigned int faceIdx2[] = {h, e, g, h, f, e, h, g, c, h, c, d, h, d, b, h, f, b};
	this->vertIdx.insert(vertIdx.end(), faceIdx1, faceIdx1+18);
	this->vertIdx.insert(vertIdx.end(), faceIdx2, faceIdx2+18);
}

void Scene::setupVBOData() {

	if (glIsVertexArray(this->vaoHandle) == GL_FALSE) { throw std::runtime_error("The vao handle generated by OpenGL has been invalidated !"); }
	///////////////////////////////////////////////
	/// CREATE VBO AND UPLOAD DATA
	///////////////////////////////////////////////
	glGenBuffers(1, &this->vboVertPosHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertPosHandle);
	glBufferData(GL_ARRAY_BUFFER, this->vertPos.size()*sizeof(glm::vec4), this->vertPos.data(), GL_DYNAMIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboVertNormHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertNormHandle);
	glBufferData(GL_ARRAY_BUFFER, this->vertNorm.size()*sizeof(glm::vec4), this->vertNorm.data(), GL_DYNAMIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboElementHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->vertIdx.size()*sizeof(unsigned int), this->vertIdx.data(), GL_DYNAMIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboIndexedDrawHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboIndexedDrawHandle);
	glBufferData(GL_ARRAY_BUFFER, this->vertIdxDraw.size()*sizeof(uvec4), this->vertIdxDraw.data(), GL_DYNAMIC_DRAW);
	GetOpenGLError();

	this->setupVAOPointers();

	this->vertPos.clear();
	this->vertNorm.clear();
	this->vertIdx.clear();
	this->vertIdxDraw.clear();
}

void Scene::setupVAOPointers() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertPosHandle);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertNormHandle);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboIndexedDrawHandle);
	glVertexAttribIPointer(2, 4, GL_UNSIGNED_INT, 0, (void*)0);
	glVertexAttribDivisor(2, 1);
	GetOpenGLError();

//	this->vao->enableVertexAttributes();
//	glVertexAttribDivisor(2, 1);
}

void Scene::generateNeighborGrid(std::size_t _x, std::size_t _y, std::size_t _z) {
	this->neighborWidth = _x;
	this->neighborHeight =_y;
	this->neighborDepth = _z;

	// Original texture cube :
	this->vertIdxDraw.emplace_back(uint(1), uint(1), uint(1), uint(0));

	/* // This is to create a neighbor grid :
	// Create the grid, in raw form :
	for (std::size_t i = 0; i < _z; ++i) {
		for (std::size_t j = 0; j < _y; ++j) {
			for (std::size_t k = 0; k < _x; ++k) {
				this->vertIdxDraw.emplace_back(k, j, i, uint(1));
			}
		}
	}
	*/

	/* // This is to create a voxel grid for a voxel-like visualisation
	for (std::size_t i = 0; i < this->gridDepth; ++i) {
		for (std::size_t j = 0; j < this->gridHeight; ++j) {
			for (std::size_t k = 0; k < this->gridWidth; ++k) {
				this->vertIdxDraw.emplace_back(k, j, i, uint(1));
			}
		}
	}
	*/
}

const unsigned char* Scene::loadEmptyImage() {
	std::size_t i = 1;
	std::size_t j = 1;
	std::size_t k = 1;

	const unsigned char* pData = static_cast<unsigned char*>(calloc(i*j*k, sizeof(unsigned char)));

	this->gridWidth = 0;
	this->gridHeight= 0;
	this->gridDepth = 0;

	return pData;
}

const unsigned char* Scene::loadEmptyVoxelGrid() {
	std::size_t i = 1;
	std::size_t j = 1;
	std::size_t k = 1;

	const unsigned char* pData = static_cast<unsigned char*>(calloc(i*j*k, sizeof(unsigned char)));

	return pData;
}

glm::vec3 Scene::getSceneBoundaries() const {
	return glm::vec3(
		static_cast<float>(std::max(this->gridWidth, this->neighborWidth)),
		static_cast<float>(std::max(this->gridHeight, this->neighborHeight)),
		static_cast<float>(std::max(this->gridDepth, this->neighborDepth))
	);
}

glm::vec3 Scene::getTexCubeBoundaries(bool realSpace) const {
	glm::vec3 baseVtx = glm::vec3(static_cast<float>(this->gridWidth), static_cast<float>(this->gridHeight), static_cast<float>(this->gridDepth));
	if (realSpace) {
		glm::mat3 transfoMat = glm::mat3(this->computeTransformationMatrix());
		return transfoMat * baseVtx;
	} else {
		return baseVtx;
	}
}

nbCoord Scene::getNeighborBoundaries(bool realSpace) const {
	/**
	 * If we are in real space, the stack is deformed. As such, we apply the transformation matrix.
	 * If we're in initial space, we can byupass this transformation.
	 */
	glm::vec3 neighbor(static_cast<float>(this->neighborWidth), static_cast<float>(this->neighborHeight), static_cast<float>(this->neighborDepth));
	glm::vec3 pos(neighborOffset);

	glm::mat3 transfoMat = glm::mat3(this->computeTransformationMatrix());
	glm::vec3 max = neighbor+pos;
	if (realSpace) {
		pos = transfoMat * pos;
		max = transfoMat * max;
	}
	return nbCoord{{pos}, {max}};
}

void Scene::hideTexCubeVBO() {
	this->cubeShown = false;
	glBindBuffer(GL_ARRAY_BUFFER, this->vboIndexedDrawHandle);
	GLuint newData[] = {0, 0, 0};
	glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(0), 3u*sizeof(unsigned int), newData);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Scene::showTexCubeVBO() {
	this->cubeShown = true;
	glBindBuffer(GL_ARRAY_BUFFER, this->vboIndexedDrawHandle);
	GLuint newData[] = {static_cast<GLuint>(this->gridWidth), static_cast<GLuint>(this->gridHeight), static_cast<GLuint>(this->gridDepth)};
	glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(0), 3u*sizeof(unsigned int), newData);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Scene::slotToggleShowTextureCube(bool show) {
	this->showTextureCube = show;
	std::cerr << "Set tex cube to " << std::boolalpha << this->showTextureCube << std::noboolalpha << '\n';
}

void Scene::slotSetNeighborXCoord(float newXCoord) {
	this->neighborOffset.x = newXCoord;
}

void Scene::slotSetNeighborYCoord(float newYCoord) {
	this->neighborOffset.y = newYCoord;
}

void Scene::slotSetNeighborZCoord(float newZCoord) {
	this->neighborOffset.z = newZCoord;
}

void Scene::slotSetTextureXCoord(uint newXCoord) {
	this->neighborPos.x = newXCoord;
}

void Scene::slotSetTextureYCoord(uint newYCoord) {
	this->neighborPos.y = newYCoord;
}

void Scene::slotSetTextureZCoord(uint newZCoord) {
	this->neighborPos.z = newZCoord;
}

void Scene::slotSetMinTexValue(uchar val) {
	this->minTexVal = val;
}

void Scene::slotSetMaxTexValue(uchar val) {
	this->maxTexVal = val;
}

void Scene::updateNeighborTetMesh() {
	/*
	glm::vec4 n = glm::vec4(this->neighborOffset.x, this->neighborOffset.y, this->neighborOffset.z, 1.);
	std::cerr << "Queried point ! " << n.x << ',' << n.y << ',' << n.z << '\n';
	glm::vec4 o = this->texStorage->convertRealSpaceToInitialSpace(n);
	std::cerr << "Inverse point ! " << o.x << ',' << o.y << ',' << o.z << ',' << o.w << '\n';
	*/
}

void Scene::slotSetCutPlaneX_Min(float coord) {
	this->cutPlaneMin.x = coord;
}

void Scene::slotSetCutPlaneY_Min(float coord) {
	this->cutPlaneMin.y = coord;
}

void Scene::slotSetCutPlaneZ_Min(float coord) {
	this->cutPlaneMin.z = coord;
}

void Scene::slotSetCutPlaneX_Max(float coord) {
	this->cutPlaneMax.x = coord;
}

void Scene::slotSetCutPlaneY_Max(float coord) {
	this->cutPlaneMax.y = coord;
}

void Scene::slotSetCutPlaneZ_Max(float coord) {
	this->cutPlaneMax.z = coord;
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

	//return transfoMat;
	return glm::mat4(1.f);
}
