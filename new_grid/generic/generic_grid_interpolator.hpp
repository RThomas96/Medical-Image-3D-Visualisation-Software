#ifndef GENERIC_GRID_INTERPOLATOR_HPP_
#define GENERIC_GRID_INTERPOLATOR_HPP_

#endif	  // GENERIC_GRID_INTERPOLATOR_HPP_

/// @brief Simple enum to define the interpolation type to use in the generation of the grid by InterpolationMesh.
enum InterpolationMethods {
	NearestNeighbor,
	TriLinear
};

/// @brief Represents a tetrahedral mesh in initial space, to interpolate a voxel's value at a given position.
/// @details This class represents a tetrahedral mesh which vertices are the dual of a voxel's direct neighbors.
/// You can analyse the neighbors of a voxel by supplying it a position, which will update the positions and
/// the values of each member of the mesh. Right now, the class only supports Nearest-Neighbor querying of
/// voxels, due to the limitations of the TextureStorage class.
class GridInterpolator {
public:
	/// @brief Pointer type for this class.
	using Ptr = std::shared_ptr<GridInterpolatorTemplated>;

public:
	/// @brief Constructs a mesh, devoid of any associated image stack.
	GridInterpolator(void);

	/// @brief Returns the currently associated input grids
	/// @return The input grids.
	std::vector<std::shared_ptr<InputGrid>> getInputGrids() const;

	/// @brief Set the grid to sample data into from the input grids.
	/// @details Sets the positions of the mesh vertices when the output mesh is set.
	/// @param toSet A raw pointer to the grid to populate for the reconstruction.
	/// @returns A reference to (this), to chain function calls.
	// GridInterpolator& setOutputGrid(const std::shared_ptr<OutputGrid>& toSet);

	/// @brief Set the grid to sample data into from the input grids.
	/// @details Sets the positions of the mesh vertices when the output mesh is set.
	/// @param toSet A raw pointer to the grid to populate for the reconstruction.
	/// @returns A reference to (this), to chain function calls.
	// GridInterpolator& setOutputGrid_raw(const std::shared_ptr<DiscreteGrid>& toSet);

	/// @brief Populate the output grid with data from the input grids.
	/// @param method The interpolation method used to determine the values of the neighbor grid.
	GridInterpolator& populateOutputGrid(InterpolationMethods method);

	/// @brief Populate the output grid with data from the input grids.
	/// @param method The interpolation method used to determine the values of the neighbor grid.
	GridInterpolator& populateOutputGrid_threaded(InterpolationMethods method, IO::ThreadedTask::Ptr&);

	/// @brief Populate the output grid with data from the input grids.
	/// @param method The interpolation method used to determine the values of the neighbor grid.
	GridInterpolator& populateOutputGrid_RGB(InterpolationMethods method);

	/// @brief Returns the interpolated value from 'grid', interpolated using 'method'
	/// @param grid The grid to sample data from
	/// @param method The interpolation method used to determine the value at the given point.
	/// @param idx The index of the voxel to fetch the value from
	data_t getInterpolatedValue(std::shared_ptr<InputGrid> grid, InterpolationMethods method, DiscreteGrid::sizevec3 idx, bool verbose = false) const;

	/// @brief Prints info about the current position and values of the neighbor grid.
	/// @returns A reference to (this), to chain function calls.
	GridInterpolator& printInfo(void);

	/// @brief Returns the rate at which we can generate voxels.
	double getGenerationRate(void) const;

	/// @brief Destructs the mesh.
	~GridInterpolator(void);

protected:
	glm::vec4 origin;	 ///< Position of the mesh's origin.
	glm::vec4 origin_WS;	///< Position of the origin of the mesh, always in world space.
	std::vector<glm::vec4> vertices;	///< Positions of the neighboring vertices

	std::vector<std::vector<std::size_t>> tetrahedra;	 ///< Tetrahedra, each represented as the index of the vertices making it up stored in an array

	/// @brief The rate at which we can generate voxels.
	/// @details Taken as the time to iterate over all images, excluding the time to write said images to disk.
	double generationRate;

private:
	/// @brief Builds the mesh around the origin. Only called once, in the constructor.
	/// @param vxdims Dimensions of a voxel
	/// @param size The size of the neighborhood to create (1 for a 3-wide cube, 2 for a 5-wide, and so on)
	void makeTetrahedra(glm::vec3 vxdims = glm::vec3(1.f), std::size_t size = 1);

	/// @brief Colour the given data with a pseudo-h&e colouring.
	/// @param _r Value for R channel
	/// @param _b Value for B channel
	/// @param _gr Grid pointer for R channel
	/// @param _gb Grid pointer for B channel
	/// @return A vec3 representing a RGB triplet in linear space.
	glm::vec<3, data_t, glm::defaultp> h_and_e_colouring(data_t _r, std::shared_ptr<InputGrid>& _rg, data_t _b, std::shared_ptr<InputGrid>& _bg);

	/// @brief Updates the relative positions of the grid to the sizes of the outputgrid's voxels.
	//  GridInterpolator& updateVoxelSizes(void);

	/// @brief Updates the output grid's size, resolution (...) when an input grid is added, or when an output grid is set.
	//  GridInterpolator& updateOutputGridData(void);

	/// @brief Get the real position of the vertex at index 'idx'.
	/// @details Since the neighborhood grid never really moves, and is only offset by a certain amount,
	/// we need to be able to get the real position they occupy in the grid space they belong in. Returns
	/// the position in the mesh, added to the origin.
	/// @returns The position of the vertex, in the space the TetMesh belongs in.
	glm::vec4 getVertexPosition(std::size_t idx) const;

	/// @brief Gets the value at vertex 'idx' from the grid in argument.
	/// @returns The value at the vertex 'idx' in this mesh, in the grid queried.
	data_t getVertexValue(const std::shared_ptr<InputGrid> grid, std::size_t idx, bool verbose = false) const;

protected:
	GridInterpolator::data_t interpolate_NearestNeighbor(const std::shared_ptr<InputGrid> grid, bool verbose = false) const;	///< Interpolates a given point in initial space with the Nearest Neighbor technique
	GridInterpolator::data_t interpolate_TriLinear(const std::shared_ptr<InputGrid> grid, bool verbose = false) const;	  ///< Interpolates a given point in initial space with the Trilinear technique
};