#ifndef TESTS_SIMPLE_SHADER_EXAMPLE_MAIN_WIDGET_HPP_
#define TESTS_SIMPLE_SHADER_EXAMPLE_MAIN_WIDGET_HPP_

#ifndef __glew_h_
#	define GLEW_STATIC 1
#	include <GL/glew.h>
#endif

#include <glm/glm.hpp>

#include <QGLViewer/qglviewer.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <functional> // for std::pair, std::make_pair and std::tie

#if not defined( NDEBUG )
	#define GetOpenGLError() __GetOpenGLError( ( char* )__FILE__, ( int )__LINE__ )
#else
	#define GetOpenGLError()
#endif

inline void __GetOpenGLError ( char* szFile, int iLine )
{
	int    retCode = 0;
	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		std::cerr << "GLError in file " << szFile << " @ line " << iLine << " : ";
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
			default:
				std::cerr << "was another error";
			break;
		}
		std::cerr << '\n';
		glErr = glGetError();
		retCode = 1;
	}
	if (retCode) {
		exit(EXIT_FAILURE);
	}
}

const char* getFileContents(std::string path, std::size_t& fileSize);
void readShaderInfo(GLenum shaType, const char* shaPath, GLuint shaHandle);
void readProgramInfo(GLuint proHandle);

class MainWidget : public QGLViewer {
		Q_OBJECT
	public:
		MainWidget(QWidget* parent = nullptr);
		~MainWidget(void);

	protected:
		virtual void init() override; ///< initializes the opengl variables
		virtual void draw() override; ///< draws the opengl data
		virtual void keyPressEvent(QKeyEvent* e) override; ///< handles keypresses

	private:
		void generatePositions(std::vector<glm::vec4>& v); ///< generates the positions of vertices
		void generateNormals(std::vector<glm::vec4>& n); ///< generates the normals of vertices
		void generateIndices(std::vector<GLuint>& i); ///< generate the indices of the faces to draw

		void setupVAOPointers();

		void compileShaders(std::string vShaPath, std::string fShaPath); ///< specify new shader paths and compile
		void recompileShaders(void); ///< recompile currently loaded shaders
		void cleanupGLResources(void);

	private:
		//
		// Variables for the shaders :
		//
		GLuint programHandle;
		GLuint vShaHandle;
		GLuint fShaHandle;
		GLuint vaoHandle;
		GLuint vboVertexPosHandle;
		GLuint vboVertexNormHandle;
		GLuint vboElementHandle;

		//
		// Variables for uniform locations (incoming)
		//
		GLint vMatrixLocation;
		GLint pMatrixLocation;

		//
		// The paths to the shader files
		//
		std::string vShaPath;
		std::string fShaPath;
};

#endif // TESTS_SIMPLE_SHADER_EXAMPLE_MAIN_WIDGET_HPP_
