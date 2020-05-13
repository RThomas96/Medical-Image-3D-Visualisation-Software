#include "../include/scene.hpp"

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

	this->textureHandle = 0;

	this->positionNormalized = glm::vec3(.0f, .0f, .0f);

	this->gridWidth = 0;
	this->gridHeight = 0;
	this->gridDepth = 0;
	this->neighborWidth = 0;
	this->neighborHeight = 0;
	this->neighborDepth = 0;

	this->drawRealVoxelSize = false;
	this->isInitialized = false;

	this->vboVertPosHandle = 0;
	this->vboUVCoordHandle = 0;
	this->vboElementHandle = 0;
	this->vaoHandle = 0;
	this->vShaHandle = 0;
	this->fShaHandle = 0;
	this->programHandle = 0;
	this->elemToDrawSeq = 0;
	this->elemToDrawIdx = 0;
}

Scene::~Scene(void) {
	glDeleteTextures(1, &this->textureHandle);
}

void Scene::initGl(std::size_t _x, std::size_t _y, std::size_t _z) {
	if (this->isInitialized == true) { return; }
	this->isInitialized = true;

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Could not start GLEW" << '\n';
		exit(EXIT_FAILURE);
	}

	this->loader = new bulk_texture_loader();
	this->generateGrid_Only(_x, _y, _z);

	///////////////////////////
	/// CREATE VAO :
	///////////////////////////
	glGenVertexArrays(1, &this->vaoHandle);
	GetOpenGLError();
	std::cerr << "Initialized VAO array idx : " << this->vaoHandle << '\n';
	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	this->setupVBOData();
	GetOpenGLError();

	this->compileShaders();
	GetOpenGLError();

	glUseProgram(this->programHandle);
	GetOpenGLError();

	this->queryImage();
	GetOpenGLError();

	glUseProgram(this->programHandle);
	this->mMatrixLocation = glGetUniformLocation(this->programHandle, "mMatrix");
	this->vMatrixLocation = glGetUniformLocation(this->programHandle, "vMatrix");
	this->pMatrixLocation = glGetUniformLocation(this->programHandle, "pMatrix");
	this->lightPosLocation = glGetUniformLocation(this->programHandle, "lightPos");
}

void Scene::compileShaders() {
	glUseProgram(0);
	GetOpenGLError();
	glDeleteShader(this->vShaHandle);
	GetOpenGLError();
	glDeleteShader(this->fShaHandle);
	GetOpenGLError();
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
	GetOpenGLError();
	glShaderSource(this->fShaHandle, 1, const_cast<const char**>(&fShaSource), 0);
	GetOpenGLError();

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
		GetOpenGLError();
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
		GetOpenGLError();
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
	GetOpenGLError();
	glAttachShader(this->programHandle, this->fShaHandle);
	GetOpenGLError();

	glLinkProgram(this->programHandle);
	GetOpenGLError();

	{
		GLint Result = 0;
		int InfoLogLength = 0;
		glGetProgramiv(this->programHandle, GL_LINK_STATUS, &Result);
		GetOpenGLError();
		glGetProgramiv(this->programHandle, GL_INFO_LOG_LENGTH, &InfoLogLength);
		GetOpenGLError();
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
	GetOpenGLError();
	glDetachShader(this->programHandle, this->fShaHandle);
	GetOpenGLError();

	glDeleteShader(this->vShaHandle);
	GetOpenGLError();
	glDeleteShader(this->fShaHandle);
	GetOpenGLError();

}

void Scene::loadImage(std::size_t i, std::size_t j, std::size_t k, const unsigned char *pData) {
	if (pData == nullptr) { this->loadEmptyImage(); }

	this->gridWidth = i;
	this->gridHeight= j;
	this->gridDepth = k;

	if (this->textureHandle == 0) {
		glGenTextures(1, &this->textureHandle);
		GetOpenGLError();
	}
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
	GetOpenGLError();

	// Set nearest neighbor :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GetOpenGLError();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GetOpenGLError();
	// Set the texture upload to not generate mimaps :
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, static_cast<GLfloat>(-1000.f));
	GetOpenGLError();
	// Stop once UV > 1 or < 0
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	GetOpenGLError();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	GetOpenGLError();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GetOpenGLError();
	// Swizzle G/B to R value, to save data upload
	GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
	glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	GetOpenGLError();

	glTexImage3D(
		GL_TEXTURE_3D,		// GLenum : Target
		static_cast<GLint>(0),	// GLint  : Level of detail of the current texture (0 = original)
		GL_RED,			// GLint  : Number of color components in the picture. Here grayscale so GL_RED
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

void Scene::drawRealSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) const {
	GetOpenGLError();
	glEnable(GL_DEPTH_TEST);
	GetOpenGLError();
	glEnable(GL_TEXTURE_3D);
	GetOpenGLError();

	glm::mat4 transfoMat = glm::mat4(1.0);
	if (this->drawRealVoxelSize) {
		transfoMat *= glm::scale(glm::vec3(0.39, 0.39, 1.927));
	}

	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	//////////////////
	/// SET UNIFORMS :
	//////////////////

	glUseProgram(this->programHandle);
	GetOpenGLError();

//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
//	GetOpenGLError();

//	GLint texDataLocation = glGetUniformLocation(this->programHandle, "texData");
//	GetOpenGLError();
//	GLint texOffsetLocation = glGetUniformLocation(this->programHandle, "texOffset");
//	GetOpenGLError();

//	glUniform1i(texDataLocation, 0);
//	GetOpenGLError();
	glUniformMatrix4fv(this->mMatrixLocation, 1, GL_FALSE, glm::value_ptr(transfoMat));
	GetOpenGLError();
	glUniformMatrix4fv(this->vMatrixLocation, 1, GL_FALSE, &mvMat[0]);
	GetOpenGLError();
	glUniformMatrix4fv(this->pMatrixLocation, 1, GL_FALSE, &pMat[0]);
	GetOpenGLError();
	glUniform4fv(this->lightPosLocation, 1, glm::value_ptr(lightPos));
	GetOpenGLError();
//	glUniform3fv(texOffsetLocation, 1, &this->positionNormalized[0]);
//	GetOpenGLError();

	glBindVertexArray(this->vaoHandle);
	std::cerr << "RSpace : attempting to bind a VAO of ID " << this->vaoHandle << '\n';
	GetOpenGLError();

	this->setupVAOPointers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	GetOpenGLError();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->vertIdx.size()), GL_UNSIGNED_BYTE, (void*)0);
	GetOpenGLError();

	glBindVertexArray(0);
	GetOpenGLError();
	glUseProgram(0);
	GetOpenGLError();
}

void Scene::drawInitialSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) const {
	GetOpenGLError();
	glEnable(GL_DEPTH_TEST);
	GetOpenGLError();
	glEnable(GL_TEXTURE_3D);
	GetOpenGLError();

	glm::mat4 transfoMat = glm::mat4(1.0);
	if (this->drawRealVoxelSize) {
		transfoMat *= glm::scale(glm::vec3(0.39, 0.39, 1.927));
	}

	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	//////////////////
	/// SET UNIFORMS :
	//////////////////

	glUseProgram(this->programHandle);
	GetOpenGLError();

//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
//	GetOpenGLError();

	glUniformMatrix4fv(this->mMatrixLocation, 1, GL_FALSE, glm::value_ptr(transfoMat));
	GetOpenGLError();
	glUniformMatrix4fv(this->vMatrixLocation, 1, GL_FALSE, &mvMat[0]);
	GetOpenGLError();
	glUniformMatrix4fv(this->pMatrixLocation, 1, GL_FALSE, &pMat[0]);
	GetOpenGLError();
	glUniform4fv(this->lightPosLocation, 1, glm::value_ptr(lightPos));
	GetOpenGLError();

	GLint current_vao = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
	glBindVertexArray(this->vaoHandle);
	std::cerr << "ISpace : attempting to bind a VAO of ID " << this->vaoHandle << " after VAO bound before was : " << current_vao << '\n';
	GetOpenGLError();

	this->setupVAOPointers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	GetOpenGLError();

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->vertIdx.size()), GL_UNSIGNED_INT, (void*)0);
	GetOpenGLError();

	glBindVertexArray(0);
	GetOpenGLError();
	glUseProgram(0);
	GetOpenGLError();
}

void Scene::generateGrid(std::size_t _x, std::size_t _y, std::size_t _z) {
	this->generateGrid_Only(_x,_y,_z);

	this->setupVBOData();

	this->elemToDrawSeq = this->vertPos.size()/4;
	this->elemToDrawIdx = this->vertIdx.size();

	std::cerr << "Normally, inserted " << this->vertIdx.size() << " elements into element buffer. " << '\n';
}

void Scene::setupVBOData() {
	if (glIsVertexArray(this->vaoHandle) == GL_FALSE) { throw std::runtime_error("The vao handle generated by OpenGL has been invalidated !"); }
	GetOpenGLError();
	///////////////////////////////////////////////
	/// CREATE VBO AND UPLOAD DATA
	///////////////////////////////////////////////
	glGenBuffers(1, &this->vboVertPosHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertPosHandle);
	glBufferData(GL_ARRAY_BUFFER, this->vertPos.size()*sizeof(glm::vec4), this->vertPos.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboVertNormHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertNormHandle);
	glBufferData(GL_ARRAY_BUFFER, this->vertNorm.size()*sizeof(glm::vec4), this->vertNorm.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboUVCoordHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboUVCoordHandle);
	glBufferData(GL_ARRAY_BUFFER, this->vertTex.size()*sizeof(glm::vec3), this->vertTex.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glGenBuffers(1, &this->vboElementHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->vertIdx.size()*sizeof(unsigned char), this->vertIdx.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	this->setupVAOPointers();

	this->elemToDrawSeq = this->vertPos.size()/4;
	this->elemToDrawIdx = this->vertIdx.size();

	std::cerr << "Normally, inserted " << this->vertIdx.size() << " elements into element buffer. " << '\n';
}

void Scene::setupVAOPointers() const {
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

void Scene::generateGrid_Only(std::size_t _x, std::size_t _y, std::size_t _z) {
	this->neighborWidth = _x;
	this->neighborHeight =_y;
	this->neighborDepth = _z;

	// make float versions ofthe sizes :
	float f_x = static_cast<float>(_x+1);
	float f_y = static_cast<float>(_y+1);
	float f_z = static_cast<float>(_z+1);

	glm::vec4 center = glm::vec4(f_x/2.f, f_y/2.f, f_z/2.f, 1.f);

	// Create the grid, in raw form :
	for (std::size_t i = 0; i <= _z; ++i) {
		for (std::size_t j = 0; j <= _y; ++j) {
			for (std::size_t k = 0; k <= _x; ++k) {
				glm::vec4 vPos = glm::vec4(static_cast<float>(k), static_cast<float>(j), static_cast<float>(i), 1.f);
				this->vertPos.push_back(vPos);
				float xtex = static_cast<float>(k) / f_x;
				float ytex = static_cast<float>(j) / f_y;
				float ztex = static_cast<float>(i) / f_z;
				this->vertNorm.push_back(center - vPos);
				this->vertTex.emplace_back(xtex, ytex, ztex);
			}
		}
	}

	std::size_t zplaneoffset = (_x+1) * (_y+1);
	std::size_t ylineoffset = (_x+1);

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
				unsigned char a = static_cast<unsigned char>((k+0) + (j+0)*ylineoffset + (i+0)*zplaneoffset);
				unsigned char b = static_cast<unsigned char>((k+1) + (j+0)*ylineoffset + (i+0)*zplaneoffset);
				unsigned char c = static_cast<unsigned char>((k+0) + (j+1)*ylineoffset + (i+0)*zplaneoffset);
				unsigned char d = static_cast<unsigned char>((k+1) + (j+1)*ylineoffset + (i+0)*zplaneoffset);
				unsigned char e = static_cast<unsigned char>((k+0) + (j+0)*ylineoffset + (i+1)*zplaneoffset);
				unsigned char f = static_cast<unsigned char>((k+1) + (j+0)*ylineoffset + (i+1)*zplaneoffset);
				unsigned char g = static_cast<unsigned char>((k+0) + (j+1)*ylineoffset + (i+1)*zplaneoffset);
				unsigned char h = static_cast<unsigned char>((k+1) + (j+1)*ylineoffset + (i+1)*zplaneoffset);

				// Draw the half of the cube atteinable from the position a :
				// Faces :	          |    1   |    2   |    3   |    4   |    5   |   6   |
				unsigned char faceIdx1[]= {a, d, c, a, b, d, e, b, a, e, f, b, g, e, a, g, a, c};
				// Draw the other side :
				// Faces :	          |    1   |    2   |    3   |    4   |    5   |   6   |
				unsigned char faceIdx2[]= {h, e, g, h, f, e, h, g, c, h, c, d, h, d, b, h, f, b};

				// Insert them into the vector :
				this->vertIdx.insert(vertIdx.end(), faceIdx1, faceIdx1+18);
				this->vertIdx.insert(vertIdx.end(), faceIdx2, faceIdx2+18);
			}
		}
	}

}

void Scene::toggleRealVoxelSize() {
	this->drawRealVoxelSize = !this->drawRealVoxelSize;
}

void Scene::loadEmptyImage() {
	if (this->textureHandle == 0) {
		glGenTextures(1, &this->textureHandle);
	}

	std::size_t i = 10;
	std::size_t j = 10;
	std::size_t k = 10;

	const char* pData = static_cast<char*>(calloc(i*j*k, sizeof(char)));

	this->gridWidth = i;
	this->gridHeight= j;
	this->gridDepth = k;


	glBindTexture(GL_TEXTURE_3D, this->textureHandle);

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
		GL_RED,			// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		static_cast<GLsizei>(i),// GLsizei: Image width
		static_cast<GLsizei>(j),// GLsizei: Image height
		static_cast<GLsizei>(k),// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
		GL_RED,			// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,	// GLenum : Type (the data type as in uchar, uint, float ...)
		pData			// void*  : Data to load into the buffer
	);

	return;
}

glm::vec3 Scene::getSceneBoundaries() const {
	return glm::vec3(
		static_cast<float>(std::max(this->gridWidth, this->neighborWidth)),
		static_cast<float>(std::max(this->gridHeight, this->neighborHeight)),
		static_cast<float>(std::max(this->gridDepth, this->neighborDepth))
	);
}
