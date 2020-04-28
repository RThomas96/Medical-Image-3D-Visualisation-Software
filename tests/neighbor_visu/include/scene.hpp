#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_HPP_

#include "gl/GLHandler/include/ShaderObject.hpp"
#include "gl/GLHandler/include/ProgramObject.hpp"
#include "gl/GLHandler/include/VAOObject.hpp"

#include "image/include/bulk_texture_loader.hpp"

#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>

#include <vector>

class Scene {

	public:
		Scene(void); ///< default constructor
		~Scene(void); ///< default destructor

		// initialize the variables of the scene
		void initGl(std::size_t _x = 10, std::size_t _y = 10, std::size_t _z = 10);

		void toggleRealVoxelSize();

		void loadImage(std::size_t i, std::size_t j, std::size_t k, const unsigned char* pData = nullptr);
		void queryImage(void);

		void reloadShaders(void) const;

		glm::vec3 getSceneBoundaries(void) const;

		void toggleTransposeMatrices(void) { this->transposeMatrices = !this->transposeMatrices; }

		// public functions :
		void drawRealSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe = false) const;
		void drawInitialSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe = false) const;

		void cleanup(void); ///< cleanup function for vbo and other parts
		bool isInitialized; ///< tracks if the scene was initialized or not

	protected:
		void generateGrid(std::size_t _x, std::size_t _y, std::size_t _z);

		bulk_texture_loader* loader; ///< texture loader

		VAOObject* vao; ///< vao to send the data to
		ProgramObject* program; ///< shader program to display the scene

		GLuint textureHandle; ///< handle for glTexImage3D

		GLuint mMatrixLocation; ///< uniform location
		GLuint vMatrixLocation; ///< uniform location
		GLuint pMatrixLocation; ///< uniform location
		GLuint transformationMatrixLocation; ///< uniform location
		GLuint texDataLocation; ///< uniform location
		GLuint texOffsetLocation; ///< uniform location
		GLuint inspectorTexSizeLocation; ///< uniform location
		GLboolean transposeMatrices; ///< do we need to transpose matrices ?

		glm::vec3 positionNormalized; ///< uniform location

		std::size_t gridWidth; ///< grid size
		std::size_t gridHeight; ///< grid size
		std::size_t gridDepth; ///< grid size
		std::size_t neighborWidth; ///< neighborhood size
		std::size_t neighborHeight; ///< neighborhood size
		std::size_t neighborDepth; ///< neighborhood size

		bool drawRealVoxelSize; ///< do we need to draw the voxels to their real sizes ?
	private:
		void loadEmptyImage();
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_HPP_

// vim : ts=8
