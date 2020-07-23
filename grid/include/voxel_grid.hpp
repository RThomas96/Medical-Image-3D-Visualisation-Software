#ifndef GRID_INCLUDE_VOXEL_GRID_HPP_
#define GRID_INCLUDE_VOXEL_GRID_HPP_

#include "./bounding_box.hpp"
#include "../../image/include/image_storage.hpp"
#include "./tetmesh.hpp"
#include "../../image/include/writer.hpp"

#include <glm/glm.hpp>

#include <QObject>

#include <iostream>
#include <vector>
#include <memory>
#include <fstream>

class GridControl; ///< Fwd-declaration of the class.
class Scene; ///< Fwd-declaration of the class.

/// @brief High precision vector3 type to store image sizes, and coordinates.
typedef glm::vec<3, std::size_t, glm::highp> svec3;

class VoxelGrid : public QObject {
	Q_OBJECT
	public:
		/// @brief Default constructor of a voxel grid. Allocated nothing, set size to 0x0x0.
		VoxelGrid(void);

		/// @brief Sized constructor of a voxel grid.
		/// @details Allocates a vector large enough to accomodate for the whole dataset we
		/// are trying to fit in it.
		VoxelGrid(std::size_t width, std::size_t height, std::size_t depth);

		/// @brief Destructor of the voxel grid. Destroys all structures and frees up memory.
		~VoxelGrid(void);

		/// @brief Sets the dimensions in voxels of the grid. If the grid was already populated, this will remove any previously computed data.
		/// @return A reference to *this, to chain function calls.
		VoxelGrid& setGridResolution(svec3 newDimensions);

		/// @brief Sets the bounding box of the image to render.
		/// @details Will be used to define the render window of the voxel grid, as well as the voxel's size.
		VoxelGrid& setRenderBoundingBox(glm::vec4 minPoint, glm::vec4 maxPoint);

		/// @brief Set the shared pointer to an image stack, to be able to sample the grid.
		VoxelGrid& setImageStack(std::shared_ptr<TextureStorage> _stack);

		/// @brief Set the shared pointer to the inspector, to be able to navigate the grid.
		VoxelGrid& setInspector(std::shared_ptr<TetMesh> _mesh);

		/// @brief Set the scene governing the voxel grid.
		VoxelGrid& setScene(Scene* _sc);

		/// @brief Iterates on each cell of the grid, and computes the grey value here.
		/// @details Renders a voxel grid of dimensions `gridDimensions` in the area
		/// defined by `renderBB`, with voxels of size `voxelDimensions`. The resulting
		/// voxel data is saved in `data`.
		VoxelGrid& populateGrid(InterpolationMethods method = InterpolationMethods::NearestNeighbor);

		/// @brief Returns the grid dimensions.
		svec3 getGridDimensions(void) const { return this->gridDimensions; }

		/// @brief Returns the voxel dimensions.
		glm::vec3 getVoxelDimensions(void) const { return this->voxelDimensions; }

		/// @brief Returns the render bounding box.
		BoundingBox_General<float> getRenderBB(void) const { return this->renderBB; }

		/// @brief Returns a read-only reference to the data vector of the voxel grid.
		const std::vector<unsigned char>& getData(void) const { return this->data; }

		/// @brief Returns the time it took to fill the grid, and only fill the grid.
		/// @details Does not take into account the time to allocate the memory needed beforehand, nor the
		/// time to load the voxel grid in OpenGL's buffers afterwards. Only the grid-filling itself is timed.
		const std::chrono::duration<double, std::ratio<1, 1>> getTimeToCompute(void) const { return this->generationDuration; }

		/// @brief Returns the position of the queried voxel.
		/// @details If the position is outside the voxel grid, it will compute the position it should have been at, but it will not be a valid position.
		glm::vec4 getVoxelPosition(std::size_t i, std::size_t j, std::size_t k) const;

		/// @brief Writes the grid to a filepath provided.
		/// This function will write the grid to a file, using an IGridWriter pointer (IGridWriter is an interface class to write the grid in many forms).
		/// If the writer writes multiple files (in the case of a multiple image writer, as the Blue dataset is) then the `path` argument will be interpreted as a template for the name.
		/// For example, if the path given is path_filename.file_extension then the files will be named path_filename_FILENUMBER.file_extension.
		VoxelGrid& writeToFile(const std::string path);

	public:
		/// @brief Sets the dimension of the grid along X
		void slotSetGridDimensionX(int newDim);
		/// @brief Sets the dimension of the grid along Y
		void slotSetGridDimensionY(int newDim);
		/// @brief Sets the dimension of the grid along Z
		void slotSetGridDimensionZ(int newDim);
		/// @brief Sets the position of the minimum point of the grid's bounding box on X
		void slotSetGridBBMinX(double newDim);
		/// @brief Sets the position of the minimum point of the grid's bounding box on Y
		void slotSetGridBBMinY(double newDim);
		/// @brief Sets the position of the minimum point of the grid's bounding box on Z
		void slotSetGridBBMinZ(double newDim);
		/// @brief Sets the position of the maximum point of the grid's bounding box on X
		void slotSetGridBBMaxX(double newDim);
		/// @brief Sets the position of the maximum point of the grid's bounding box on X
		void slotSetGridBBMaxY(double newDim);
		/// @brief Sets the position of the maximum point of the grid's bounding box on X
		void slotSetGridBBMaxZ(double newDim);
		/// @brief Sets the controller of this grid, allowing remote control of its properties.
		VoxelGrid& setController(GridControl* g) { this->controller = g; return *this; }

	protected:
		void reserveSpace(void); // Allocate the needed space for filling the data progressively.
		void computeData(InterpolationMethods method); // Compute te values to put in the grid
		void updateVoxelSizes(void); // Compute the size of voxels after renderBB or gridDimensions has changed.

	private:
		/// @brief Dimensions of the grid, in voxels or (pixels+stack depth), however you represent it.
		svec3 gridDimensions;
		/// @brief The coordinates of the axis-aligned bounding box surrounding the voxels area.
		BoundingBox_General<float> renderBB;
		/// @brief Dimensions of the voxels along the X, Y, and Z dimensions.
		glm::vec3 voxelDimensions;
		/// @brief The data generated by the voxel grid.
		std::vector<unsigned char> data;
		/// @brief Pointer to the texture stack
		std::shared_ptr<TextureStorage> imageStack;
		/// @brief Pointer to a mesh to explore the data
		std::shared_ptr<TetMesh> inspectorMesh;
		/// @brief Pointer to the grid controller, in order to modify the values of this voxel grid from outside and have a feedback loop
		GridControl* controller;
		/// @brief Pointer to a grid writer to write the grid to disk once populated.
		IO::GenericGridWriter* writer;
		/// @brief Pointer to the scene, to display the generated data once done.
		Scene* scene;
		/// @brief Time to generate a grid
		std::chrono::duration<double, std::ratio<1,1>> generationDuration;
};

#endif // GRID_INCLUDE_VOXEL_GRID_HPP_
