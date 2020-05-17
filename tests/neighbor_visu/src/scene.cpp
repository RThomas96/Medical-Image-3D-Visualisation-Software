#include "../include/scene.hpp"
#include "../include/scene_control.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QOpenGLContext>
#include <QSurface>

#include <fstream>
// TODO : change the inspectorPosNormlized by inspectorSizeNormalized and inspectorPosition
// TODO : check we passed all attributes to the shader
// TODO	: allow for the texture cube to be drawn
// TODO : test the class

Scene::Scene(void) {
	this->loader = nullptr;
	this->controlPanel = nullptr;

	this->gridWidth = 0;
	this->gridHeight = 0;
	this->gridDepth = 0;
	this->neighborWidth = 0;
	this->neighborHeight = 0;
	this->neighborDepth = 0;

	this->showTextureCube = false;
	this->renderSize = 0;

	this->drawRealVoxelSize = false;
	this->isInitialized = false;

	this->textureHandle = 0;
	this->vboVertPosHandle = 0;
	this->vboUVCoordHandle = 0;
	this->vboElementHandle = 0;
	this->vaoHandle = 0;
	this->vShaHandle = 0;
	this->fShaHandle = 0;
	this->programHandle = 0;
	this->polygonMode = GL_FILL;

	this->neighborOffset = glm::vec3(.0f, .0f, .0f);

	// Uniform locations :
	this->mMatrixLocation = -1;
	this->vMatrixLocation = -1;
	this->pMatrixLocation = -1;
	this->lightPosLocation = -1;
	this->neighborOffsetLocation = -1;
}

Scene::~Scene(void) {
	glDeleteTextures(1, &this->textureHandle);
}

void Scene::initGl(QOpenGLContext* _context, std::size_t _x, std::size_t _y, std::size_t _z) {
	if (this->isInitialized == true) { return; }
	this->isInitialized = true;

	this->context = _context;

	if (this->context == 0) { std::cerr << "Warning ! this->context() returned 0 !" << '\n' ; }
	if (this->context == nullptr) { std::cerr << "Warning ! Initializing a scene without a valid OpenGL context !" << '\n' ; }

	this->initializeOpenGLFunctions();

	this->loader = new bulk_texture_loader();
	this->loader->enable_downsampling(true);

	///////////////////////////
	/// CREATE VAO :
	///////////////////////////
	glGenVertexArrays(1, &this->vaoHandle);
	GetOpenGLError();
	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	this->compileShaders();

	this->queryImage();

	this->generateGrid(_x, _y, _z);

	int bounds[] = {0, static_cast<int>(this->gridWidth - this->neighborWidth), 0, static_cast<int>(this->gridHeight - this->neighborHeight), 0, static_cast<int>(this->gridDepth - this->neighborDepth)};
	if (this->gridWidth <= this->neighborWidth || this->gridHeight <= this->neighborHeight || this->gridDepth <= this->neighborDepth) {
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
	GetOpenGLError();
}

void Scene::compileShaders() {
	glUseProgram(0);
	glDeleteShader(this->vShaHandle);
	glDeleteShader(this->fShaHandle);
	glDeleteProgram(this->programHandle);
	GetOpenGLError();

	///////////////////////////
	/// CREATE SHADERS AND COMPILE :
	///////////////////////////
	this->vShaHandle = glCreateShader(GL_VERTEX_SHADER);
	GetOpenGLError();
	this->fShaHandle = glCreateShader(GL_FRAGMENT_SHADER);
	GetOpenGLError();

	// shaderPaths shader paths
	std::string vShaPath = "./shaders/base.vert";
	std::string fShaPath = "./shaders/base.frag";

	// Open shader file for reading :
	std::ifstream vShaFile = std::ifstream(vShaPath, std::ios_base::in);
	std::ifstream fShaFile = std::ifstream(fShaPath, std::ios_base::in);

	if (!vShaFile.is_open()) {
		std::cerr << "Error : could not get the contents of shader file " << vShaPath << '\n';
		exit(EXIT_FAILURE);
	}
	if (!fShaFile.is_open()) {
		vShaFile.close();
		std::cerr << "Error : could not get the contents of shader file " << fShaPath << '\n';
		exit(EXIT_FAILURE);
	}

	// Get file size by seeking end and rewinding :
	vShaFile.seekg(0, vShaFile.end);
	std::size_t vShaFileSize = static_cast<std::size_t>(vShaFile.tellg());
	vShaFile.seekg(0, vShaFile.beg);
	fShaFile.seekg(0, fShaFile.end);
	std::size_t fShaFileSize = static_cast<std::size_t>(fShaFile.tellg());
	fShaFile.seekg(0, fShaFile.beg);

	// Get shader file contents :
	char* vShaSource = new char[vShaFileSize+1];
	vShaFile.read(vShaSource, vShaFileSize);
	vShaSource[vShaFileSize] = '\0';
	char* fShaSource = new char[fShaFileSize+1];
	fShaFile.read(fShaSource, fShaFileSize);
	fShaSource[fShaFileSize] = '\0';

	glShaderSource(this->vShaHandle, 1, const_cast<const char**>(&vShaSource), 0);
	glShaderSource(this->fShaHandle, 1, const_cast<const char**>(&fShaSource), 0);

	delete[] vShaSource;
	delete[] fShaSource;

	vShaFile.close();
	fShaFile.close();

	glCompileShader(this->vShaHandle);
	GetOpenGLError();

	{
		GLint shaderInfoLength = 0;
		GLint charsWritten = 0;
		char* shaderInfoLog = nullptr;

		glGetShaderiv(this->vShaHandle, GL_INFO_LOG_LENGTH, &shaderInfoLength);
		if (shaderInfoLength > 1) {
			std::cerr << __PRETTY_FUNCTION__ << " : start Log ***********************************************" << '\n';

			std::cerr << __FUNCTION__ << " : Information about shader " << vShaPath << " : " << '\n';
			std::cerr << __FUNCTION__ << " : Shader was a vertex shader ";
			shaderInfoLog = new char[shaderInfoLength];
			glGetShaderInfoLog(this->vShaHandle, shaderInfoLength, &charsWritten, shaderInfoLog);
			GetOpenGLError();
			std::cerr << shaderInfoLog << '\n';
			delete[] shaderInfoLog;

			std::cerr << __PRETTY_FUNCTION__ << " : end Log ***********************************************" << '\n';
		} else {
			std::cerr << "No more info about shader " << vShaPath << '\n';
		}
	}

	glCompileShader(this->fShaHandle);
	GetOpenGLError();

	{
		GLint shaderInfoLength = 0;
		GLint charsWritten = 0;
		char* shaderInfoLog = nullptr;

		glGetShaderiv(this->fShaHandle, GL_INFO_LOG_LENGTH, &shaderInfoLength);
		if (shaderInfoLength > 1) {
			std::cerr << __PRETTY_FUNCTION__ << " : start Log ***********************************************" << '\n';

			std::cerr << __FUNCTION__ << " : Information about shader " << fShaPath << " : " << '\n';
			std::cerr << __FUNCTION__ << " : Shader was a fragment shader ";
			shaderInfoLog = new char[shaderInfoLength];
			glGetShaderInfoLog(this->fShaHandle, shaderInfoLength, &charsWritten, shaderInfoLog);
			GetOpenGLError();
			std::cerr << shaderInfoLog << '\n';
			delete[] shaderInfoLog;

			std::cerr << __PRETTY_FUNCTION__ << " : end Log ***********************************************" << '\n';
		} else {
			std::cerr << "No more info about shader " << fShaPath << '\n';
		}
	}

	///////////////////////////
	/// CREATE PROGRAM AND LINK :
	///////////////////////////
	this->programHandle = glCreateProgram();
	GetOpenGLError();
	glAttachShader(this->programHandle, this->vShaHandle);
	glAttachShader(this->programHandle, this->fShaHandle);

	glLinkProgram(this->programHandle);
	GetOpenGLError();

	{
		GLint Result = 0;
		int InfoLogLength = 0;
		glGetProgramiv(this->programHandle, GL_LINK_STATUS, &Result);
		glGetProgramiv(this->programHandle, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if ( InfoLogLength > 0 ){
			std::vector<char> ProgramErrorMessage(InfoLogLength+1);
			glGetProgramInfoLog(this->programHandle, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			GetOpenGLError();
			std::cerr << __FUNCTION__ << " : Warning : errors while linking program :" << '\n';
			std::cerr << "------------------------------------------------------------------" << '\n';
			std::cerr << "------------------------------------------------------------------" << '\n';
			std::cerr << ProgramErrorMessage.data() << '\n';
			std::cerr << "------------------------------------------------------------------" << '\n';
			std::cerr << "------------------------------------------------------------------" << '\n';
		} else {
			std::cerr << "Linking of program happened just fine." << '\n';
		}

	}

	glDetachShader(this->programHandle, this->vShaHandle);
	glDetachShader(this->programHandle, this->fShaHandle);
	GetOpenGLError();

	glDeleteShader(this->vShaHandle);
	glDeleteShader(this->fShaHandle);

}

void Scene::loadImage(std::size_t i, std::size_t j, std::size_t k, const unsigned char *pData) {
	glEnable(GL_TEXTURE_3D);

	this->gridWidth = i;
	this->gridHeight= j;
	this->gridDepth = k;

	if (pData == nullptr) { this->loadEmptyImage(); }

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
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// Swizzle G/B to R value, to save data upload
	GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
	glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	glTexImage3D(
		GL_TEXTURE_3D,		// GLenum : Target
		static_cast<GLint>(0),	// GLint  : Level of detail of the current texture (0 = original)
		GL_R8UI,		// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		static_cast<GLsizei>(i),// GLsizei: Image width
		static_cast<GLsizei>(j),// GLsizei: Image height
		static_cast<GLsizei>(k),// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
		GL_RED,			// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,	// GLenum : Type (the data type as in uchar, uint, float ...)
		pData			// void*  : Data to load into the buffer
	);
	GetOpenGLError();
}

void Scene::queryImage(void) {
	const unsigned char* image = this->loader->load_stack_from_folder();
	std::size_t i = this->loader->get_image_width();
	std::size_t j = this->loader->get_image_height();
	std::size_t k = this->loader->get_image_depth();

	this->loadImage(i, j, k, image);
}

void Scene::drawRealSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_3D);
	GetOpenGLError();

	bDrawWireframe = true;

	if (this->context == nullptr) { std::cerr << "Warning ! Drawing in real space without a valid OpenGL context !" << '\n' ; }

	glm::mat4 transfoMat = glm::mat4(1.0);
	if (this->drawRealVoxelSize) {
		transfoMat *= glm::scale(glm::vec3(0.39, 0.39, 1.927));
	}

	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	//////////////////
	/// SET UNIFORMS :
	//////////////////

	glUseProgram(this->programHandle);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
	GetOpenGLError();

	glUniform1i(this->texDataLocation, 0);
	GetOpenGLError();
	glUniformMatrix4fv(this->mMatrixLocation, 1, GL_FALSE, glm::value_ptr(transfoMat));
	glUniformMatrix4fv(this->vMatrixLocation, 1, GL_FALSE, &mvMat[0]);
	glUniformMatrix4fv(this->pMatrixLocation, 1, GL_FALSE, &pMat[0]);
	glUniform4fv(this->lightPosLocation, 1, glm::value_ptr(lightPos));

	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	this->setupVAOPointers();

	glPolygonMode(GL_FRONT_AND_BACK, this->polygonMode);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);

	if (this->showTextureCube) {
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0);
	} else {
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize-36u), GL_UNSIGNED_INT, (void*)0);
	}

	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Scene::drawInitialSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_3D);

	bDrawWireframe = true;

	if (this->context == nullptr) { std::cerr << "Warning ! Drawing in initial space without a valid OpenGL context !" << '\n' ; }

	glm::mat4 transfoMat = glm::mat4(1.0);
	//if (this->drawRealVoxelSize) {
		transfoMat *= glm::scale(glm::vec3(0.39, 0.39, 1.927));
	//}

	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	//////////////////
	/// SET UNIFORMS :
	//////////////////

	glUseProgram(this->programHandle);

//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
//	GetOpenGLError();
	glUniformMatrix4fv(this->mMatrixLocation, 1, GL_FALSE, glm::value_ptr(transfoMat));
	glUniformMatrix4fv(this->vMatrixLocation, 1, GL_FALSE, &mvMat[0]);
	glUniformMatrix4fv(this->pMatrixLocation, 1, GL_FALSE, &pMat[0]);
	glUniform4fv(this->lightPosLocation, 1, glm::value_ptr(lightPos));

	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	this->setupVAOPointers();

	glPolygonMode(GL_FRONT_AND_BACK, this->polygonMode);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);

	if (this->showTextureCube) {
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0);
	} else {
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize-36), GL_UNSIGNED_INT, (void*)(0));
	}
	GetOpenGLError();

	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Scene::generateGrid(std::size_t _x, std::size_t _y, std::size_t _z) {
	this->vertPos.clear();
	this->vertNorm.clear();
	this->vertTex.clear();
	this->vertIdx.clear();

	this->generateNeighborGrid(_x,_y,_z);
	this->generateTexCube();

	this->neighborOffset = glm::vec3(.0f, .0f, .0f);
	this->controlPanel->setXCoord(0);
	this->controlPanel->setYCoord(0);
	this->controlPanel->setZCoord(0);

	this->setupVBOData();
}

void Scene::generateTexCube() {

	// Generate statically a cube of dimension gridSize :
	float gw = static_cast<float>(this->gridWidth);
	float gh = static_cast<float>(this->gridHeight);
	float gd = static_cast<float>(this->gridDepth);
	glm::vec4 center = glm::vec4(gw/2.f, gh/2.f, gd/2.f, 1.f);
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
	glm::vec4 apos = glm::vec4(.0, .0, .0, 1.); glm::vec4 anorm = apos - center; glm::vec3 atex = glm::vec3(.0, .0, .0);
	glm::vec4 bpos = glm::vec4(gw, .0, .0, 1.); glm::vec4 bnorm = bpos - center; glm::vec3 btex = glm::vec3(1., .0, .0);
	glm::vec4 cpos = glm::vec4(.0, gh, .0, 1.); glm::vec4 cnorm = cpos - center; glm::vec3 ctex = glm::vec3(.0, 1., .0);
	glm::vec4 dpos = glm::vec4(gw, gh, .0, 1.); glm::vec4 dnorm = dpos - center; glm::vec3 dtex = glm::vec3(1., 1., .0);
	glm::vec4 epos = glm::vec4(.0, .0, gd, 1.); glm::vec4 enorm = epos - center; glm::vec3 etex = glm::vec3(.0, .0, 1.);
	glm::vec4 fpos = glm::vec4(gw, .0, gd, 1.); glm::vec4 fnorm = fpos - center; glm::vec3 ftex = glm::vec3(1., .0, 1.);
	glm::vec4 gpos = glm::vec4(.0, gh, gd, 1.); glm::vec4 gnorm = gpos - center; glm::vec3 gtex = glm::vec3(.0, 1., 1.);
	glm::vec4 hpos = glm::vec4(gw, gh, gd, 1.); glm::vec4 hnorm = hpos - center; glm::vec3 htex = glm::vec3(1., 1., 1.);
	this->vertPos.push_back(apos); this->vertNorm.push_back(anorm); this->vertTex.push_back(atex);
	this->vertPos.push_back(bpos); this->vertNorm.push_back(bnorm); this->vertTex.push_back(btex);
	this->vertPos.push_back(cpos); this->vertNorm.push_back(cnorm); this->vertTex.push_back(ctex);
	this->vertPos.push_back(dpos); this->vertNorm.push_back(dnorm); this->vertTex.push_back(dtex);
	this->vertPos.push_back(epos); this->vertNorm.push_back(enorm); this->vertTex.push_back(etex);
	this->vertPos.push_back(fpos); this->vertNorm.push_back(fnorm); this->vertTex.push_back(ftex);
	this->vertPos.push_back(gpos); this->vertNorm.push_back(gnorm); this->vertTex.push_back(gtex);
	this->vertPos.push_back(hpos); this->vertNorm.push_back(hnorm); this->vertTex.push_back(htex);
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

	glGenBuffers(1, &this->vboUVCoordHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboUVCoordHandle);
	glBufferData(GL_ARRAY_BUFFER, this->vertTex.size()*sizeof(glm::vec3), this->vertTex.data(), GL_DYNAMIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboElementHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->vertIdx.size()*sizeof(unsigned int), this->vertIdx.data(), GL_DYNAMIC_DRAW);
	GetOpenGLError();

	this->setupVAOPointers();

	std::cerr << "Normally, inserted " << this->vertIdx.size() << " elements into element buffer. " << '\n';

	this->renderSize = this->vertIdx.size();
	this->vertPos.clear();
	this->vertNorm.clear();
	this->vertTex.clear();
	this->vertIdx.clear();
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
	glBindBuffer(GL_ARRAY_BUFFER, this->vboUVCoordHandle);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GetOpenGLError();
}

void Scene::generateNeighborGrid(std::size_t _x, std::size_t _y, std::size_t _z) {
	this->neighborWidth = _x;
	this->neighborHeight =_y;
	this->neighborDepth = _z;

	// make float versions ofthe sizes :
	float f_x = static_cast<float>(_x);
	float f_y = static_cast<float>(_y);
	float f_z = static_cast<float>(_z);

	glm::vec4 center = glm::vec4(f_x/2.f, f_y/2.f, f_z/2.f, 1.f);

	float scale = 50.f;

	// Create the grid, in raw form :
	for (std::size_t i = 0; i <= _z; ++i) {
		for (std::size_t j = 0; j <= _y; ++j) {
			for (std::size_t k = 0; k <= _x; ++k) {
				float f_i = static_cast<float>(i) * scale;
				float f_j = static_cast<float>(j) * scale;
				float f_k = static_cast<float>(k) * scale;
				// vertex coordinates
				glm::vec4 vPos = glm::vec4(f_k, f_j, f_i, 1.f);
				glm::vec4 vNorm = glm::normalize(vPos - center);
				// tex coordinates
				float xtex = f_k / f_x;
				float ytex = f_j / f_y;
				float ztex = f_i / f_z;
				glm::vec3 vTex = glm::vec3(xtex, ytex, ztex);
				// set data :
				this->vertPos.push_back(vPos);
				this->vertNorm.push_back(vNorm);
				this->vertTex.push_back(vTex);
			}
		}
	}

	unsigned int offset = this->vertIdx.size();
	std::size_t ylineoffset = (_x+1);
	std::size_t zplaneoffset = ylineoffset * (_y+1);

	// Create the index buffer used to draw it in order :
	// (see notes for reference about draw order)
	for (std::size_t i = 0; i < _z; ++i) {
		for (std::size_t j = 0; j < _y; ++j) {
			for (std::size_t k = 0; k < _x; ++k) {
				/**
				 * Name references :
				 *     +i
				 *     A
				 *     a-------b-->+k
				 *    /|      /|
				 *   c-------d |
				 *  /| |     | |
				 *+j | |e----|-|f
				 *   |/      |/
				 *   g-------h
				 */
				unsigned int a = offset + static_cast<unsigned int>( (i+0u)*zplaneoffset + (j+0u)*ylineoffset + (k+0u) );
				unsigned int b = offset + static_cast<unsigned int>( (i+0u)*zplaneoffset + (j+0u)*ylineoffset + (k+1u) );
				unsigned int c = offset + static_cast<unsigned int>( (i+0u)*zplaneoffset + (j+1u)*ylineoffset + (k+0u) );
				unsigned int d = offset + static_cast<unsigned int>( (i+0u)*zplaneoffset + (j+1u)*ylineoffset + (k+1u) );
				unsigned int e = offset + static_cast<unsigned int>( (i+1u)*zplaneoffset + (j+0u)*ylineoffset + (k+0u) );
				unsigned int f = offset + static_cast<unsigned int>( (i+1u)*zplaneoffset + (j+0u)*ylineoffset + (k+1u) );
				unsigned int g = offset + static_cast<unsigned int>( (i+1u)*zplaneoffset + (j+1u)*ylineoffset + (k+0u) );
				unsigned int h = offset + static_cast<unsigned int>( (i+1u)*zplaneoffset + (j+1u)*ylineoffset + (k+1u) );

				// Draw the half of the cube atteinable from the position a :
				// Faces :		 |    1   |    2   |    3   |    4   |    5   |   6   |
				unsigned int faceIdx1[]= {a, d, c, a, b, d, e, b, a, e, f, b, g, e, a, g, a, c};
				// Draw the other side, atteinable from the position h :
				// Faces :               |    1   |    2   |    3   |    4   |    5   |   6   |
				unsigned int faceIdx2[]= {h, e, g, h, f, e, h, g, c, h, c, d, h, d, b, h, f, b};

				// Insert them into the vector :
				this->vertIdx.insert(vertIdx.end(), faceIdx1, faceIdx1+18);
				this->vertIdx.insert(vertIdx.end(), faceIdx2, faceIdx2+18);
			}
		}
	}

}

const unsigned char* Scene::loadEmptyImage() {
	std::size_t i = 1;
	std::size_t j = 1;
	std::size_t k = 1;

	const unsigned char* pData = static_cast<unsigned char*>(calloc(i*j*k, sizeof(unsigned char)));

	this->gridWidth = i;
	this->gridHeight= j;
	this->gridDepth = k;

	return pData;
}

glm::vec3 Scene::getSceneBoundaries() const {
	return glm::vec3(
		static_cast<float>(std::max(this->gridWidth, this->neighborWidth)),
		static_cast<float>(std::max(this->gridHeight, this->neighborHeight)),
		static_cast<float>(std::max(this->gridDepth, this->neighborDepth))
	);
}

void Scene::slotTogglePolygonMode(bool status) {
	this->togglePolygonMode(status);
}

void Scene::slotToggleShowTextureCube(bool show) {
	this->showTextureCube = show;
}

void Scene::slotSetTextureXCoord(int newXCoord) {
	this->neighborOffset.x = static_cast<float>(newXCoord);
}

void Scene::slotSetTextureYCoord(int newYCoord) {
	this->neighborOffset.x = static_cast<float>(newYCoord);
}

void Scene::slotSetTextureZCoord(int newZCoord) {
	this->neighborOffset.x = static_cast<float>(newZCoord);
}
