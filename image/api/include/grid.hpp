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

			ImageDataType getInternalDataType(void) const;

			/// @b Returns the name of this grid, if applicable.
			std::string getImageName(void) const;

			/// @b Returns true if the grid has no parent, false otherwise.
			bool isRootGrid(void) const;

			/// @b Returns the parent grid, or nullptr if the grid is a 'root' grid.
			Grid::Ptr getParentGrid(void) const;

			/// @b Creates a new grid, which will be a sub-region of this grid
			Grid::Ptr requestSubRegion(svec3 begin, svec3 end);

			/// @b Creates a new grid, which will be a downsampled version of this grid
			Grid::Ptr requestDownsampledVersion(svec3 target_size);

			/// @b Static function which automatially creates a grid of the right type.
			/// @details If such a grid cannot be created, returns nullptr. This function will iterate over the known
			/// image backends to try and generate a right grid object which can read the data from the images as the
			/// user requested.
			static Grid::Ptr createGrid(std::vector<std::vector<std::string>>& _filenames);

		protected:
			/// @b The opaque pointer which will perform all the logic in regards to the reading of data.
			ImageBackendImpl::Ptr pImpl;
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
