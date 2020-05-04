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

#ifndef RAW_GL
	this->vao = nullptr;
	this->program = nullptr;
#endif

	this->textureHandle = 0;

	this->transposeMatrices = GL_FALSE;

	this->positionNormalized = glm::vec3(.0f, .0f, .0f);

	this->gridWidth = 0;
	this->gridHeight = 0;
	this->gridDepth = 0;
	this->neighborWidth = 0;
	this->neighborHeight = 0;
	this->neighborDepth = 0;

	this->drawRealVoxelSize = false;
	this->isInitialized = false;

#ifdef RAW_GL
	this->vboVertPosHandle = 0;
	this->vboUVCoordHandle = 0;
	this->vboElementHandle = 0;
	this->vaoHandle = 0;
	this->vShaHandle = 0;
	this->fShaHandle = 0;
	this->programHandle = 0;
	this->elemToDrawSeq = 0;
	this->elemToDrawIdx = 0;
#endif
}

Scene::~Scene(void) {
#ifndef RAW_GL
	delete this->vao;
	delete this->program;
#endif

	glDeleteTextures(1, &this->textureHandle);
}

void Scene::initGl(QOpenGLContext* const _context, std::size_t _x, std::size_t _y, std::size_t _z) {
	if (this->isInitialized == true) { return; }
	this->isInitialized = true;

	glewExperimental = false;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Could not start GLEW" << '\n';
		exit(EXIT_FAILURE);
	}

#ifndef RAW_GL
	std::size_t pos;
	VBOCreator vert = {pos, 4, GL_FLOAT};
	VBOCreator uvco = {pos, 3, GL_FLOAT};
	VBOCreator elem = {pos, 1, GL_UNSIGNED_INT};
	this->vao = new VAOObject(VBOTypes::Vertex, vert, VBOTypes::TexCoords, uvco, VBOTypes::IndexBuffer, elem);

	// Create the vertex, fragment shaders and program
	std::shared_ptr<ShaderObject> vSha = std::make_shared<ShaderObject>("./base_scene_vshader.glsl", GL_VERTEX_SHADER);
	std::shared_ptr<ShaderObject> fSha = std::make_shared<ShaderObject>("./base_scene_fshader.glsl", GL_FRAGMENT_SHADER);
	this->program = new ProgramObject(vSha, nullptr, fSha);
	this->program->linkShaders();

	this->generateGrid(_x,_y,_z);
	this->queryImage();

	this->program->use();
#else
	this->context = _context;
	QSurface* s = this->context->surface();
	if (s == nullptr) {
		std::cerr << __PRETTY_FUNCTION__ << " : The surface was nullptr !" << '\n';
	}
	this->context->makeCurrent(s);
	this->loader = new bulk_texture_loader();

	///////////////////////////
	/// CREATE VAO :
	///////////////////////////
	glGenVertexArrays(1, &this->vaoHandle);
	glBindVertexArray(this->vaoHandle);

	this->generateGrid(_x,_y,_z);
	this->queryImage();

	///////////////////////////
	/// CREATE SHADERS AND COMPILE :
	///////////////////////////
	this->vShaHandle = glCreateShader(GL_VERTEX_SHADER);
	this->fShaHandle = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vShaPath = "./base_scene_vshader.glsl";
	std::string fShaPath = "./base_scene_fshader.glsl";

	// Open shader file for reading :
	std::ifstream vShaFile = std::ifstream(vShaPath, std::ios_base::in);
	std::ifstream fShaFile = std::ifstream(fShaPath, std::ios_base::in);

	if (!vShaFile.is_open()) {
		std::cerr << "Error : could not get the contents of shader file " << vShaPath << '\n';
		exit(EXIT_FAILURE);
	}
	if (!fShaFile.is_open()) {
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
			std::cerr << shaderInfoLog << '\n';
			delete[] shaderInfoLog;

			std::cerr << __PRETTY_FUNCTION__ << " : end Log ***********************************************" << '\n';
		} else {
			std::cerr << "No more info about shader " << vShaPath << '\n';
		}
	}

	glCompileShader(this->fShaHandle);

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
	glAttachShader(this->programHandle, this->vShaHandle);
	glAttachShader(this->programHandle, this->fShaHandle);

	glLinkProgram(this->programHandle);

	{
		GLint Result = 0;
		int InfoLogLength = 0;
		glGetProgramiv(this->programHandle, GL_LINK_STATUS, &Result);
		glGetProgramiv(this->programHandle, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if ( InfoLogLength > 0 ){
			std::vector<char> ProgramErrorMessage(InfoLogLength+1);
			glGetProgramInfoLog(this->programHandle, InfoLogLength, NULL, &ProgramErrorMessage[0]);
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

	glDeleteShader(this->vShaHandle);
	glDeleteShader(this->fShaHandle);

	glUseProgram(this->programHandle);
#endif
}

void Scene::loadImage(std::size_t i, std::size_t j, std::size_t k, const unsigned char *pData) {
	if (pData == nullptr) { this->loadEmptyImage(); }

	this->gridWidth = i;
	this->gridHeight= j;
	this->gridDepth = k;

	if (this->textureHandle == 0) {
		glGenTextures(1, &this->textureHandle);
	}
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
}

void Scene::queryImage(void) {
	const unsigned char* image = this->loader->load_stack_from_folder();
	std::size_t i = this->loader->get_image_width();
	std::size_t j = this->loader->get_image_height();
	std::size_t k = this->loader->get_image_depth();

	this->loadImage(i, j, k, image);
}

void Scene::reloadShaders(void) const {
#ifndef RAW_GL
	if (this->program != nullptr) {
		this->program->reloadShaders();
	}
#endif
}

void Scene::drawRealSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) const {
	/*QSurface* s = this->context->surface();
	if (s == nullptr) {
		std::cerr << __PRETTY_FUNCTION__ << " : The surface was nullptr !" << '\n';
	}
	this->context->makeCurrent(s);*/
#ifndef RAW_GL
	if (this->program == nullptr || this->vao == nullptr) { std::cerr << "Nothing here " << std::endl; return; }

	this->program->use();
	this->vao->bind();
	this->vao->enableVertexAttributes();
#else
	glUseProgram(this->programHandle);
	glBindVertexArray(this->vaoHandle);
#endif
//	if (bDrawWireframe) {
		// Set the mode to wireframe :
	//	glLineWidth(10.0);
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	}

	glm::mat4 transfoMat = glm::mat4();
	if (this->drawRealVoxelSize) {
		transfoMat *= glm::scale(glm::vec3(0.39, 0.39, 1.927));
	}
	glm::vec3 posAbsolute(static_cast<float>(this->gridWidth) * this->positionNormalized.x,
			      static_cast<float>(this->gridHeight)* this->positionNormalized.y,
			      static_cast<float>(this->gridDepth) * this->positionNormalized.z);
	transfoMat *= glm::translate(posAbsolute);

	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
#ifndef RAW_GL
	this->program->setUniform1ui("texData", 0); // set texture sampler in texture slot 0

	this->program->setUniformMatrix4fv("mMatrix", 1, this->transposeMatrices, glm::value_ptr(transfoMat[0]));
	this->program->setUniformMatrix4fv("vMatrix", 1, this->transposeMatrices, &mvMat[0]);
	this->program->setUniformMatrix4fv("pMatrix", 1, this->transposeMatrices, &pMat[0]);
	this->program->setUniform3fv("texOffset", 1, &this->positionNormalized[0]);

	glPointSize(10.0);
	this->vao->pVBOElementBuffer->bind();
	glDrawElements(GL_POINTS, this->vao->pVBOElementBuffer->getValueCount(), GL_UNSIGNED_INT, 0);
	this->vao->pVBOElementBuffer->unBind();

	this->vao->disableVertexAttributes();
	this->vao->unBind();
	this->program->release();
#else
	//////////////////
	/// SET UNIFORMS :
	//////////////////
	glFrontFace(GL_CW);

	GLuint texDataLocation = glGetUniformLocation(this->programHandle, "texData");
	GLuint mMatrixLocation = glGetUniformLocation(this->programHandle, "mMatrix");
	GLuint vMatrixLocation = glGetUniformLocation(this->programHandle, "vMatrix");
	GLuint pMatrixLocation = glGetUniformLocation(this->programHandle, "pMatrix");
	GLuint texOffsetLocation = glGetUniformLocation(this->programHandle, "texOffset");

	glUniform1i(texDataLocation, 0);
	glUniformMatrix4fv(mMatrixLocation, 1, this->transposeMatrices, glm::value_ptr(transfoMat[0]));
	glUniformMatrix4fv(vMatrixLocation, 1, this->transposeMatrices, &mvMat[0]);
	glUniformMatrix4fv(pMatrixLocation, 1, this->transposeMatrices, &pMat[0]);
	glUniform3fv(texOffsetLocation, 1, &this->positionNormalized[0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertPosHandle);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboUVCoordHandle);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	GLint b = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
	std::cerr << "Buffer has " << b << " elements" << '\n';
	glDrawElements(GL_TRIANGLES, this->elemToDrawIdx, GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindVertexArray(0);
	glUseProgram(0);
#endif
}

void Scene::drawInitialSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) const {
#ifndef RAW_GL
	if (this->program == nullptr || this->vao == nullptr) { std::cerr << "Nothing here " << std::endl; return; }

	this->program->use();
	this->vao->bind();
	this->vao->enableVertexAttributes();
#else
	glUseProgram(this->programHandle);
	glBindVertexArray(this->vaoHandle);
#endif

//	if (bDrawWireframe) {
		// Set the mode to wireframe :
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	}

	glm::mat4 transfoMat = glm::mat4(1.0);

	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
#ifndef RAW_GL
	this->program->setUniform1ui("texData", 0); // set texture sampler in texture slot 0

	this->program->setUniformMatrix4fv("mMatrix", 1, this->transposeMatrices, glm::value_ptr(transfoMat[0]));
	this->program->setUniformMatrix4fv("vMatrix", 1, this->transposeMatrices, &mvMat[0]);
	this->program->setUniformMatrix4fv("pMatrix", 1, this->transposeMatrices, &pMat[0]);
	this->program->setUniform3fv("texOffset", 1, &this->positionNormalized[0]);

	glPointSize(10.0);
	this->vao->pVBOElementBuffer->bind();
	glDrawElements(GL_POINTS, this->vao->pVBOElementBuffer->getValueCount(), GL_UNSIGNED_INT, 0);
	this->vao->pVBOElementBuffer->unBind();

	this->vao->disableVertexAttributes();
	this->vao->unBind();
	this->program->release();
#else
	//////////////////
	/// SET UNIFORMS :
	//////////////////

	GLuint texDataLocation = glGetUniformLocation(this->programHandle, "texData");
	GLuint mMatrixLocation = glGetUniformLocation(this->programHandle, "mMatrix");
	GLuint vMatrixLocation = glGetUniformLocation(this->programHandle, "vMatrix");
	GLuint pMatrixLocation = glGetUniformLocation(this->programHandle, "pMatrix");
	GLuint texOffsetLocation = glGetUniformLocation(this->programHandle, "texOffset");

	glUniform1i(texDataLocation, 0);
	glUniformMatrix4fv(mMatrixLocation, 1, this->transposeMatrices, glm::value_ptr(transfoMat[0]));
	glUniformMatrix4fv(vMatrixLocation, 1, this->transposeMatrices, &mvMat[0]);
	glUniformMatrix4fv(pMatrixLocation, 1, this->transposeMatrices, &pMat[0]);
	glUniform3fv(texOffsetLocation, 1, &this->positionNormalized[0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertPosHandle);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboUVCoordHandle);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

	glDrawArrays(GL_POINTS, 0, this->elemToDrawSeq);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindVertexArray(0);
	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

void Scene::generateGrid(std::size_t _x, std::size_t _y, std::size_t _z) {
	std::vector<float> vertPos;
	std::vector<float> vertTex;
	std::vector<GLuint> vertIdx;
	this->neighborWidth = _x;
	this->neighborHeight =_y;
	this->neighborDepth = _z;

	// Create the grid, in raw form :
	for (std::size_t i = 0; i <= _z; ++i) {
		for (std::size_t j = 0; j <= _y; ++j) {
			for (std::size_t k = 0; k <= _x; ++k) {
				vertPos.emplace_back(static_cast<float>(k));
				vertPos.emplace_back(static_cast<float>(j));
				vertPos.emplace_back(static_cast<float>(i));
				vertPos.emplace_back(1.f);
				float xtex = static_cast<float>(k) / static_cast<float>(_x);
				float ytex = static_cast<float>(j) / static_cast<float>(_y);
				float ztex = static_cast<float>(i) / static_cast<float>(_z);
				vertTex.emplace_back(xtex);
				vertTex.emplace_back(ytex);
				vertTex.emplace_back(ztex);
			}
		}
	}

	// Create the index buffer used to draw it in order :
	// (see notes for reference about draw order)
	for (std::size_t i = 0; i < _z; ++i) {
		for (std::size_t j = 0; j < _y; ++j) {
			for (std::size_t k = 0; k < _x; ++k) {
				/**
				 * Name references :
				 *     a-------b-->+k
				 *    /|      /|
				 *   c-------d |
				 *  /| |     | |
				 *+j | |e----|-|f
				 *   |/|     |/
				 *   g-------h
				 *     V
				 *    +i
				 */
				GLuint a = static_cast<GLuint>((k+0) + (j+0)*_x + (i+0)*_x*_y);
				GLuint b = static_cast<GLuint>((k+1) + (j+0)*_x + (i+0)*_x*_y);
				GLuint c = static_cast<GLuint>((k+0) + (j+1)*_x + (i+0)*_x*_y);
				GLuint d = static_cast<GLuint>((k+1) + (j+1)*_x + (i+0)*_x*_y);
				GLuint e = static_cast<GLuint>((k+0) + (j+0)*_x + (i+1)*_x*_y);
				GLuint f = static_cast<GLuint>((k+1) + (j+0)*_x + (i+1)*_x*_y);
				GLuint g = static_cast<GLuint>((k+0) + (j+1)*_x + (i+1)*_x*_y);
				GLuint h = static_cast<GLuint>((k+1) + (j+1)*_x + (i+1)*_x*_y);

				// Draw the half of the cube atteinable from the position a :
				// Faces :	   |    1   |    2   |    3   |    4   |    5   |   6   |
				GLuint faceIdx1[]= {a, d, c, a, b, c, e, b, a, e, f, b, g, e, a, g, a, c};
				// Draw the other side :
				// Faces :	   |    1   |    2   |    3   |    4   |    5   |   6   |
				GLuint faceIdx2[]= {h, e, g, h, f, e, h, g, c, h, c, d, h, d, b, h, d, f};

				// Insert them into the vector :
				vertIdx.insert(vertIdx.end(), faceIdx1, faceIdx1+18);
				vertIdx.insert(vertIdx.end(), faceIdx2, faceIdx2+18);
			}
		}
	}

	for (std::size_t i = 0; i < vertPos.size()/4; ++i) {
		std::cerr << "Vertex : (" << vertPos[4*i+0] << ',' << vertPos[4*i+1] << ',' << vertPos[4*i+2] << ',' << vertPos[4*i+3] << ") , (" << vertTex[3*i+0] << ',' << vertTex[3*i+1] << ',' << vertTex[3*i+2] << ")\n";
	}

#ifndef RAW_GL
	// Upload vertex positions :
	this->vao->setVBOData<VBOTypes::Vertex>(vertPos.size(), 4, vertPos.size()*sizeof(glm::vec4), vertPos.data(), GL_STATIC_DRAW, GL_FLOAT);
	// Upload texture coordinates :
	this->vao->setVBOData<VBOTypes::TexCoords>(vertTex.size(), 3, vertTex.size()*sizeof(glm::vec3), vertTex.data(), GL_STATIC_DRAW, GL_FLOAT);
	// Upload the index buffer :
	this->vao->setVBOData<VBOTypes::IndexBuffer>(vertIdx.size(), 1, vertIdx.size()*sizeof(GLuint), vertIdx.data(), GL_STATIC_DRAW, GL_UNSIGNED_INT);
#else
	///////////////////////////////////////////////
	/// CREATE VBO AND UPLOAD DATA
	///////////////////////////////////////////////
	glGenBuffers(1, &this->vboVertPosHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertPosHandle);
	glBufferData(GL_ARRAY_BUFFER, vertPos.size() * sizeof(float), vertPos.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &this->vboUVCoordHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboUVCoordHandle);
	glBufferData(GL_ARRAY_BUFFER, vertTex.size() * sizeof(float), vertTex.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &this->vboElementHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertIdx.size() * sizeof(GLuint), vertIdx.data(), GL_STATIC_DRAW);

	this->elemToDrawSeq = vertPos.size()/4;
	this->elemToDrawIdx = vertIdx.size();
#endif
	std::cerr << "Normally, inserted " << vertIdx.size() << " elements into element buffer. " << '\n';
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
