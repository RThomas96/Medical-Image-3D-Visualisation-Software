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
	this->controlPanel = nullptr;
	this->texStorage = nullptr;
	this->mesh = nullptr;

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
	this->vShaHandle = 0;
	this->fShaHandle = 0;
	this->programHandle = 0;

	this->neighborOffset = glm::vec3(0, 0, 0);
	this->neighborPos = uvec3(0, 0, 0);

	// Uniform locations :
	this->mMatrixLocation = -1;
	this->vMatrixLocation = -1;
	this->pMatrixLocation = -1;
	this->lightPosLocation = -1;
	this->neighborOffsetLocation = -1;
	this->scaledCubes = 1;
}

Scene::~Scene(void) {
	glDeleteTextures(1, &this->textureHandle);
}

void Scene::initGl(QOpenGLContext* _context, std::size_t _x, std::size_t _y, std::size_t _z) {
	if (this->isInitialized == true) { return; }
	this->isInitialized = true;
	std::cerr << "Entering scene initialization ! " << '\n';

	this->context = _context;

	if (this->context == 0) { std::cerr << "Warning ! this->context() returned 0 !" << '\n' ; }
	if (this->context == nullptr) { std::cerr << "Warning ! Initializing a scene without a valid OpenGL context !" << '\n' ; }

	this->initializeOpenGLFunctions();

	this->texStorage = new TextureStorage();
	this->texStorage->enableDownsampling(true);

	this->mesh = new TetMesh(this->texStorage);
	this->mesh->setTransformationMatrix(this->computeTransformationMatrix());

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
	this->neighborOffsetLocation = glGetUniformLocation(this->programHandle, "neighborOffset");
	if (this->neighborOffsetLocation < 0) {
		std::cerr << "Cannot find " << "neighborOffset" << " uniform" << '\n';
	}
	GetOpenGLError();

	if (this->showTextureCube == false && this->cubeShown == true) {
		this->hideTexCubeVBO();
	}
	if (this->showTextureCube == true && this->cubeShown == false) {
		this->showTexCubeVBO();
	}
}

void Scene::compileShaders() {
	glUseProgram(0);
	glDeleteShader(this->vShaHandle);
	glDeleteShader(this->gShaHandle);
	glDeleteShader(this->fShaHandle);
	glDeleteProgram(this->programHandle);
	GetOpenGLError();

	///////////////////////////
	/// CREATE SHADERS AND COMPILE :
	///////////////////////////
	this->vShaHandle = glCreateShader(GL_VERTEX_SHADER);
	GetOpenGLError();
	this->gShaHandle = glCreateShader(GL_GEOMETRY_SHADER);
	GetOpenGLError();
	this->fShaHandle = glCreateShader(GL_FRAGMENT_SHADER);
	GetOpenGLError();

	// shaderPaths shader paths
	std::string vShaPath = "./shaders/base.vert";
	std::string gShaPath = "./shaders/base.geom";
	std::string fShaPath = "./shaders/base.frag";

	// Open shader file for reading :
	std::ifstream vShaFile = std::ifstream(vShaPath, std::ios_base::in);
	std::ifstream gShaFile = std::ifstream(gShaPath, std::ios_base::in);
	std::ifstream fShaFile = std::ifstream(fShaPath, std::ios_base::in);

	if (!vShaFile.is_open()) {
		std::cerr << "Error : could not get the contents of shader file " << vShaPath << '\n';
		exit(EXIT_FAILURE);
	}
	if (!gShaFile.is_open()) {
		vShaFile.close();
		std::cerr << "Error : could not get the contents of shader file " << gShaPath << '\n';
		exit(EXIT_FAILURE);
	}
	if (!fShaFile.is_open()) {
		vShaFile.close();
		gShaFile.close();
		std::cerr << "Error : could not get the contents of shader file " << fShaPath << '\n';
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

	glShaderSource(this->vShaHandle, 1, const_cast<const char**>(&vShaSource), 0);
	glShaderSource(this->gShaHandle, 1, const_cast<const char**>(&gShaSource), 0);
	glShaderSource(this->fShaHandle, 1, const_cast<const char**>(&fShaSource), 0);

	delete[] vShaSource;
	delete[] gShaSource;
	delete[] fShaSource;

	vShaFile.close();
	gShaFile.close();
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

	glCompileShader(this->gShaHandle);
	GetOpenGLError();

	{
		GLint shaderInfoLength = 0;
		GLint charsWritten = 0;
		char* shaderInfoLog = nullptr;

		glGetShaderiv(this->gShaHandle, GL_INFO_LOG_LENGTH, &shaderInfoLength);
		if (shaderInfoLength > 1) {
			std::cerr << __PRETTY_FUNCTION__ << " : start Log ***********************************************" << '\n';

			std::cerr << __FUNCTION__ << " : Information about shader " << gShaPath << " : " << '\n';
			std::cerr << __FUNCTION__ << " : Shader was a geometry shader ";
			shaderInfoLog = new char[shaderInfoLength];
			glGetShaderInfoLog(this->gShaHandle, shaderInfoLength, &charsWritten, shaderInfoLog);
			GetOpenGLError();
			std::cerr << shaderInfoLog << '\n';
			delete[] shaderInfoLog;

			std::cerr << __PRETTY_FUNCTION__ << " : end Log ***********************************************" << '\n';
		} else {
			std::cerr << "No more info about shader " << gShaPath << '\n';
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
	glAttachShader(this->programHandle, this->gShaHandle);
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
	glDetachShader(this->programHandle, this->gShaHandle);
	glDetachShader(this->programHandle, this->fShaHandle);
	GetOpenGLError();

	glDeleteShader(this->vShaHandle);
	glDeleteShader(this->gShaHandle);
	glDeleteShader(this->fShaHandle);

}

void Scene::loadImage(std::size_t i, std::size_t j, std::size_t k, const unsigned char *pData) {
	glEnable(GL_TEXTURE_3D);

	this->gridWidth = i;
	this->gridHeight= j;
	this->gridDepth = k;

	if (pData == nullptr) { pData = this->loadEmptyImage(); }

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
	//glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, static_cast<GLfloat>(-1000.f));
	// Stop once UV > 1 or < 0
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
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
		GL_RED_INTEGER,			// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,	// GLenum : Type (the data type as in uchar, uint, float ...)
		pData			// void*  : Data to load into the buffer
	);
	GetOpenGLError();
}

void Scene::queryImage(void) {
	this->texStorage->loadImages();
	std::vector<unsigned char> image = this->texStorage->getData();
	std::vector<std::size_t> imageSizes = this->texStorage->getImageSize();
	std::size_t i = imageSizes[0];
	std::size_t j = imageSizes[1];
	std::size_t k = imageSizes[2];

	std::cerr << "Loading image of size " << i << ',' << j << ',' << k << '\n';

	this->loadImage(i, j, k, image.data());
}

void Scene::prepUniforms(glm::mat4 transfoMat, GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
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

	GLint scaledLoc = glGetUniformLocation(this->programHandle, "scaledCubes");
	if (scaledLoc < 0) {
		std::cerr << "Cannot find scaled cubes !" << '\n';
	} else {
		glUniform1ui(scaledLoc, this->scaledCubes);
	}
	glUniform1i(this->texDataLocation, 0);
	GetOpenGLError();
	glUniformMatrix4fv(this->mMatrixLocation, 1, GL_FALSE, glm::value_ptr(transfoMat));
	glUniformMatrix4fv(this->vMatrixLocation, 1, GL_FALSE, &mvMat[0]);
	glUniformMatrix4fv(this->pMatrixLocation, 1, GL_FALSE, &pMat[0]);
	glUniform4fv(this->lightPosLocation, 1, glm::value_ptr(lightPos));
}

void Scene::drawRealSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_3D);
	GetOpenGLError();


	glm::vec4 n = glm::vec4(this->neighborOffset.x, this->neighborOffset.y, this->neighborOffset.z, 1.);
	std::cerr << "Queried point ! " << n.x << ',' << n.y << ',' << n.z << '\n';
	glm::vec4 o = glm::inverse(this->computeTransformationMatrix()) * n;
	std::cerr << "Inverse point ! " << o.x << ',' << o.y << ',' << o.z << ',' << o.w << '\n';
	this->neighborPos = this->texStorage->getVoxelIndexFromPosition(o);

	bDrawWireframe = true;

	if (this->context == nullptr) { std::cerr << "Warning ! Drawing in real space without a valid OpenGL context !" << '\n' ; }

	glm::mat4 transfoMat = this->computeTransformationMatrix();

	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	//////////////////
	/// SET UNIFORMS :
	//////////////////

	glUseProgram(this->programHandle);

	this->prepUniforms(transfoMat, mvMat, pMat, lightPos);

	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	this->setupVAOPointers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);

	/*if (this->drawMode == DrawMode::Wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}*/

	if (this->showTextureCube == true) {
		if (this->cubeShown == false) { this->showTexCubeVBO(); }
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(this->drawCalls));
	} else {
		if (this->cubeShown == true) { this->hideTexCubeVBO(); }
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(this->drawCalls));
	}

	glBindVertexArray(0);
	glUseProgram(0);

	std::vector<glm::vec4> vertices = this->mesh->getVertices();
	std::vector<unsigned char> values = this->mesh->getVertexValues();

	glPointSize(5.0);
	glBegin(GL_POINTS);
	for (std::size_t i = 0; i < vertices.size(); ++i) {
		glm::vec3 rgb = ucharToRGB(values[i], 5, 255, 50, 200);
		glColor3f(rgb.r, rgb.g, rgb.b);
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	}
	glEnd();
}

void Scene::drawInitialSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_3D);
	GetOpenGLError();

	glm::vec4 n = glm::vec4(this->neighborOffset.x, this->neighborOffset.y, this->neighborOffset.z, 1.);
	std::cerr << "Queried point ! " << n.x << ',' << n.y << ',' << n.z << '\n';
	glm::vec4 o = glm::inverse(this->computeTransformationMatrix()) * n;
	std::cerr << "Inverse point ! " << o.x << ',' << o.y << ',' << o.z << ',' << o.w << '\n';
	this->neighborPos = this->texStorage->getVoxelIndexFromPosition(o);

	bDrawWireframe = true;

	if (this->context == nullptr) { std::cerr << "Warning ! Drawing in initial space without a valid OpenGL context !" << '\n' ; }

	glm::mat4 transfoMat = glm::mat4(1.);

	glm::vec4 lightPos = glm::vec4(-0.25, -0.25, -0.25, 1.0);

	//////////////////
	/// SET UNIFORMS :
	//////////////////

	glUseProgram(this->programHandle);
	GetOpenGLError();

	this->prepUniforms(transfoMat, mvMat, pMat, lightPos);
	GetOpenGLError();

	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	this->setupVAOPointers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);

	/*if (this->drawMode == DrawMode::Wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}*/

	if (this->showTextureCube) {
		if (this->cubeShown == false) { this->showTexCubeVBO(); }
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(this->drawCalls));
	} else {
		if (this->cubeShown) { this->hideTexCubeVBO(); }
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(this->renderSize), GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(this->drawCalls));
	}
	GetOpenGLError();

	glBindVertexArray(0);
	glUseProgram(0);

	std::vector<glm::vec4> vertices = this->mesh->getVertices();
	std::vector<unsigned char> values = this->mesh->getVertexValues();
	glm::mat4 imat = glm::inverse(this->computeTransformationMatrix());

	glPointSize(5.0);
	glBegin(GL_POINTS);
	for (std::size_t i = 0; i < vertices.size(); ++i) {
		glm::vec3 rgb = ucharToRGB(values[i], 5, 255, 50, 200);
		glColor3f(rgb.x, rgb.y, rgb.z);
		glm::vec4 v = imat * vertices[i];
		glVertex3f(v.x, v.y, v.z);
		glColor3f(rgb.x, rgb.y, rgb.z);
	}
	glEnd();
}

void Scene::generateGrid(std::size_t _x, std::size_t _y, std::size_t _z) {
	this->vertPos.clear();
	this->vertNorm.clear();
	this->vertIdx.clear();
	this->vertIdxDraw.clear();

	this->generateNeighborGrid(_x,_y,_z);
	this->generateTexCube();

	this->neighborOffset = glm::vec3(0, 0, 0);
	this->controlPanel->setXCoord(0);
	this->controlPanel->setYCoord(0);
	this->controlPanel->setZCoord(0);

	this->setupVBOData();
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

/*	this->vboVertPos->setData(this->vertPos.size(), 4, this->vertPos.size()*sizeof(float), this->vertPos.data(), GL_DYNAMIC_DRAW, GL_FLOAT);
	this->vboVertNorm->setData(this->vertNorm.size(), 4, this->vertNorm.size()*sizeof(float), this->vertNorm.data(), GL_DYNAMIC_DRAW, GL_FLOAT);
	this->vboVertElement->setData(this->vertIdx.size(), 1, this->vertIdx.size()*sizeof(unsigned int), this->vertIdx.data(), GL_DYNAMIC_DRAW, GL_UNSIGNED_INT);
	this->vboIndexedDraw->setData(this->vertIdxDraw.size(), 4, this->vertIdxDraw.size()*sizeof(unsigned int), this->vertIdxDraw.data(), GL_DYNAMIC_DRAW, GL_UNSIGNED_INT);
*/
	this->setupVAOPointers();

	std::cerr << "Normally, inserted " << this->vertIdx.size() << " elements into element buffer. " << '\n';

	this->renderSize = this->vertIdx.size();
	this->drawCalls	= this->vertIdxDraw.size();
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
	glVertexAttribDivisor(2, 1);
}

void Scene::generateNeighborGrid(std::size_t _x, std::size_t _y, std::size_t _z) {
	this->neighborWidth = _x;
	this->neighborHeight =_y;
	this->neighborDepth = _z;

	// Original texture cube :
	this->vertIdxDraw.emplace_back(uint(1), uint(1), uint(1), uint(0));

	// Queriable cube :
	//this->vertIdxDraw.emplace_back(uint(1), uint(1), uint(1), uint(0));

	// Create the grid, in raw form :
	for (std::size_t i = 0; i < _z; ++i) {
		for (std::size_t j = 0; j < _y; ++j) {
			for (std::size_t k = 0; k < _x; ++k) {
				this->vertIdxDraw.emplace_back(k, j, i, uint(1));
			}
		}
	}
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
		return baseVtx;
	} else {
		glm::mat3 transfoMat = glm::mat3(this->computeTransformationMatrix());
		return transfoMat * baseVtx;
	}
}

nbCoord Scene::getNeighborBoundaries(bool realSpace) const {
	glm::vec3 neighbor(static_cast<float>(this->neighborWidth), static_cast<float>(this->neighborHeight), static_cast<float>(this->neighborDepth));
	glm::vec3 pos(neighborOffset);

	glm::mat3 transfoMat = glm::mat3(this->computeTransformationMatrix());
	glm::vec3 max = neighbor+pos;
	if (not realSpace) {
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
//	this->vboIndexedDraw->bind();
//	this->vboIndexedDraw->updateData(static_cast<GLintptr>(0), 3u*sizeof(uint), newData);
//	this->vboIndexedDraw->unBind();
}

void Scene::showTexCubeVBO() {
	this->cubeShown = true;
	glBindBuffer(GL_ARRAY_BUFFER, this->vboIndexedDrawHandle);
	GLuint newData[] = {static_cast<GLuint>(this->gridWidth), static_cast<GLuint>(this->gridHeight), static_cast<GLuint>(this->gridDepth)};
	glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(0), 3u*sizeof(unsigned int), newData);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	this->vboIndexedDraw->bind();
//	this->vboIndexedDraw->updateData(static_cast<GLintptr>(0), 3u*sizeof(uint), newData);
//	this->vboIndexedDraw->unBind();
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

void Scene::updateNeighborTetMesh() {
	glm::vec4 n = glm::vec4(this->neighborOffset.x, this->neighborOffset.y, this->neighborOffset.z, 1.);
	std::cerr << "Queried point ! " << n.x << ',' << n.y << ',' << n.z << '\n';
	glm::vec4 o = glm::inverse(this->computeTransformationMatrix()) * n;
	std::cerr << "Inverse point ! " << o.x << ',' << o.y << ',' << o.z << ',' << o.w << '\n';
	this->mesh->setOrigin(n);
	this->mesh->printInfo();
}

glm::mat4 Scene::computeTransformationMatrix() const {
	glm::mat4 transfoMat = glm::mat4(1.0);

	double angleDeg = 45.;
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
}
