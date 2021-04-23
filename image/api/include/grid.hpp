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

			/// @b Returns the internal data type represented by the grid.
			ImageDataType getInternalDataType(void) const;

			/// @b Returns the number of components in each voxel of this grid.
			std::size_t getVoxelDimensionality(void) const;

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
			template <typename data_t> void getRangeValues(glm::vec<2, data_t, glm::defaultp>& _range);

			/// @b Template to read a single pixel's value(s) in the image.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> void readPixel(svec3 index, std::vector<data_t>& values);

			/// @b Template to read a single line of voxels in ihe image.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> void readLine(svec2 line_idx, std::vector<data_t>& values);

			/// @b Template to read a whole slice of voxels in the image at once.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> void readSlice(std::size_t slice_idx, std::vector<data_t>& values);

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

}

#endif // VISUALIZATION_IMAGE_API_GRID_HPP_
