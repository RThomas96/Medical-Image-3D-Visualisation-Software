#include "./glwidget.hpp"

#include "./main_widget.hpp"

#include <iostream>

MyGLWidget::MyGLWidget(MainWidget* _main, QWidget* parent) : QOpenGLWidget(parent) {
	this->vaoHandle = 0;
	this->vboHandle = 0;
	this->idxHandle = 0;
	this->progHandle = 0;
	this->main = _main;
	this->f = nullptr;
	this->logger = nullptr;
	std::cerr << "MyGLWidget created" << '\n';
}

MyGLWidget::~MyGLWidget() {
	//
}

void MyGLWidget::initializeGL() {
	if (this->context() == nullptr) { throw std::runtime_error("Context was null"); }
	this->f = new QOpenGLFunctions_4_5_Core;
	this->f->initializeOpenGLFunctions();

	if (this->context()->hasExtension(QByteArray("GL_KHR_debug"))) {
		this->main->addUserMessage("Setting up QopenGLDebugLogger.");
		this->logger = new QOpenGLDebugLogger;
		if (this->logger->initialize()) {
			QObject::connect(this->logger, &QOpenGLDebugLogger::messageLogged, this->main, &MainWidget::addOpenGLMessage);
			this->logger->startLogging(QOpenGLDebugLogger::LoggingMode::SynchronousLogging);
			this->main->addUserMessage("QOpenGLDebugLogger initialized and started logging ...");
		} else {
			this->main->addUserMessage("Could not initialize the QOpenGLDebugLogger. No messages will be logged.");
		}
	} else {
		this->main->addUserMessage("Context does not support the GL_KHR_debug extension.");
	}

	const GLfloat positions[] = {
		-.5, -.5, .0, 1.,
		 .5, -.5, .0, 1.,
		-.5,  .5, .0, 1.,
		 .5,  .5, .0, 1.,
	};
	const GLuint indices[] = {0, 1, 2, 1, 2, 3};

	//===================//
	// Buffers and VAO : //
	//===================//
	this->f->glCreateVertexArrays(1, &this->vaoHandle);
	this->f->glBindVertexArray(this->vaoHandle);

	// Upload position data :
	this->f->glCreateBuffers(1, &this->vboHandle);
	this->f->glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle);
	this->f->glBufferData(GL_ARRAY_BUFFER, 16*sizeof(GLfloat), positions, GL_STATIC_DRAW);
	this->f->glEnableVertexAttribArray(0);
	this->f->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Upload indices :
	this->f->glCreateBuffers(1, &this->idxHandle);
	this->f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->idxHandle);
	this->f->glBufferData(GL_ARRAY_BUFFER, 6*sizeof(GLuint), indices, GL_STATIC_DRAW);

	// unbind buffers :
	this->f->glBindBuffer(GL_ARRAY_BUFFER, 0);
	this->f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//==================//
	// Create Shaders : //
	//==================//
	GLuint vsha = this->compileShader(GL_VERTEX_SHADER, globalVertexShaderSource);
	GLuint fsha = this->compileShader(GL_FRAGMENT_SHADER, globalFragmentShaderSource);
	// Compiled, have to link them now :
	this->progHandle = this->linkProgram({vsha, fsha});
}

GLuint MyGLWidget::compileShader(GLenum shatype, std::string _source) {
	const char* src = _source.c_str();

	GLuint sha = this->f->glCreateShader(shatype);

	this->f->glShaderSource(sha, 1, const_cast<const char**>(&src), 0);

	this->f->glCompileShader(sha);

	GLint shaderInfoLength = 0;
	GLint charsWritten = 0;
	char* shaderInfoLog = nullptr;

	// Get shader information after compilation :
	this->f->glGetShaderiv(sha, GL_INFO_LOG_LENGTH, &shaderInfoLength);
	if (shaderInfoLength > 1) {
		shaderInfoLog = new char[shaderInfoLength];
		this->f->glGetShaderInfoLog(sha, shaderInfoLength, &charsWritten, shaderInfoLog);
		QString msg = shaderInfoLog;
		this->main->addUserMessage("Shader compilation generated additional information :<br/>" + msg);
		delete[] shaderInfoLog;
	}

	GLint result = GL_FALSE;
	this->f->glGetShaderiv(sha, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		this->main->addUserMessage("Could not compile shader.");
		return 0;
	}

	return sha;
}

GLuint MyGLWidget::linkProgram(std::initializer_list<GLuint> _shaders) {
	GLuint prog = this->f->glCreateProgram();

	for (const GLuint& shaHandle : _shaders) {
		this->f->glAttachShader(prog, shaHandle);
	}

	this->f->glLinkProgram(prog);

	int InfoLogLength = 0;
	this->f->glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		this->f->glGetProgramInfoLog(prog, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		QString msg = ProgramErrorMessage.data();
		this->main->addUserMessage("Warning : errors while linking program :<br/>" + msg);
	}

	GLint Result = 0;
	this->f->glGetProgramiv(prog, GL_LINK_STATUS, &Result);
	if (Result == GL_FALSE) {
		// Return 0 (no program created) :
		this->main->addUserMessage("Could not link shader.");
		return 0;
	}

	return prog;
}

void MyGLWidget::paintGL() {
	this->f->glClearColor(1., 1., 1., 1.);

	this->f->glUseProgram(this->progHandle);
	this->f->glBindVertexArray(this->vaoHandle);
	this->f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->idxHandle);

	this->f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

	this->f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	this->f->glBindVertexArray(0);
	this->f->glUseProgram(0);
}

QSize MyGLWidget::sizeHint() const {
	return QSize(1366, 768);
}

void MyGLWidget::createTexture3DOnce() {
	//
}

void MyGLWidget::createTexture3DMultiple() {
	//
}
