#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_TETMESH_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_TETMESH_HPP_

#include "./image_storage.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <iostream>

class TetMesh {
	public:
		TetMesh(const TextureStorage* const texLoader);
		TetMesh(const TextureStorage* const texLoader, glm::vec4 position);
		~TetMesh(void);
		TetMesh& setOrigin(const glm::vec4 position);
		TetMesh& setOrigin(std::size_t i, std::size_t j, std::size_t k);
		TetMesh& setTransformationMatrix(const glm::mat4& transfoMat);
		TetMesh& updatePositions(void);
		TetMesh& resetPositions(void);
		TetMesh& updateValues(void);
		TetMesh& printInfo(void);
		std::vector<glm::vec4> getVertices(void) const;
		std::vector<unsigned char> getVertexValues(void) const;
	private:
		const TextureStorage* const texLoader;

		glm::vec4 origin; /// @brief queried point
		unsigned char originalValue;
		glm::mat4 transformationMatrix;
		glm::mat4 inverseTransformationMatrix;

		std::vector<glm::vec4> vertices;
		std::vector<unsigned char> vertexValues;
		std::vector<std::vector<std::size_t>> tetrahedra;
	private:
		void makeTetrahedra(void);
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_TETMESH_HPP_
