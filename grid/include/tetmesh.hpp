#ifndef GRID_INCLUDE_TETMESH_HPP_
#define GRID_INCLUDE_TETMESH_HPP_

#include "../../image/include/image_storage.hpp"
#include "./discrete_grid.hpp"
#include "./input_discrete_grid.hpp"
#include "./output_discrete_grid.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <iostream>
#include <memory>

#define DIRECT_CASTING_FROM_FETCH

#define TETMESH_OPTI __attribute__((flatten))

enum InterpolationMethods {
	NearestNeighbor,
	TriLinear,
	TriCubic,
	Barycentric
};

/// @brief Represents a tetrahedral mesh in initial space, to interpolate a voxel's value at a given position.
/// @details This class represents a tetrahedral mesh which vertices are the dual of a voxel's direct neighbors.
/// You can analyse the neighbors of a voxel by supplying it a position, which will update the positions and
/// the values of each member of the mesh. Right now, the class only supports Nearest-Neighbor querying of
/// voxels, due to the limitations of the TextureStorage class.
class TetMesh {
	public:
		// Testing to slowly template this class.
		using DataType = unsigned char;
	public:
		/// @brief Constructs a mesh, associated with the given stack of images.
		TetMesh(void);

		/// @brief Add a grid as an input to the mesh's reconstruction algorithm.
		/// @param toAdd A shared pointer to the grid to add to this mesh as an input for data reconstruction.
		/// @returns A reference to (this), to chain function calls.
		TetMesh& addInputGrid(const std::shared_ptr<InputGrid>& toAdd);

		/// @brief Set the grid to sample data into from the input grids.
		/// @details Sets the positions of the mesh vertices when the output mesh is set.
		/// @param toSet A raw pointer to the grid to populate for the reconstruction.
		/// @returns A reference to (this), to chain function calls.
		TetMesh& setOutputGrid(const std::shared_ptr<OutputGrid>& toSet);

		/// @brief Populate the output grid with data from the input grids.
		TetMesh& populateOutputGrid(InterpolationMethods method);

		/// @brief Get the positions of neighboring vertices and of the origin.
		/// @return The vertices of the mesh, in world space.
		std::vector<glm::vec4> getVertices_WorldSpace(void) const;

#if 0
		/// @brief Get the interpolated value at the specified position, using the specified interpolation method.
		/// @param pos_ws The position to query for, in world space.
		/// @param method The method to use for the interpolation.
		/// @warning This method assumes the point given is in real space. Since there's no way to check if it's true, no checks are done.
		TetMesh::DataType getInterpolatedValue(glm::vec4 pos_ws, InterpolationMethods method = InterpolationMethods::NearestNeighbor);

		/// @brief Get the interpolated value at the specified position, using the specified interpolation method.
		/// @param pos_is The position to query for, in initial space.
		/// @param method The method to use for the interpolation.
		/// @warning This method assumes the point given is in initial space. Since there's no way to check if it's true, no checks are done.
		TetMesh::DataType getInterpolatedValueInitialSpace(glm::vec4 pos_is, InterpolationMethods method = InterpolationMethods::NearestNeighbor);
#endif
		/// @brief Returns the interpolated value from 'grid', interpolated using 'method'
		/// @param grid The grid to sample data from
		/// @param method The interpolation method used to determine the value at the given point.
		/// @param idx The index of the voxel to fetch the value from
		DiscreteGrid::DataType getInterpolatedValue(std::shared_ptr<InputGrid> grid, InterpolationMethods method, DiscreteGrid::sizevec3 idx) const;

		/// @brief Prints info about the current position and values of the neighbor grid.
		/// @returns A reference to (this), to chain function calls.
		TetMesh& printInfo(void);

		/// @brief Destructs the mesh.
		~TetMesh(void);
	protected:
		std::vector<std::shared_ptr<InputGrid>> inputGrids; ///< Input grids, to sample data from
		std::shared_ptr<OutputGrid> outputGrid; ///< Output grid, to populate

		glm::vec4 origin; ///< Position of the mesh's origin.
		glm::vec4 origin_WS; ///< Position of the origin of the mesh, always in world space.
		std::vector<glm::vec4> vertices; ///< Positions of the neighboring vertices

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

		/// @brief Updates the relative positions of the grid to the sizes of the outputgrid's voxels.
		TetMesh& updateVoxelSizes(void);

		/// @brief Updates the output grid's size, resolution (...) when an input grid is added, or when an output grid is set.
		TetMesh& updateOutputGridData(void);

		/// @brief Get the real position of the vertex at index 'idx'.
		/// @details Since the neighborhood grid never really moves, and is only offset by a certain amount,
		/// we need to be able to get the real position they occupy in the grid space they belong in. Returns
		/// the position in the mesh, added to the origin.
		/// @returns The position of the vertex, in the space the TetMesh belongs in.
		glm::vec4 getVertexPosition(std::size_t idx) const;

		/// @brief Gets the value at vertex 'idx' from the grid in argument.
		/// @returns The value at the vertex 'idx' in this mesh, in the grid queried.
		DataType getVertexValue(const std::shared_ptr<InputGrid> grid, std::size_t idx) const;

	protected:
		TetMesh::DataType interpolate_NearestNeighbor(const std::shared_ptr<InputGrid> grid) const; ///< Interpolates a given point in initial space with the Nearest Neighbor technique
		TetMesh::DataType interpolate_TriLinear(const std::shared_ptr<InputGrid> grid) const; ///< Interpolates a given point in initial space with the Trilinear technique
};

#ifndef GLM_CROSS_VEC4_OVERRIDE
#define GLM_CROSS_VEC4_OVERRIDE
namespace glm {
	namespace detail {
		///! @brief Redefinition of glm::detail::compute_cross<> for vec4s instead of vec3s (for convenience)
		template<typename T, qualifier Q, bool Aligned>
		struct compute_cross_vec4
		{
			GLM_FUNC_QUALIFIER static vec<4, T, Q> call(vec<4, T, Q> const& x, vec<4, T, Q> const& y)
			{
				GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'cross' accepts only floating-point inputs");

				return vec<4, T, Q>(
					x.y * y.z - y.y * x.z,
					x.z * y.x - y.z * x.x,
					x.x * y.y - y.x * x.y, .0f);
			}
		};
	}
	///! @brief Computes glm::cross(), but with vec4s instead of vec3s (ignores last component)
	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER vec<4, T, Q> cross(vec<4, T, Q> const& x, vec<4, T, Q> const& y)
	{
		return detail::compute_cross_vec4<T, Q, detail::is_aligned<Q>::value>::call(x, y);
	}
}
#endif

#endif // GRID_INCLUDE_TETMESH_HPP_
