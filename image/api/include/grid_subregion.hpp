#ifndef VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_HPP_

#include "./grid.hpp"
#include "./backend.hpp"

namespace Image {

	/// @b The GridSubRegion class is the base class for all versions of a backend sampling a sub region of a grid.
	class GridSubregion : public ImageBackendImpl {
		public:
			/// @b Pointer type for this backend image implementation.
			typedef std::unique_ptr<GridSubregion> Ptr;

		protected:
			/// @b Default ctor for the class, creates a backend that will sample the given grid in the given region.
			GridSubregion(svec3 origin, svec3 size, Grid::Ptr parent_grid);

		public:
			/// @b Dtor for the class. Releases all allocated resources.
			virtual ~GridSubregion(void) = default;

			/// @b Returns the internal type of the backend, based on the internal type of the grid sampled.
			virtual ImageDataType getInternalDataType(void) const override;

			/// @b Should parse images, but since we're sampling a known grid this just initializes the right variables.
			virtual ThreadedTask::Ptr parseImageInfo(ThreadedTask::Ptr pre_existing_task,
													 const std::vector<std::vector<std::string>>& _filenames
													 ) noexcept(false) override;

			/// @b Checks if the information present in this implementation is present on disk, or in memory.
			virtual bool presentOnDisk(void) const override;

			/// @b Returns the number of channels of the image
			virtual std::size_t getVoxelDimensionality(void) const override;

			/// @b Returns the image's defined voxel resolutions, if applicable.
			virtual glm::vec3 getVoxelDimensions(void) const override;

			/// @b Returns the dimensions of the image.
			virtual svec3 getResolution(void) const override;

			/// @b Allows to get the name of the loaded image(s).
			/// @details If the file format does not support defining the name of the grid in its files or metadata
			/// (like the TIFF format for example), then the name returned is either a previously user-defined name, or
			/// the name of the first image/file loaded.
			virtual std::string getImageName(void) const override;

			/// @b Allows for the user to specify a custom name for the grid.
			virtual void setImageName(std::string& _user_defined_name_) override;

			/// @b Returns the image bounding box, either as computed (voxel sizes x res), or defined in file.
			virtual BoundingBox_General<float> getBoundingBox(void) const override;

		protected:
			/// @b The origin of the sampling region, expressed as voxel indices.
			svec3 sampling_region_origin;

			/// @b The size of the sampling region, expressed as voxel indices.
			svec3 sampling_region_size;

			/// @b The grid to sample data from.
			Grid::Ptr parent_grid;

			/// @b This image's name. By default, is set to <parent grid name>_subregion but can be user-defined.
			std::string custom_name;
	};

} // namespace Image

#endif // VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_HPP_
