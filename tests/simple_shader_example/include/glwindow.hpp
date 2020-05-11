#ifndef TESTS_SIMPLE_SHADER_EXAMPLE_GLWINDOW_HPP_
#define TESTS_SIMPLE_SHADER_EXAMPLE_GLWINDOW_HPP_

#include <glm/glm.hpp>

#include <QGLViewer/qglviewer.h>
#include <QOpenGLFunctions_4_5_Core>

#include <iostream>
#include <vector>

class GLProgram : protected QOpenGLFunctions_4_5_Core {
	public:
		///! default constructor
		GLProgram() : QOpenGLFunctions_4_5_Core(), programHandle(0),
		vShaHandle(0), fShaHandle(0) {
			initializeOpenGLFunctions();
		}
		~GLProgram(void) {
			if (this->vShaHandle) { glDeleteShader(this->vShaHandle); }
			if (this->fShaHandle) { glDeleteShader(this->fShaHandle); }
			if (this->programHandle) { glDeleteProgram(this->programHandle); }
		}
		void compileShaders(void); // used for recompiling the shaders
		void compileShaders(const std::string vShaPathNew, const std::string fShaPathNew);
		void bind() const;
		void unbind() const;
		GLint uniformLocation(const std::string uniformName);
		constexpr GLuint handle(void) { return this->programHandle; }
	protected:
		GLchar* sourceShaderFile(const std::string pathname, GLint& size);
		void readShaderStatus(const GLuint handle);

		GLuint programHandle;
		GLuint vShaHandle;
		GLuint fShaHandle;
		std::string vShaPath;
		std::string fShaPath;
};

class GLWidget : public QGLViewer, public QOpenGLFunctions_4_5_Core {
		Q_OBJECT
	public:
		GLWidget(QWidget* parent = nullptr) : QGLViewer(parent), QOpenGLFunctions_4_5_Core() {
			// Do nothing here
		}
		~GLWidget(void) {}
	protected:
		virtual void init(void) override;
		virtual void draw(void) override;
		virtual void keyPressEvent(QKeyEvent* e) override;
	private:
		GLProgram* program;
};

#endif // TESTS_SIMPLE_SHADER_EXAMPLE_GLWINDOW_HPP_
