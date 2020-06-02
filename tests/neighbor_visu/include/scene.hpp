#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_HPP_

//#include "gl/GLHandler/include/ShaderObject.hpp"
//#include "gl/GLHandler/include/ProgramObject.hpp"
//#include "gl/GLHandler/include/VAOObject.hpp"

#include "image/include/bulk_texture_loader.hpp"

#include "./image_storage.hpp"
#include "./tetmesh.hpp"

#include <QOpenGLFunctions_4_0_Core>
#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>

#include <vector>
#include <mutex>

#if not defined( NDEBUG )
	#define GetOpenGLError() __GetOpenGLError( ( char* )__FILE__, ( int )__LINE__ )
#else
	#define GetOpenGLError()
#endif

enum DrawMode {
	Solid,
	SolidAndWireframe,
	Wireframe
};

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
//	if (retCode) {
//		exit(EXIT_FAILURE);
//	}
	return retCode;
}

class ControlPanel; // forward declaration

typedef struct neighborCoordinates {
	union {glm::vec3 origin, startingPoint, o;};
	union {glm::vec3 limit, endPoint, p;};
} nbCoord;

class Scene : public QOpenGLFunctions_4_0_Core {
		typedef glm::vec<4, unsigned int, glm::defaultp> uvec4;
		typedef glm::vec<3, int, glm::defaultp> ivec3;
	public:
		Scene(void); ///< default constructor
		~Scene(void); ///< default destructor

		void setControlPanel(ControlPanel* cp) { this->controlPanel = cp; }

		// initialize the variables of the scene
		void initGl(QOpenGLContext* context, std::size_t _x = 1, std::size_t _y = 1, std::size_t _z = 1);

		void compileShaders();
		void setupVBOData();
		void setupVAOPointers();

		void drawRealSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe = false);
		void drawInitialSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe = false);

		void queryImage(void);
		void loadImage(std::size_t i, std::size_t j, std::size_t k, const unsigned char* pData = nullptr);

		void toggleTexCubeVisibility(bool visibility) { this->showTextureCube = visibility; }
		void toggleTexCubeVisibility() { this->toggleTexCubeVisibility(!this->showTextureCube); }

		glm::vec3 getSceneBoundaries(void) const;
		glm::vec3 getTexCubeBoundaries(bool realSpace) const;
		nbCoord getNeighborBoundaries(bool realSpace) const;

		void setDrawModeSolid() { this->drawMode = DrawMode::Solid; }
		void setDrawModeSolidAndWireframe() { this->drawMode = DrawMode::SolidAndWireframe; }
		void setDrawModeWireframe() { this->drawMode = DrawMode::Wireframe; }

		void cleanup(void); ///< cleanup function for vbo and other parts
		bool isInitialized; ///< tracks if the scene was initialized or not

		// Simili-slots (this scene cannot be a QObject, as such we cannot have slot/signals) :

		void slotTogglePolygonMode(bool show);
		void slotToggleShowTextureCube(bool show);
		void slotSetNeighborXCoord(int newXCoord);
		void slotSetNeighborYCoord(int newYCoord);
		void slotSetNeighborZCoord(int newZCoord);
		void slotSetTextureXCoord(int newXCoord);
		void slotSetTextureYCoord(int newYCoord);
		void slotSetTextureZCoord(int newZCoord);
	protected:
		void generateGrid(std::size_t _x, std::size_t _y, std::size_t _z);

		QOpenGLContext* context; ///< context given by the viewers
		ControlPanel* controlPanel; ///< pointer to the control panel
		TextureStorage* texStorage; ///< textureLoader and 'manager'
		TetMesh* mesh; ///< creates a mesh around the queried point

		std::size_t gridWidth; ///< grid size
		std::size_t gridHeight; ///< grid size
		std::size_t gridDepth; ///< grid size
		std::size_t neighborWidth; ///< neighborhood size
		std::size_t neighborHeight; ///< neighborhood size
		std::size_t neighborDepth; ///< neighborhood size

		std::size_t renderSize;
		bool showTextureCube; ///< does the user want to show the texture cube ?
		bool cubeShown; ///< is the texture cube shown ?

		// Generated data (positions, normals, texture
		// coordinates, and indexed draw order) :
		std::vector<glm::vec4> vertPos;
		std::vector<glm::vec4> vertNorm;
		std::vector<unsigned int> vertIdx;
		std::vector<uvec4> vertIdxDraw;
		uint drawCalls;
		uint scaledCubes;

		ivec3 neighborOffset;
		ivec3 neighborPos;
		DrawMode drawMode;

		// OpenGL data :

		// Uniform locations :
		GLint mMatrixLocation;
		GLint vMatrixLocation;
		GLint pMatrixLocation;
		GLint texDataLocation;
		GLint lightPosLocation;
		GLint neighborOffsetLocation;

		GLuint vboVertPosHandle;
		GLuint vboVertNormHandle;
		GLuint vboElementHandle;
		GLuint vboIndexedDrawHandle;
		GLuint vaoHandle;
		GLuint vShaHandle;
		GLuint gShaHandle;
		GLuint fShaHandle;
		GLuint programHandle;

		GLuint textureHandle; ///< handle for glTexImage3D
	private:
		void generateTexCube(void);
		const unsigned char* loadEmptyImage();
		void generateNeighborGrid(std::size_t _x, std::size_t _y, std::size_t _z);
		void prepUniforms(glm::mat4 transfoMat, GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos);
		void showTexCubeVBO();
		void hideTexCubeVBO();
		glm::mat4 computeTransformationMatrix() const;
		void queryNeighborsOfPoint();
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_HPP_

// vim : ts=8
