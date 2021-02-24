#include "./glwidget.hpp"

#include "./main_widget.hpp"
#include "./time.hpp"

#include <iostream>
#include <fstream>
#include <random>

MyGLWidget::MyGLWidget(MainWidget* _main, QWidget* parent) : QOpenGLWidget(parent) {
	this->vaoHandle = 0;
	this->vboHandle = 0;
	this->idxHandle = 0;
	this->progHandle = 0;
	this->main = _main;
	this->logger = nullptr;
	this->init = false;
	this->timer_refresh = nullptr;
}

MyGLWidget::~MyGLWidget() {
	this->makeCurrent();
	this->glDeleteBuffers(1, &this->vboHandle);
	this->glDeleteBuffers(1, &this->idxHandle);
	this->glDeleteVertexArrays(1, &this->vaoHandle);

	this->glDeleteProgram(this->progHandle);

	if (this->logger) {
		this->logger->stopLogging();
		this->logger->disconnect();
		delete this->logger;
	}

	this->timer_refresh->disconnect();
	delete this->timer_refresh;
	this->doneCurrent();
}

void MyGLWidget::initializeGL() {
	if (this->context() == nullptr) { throw std::runtime_error("Context was null"); }
	if (this->init) { return; }
	this->init = true;

	this->initializeOpenGLFunctions();

	initPlatformTimer();

	auto context = this->context()->format().version();

	QString msg = "OpenGL context version " + QString::number(context.first) + "." + QString::number(context.second);
	this->main->addUserMessage(msg);

	if (this->context()->isOpenGLES()) {
		this->main->addUserMessage("OpenGL used is OpenGLES.");
	}

	if (this->context()->hasExtension(QByteArray("GL_KHR_debug"))) {
		this->main->addUserMessage("Setting up QopenGLDebugLogger.");
		this->logger = new QOpenGLDebugLogger;
		if (this->logger->initialize()) {
			QObject::connect(this->logger, &QOpenGLDebugLogger::messageLogged, this->main, &MainWidget::addOpenGLMessage);
			this->logger->startLogging(QOpenGLDebugLogger::LoggingMode::SynchronousLogging);
			this->main->addUserMessage("QOpenGLDebugLogger initialized and started logging ...");
		} else {
			this->main->addUserMessage("Could not initialize the QOpenGLDebugLogger. No messages will be logged.");
			delete this->logger;
			this->logger = nullptr;
		}
	} else {
		this->main->addUserMessage("Context does not support the GL_KHR_debug extension.");
	}

	GLfloat min = -.9, max = .9;

	const GLfloat positions[] = {
		min, min, .0, 1.,
		max, min, .0, 1.,
		min, max, .0, 1.,
		max, max, .0, 1.,
	};
	const GLuint indices[] = {0, 1, 2, 1, 2, 3};
	this->texHandle = 0;

	this->main->addUserMessage("Creating VAO");
	//===================//
	// Buffers and VAO : //
	//===================//
	this->glGenVertexArrays(1, &this->vaoHandle);
	this->glBindVertexArray(this->vaoHandle);

	this->main->addUserMessage("Creating position VBO");
	// Upload position data :
	this->glGenBuffers(1, &this->vboHandle);
	this->glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle);
	this->glBufferData(GL_ARRAY_BUFFER, 16*sizeof(GLfloat), positions, GL_STATIC_DRAW);
	this->glEnableVertexAttribArray(0);
	this->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	this->glBindBuffer(GL_ARRAY_BUFFER, 0);

	this->main->addUserMessage("Creating index VBO");
	// Upload indices :
	this->glGenBuffers(1, &this->idxHandle);
	this->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->idxHandle);
	this->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLuint), indices, GL_STATIC_DRAW);
	this->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	this->glBindVertexArray(0);
	this->main->addUserMessage("Unbinding buffers");

	//==================//
	// Create Shaders : //
	//==================//
	this->main->addUserMessage("Compiling shaders");
	GLuint vsha = this->compileShader(GL_VERTEX_SHADER, globalVertexShaderSource);
	GLuint fsha = this->compileShader(GL_FRAGMENT_SHADER, globalFragmentShaderSource);
	// Compiled, have to link them now :
	this->progHandle = this->linkProgram({vsha, fsha});
	this->main->addUserMessage("Compiled shaders.");

	//================//
	// Create Timer : //
	//================//
	this->timer_refresh = new QTimer;
	this->timer_refresh->setSingleShot(false);
	QObject::connect(this->timer_refresh, &QTimer::timeout, [this]() { this->update(); });
	this->timer_refresh->start(1000/144);

	int vmi = 0, vma = 0, ext = 0;
	this->glGetIntegerv(GL_MAJOR_VERSION, &vma);
	this->glGetIntegerv(GL_MINOR_VERSION, &vmi);

	this->glGetIntegerv(GL_NUM_EXTENSIONS, &ext);

	std::cerr << "Version :\n";
	std::cerr << "\tGL_VERSION : " << this->glGetString(GL_VERSION) << '\n';
	std::cerr << "\tGL_MAJOR_VERSION : " << +vma << '\n';
	std::cerr << "\tGL_MINOR_VERSION : " << +vmi << '\n';
	std::cerr << '\n';
	std::cerr << "Extensions : (" << ext << ")\n";
	for (int i = 0; i < ext; ++i) {
		std::cerr << this->glGetStringi(GL_EXTENSIONS, i) << ",\t";
		if (i%4 == 0 || i == ext-1) { std::cerr << '\n'; }
	}
}

GLuint MyGLWidget::compileShader(GLenum shatype, std::string _source) {
	const char* src = _source.c_str();

	GLuint sha = this->glCreateShader(shatype);

	this->glShaderSource(sha, 1, const_cast<const char**>(&src), 0);

	this->glCompileShader(sha);

	GLint shaderInfoLength = 0;
	GLint charsWritten = 0;
	char* shaderInfoLog = nullptr;

	// Get shader information after compilation :
	this->glGetShaderiv(sha, GL_INFO_LOG_LENGTH, &shaderInfoLength);
	if (shaderInfoLength > 1) {
		shaderInfoLog = new char[shaderInfoLength];
		this->glGetShaderInfoLog(sha, shaderInfoLength, &charsWritten, shaderInfoLog);
		QString msg = shaderInfoLog;
		this->main->addUserMessage("Shader compilation generated additional information :<br/>" + msg);
		delete[] shaderInfoLog;
	}

	GLint result = GL_FALSE;
	this->glGetShaderiv(sha, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		this->main->addUserMessage("Could not compile shader.");
		return 0;
	}

	return sha;
}

GLuint MyGLWidget::linkProgram(std::initializer_list<GLuint> _shaders) {
	GLuint prog = this->glCreateProgram();

	for (const GLuint& shaHandle : _shaders) {
		this->glAttachShader(prog, shaHandle);
	}

	this->glLinkProgram(prog);

	int InfoLogLength = 0;
	this->glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		this->glGetProgramInfoLog(prog, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		QString msg = ProgramErrorMessage.data();
		this->main->addUserMessage("Warning : errors while linking program :<br/>" + msg);
	}

	GLint Result = 0;
	this->glGetProgramiv(prog, GL_LINK_STATUS, &Result);
	if (Result == GL_FALSE) {
		// Return 0 (no program created) :
		this->main->addUserMessage("Could not link shader.");
		return 0;
	}

	return prog;
}

void MyGLWidget::paintGL() {
	double time = std::fmod(getTime(), 2.);
	if (time > 1.) { time = 1. - (time - 1.); }
	this->glClearColor(1., 1., 1., 1.);

	this->glUseProgram(this->progHandle);

	this->glUniform1f(this->glGetUniformLocation(this->progHandle, "time"), static_cast<float>(time));
	this->glUniform1i(this->glGetUniformLocation(this->progHandle, "showTex"), (this->texHandle == 0u) ? 0 : 1);

	this->glActiveTexture(GL_TEXTURE0);
	this->glBindTexture(GL_TEXTURE_3D, this->texHandle);
	this->glUniform1i(this->glGetUniformLocation(this->progHandle, "userGen"), 0);

	this->glBindVertexArray(this->vaoHandle);
	this->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->idxHandle);

	this->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

	this->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	this->glBindVertexArray(0);
	this->glUseProgram(0);
}

QSize MyGLWidget::sizeHint() const {
	return QSize(640,360);
}

void MyGLWidget::createTexture3DOnce() {
	this->makeCurrent();
	if (this->texHandle != 0) {
		this->main->addUserMessage("Deleting old texture ...");
		this->glDeleteTextures(1, &this->texHandle);
	}
	this->main->addUserMessage("Creating random data array ...");

	std::random_device rd;
	std::mt19937 mt{rd()};
	std::uniform_int_distribution<uint16_t> distrib{};
	auto generateRandom = [&mt, &distrib]() -> uint16_t {
		return distrib(mt);
	};

	std::size_t x = 4096;
	std::size_t y = 1024;
	std::size_t z =  600;

	std::size_t elementCount = x*y*z;
	std::vector<std::uint16_t> data(elementCount, uint16_t(0));
	std::generate(std::begin(data), std::end(data), generateRandom);

	this->main->addUserMessage("Random data generated.");

	GLuint tex = 0;
	this->glEnable(GL_TEXTURE_3D);

	this->glGenTextures(1, &tex);
	this->glBindTexture(GL_TEXTURE_3D, tex);

	// Min and mag filters :
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Set the min and max LOD values :
	this->glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_LOD, -1000.f);
	this->glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, -1000.f);

	// Set the wrap parameters :
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set the swizzle the user wants :
	GLint swizzle[] = {GL_RED, GL_GREEN, GL_ZERO, GL_ONE};
	this->glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);

	// Set the pixel alignment :
	this->glPixelStorei(GL_PACK_ALIGNMENT, 1);
	this->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	this->main->addUserMessage("Attempting to upload texture ...");

	this->glTexImage3D(
		GL_TEXTURE_3D,		// GLenum : Target
		0,			// GLint  : Level of detail of the current texture (0 = original)
		GL_RG16UI,		// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		x,			// GLsizei: Image width
		y,			// GLsizei: Image height
		z,			// GLsizei: Image depth (number of layers)
		GLint(0),		// GLint  : Border. This value MUST be 0.
		GL_RG_INTEGER,		// GLenum : Format of the pixel data
		GL_UNSIGNED_SHORT,	// GLenum : Type (the data type as in uchar, uint, float ...)
		data.data()		// void*  : Data to load into the buffer
	);

	this->main->addUserMessage("Texture upload attempted.");
	this->texHandle = tex;
	this->doneCurrent();
}

void MyGLWidget::createTexture3DMultiple() {
	this->makeCurrent();
	if (this->texHandle != 0) {
		this->main->addUserMessage("Deleting old texture ...");
		this->glDeleteTextures(1, &this->texHandle);
	}
	this->main->addUserMessage("Creating random data array ...");

	std::random_device rd;
	std::mt19937 mt{rd()};
	std::uniform_int_distribution<uint16_t> distrib{};
	// Generates a random uint16_t number :
	auto generateRandom = [&mt, &distrib]() -> uint16_t {
		return distrib(mt);
	};

	std::size_t x = 4096;
	std::size_t y = 1024;
	std::size_t z =  600;

	std::size_t elementCount = x*y*z;
	std::vector<std::uint16_t> data(elementCount+1, uint16_t(0));
	std::generate(std::begin(data), std::end(data), generateRandom);

	this->main->addUserMessage("Random data generated.");

	GLuint tex = 0;
	this->glEnable(GL_TEXTURE_3D);

	this->glGenTextures(1, &tex);
	this->glBindTexture(GL_TEXTURE_3D, tex);

	// Min and mag filters :
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Set the min and max LOD values :
	this->glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_LOD, -1000.f);
	this->glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, -1000.f);

	// Set the wrap parameters :
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	this->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set the swizzle the user wants :
	GLint swizzle[] = {GL_RED, GL_GREEN, GL_ZERO, GL_ONE};
	this->glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);

	// Set the pixel alignment :
	this->glPixelStorei(GL_PACK_ALIGNMENT, 2);
	this->glPixelStorei(GL_UNPACK_ALIGNMENT, 2);

	this->main->addUserMessage("Attempting to upload texture ...");

	this->glTexImage3D(
		GL_TEXTURE_3D,		// GLenum : Target
		0,			// GLint  : Level of detail of the current texture (0 = original)
		GL_RG16UI,		// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		x,			// GLsizei: Image width
		y,			// GLsizei: Image height
		z,			// GLsizei: Image depth (number of layers)
		GLint(0),		// GLint  : Border. This value MUST be 0.
		GL_RG_INTEGER,		// GLenum : Format of the pixel data
		GL_UNSIGNED_SHORT,	// GLenum : Type (the data type as in uchar, uint, float ...)
		nullptr			// void*  : Data to load into the buffer
	);

	this->main->addUserMessage("Texture upload attempted.");

	std::size_t i = 0;
	for (; i < z; ++i) {
		const uint16_t* offsetdata = &(data[0]) + (x*y*i);
		std::cerr << "Last index : " << i << " baseOffset = " << x*y*i << " elements out of " << data.size() << " (" << (data.size() - x*y*i) << " left) ... ";
		this->glTexSubImage3D(
			GL_TEXTURE_3D,
			0,
			0, 0, i,
			x, y, 1,
			GL_RG_INTEGER,
			GL_UNSIGNED_SHORT,
			offsetdata
		);
		std::cerr << x*y << " elements uploaded.\n";
		this->main->addUserMessage("Uploaded slice " + QString::number(i+1));
	}
	data.clear();

	this->texHandle = tex;
	this->doneCurrent();
}
