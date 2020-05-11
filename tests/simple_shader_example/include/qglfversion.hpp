#ifndef TESTS_SIMPLE_SHADER_EXAMPLE_QGLFVERSION_HPP_
#define TESTS_SIMPLE_SHADER_EXAMPLE_QGLFVERSION_HPP_

#include "./logoutput.hpp"

#include <glm/glm.hpp>

#include <QGLViewer/qglviewer.h>

#include <QOpenGLFunctions_4_5_Core>
#include <QPlainTextEdit>
#include <QKeyEvent>

#include <iostream>
#include <vector>

#ifndef NDEBUG
#	define GetGLError() __GetOpenGLError ( (char*)__FILE__, (char*)__FUNCTION__, (int)__LINE__ );
#else
#	define GetGLError()
#endif

void __GetOpenGLError ( char* szFile, char* szFunction, int iLine );
std::vector<GLchar> readShaderFile(const std::string pathname);

class GLWidget : public QGLViewer, public QOpenGLFunctions_4_5_Core {
		Q_OBJECT
	public:
		GLWidget(QWidget* parent = nullptr); ///< default construtor
		~GLWidget(void); ///< default destructor

	protected:
		virtual void init(void) override; ///< init the context
		virtual void draw(void) override; ///< draw the context
		virtual void keyPressEvent(QKeyEvent* e) override; ///< handle keypresses
		void compileShaders(const std::string _vShaPath, const std::string _fShaPath); ///< compile or recompile shaders

	private:
		void setVAODataPointers(void);
		void readProgramInfo(const GLuint proHandle);
		void readShaderState(GLenum shaType, const std::string pathName, GLuint shaHandle);

		void generateCube(std::vector<glm::vec4>& v, std::vector<glm::vec4>& n, std::vector<GLuint>& i) const;

		void cleanupResources(void);

	private:
		GLuint programHandle;
		GLuint vShaHandle;
		GLuint gShaHandle;
		GLuint fShaHandle;
		GLuint vaoHandle;
		GLuint vboVertexPosHandle;
		GLuint vboVertexNormHandle;
		GLuint vboElementHandle;

		GLenum frontFaceOrder;
		GLenum transposeMatrices;

		std::string vShaPath;
		std::string fShaPath;
};

#endif // TESTS_SIMPLE_SHADER_EXAMPLE_QGLFVERSION_HPP_
