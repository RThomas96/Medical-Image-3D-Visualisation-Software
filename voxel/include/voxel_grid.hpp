#ifndef VISUALISATION_VOXEL_INCLUDE_VOXEL_GRID_HPP
#define VISUALISATION_VOXEL_INCLUDE_VOXEL_GRID_HPP

// Defines a voxel grid, optimized for memory size

#include <ios> // just to define std namespace
#include <vector>

// TODO : Implement offset
// TODO : define the class as a template (used in data type for the voxels)
// TODO : add some bounding box element (with bbmin and bbmax for example)
// TODO : add a clipping plane
// TODO : add some subdomains

template <typename grid_type, typename base_vector> class voxel_grid {
	public:
		/**
		 * @brief Default constructor, initializes an empty grid.
		 */
		voxel_grid();
	protected:
		/**
		 * @brief Stores the number of voxels along the X, Y, and Z axes.
		 */
		double _dimensions[3];
		/**
		 * @brief Describes the size of a voxel, in the X, Y, and Z axes.
		 */
		double _voxel_size[3];
		/**
		 * @brief The number of voxels in the grid
		 */
		std::size_t _voxel_count;
};

#endif // VISUALISATION_VOXEL_INCLUDE_VOXEL_GRID_HPP
