#ifndef GRID_INCLUDE_DISCRETE_GRID_HPP_
#define GRID_INCLUDE_DISCRETE_GRID_HPP_

#include "../../image/include/reader.hpp"
#include "./bounding_box.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <memory>

#define INSERTION_BASED_COPY

/// @brief Computes a transformation matrix from an origin and an angle, for our use case.
/// @details This computes a transformation matrix to fit our purpose, might not be adapted
/// to any use case !
glm::mat4 computeTransfoShear(double angleDeg, glm::vec3 origin = glm::vec3(.0f));

/// @brief Representation of a discrete grid (as a stack of images, or a voxel grid) which can be queried from world space.
/// @note Although some functions in this class may mention 'texels', they are in no way, shape, or form tied to the visualization aspect of the project.
class DiscreteGrid {

	friend class GridControl;

	public:
		/// @brief Definition of a 3 dimensionnal vector to store this grid's dimensions, amongst other things.
		typedef glm::vec<3, std::size_t, glm::defaultp> sizevec3;
		/// @brief Simple typedef in order to to a templat-ing of this class later.
		typedef unsigned char DataType;
		/// @brief Type of bounding box used
		typedef BoundingBox_General<float> bbox_t;

	public:
		/// @brief Default constructor, creates an empty grid.
		DiscreteGrid(bool _modifiable = true);

		/// @brief Creates a grid using content from a grid reader.
		DiscreteGrid(IO::GenericGridReader& reader);

		/// @brief Default destructor, removes any storage associated with the grid.
		~DiscreteGrid(void);

		/// @brief Updates this grid's data with data computed from a grid reader.
		virtual DiscreteGrid& fromGridReader(IO::GenericGridReader& reader);

		/// @brief Recomputes the bounding box surrounding data with the threshold for "data" set to "threshold".
		virtual DiscreteGrid& recomputeBoundingBox(DataType threshold);

		/// @brief Returns the given point (originally world space) in grid space.
		virtual glm::vec4 toGridSpace(glm::vec4 pos_ws) const;

		/// @brief Returns the given point (originally grid space) in world space.
		virtual glm::vec4 toWorldSpace(glm::vec4 pos_gs) const;

		/// @brief Fetches the voxel at the given position, in grid space.
		virtual DataType fetchTexelGridSpace(glm::vec4 pos_gs) const;

		/// @brief Fetches the voxel at the given position, in world space.
		virtual DataType fetchTexelWorldSpace(glm::vec4 pos_ws) const;

		/// @brief Fetches the voxel at the given position index, in the grid.
		virtual DataType fetchTexelIndex(sizevec3 idx) const;

		/// @brief Get the voxel grid's data.
		virtual const std::vector<DataType>& getData(void) const;

		/// @brief Returns the matrix used to go from world space to grid space.
		const glm::mat4& getTransform_WorldToGrid(void) const;

		/// @brief Returns the matrix used to go from grid space to world space.
		const glm::mat4& getTransform_GridToWorld(void) const;

		/// @brief Returns the resolution of the voxel grid,
		const sizevec3& getGridDimensions(void) const;

		/// @brief Voxel dimensions, in grid space.
		const glm::vec3 getVoxelDimensions(void) const;

		/// @brief Gets the bounding box of the grid.
		const bbox_t& getBoundingBox(void) const;

		/// @brief Get the data's bounding box.
		const bbox_t& getDataBoundingBox(void) const;

		/// @brief Gets the bounding box of the grid.
		bbox_t getBoundingBoxWorldSpace(void) const;

		/// @brief Checks if the grid is modifiable, for controllers
		bool isModifiable(void) const;

		/// @brief Returns the position of the voxel asked for in grid space.
		/// @note Returns the position of the __center__ of the voxel.
		virtual glm::vec4 getVoxelPositionGridSpace(sizevec3 idx);

		/// @brief Returns the position of the voxel asked for in world space.
		/// @note Returns the position of the __center__ of the voxel.
		virtual glm::vec4 getVoxelPositionWorldSpace(sizevec3 idx);

		/// @brief Sets the associated data with the discrete grid.
		virtual DiscreteGrid& setData(std::vector<DataType>& _data);

		/// @brief Sets the grid to be modifiable or not. Can be changed in daughter classes.
		virtual DiscreteGrid& setModifiable(bool _mod = true);

		/// @brief Sets the grid's resolution.
		virtual DiscreteGrid& setResolution(sizevec3 dims);

		/// @brief Sets the bounding box of the discrete grid.
		virtual DiscreteGrid& setBoundingBox(bbox_t renderWindow);

		/// @brief Updates the bounding box of the discrete grid with another bounding box.
		virtual DiscreteGrid& updateBoundingBox(bbox_t renderWindow);

		/// @brief Sets the associated transform from world space to grid space.
		virtual DiscreteGrid& setTransform_WorldToGrid(glm::mat4 _w2g);

		/// @brief Sets the associated transform from grid space to world space.
		virtual DiscreteGrid& setTransform_GridToWorld(glm::mat4 _g2w);

		/// @brief Sets the grid's name, to be identified in a GUI window.
		virtual DiscreteGrid& setGridName(std::string name);

		/// @brief Get the grid name, in order to indentify them in a controller
		const std::string& getGridName(void) const;

		/// @brief Checks if the grid contains the point given in world space.
		/// @details Allows to check whether we need to sample from this grid or not. If the point
		/// (defined in world space) is contained within the grid's bounding box when transformed
		/// in grid space, then this function returns true. Returns false otherwise.
		bool includesPointWorldSpace(glm::vec4 point) const;

		/// @brief Checks if the grid contains the point given in world space.
		/// @details Allows to check whether we need to sample from this grid or not. If the point
		/// (defined in grid space) is contained within the grid's bounding box then this function
		/// returns true. Returns false otherwise.
		bool includesPointGridSpace(glm::vec4 point) const;

	protected:
		/// @brief Updates the voxel dimensions of the grid, each time the BB or the resolution changes.
		void updateVoxelDimensions(void);

	protected:
		/// @brief Checks if the grid's properties can be modified (data will always be modifiable, dimensions might not)
		/// @details Output grids can be modified, input grids however, cannot.
		bool modifiable;
		/// @brief Stores the data associated with the grid.
		/// @details Arranged in order : X, Y, Z. Meaning, we first get a 'width'-sized
		/// array of values, followed by 'height'-sized arrays of 'width'-sized arrays
		/// of values, 'depth' times. To get to the value at (x, y, z), you query the
		/// value at index (x + y*width + z*width*depth).
		std::vector<DataType> data;
		/// @brief Stores the dimensions in this order : width, height, depth.
		sizevec3 gridDimensions;
		/// @brief Voxel dimensions along the X, Y, and Z axis in grid space.
		glm::vec3 voxelDimensions;
		/// @brief The matrix used to go from world space to grid space.
		/// @details Usually precomputed from transform_gridToWorld's inverse, but can be set
		/// manually (which will trigger an update of transform_gridToWorld as well).
		glm::mat4 transform_worldToGrid;
		/// @brief The matrix used to go from grid space to world space.
		glm::mat4 transform_gridToWorld;
		/// @brief The bounding box of the grid, in grid space :
		bbox_t boundingBox;
		/// @brief The threshold from which to consider a voxel as 'data', instead of the background.
		DataType dataThreshold;
		/// @brief Bounding box where data lives :
		bbox_t dataBoundingBox;
		/// @brief The name of the grid, used to identify it on a
		std::string gridName;
};

extern std::vector<std::shared_ptr<DiscreteGrid>> gridIndex;

#endif // GRID_INCLUDE_DISCRETE_GRID_HPP_
