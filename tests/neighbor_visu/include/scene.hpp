#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_HPP_

#include "gl/GLHandler/include/ShaderObject.hpp"
#include "gl/GLHandler/include/ProgramObject.hpp"
#include "gl/GLHandler/include/VAOObject.hpp"

#include "image/include/bulk_texture_loader.hpp"

#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>

#include <vector>
#include <mutex>

#define RAW_GL

#if not defined( NDEBUG )
	#define GetOpenGLError() __GetOpenGLError( ( char* )__FILE__, ( int )__LINE__ )
#else
	#define GetOpenGLError()
#endif

inline int __GetOpenGLError ( char* szFile, int iLine )
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

class Scene {

	public:
		Scene(void); ///< default constructor
		~Scene(void); ///< default destructor

		// initialize the variables of the scene
		void initGl(QOpenGLContext* const context, std::size_t _x = 1, std::size_t _y = 1, std::size_t _z = 1);

		void toggleRealVoxelSize();

		void loadImage(std::size_t i, std::size_t j, std::size_t k, const unsigned char* pData = nullptr);
		void queryImage(void);

		glm::vec3 getSceneBoundaries(void) const;

		void toggleTransposeMatrices(void) { this->transposeMatrices = !this->transposeMatrices; }

		// public functions :
		void drawRealSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe = false) const;
		void drawInitialSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe = false) const;
		void compileShaders();

		void setupVBOData();

		void setupVAOPointers() const;

		void cleanup(void); ///< cleanup function for vbo and other parts
		bool isInitialized; ///< tracks if the scene was initialized or not

	protected:
		void generateGrid(std::size_t _x, std::size_t _y, std::size_t _z);

		bulk_texture_loader* loader; ///< texture loader

		GLuint textureHandle; ///< handle for glTexImage3D

		GLboolean transposeMatrices; ///< do we need to transpose matrices ?

		glm::vec3 positionNormalized; ///< uniform location

		std::size_t gridWidth; ///< grid size
		std::size_t gridHeight; ///< grid size
		std::size_t gridDepth; ///< grid size
		std::size_t neighborWidth; ///< neighborhood size
		std::size_t neighborHeight; ///< neighborhood size
		std::size_t neighborDepth; ///< neighborhood size

		bool drawRealVoxelSize; ///< do we need to draw the voxels to their real sizes ?

		QOpenGLContext* context;
		GLuint vboVertPosHandle;
		GLuint vboUVCoordHandle;
		GLuint vboElementHandle;
		GLuint vaoHandle;
		GLuint vShaHandle;
		GLuint fShaHandle;
		GLuint programHandle;
		std::size_t elemToDrawIdx;
		std::size_t elemToDrawSeq;

		std::vector<glm::vec4> vertPos;
		std::vector<glm::vec3> vertTex;
		std::vector<unsigned char> vertIdx;
		mutable std::size_t frameCount1;
		mutable std::size_t frameCount2;
	private:
		void loadEmptyImage();
		void generateGrid_Only(std::size_t _x, std::size_t _y, std::size_t _z);
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_HPP_

// vim : ts=8
