#ifndef NVIDIA_BUG_REPORT_GLWIDGET_HPP_
#define NVIDIA_BUG_REPORT_GLWIDGET_HPP_

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLDebugLogger>

#include <glm/glm.hpp>

#ifndef __GL_H__
#	include <GL/gl.h>
#endif

class MainWidget; // fwd-declaration for func headers

class MyGLWidget : public QOpenGLWidget {
		Q_OBJECT
	public:
		MyGLWidget(MainWidget* _main, QWidget* parent = nullptr);
		~MyGLWidget();
	public:
		void initializeGL() override;
		void paintGL() override;
		QSize sizeHint() const override;
	public slots:
		void createTexture3DOnce();	// Creates a 3D texture by allocating and filling data at once
		void createTexture3DMultiple();	// Creates a 3D texture by allocating first, then filling with glTexSubImage
	protected:
		GLuint compileShader(GLenum type, std::string);
		GLuint linkProgram(std::initializer_list<GLuint> shaders);
	protected:
		MainWidget* main;
		// GL functions :
		QOpenGLFunctions_4_5_Core* f;
		// OpenGL-managed storage buffers / vao :
		GLuint vaoHandle;
		GLuint vboHandle;
		GLuint idxHandle;
		// Program ID :
		GLuint progHandle;
		QOpenGLDebugLogger* logger;
};

// VShader source :
const std::string globalVertexShaderSource = R"src(
#version 450
// Input, single location :
layout(location=0) in vec4 position;
// Output, single location no transformation/projection :
out vec4 globalPos;

void main() {
	globalPos=position;
}
)src";

// FShader source :
const std::string globalFragmentShaderSource = R"src(
#version 450
// Single input :
in vec4 globalPos;
// Single color output :
out vec4 color;

void main() {
	color = globalPos.rgra;
}
)src";

#endif // NVIDIA_BUG_REPORT_GLWIDGET_HPP_
