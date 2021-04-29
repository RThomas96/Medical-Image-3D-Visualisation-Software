#ifndef VISUALIZATION_IMAGE_API_GRID_HPP_
#define VISUALIZATION_IMAGE_API_GRID_HPP_

#include "image_api_common.hpp"
#include "backend.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Image {

	class Grid : public std::enable_shared_from_this<Grid> {
		public:
			/// @b Handy typedef for a pointer to a grid
			typedef std::shared_ptr<Grid> Ptr;

		protected:
			/// @b Default grid ctor, made protected so it is not instanciated directly.
			Grid(ImageBackendImpl::Ptr _backend);

			/// @b Creates a grid, downsampled from their parent.
			explicit Grid(Grid::Ptr parent, svec3 size);

			/// @b Creates a grid from a sub-region of this grid.
			explicit Grid(Grid::Ptr parent, svec3 begin, svec3 end);

		public:
			/// @b Default dtor of the Grid class. De-allocates averything associated with the grid.
			virtual ~Grid(void) = default;

			//////////////////////////////////////////////////////
			//													//
			//    GRID CONSTRUCTION FROM MULTIPLE FILENAMES		//
			//													//
			//////////////////////////////////////////////////////

			/// @b Static function which automatially creates a grid of the right type.
			/// @details If such a grid cannot be created, returns nullptr. This function will iterate over the known
			/// image backends to try and generate a right grid object which can read the data from the images as the
			/// user requested.
			/// @warning For now, it is a huge if/else mega function which returns a suitable grid type based on
			/// extensions of the first few files given. Should be re-done in the future (is not suitable when many
			/// filetypes are supported).
			static Grid::Ptr createGrid(std::vector<std::vector<std::string>>& _filenames);

			//////////////////////////////////////////////////////
			//													//
			//               GRID HIERARCHY CONTROL             //
			//													//
			//////////////////////////////////////////////////////

			/// @b Returns true if the grid has no parent, false otherwise.
			bool isRootGrid(void) const;

			/// @b Returns the parent grid, or nullptr if the grid is a 'root' grid.
			Grid::Ptr getParentGrid(void) const;

			/// @b Creates a new grid, which will be a sub-region of this grid
			Grid::Ptr requestSubRegion(svec3 begin, svec3 end);

			/// @b Creates a new grid, which will be a downsampled version of this grid
			Grid::Ptr requestDownsampledVersion(svec3 target_size);

			//////////////////////////////////////////////////////
			//													//
			//            GETTERS FOR GRID PROPERTIES           //
			//													//
			//////////////////////////////////////////////////////

			/// @b Checks the grid holds an image implementation, and that this implementation is valid.
			constexpr bool hasValidImplementation(void) const;

			/// @b Simply calls the parse function for the image implementation^
			ThreadedTask::Ptr updateInfoFromDisk(void);

			/// @b Returns the internal data type represented by the grid.
			ImageDataType getInternalDataType(void) const;

			/// @b Returns the number of components in each voxel of this grid.
			std::size_t getVoxelDimensionality(void) const;

			/// @b Returns the voxel dimensions, from the info provided by the image implementation.
			glm::vec3 getVoxelDimensions(void) const;

			/// @b Returns the resolution of the grid, as read by the backend implementation.
			svec3 getResolution(void) const;

			/// @b Returns the name of this grid, if applicable.
			std::string getImageName(void) const;

			//////////////////////////////////////////////////////
			//													//
			//			  READ GRID DATA INTO A BUFFER			//
			//													//
			//////////////////////////////////////////////////////

			/// @b Template to return the minimum and maximum values stored in the file, if given.
			/// @note By default, returns the internal data type's min and max values.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> bool getRangeValues(glm::vec<2, data_t, glm::defaultp>& _range);

			/// @b Template to read a single pixel's value(s) in the image.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> bool readPixel(svec3 index, std::vector<data_t>& values);

			/// @b Template to read a single line of voxels in ihe image.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> bool readLine(svec2 line_idx, std::vector<data_t>& values);

			/// @b Template to read a whole slice of voxels in the image at once.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> bool readSlice(std::size_t slice_idx, std::vector<data_t>& values);

			template <typename data_t> bool readData(std::vector<data_t>& data) {
				return this->pImpl->readData(data);
			}

		protected:
#ifdef PIMPL_USE_EXPERIMENTAL_PROPAGATE_CONST
			/// @b The opaque pointer which will perform all the logic in regards to the reading of data.
			std::experimental::propagate_const<ImageBackendImpl::Ptr> pImpl;
#else
			/// @b The opaque pointer which will perform all the logic in regards to the reading of data.
			ImageBackendImpl::Ptr pImpl;
#endif
			/// @b The grid from which this one is either a downsampled version or a sub-region.
			Grid::Ptr parentGrid;
			/// @b If the grid has a parent, stores the size of this current grid (whether sub-region, or downsampled).
			svec3 imageSize;
			/// @b If the grid is a sub-region of a bigger grid, this will be its starting point in the parent.
			svec3 voxelOffset;
			/// @b This grid's name,
			std::string gridName;
	};

	template <typename data_t>
	bool Grid::readPixel(svec3 index, std::vector<data_t>& values) {
		// Checks the position is valid, the backend implementation is valid and returns the value
		if (index.x < this->imageSize.x && index.y < this->imageSize.y && index.z < this->imageSize.z) {
			if (this->pImpl) {
				this->pImpl->readPixel<data_t>(index, values);
				return true;
			}
			return false;
		}
		// position is out of bounds :
		return false;
	}

	template <typename data_t>
	bool Grid::readLine(svec2 index, std::vector<data_t>& values) {
		// Checks the position requested is valid, then calls the implementation's function if valid.
		if (index.x < this->imageSize.y && index.y < this->imageSize.z) {
			if (this->pImpl) {
				this->pImpl->readLine<data_t>(index, values);
				return true;
			}
			return false;
		}
		return false;
	}

	template <typename data_t>
	bool Grid::readSlice(std::size_t slice_idx, std::vector<data_t>& values) {
		// Checks the slice index is valid, then reads it if the implementation is valid :
		if (slice_idx < this->imageSize.z) {
			if (this->pImpl) {
				this->pImpl->readSlice<data_t>(slice_idx, values);
				return true;
			}
			return false;
		}
		return false;
	}

}

#endif // VISUALIZATION_IMAGE_API_GRID_HPP_
