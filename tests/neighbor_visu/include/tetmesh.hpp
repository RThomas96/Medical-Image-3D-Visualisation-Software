#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_TETMESH_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_TETMESH_HPP_

#include "./image_storage.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <iostream>

enum InterpolationMethods {
	NearestNeighbor,
	TriLinear,
	TriCubic,
	Barycentric
};

/// TODO (1) : move the interpolation function from TextureStorage to here, we'll need it here in order to account for the user's prefered interpolation method.
/// TODO (2) : remove the const reference to a stack loader, we can only have one mesh and update it as necessary, since all stacks are in the same initial space.

/// @brief Represents a tetrahedral mesh in initial space, to interpolate a voxel's value at a given position.
/// @details This class represents a tetrahedral mesh which vertices are the dual of a voxel's direct neighbors.
/// You can analyse the neighbors of a voxel by supplying it a position, which will update the positions and
/// the values of each member of the mesh. Right now, the class only supports Nearest-Neighbor querying of
/// voxels, due to the limitations of the TextureStorage class.
class TetMesh {
	public:
		/// @brief Constructs a mesh, associated with the given stack of images.
		TetMesh(const TextureStorage* const texLoader);

		/// @brief Sets the new origin of the mesh, as an XYZ position.
		/// @details Takes the XYZ position given (in initial space) and sets
		/// the center of the mesh to the center of the nearest voxel in the grid.
		/// @returns A reference to (this), to chain function calls.
		TetMesh& setOriginInitialSpace(const glm::vec4 position);

		/// @brief Get the positions of neighboring vertices and of the origin.
		std::vector<glm::vec4> getVertices(void) const;

		/// @brief Get the values associated with each element in TetMesh::getVertices(void).
		std::vector<unsigned char> getVertexValues(void) const;

		unsigned char getInterpolatedValue(InterpolationMethods method, glm::vec4 realPosition);

		/// @brief Prints info about the current position and values of the neighbor grid.
		/// @returns A reference to (this), to chain function calls.
		TetMesh& printInfo(void);

		/// @brief Destructs the mesh.
		~TetMesh(void);
	private:
		const TextureStorage* const texLoader; ///< reference to the image stack

		glm::vec4 origin; ///< Position of the mesh's origin
		std::vector<glm::vec4> vertices; ///< Positions of the neighboring vertices
		std::vector<unsigned char> vertexValues; ///< Voxel grid values at this location
		std::vector<std::vector<std::size_t>> tetrahedra; ///< Tetrahedra, each represented as the index of the vertices making it up stored in an array
	private:
		/// @brief Builds the mesh around the origin. Only called once, in the constructor.
		void makeTetrahedra(void);

		/// @brief Updates the positions of the origin and its neighbors, updating the values as well.
		/// @returns A reference to (this), to chain function calls.
		TetMesh& updatePositions(glm::vec4 newOrigin);

		/// @brief Resets the position of the grid to the origin.
		/// @returns A reference to (this), to chain function calls.
		TetMesh& resetPositions(void);

		/// @brief Updates the values of the neighbors in the grid.
		/// @returns A reference to (this), to chain function calls.
		TetMesh& updateValues(void);
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_TETMESH_HPP_
