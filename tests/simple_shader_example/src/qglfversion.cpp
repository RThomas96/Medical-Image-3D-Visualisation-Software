#include "../include/qglfversion.hpp"

#include <fstream>
#include <string>

std::vector<GLchar> readShaderFile(const std::string pathname) {
	std::ifstream shaFile(pathname, std::ios::in);
	if (!shaFile.is_open()) {
		std::cerr << "No shader could be opened for " << pathname << '\n';
		return std::vector<GLchar>();
	}

	// Get file length :
	shaFile.seekg(0, shaFile.end);
	std::size_t shaFileSize = static_cast<std::size_t>(shaFile.tellg());
	shaFile.seekg(0, shaFile.beg);

	std::vector<GLchar> shaFileContents;
	shaFileContents.resize(shaFileSize+1);

	// Read contents :
	shaFile.read(shaFileContents.data(), shaFileSize);

	// Terminate string just in case :
	shaFileContents[shaFileSize] = '\0';
	// Close fstream :
	shaFile.close();

	return shaFileContents;
}

void __GetOpenGLError ( char* szFile, char* szFunction, int iLine ) {
	int retCode = 0;
	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		std::cerr << "OpenGL error in file " << szFile << ", in function " << szFunction << " @ line " << iLine << " : ";
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
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				std::cerr << "was an invalid framebuffer operation";
			break;
			case GL_STACK_OVERFLOW:
				std::cerr << "was a stack overflox error";
			break;
			case GL_STACK_UNDERFLOW:
				std::cerr << "was a stack overflox error";
			break;
			case GL_OUT_OF_MEMORY:
				std::cerr << "was an Out Of Memory error";
			break;
			default:
				std::cerr << "was another error : " << std::hex << glErr ;
			break;
		}
		std::cerr << '\n';
		glErr = glGetError();
		retCode = 1;
	}
	if (retCode) {
		//exit(EXIT_FAILURE);
	}
}

GLWidget::GLWidget(QWidget* parent) : QGLViewer(parent), QOpenGLFunctions_4_5_Core() {
	this->programHandle = 0;
	this->vShaHandle = 0;
	this->gShaHandle = 0;
	this->fShaHandle = 0;
	this->vaoHandle = 0;
	this->vboVertexPosHandle = 0;
	this->vboVertexNormHandle = 0;
	this->vboElementHandle = 0;

	this->frontFaceOrder = GL_CCW;
	this->transposeMatrices = GL_FALSE;

	this->vShaPath = "./shaders/base.vert";
	this->fShaPath = "./shaders/base.frag";

	this->format().setMajorVersion(4);
	this->format().setMinorVersion(5);
	this->format().setProfile(QSurfaceFormat::CoreProfile);
}

GLWidget::~GLWidget() {
	this->cleanupResources();
}

void GLWidget::init() {
	// Initialize the context and OpenGL functions :
	this->makeCurrent();
	this->initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GetGLError();

	// Generate a VAO object :
	glGenVertexArrays(1, &this->vaoHandle);
	glBindVertexArray(this->vaoHandle);

	GetGLError();

	// Generate cube vertices, normals, and drawing order :
	std::vector<glm::vec4> vPos;
	std::vector<glm::vec4> vNorms;
	std::vector<GLuint> idx;
	this->generateCube(vPos, vNorms, idx);

	std::cerr << "vPos :" << vPos.size() << '\n';
	std::cerr << "vNorms :" << vNorms.size() << '\n';
	std::cerr << "idx :" << idx.size() << '\n';

	// Generate and allocate video-side vertex positions buffer :
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &this->vboVertexPosHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertexPosHandle);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec4), vPos.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);


	// Generate and allocate video-side vertex normals buffer :
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &this->vboVertexNormHandle);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertexNormHandle);
	glBufferData(GL_ARRAY_BUFFER, vNorms.size() * sizeof(glm::vec4), vNorms.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

	GetGLError();

	// Generate and allocate video-side element buffer :
	glGenBuffers(1, &this->vboElementHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

	GetGLError();

	this->compileShaders(this->vShaPath, this->fShaPath);
}

void GLWidget::draw() {
	//this->makeCurrent();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glFrontFace(this->frontFaceOrder);

	glUseProgram(this->programHandle);

	// Setup view and projection matrices :
	GLfloat* vMat = new GLfloat[16];
	GLfloat* pMat = new GLfloat[16];
	this->camera()->getModelViewMatrix(vMat);
	this->camera()->getProjectionMatrix(pMat);
	glUniformMatrix4fv(glGetUniformLocation(this->programHandle, "vMatrix"), 1, this->transposeMatrices, vMat);
	glUniformMatrix4fv(glGetUniformLocation(this->programHandle, "pMatrix"), 1, this->transposeMatrices, pMat);
	GetGLError();

	glBindVertexArray(this->vaoHandle);
	this->setVAODataPointers();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	GetGLError();

	glDrawElements(GL_TRIANGLES, GLsizei(36), GL_UNSIGNED_INT, nullptr);
	GetGLError();

	// Break the binding of everything :
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	GetGLError();
}

void GLWidget::keyPressEvent(QKeyEvent *e) {
	switch (e->key()) {
		case Qt::Key::Key_R:
			this->compileShaders(this->vShaPath, this->fShaPath);
			this->update();
		break;
		case Qt::Key::Key_F:
			this->frontFaceOrder = (this->frontFaceOrder == GL_CCW) ? GL_CW : GL_CCW;
			this->update();
		break;
		case Qt::Key::Key_T:
			this->transposeMatrices = (this->transposeMatrices == GL_FALSE) ? GL_TRUE : GL_FALSE;
			this->update();
		break;
		default: QGLViewer::keyPressEvent(e); break;
	}
}

void GLWidget::compileShaders(const std::string _vShaPath, const std::string _fShaPath) {
	glUseProgram(0);
	glDeleteProgram(this->programHandle);

	// Create new handles :
	this->vShaHandle = glCreateShader(GL_VERTEX_SHADER);
	this->fShaHandle = glCreateShader(GL_FRAGMENT_SHADER);

	GetGLError();

	// Read files :
	std::vector<GLchar> vShaSource = readShaderFile(_vShaPath);
	std::vector<GLchar> fShaSource = readShaderFile(_fShaPath);
	GLint vShaLengthi = static_cast<GLint>(vShaSource.size());
	GLint fShaLengthi = static_cast<GLint>(fShaSource.size());
	const GLchar* vShaContents = vShaSource.data();
	const GLchar* fShaContents = fShaSource.data();

	// Source the contents to OpenGL :
	glShaderSource(this->vShaHandle, 1, &vShaContents, &vShaLengthi);
	GetGLError();
	glShaderSource(this->fShaHandle, 1, &fShaContents, &fShaLengthi);
	GetGLError();

	// Compile vShader :
	glCompileShader(this->vShaHandle);
	this->readShaderState(GL_VERTEX_SHADER, _vShaPath.c_str(), this->vShaHandle);

	GetGLError();

	// Compile fShader :
	glCompileShader(this->fShaHandle);
	this->readShaderState(GL_FRAGMENT_SHADER, _fShaPath.c_str(), this->fShaHandle);

	GetGLError();

	// Create new program handle, and link :
	this->programHandle = glCreateProgram();
	glAttachShader(this->programHandle, this->vShaHandle);
	GetGLError();
	glAttachShader(this->programHandle, this->fShaHandle);
	GetGLError();
	glLinkProgram(this->programHandle);
	GetGLError();
	readProgramInfo(this->programHandle);

	glDetachShader(this->programHandle, this->vShaHandle);
	GetGLError();
	glDetachShader(this->programHandle, this->fShaHandle);
	GetGLError();

	// Delete shader, don't need them once compiled and linked
	glDeleteShader(this->vShaHandle);
	GetGLError();
	glDeleteShader(this->fShaHandle);
	GetGLError();

	this->vShaHandle = 0;
	this->fShaHandle = 0;
}

void GLWidget::setVAODataPointers() {
	glEnableVertexAttribArray(0);
	GetGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertexPosHandle);
	GetGLError();
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

	GetGLError();

	glEnableVertexAttribArray(1);
	GetGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertexNormHandle);
	GetGLError();
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

	GetGLError();
}

void GLWidget::readProgramInfo(const GLuint proHandle) {
	GLint Result = 0;
	int InfoLogLength = 0;
	glGetProgramiv(proHandle, GL_LINK_STATUS, &Result);
	GetGLError();
	glGetProgramiv(proHandle, GL_INFO_LOG_LENGTH, &InfoLogLength);
	GetGLError();
	if ( InfoLogLength > 0 ){
		char* ProgramErrorMessage = new char[InfoLogLength+1];
		glGetProgramInfoLog(proHandle, InfoLogLength, NULL, ProgramErrorMessage);
		GetGLError();
		std::cerr << __FUNCTION__ << " : Warning : errors while linking program :" << '\n';
		std::cerr << "------------------------------------------------------------------" << '\n';
		std::cerr << "------------------------------------------------------------------" << '\n';
		std::cerr << ProgramErrorMessage << '\n';
		std::cerr << "------------------------------------------------------------------" << '\n';
		std::cerr << "------------------------------------------------------------------" << '\n';
		delete[] ProgramErrorMessage;
	} else {
		std::cerr << "Linking of program happened just fine." << '\n';
	}
}

void GLWidget::readShaderState(GLenum shaType, const std::string pathName, GLuint shaHandle) {
	GLint shaderInfoLength = 0;
	GLint charsWritten = 0;
	char* shaderInfoLog = nullptr;

	glGetShaderiv(shaHandle, GL_INFO_LOG_LENGTH, &shaderInfoLength);
	GetGLError();
	if (shaderInfoLength > 1) {
		std::cerr << __PRETTY_FUNCTION__ << " : start Log ***********************************************" << '\n';

		std::cerr << __FUNCTION__ << " : Information about shader " << pathName << " : " << '\n';
		std::cerr << __FUNCTION__ << " : Shader was a " << ((shaType == GL_VERTEX_SHADER) ? "vertex" : "fragment") << " shader\n";
		shaderInfoLog = new char[shaderInfoLength];
		glGetShaderInfoLog(shaHandle, shaderInfoLength, &charsWritten, shaderInfoLog);
		GetGLError();
		std::cerr << shaderInfoLog << '\n';
		delete[] shaderInfoLog;

		std::cerr << __PRETTY_FUNCTION__ << " : end Log ***********************************************" << '\n';
	} else {
		std::cerr << "No more info about shader " << pathName << '\n';
	}
}

void GLWidget::generateCube(std::vector<glm::vec4> &v, std::vector<glm::vec4> &n, std::vector<GLuint> &i) const {
	// Generate positions of a fixed cube. The cube will
	// have a sidelength of 1, and have only 8 vertices
	// indexed by an index buffer somewhere else.
	v.clear();
	n.clear();
	v.resize(8);
	n.resize(8);
	i.clear();

	GLuint a = 0; GLuint b = 1;
	GLuint c = 2; GLuint d = 3;
	GLuint e = 4; GLuint f = 5;
	GLuint g = 6; GLuint h = 7;

	v[a] = glm::vec4(.0f, .0f, 1.f, 1.f); // Vertex A
	v[b] = glm::vec4(1.f, .0f, 1.f, 1.f); // Vertex B
	v[c] = glm::vec4(1.f, 1.f, 1.f, 1.f); // Vertex C
	v[d] = glm::vec4(.0f, 1.f, 1.f, 1.f); // Vertex D
	v[e] = glm::vec4(.0f, .0f, .0f, 1.f); // Vertex E
	v[f] = glm::vec4(1.f, .0f, .0f, 1.f); // Vertex F
	v[g] = glm::vec4(1.f, 1.f, .0f, 1.f); // Vertex G
	v[h] = glm::vec4(.0f, 1.f, .0f, 1.f); // Vertex H

	n[a] = glm::normalize(glm::vec4(-1.f, -1.f,  1.f, 0.f)); // Normal of A
	n[b] = glm::normalize(glm::vec4( 1.f, -1.f,  1.f, 0.f)); // Normal of B
	n[c] = glm::normalize(glm::vec4( 1.f,  1.f,  1.f, 0.f)); // Normal of C
	n[d] = glm::normalize(glm::vec4(-1.f,  1.f,  1.f, 0.f)); // Normal of D
	n[e] = glm::normalize(glm::vec4(-1.f, -1.f, -1.f, 0.f)); // Normal of E
	n[f] = glm::normalize(glm::vec4( 1.f, -1.f, -1.f, 0.f)); // Normal of F
	n[g] = glm::normalize(glm::vec4( 1.f,  1.f, -1.f, 0.f)); // Normal of G
	n[h] = glm::normalize(glm::vec4(-1.f,  1.f, -1.f, 0.f)); // Normal of H

	GLuint face1[] = {a, b, c, a, c, d};
	GLuint face2[] = {a, f, b, a, e, f};
	GLuint face3[] = {c, b, f, c, f, g};
	GLuint face4[] = {d, c, g, d, g, h};
	GLuint face5[] = {a, d, h, a, h, e};
	GLuint face6[] = {e, h, g, e, g, f};

	i.insert(i.end(), face1, face1+6);
	i.insert(i.end(), face2, face2+6);
	i.insert(i.end(), face3, face3+6);
	i.insert(i.end(), face4, face4+6);
	i.insert(i.end(), face5, face5+6);
	i.insert(i.end(), face6, face6+6);

	return;
}

void GLWidget::cleanupResources() {
	// TODO : cleanup actual resources
}
