#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_VIEWER_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_VIEWER_HPP_

#include "./scene.hpp"

#include <QGLViewer/qglviewer.h>

// TODO : link control panels's actions to signals/slots here
// TODO : test the class
// TODO : work on shaders

class Viewer : public QGLViewer {
		Q_OBJECT
	public:
		Viewer(QWidget* parent = nullptr);
	protected:
		virtual void init() override;
		virtual void draw() override;
		virtual void keyPressEvent(QKeyEvent* e) override;

		void initGLVariables(std::size_t _x = 1, std::size_t _y = 1, std::size_t _z = 1);

		void loadImage(std::size_t i, std::size_t j, std::size_t k, const unsigned char* pData = nullptr);
		void queryImage(void);

		void generateGrid(std::size_t _x, std::size_t _y, std::size_t _z);

		glm::vec3 getSceneBoundaries(void) const;

		void toggleTransposeMatrices(void) { this->transposeMatrices = !this->transposeMatrices; }

		// public functions :
		void drawRealSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe = false) const;
		void drawInitialSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe = false) const;
		void compileShaders();

		void setupVBOData();

		void setupVAOPointers() const;

		void cleanup(void); ///< cleanup function for vbo and other parts
	private:
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

		float sceneSize;

		std::vector<glm::vec4> vertPos;
		std::vector<glm::vec3> vertTex;
		std::vector<unsigned char> vertIdx;
		mutable std::size_t frameCount1;
		mutable std::size_t frameCount2;
		bool isInitialized; ///< tracks if the scene was initialized or not

		void loadEmptyImage();
		void generateGrid_Only(std::size_t _x, std::size_t _y, std::size_t _z);
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_VIEWER_HPP_
