#ifndef NVIDIA_BUG_REPORT_GLWIDGET_HPP_
#define NVIDIA_BUG_REPORT_GLWIDGET_HPP_

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLDebugLogger>
#include <QTimer>

#include <glm/glm.hpp>

#ifndef __GL_H__
#	include <GL/gl.h>
#endif

class MainWidget; // fwd-declaration for func headers

class MyGLWidget : public QOpenGLWidget, public QOpenGLFunctions_4_5_Core {
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
		// Parent widget, where the GL log resides.
		MainWidget* main;
		// OpenGL-managed storage buffers / vao :
		GLuint vaoHandle;
		GLuint vboHandle;
		GLuint idxHandle;
		// Program ID :
		GLuint progHandle;
		// Qt's class to intercept messages emmitted when the GL_KHR_debug extension is enabled.
		QOpenGLDebugLogger* logger;
		// Texture handle :
		GLuint texHandle;
		// Has OpenGL been initialized ?
		bool init;
		// Automatic window refresh.
		QTimer* timer_refresh;
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
	gl_Position=globalPos;
}
)src";

// FShader source :
const std::string globalFragmentShaderSource = R"src(
#version 450
// Single input :
in vec4 globalPos;
// Single color output :
out vec4 color;

// Uniforms :
// User-generated 3D texture :
uniform usampler3D userGen;
// Time (in [0.f, 1000.f]) :
uniform float time;
// Should show tex or not :
uniform bool showTex;

void main() {
	if (showTex == true) {
		vec3 pos = vec3(globalPos.xy, time);
		vec4 rawTexVal = texture(userGen, pos);
		color = vec4(
			float(rawTexVal.x),
			float(rawTexVal.y),
			float(rawTexVal.z),
			1.
		);
	} else {
		color = vec4(globalPos.rg, time, 1.f);
	}
}
)src";

#endif // NVIDIA_BUG_REPORT_GLWIDGET_HPP_
