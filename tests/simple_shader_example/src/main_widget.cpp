#include "../include/main_widget.hpp"

#include <QKeyEvent>

const char* getFileContents(std::string path, std::size_t& fileSize) {
	std::ifstream file(path, std::ios::in);

	// Check open state :
	if (not file.is_open()) {
		std::string errMessage = "Error : could not open shader file" + path;
		throw std::runtime_error(errMessage);
	}

	// Get file length :
	file.seekg(0, file.end);
	fileSize = static_cast<std::size_t>(file.tellg());
	file.seekg(0, file.beg);

	// Allocate char array for contents :
	char* fileSource = static_cast<char*>(calloc(fileSize+1, sizeof(char)));
	// Read contents :
	file.read(fileSource, fileSize);

	// Terminate string just in case :
	fileSource[fileSize] = '\0';
	// Close fstream :
	file.close();

	return fileSource;
}

void readShaderInfo(GLenum shaType, const char* shaPath, GLuint shaHandle) {
	GLint shaderInfoLength = 0;
	GLint charsWritten = 0;
	char* shaderInfoLog = nullptr;

	glGetShaderiv(shaHandle, GL_INFO_LOG_LENGTH, &shaderInfoLength);
	GetOpenGLError();
	if (shaderInfoLength > 1) {
		std::cerr << __PRETTY_FUNCTION__ << " : start Log ***********************************************" << '\n';

		std::cerr << __FUNCTION__ << " : Information about shader " << shaPath << " : " << '\n';
		std::cerr << __FUNCTION__ << " : Shader was a " << ((shaType == GL_VERTEX_SHADER) ? "vertex" : "fragment") << " shader\n";
		shaderInfoLog = new char[shaderInfoLength];
		glGetShaderInfoLog(shaHandle, shaderInfoLength, &charsWritten, shaderInfoLog);
		GetOpenGLError();
		std::cerr << shaderInfoLog << '\n';
		delete[] shaderInfoLog;

		std::cerr << __PRETTY_FUNCTION__ << " : end Log ***********************************************" << '\n';
	} else {
		std::cerr << "No more info about shader " << shaPath << '\n';
	}
}

void readProgramInfo(GLuint proHandle) {
	GLint Result = 0;
	int InfoLogLength = 0;
	glGetProgramiv(proHandle, GL_LINK_STATUS, &Result);
	GetOpenGLError();
	glGetProgramiv(proHandle, GL_INFO_LOG_LENGTH, &InfoLogLength);
	GetOpenGLError();
	if ( InfoLogLength > 0 ){
		char* ProgramErrorMessage = new char[InfoLogLength+1];
		glGetProgramInfoLog(proHandle, InfoLogLength, NULL, ProgramErrorMessage);
		GetOpenGLError();
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

MainWidget::MainWidget(QWidget* parent) : QGLViewer(parent) {
	this->programHandle = 0;
	this->vShaHandle = 0;
	this->fShaHandle = 0;
	this->vaoHandle = 0;
	this->vboVertexPosHandle = 0;
	this->vboVertexNormHandle = 0;
	this->vboElementHandle = 0;

	this->vMatrixLocation = -1;
	this->pMatrixLocation = -1;

	this->format().setVersion(4,0);

	this->vShaPath = "./shaders/base.vert";
	this->fShaPath = "./shaders/base.frag";
}

MainWidget::~MainWidget() {
	this->cleanupGLResources();
}

void MainWidget::init() {
	// Initialize GLEW :
	glewExperimental = GL_TRUE;
	GLenum initResult = glewInit();
	if (initResult != GLEW_OK) {
		throw std::runtime_error("Error : could not start GLEW !");
	}

	// Intitialize the GL state machine to a state we want :
	GetOpenGLError();
	glEnable(GL_DEPTH_TEST);
	GetOpenGLError();
	glDepthFunc(GL_LESS);
	GetOpenGLError();
	glEnable(GL_CULL_FACE);
	GetOpenGLError();

	// Generate VAO
	glGenVertexArrays(1, &this->vaoHandle);
	GetOpenGLError();

	// Bind it
	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();

	// Generate the vertices positions and normals :
	std::vector<glm::vec4> vPos;
	std::vector<glm::vec4> vNorms;
	std::vector<GLuint> idx;
	this->generatePositions(vPos);
	this->generateNormals(vNorms);
	this->generateIndices(idx);

	// Generate, allocate and populate the VBOs :
	glGenBuffers(1, &this->vboVertexPosHandle);
	GetOpenGLError();
	glGenBuffers(1, &this->vboVertexNormHandle);
	GetOpenGLError();
	glGenBuffers(1, &this->vboElementHandle);
	GetOpenGLError();

	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertexPosHandle);
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec4), vPos.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertexNormHandle);
	GetOpenGLError();
	glBufferData(GL_ARRAY_BUFFER, vNorms.size() * sizeof(glm::vec4), vNorms.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	GetOpenGLError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);
	GetOpenGLError();

	this->setupVAOPointers();

	this->recompileShaders();

	glUseProgram(this->programHandle);

	this->vMatrixLocation = glGetUniformLocation(this->programHandle, "vMatrix");
	GetOpenGLError();
	this->pMatrixLocation = glGetUniformLocation(this->programHandle, "pMatrix");
	GetOpenGLError();

	glBindVertexArray(0);
	GetOpenGLError();
	glUseProgram(0);
	GetOpenGLError();
}

void MainWidget::setupVAOPointers() {
	GLint vaoBound = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBound);
	GetOpenGLError();
	if (this->vaoHandle != static_cast<GLuint>(vaoBound)) {
		std::cerr << "VAO bound was not the one we wanted. Changing that !\n";
		glBindVertexArray(this->vaoHandle);
		GetOpenGLError();
	}

	glEnableVertexAttribArray(0);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertexPosHandle);
	GetOpenGLError();
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	GetOpenGLError();

	glEnableVertexAttribArray(1);
	GetOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, this->vboVertexNormHandle);
	GetOpenGLError();
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	GetOpenGLError();
}

void MainWidget::compileShaders(std::string _vShaPath, std::string _fShaPath) {
	glUseProgram(0);
	GetOpenGLError();
	glDeleteProgram(this->programHandle);
	GetOpenGLError();

	// Create new handles :
	this->vShaHandle = glCreateShader(GL_VERTEX_SHADER);
	GetOpenGLError();
	this->fShaHandle = glCreateShader(GL_FRAGMENT_SHADER);
	GetOpenGLError();

	// Read files :
	std::size_t vShaLength = 0, fShaLength = 0;
	const char* vShaSource = getFileContents(_vShaPath, vShaLength);
	const char* fShaSource= getFileContents(_fShaPath, fShaLength);
	GLint vShaLengthi = static_cast<GLint>(vShaLength), fShaLengthi = static_cast<GLint>(fShaLength);

	// Source the contents to OpenGL :
	glShaderSource(this->vShaHandle, 1, &vShaSource, &vShaLengthi);
	GetOpenGLError();
	glShaderSource(this->fShaHandle, 1, &fShaSource, &fShaLengthi);
	GetOpenGLError();

	// Compile vShader :
	glCompileShader(this->vShaHandle);
	GetOpenGLError();
	readShaderInfo(GL_VERTEX_SHADER, _vShaPath.c_str(), this->vShaHandle);

	// Compile fShader :
	glCompileShader(this->fShaHandle);
	GetOpenGLError();
	readShaderInfo(GL_FRAGMENT_SHADER, _fShaPath.c_str(), this->fShaHandle);

	// Create new program handle, and link :
	this->programHandle = glCreateProgram();
	glAttachShader(this->programHandle, this->vShaHandle);
	GetOpenGLError();
	glAttachShader(this->programHandle, this->fShaHandle);
	GetOpenGLError();
	glLinkProgram(this->programHandle);
	GetOpenGLError();
	readProgramInfo(this->programHandle);

	glDetachShader(this->programHandle, this->vShaHandle);
	GetOpenGLError();
	glDetachShader(this->programHandle, this->fShaHandle);
	GetOpenGLError();

	// Delete shader, don't need them once compiled and linked
	glDeleteShader(this->vShaHandle);
	GetOpenGLError();
	glDeleteShader(this->fShaHandle);
	GetOpenGLError();

	this->vShaHandle = 0;
	this->fShaHandle = 0;

	delete[] vShaSource;
	delete[] fShaSource;
}

void MainWidget::recompileShaders() {
	this->compileShaders(this->vShaPath, this->fShaPath);
}

void MainWidget::draw() {
	GetOpenGLError();
	glEnable(GL_DEPTH_TEST);
	GetOpenGLError();
	glDepthFunc(GL_LESS);
	GetOpenGLError();
	glEnable(GL_CULL_FACE);
	GetOpenGLError();

	glUseProgram(this->programHandle);
	GetOpenGLError();

	GLfloat* vMat = new GLfloat[16];
	GLfloat* pMat = new GLfloat[16];
	this->camera()->getModelViewMatrix(vMat);
	this->camera()->getProjectionMatrix(pMat);

	glUniformMatrix4fv(this->vMatrixLocation, 1, GL_FALSE, vMat);
	GetOpenGLError();
	glUniformMatrix4fv(this->pMatrixLocation, 1, GL_FALSE, pMat);
	GetOpenGLError();

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	GetOpenGLError();
	glEnableClientState(GL_VERTEX_ARRAY);
	GetOpenGLError();
	glEnableClientState(GL_NORMAL_ARRAY);
	GetOpenGLError();
	glBindVertexArray(this->vaoHandle);
	GetOpenGLError();
	this->setupVAOPointers();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboElementHandle);
	GetOpenGLError();
	glDrawElements(GL_TRIANGLES, GLsizei(36), GL_UNSIGNED_INT, nullptr);
	GetOpenGLError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GetOpenGLError();

	glBindVertexArray(0);
	GetOpenGLError();
	glDisableClientState(GL_VERTEX_ARRAY);
	GetOpenGLError();
	glDisableClientState(GL_NORMAL_ARRAY);
	GetOpenGLError();
	glPopClientAttrib();
	GetOpenGLError();
	glUseProgram(0);
	GetOpenGLError();
}

void MainWidget::keyPressEvent(QKeyEvent *e) {
	switch (e->key()) {
		case Qt::Key::Key_R:
			this->recompileShaders();
			std::cerr << "Reloading of shader done !" << '\n';
			this->update();
		break;
		default:
			QGLViewer::keyPressEvent(e);
		break;
	}
}

void MainWidget::generatePositions(std::vector<glm::vec4> &v) {
	// Generate positions of a fixed cube. The cube will
	// have a sidelength of 1, and have only 8 vertices
	// indexed by an index buffer somewhere else.
	v.clear();

	v.emplace_back(.0f, .0f, 1.f, 1.f); // Vertex A
	v.emplace_back(1.f, .0f, 1.f, 1.f); // Vertex B
	v.emplace_back(1.f, 1.f, 1.f, 1.f); // Vertex C
	v.emplace_back(.0f, 1.f, 1.f, 1.f); // Vertex D
	v.emplace_back(.0f, .0f, .0f, 1.f); // Vertex E
	v.emplace_back(1.f, .0f, .0f, 1.f); // Vertex F
	v.emplace_back(1.f, 1.f, .0f, 1.f); // Vertex G
	v.emplace_back(.0f, 1.f, .0f, 1.f); // Vertex H

	return;
}

void MainWidget::generateNormals(std::vector<glm::vec4> &n) {
	// Generate normals of a fixed cube. The cube will
	// have a sidelength of 1, and have only 8 vertices
	// indexed by an index buffer somewhere else.
	n.clear();
	n.resize(8);

	n[0] = glm::normalize(glm::vec4(-1.f, -1.f,  1.f, 0.f)); // Normal of A
	n[1] = glm::normalize(glm::vec4( 1.f, -1.f,  1.f, 0.f)); // Normal of B
	n[2] = glm::normalize(glm::vec4( 1.f,  1.f,  1.f, 0.f)); // Normal of C
	n[3] = glm::normalize(glm::vec4(-1.f,  1.f,  1.f, 0.f)); // Normal of D
	n[4] = glm::normalize(glm::vec4(-1.f, -1.f, -1.f, 0.f)); // Normal of E
	n[5] = glm::normalize(glm::vec4( 1.f, -1.f, -1.f, 0.f)); // Normal of F
	n[6] = glm::normalize(glm::vec4( 1.f,  1.f, -1.f, 0.f)); // Normal of G
	n[7] = glm::normalize(glm::vec4(-1.f,  1.f, -1.f, 0.f)); // Normal of H

	return;
}

void MainWidget::generateIndices(std::vector<GLuint> &i) {
	// This is the index buffer for the cube talked about
	// in the functions generate{Positions|Normals}().
	// Indexes the vertex positions to draw, in order, so
	// a cube can be drawn. All are in CW order. Might need
	// to change the OpenGL face for front polygons, but
	// everything should be good.
	i.clear();

	GLuint a = 0; GLuint b = 1;
	GLuint c = 2; GLuint d = 3;
	GLuint e = 4; GLuint f = 5;
	GLuint g = 6; GLuint h = 7;

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

void MainWidget::cleanupGLResources() {}
