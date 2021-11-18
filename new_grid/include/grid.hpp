#ifndef VISUALIZATION_IMAGE_API_GRID_HPP_
#define VISUALIZATION_IMAGE_API_GRID_HPP_

#include "../../image/generic/include/generic_image_reader.hpp"
#include "../../image/transforms/include/transform_stack.hpp"
#include "../../image/utils/include/image_api_common.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Image {

	/// @ingroup newgrid
	/// @brief The Grid class is the new and recommended representation of a voxel grid.
	/// @details Due to the breadth of file types and data types that the program must support, the decision to overhaul
	/// the DiscreteGrid set of classes was taken, and this is what resulted. This class uses the pImpl idiom in order
	/// to provide a stable, non-templated function interface to an underlying image representation.
	/// @note Some parts of this function interface are subject to change as it develops.
	class Grid : public std::enable_shared_from_this<Grid> {
	public:
		/// @brief Handy typedef for a pointer to a grid
		typedef std::shared_ptr<Grid> Ptr;

	protected:
		/// @brief Default grid ctor, made protected so it is not instanciated directly.
		Grid(GenericImageReader::Ptr _backend);

		/// @brief Creates a grid, downsampled from their parent.
		explicit Grid(Grid::Ptr parent, svec3 size, ImageResamplingTechnique sampler);

		/// @brief Creates a grid from a sub-region of this grid.
		explicit Grid(Grid::Ptr parent, svec3 begin, svec3 end);

	public:
		/// @brief Default dtor of the Grid class. De-allocates averything associated with the grid.
		virtual ~Grid(void) = default;

		//////////////////////////////////////////////////////
		//													//
		//    GRID CONSTRUCTION FROM MULTIPLE FILENAMES		//
		//													//
		//////////////////////////////////////////////////////

		/// @brief Static function which automatially creates a grid of the right type.
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

		/// @brief Returns true if the grid has no parent, false otherwise.
		bool isRootGrid(void) const;

		/// @brief Returns the parent grid, or nullptr if the grid is a 'root' grid.
		Grid::Ptr getParentGrid(void) const;

		/// @brief Creates a new grid, which will be a sub-region of this grid
		Grid::Ptr requestSubRegion(svec3 begin, svec3 end);

		/// @brief Creates a new grid, which will be a downsampled version of this grid
		Grid::Ptr requestDownsampledVersion(svec3 target_size, ImageResamplingTechnique sampler);

		//////////////////////////////////////////////////////
		//													//
		//            GETTERS FOR GRID PROPERTIES           //
		//													//
		//////////////////////////////////////////////////////

		/// @brief Checks the grid holds an image implementation, and that this implementation is valid.
		constexpr bool hasValidImplementation(void) const;

		/// @brief Simply calls the parse function for the image implementation
		ThreadedTask::Ptr updateInfoFromDisk(const std::vector<std::vector<std::string>>& filenames);

		/// @brief Once the information from the implementation is parsed, update the internal contents of the grid structure.
		void updateInfoFromGrid(void);
		// #warning TODO : replace updateInfoFromGrid with a more user-friendly interface (wouldnt have to do this)

		/// @brief Returns the internal data type represented by the grid.
		ImageDataType getInternalDataType(void) const;

		/// @brief Returns the number of components in each voxel of this grid.
		std::size_t getVoxelDimensionality(void) const;

		/// @brief Returns the voxel dimensions, from the info provided by the image implementation.
		glm::vec3 getVoxelDimensions(void) const;

		/// @brief Returns the resolution of the grid, as read by the backend implementation.
		svec3 getResolution(void) const;

		/// @brief Returns the name of this grid, if applicable.
		std::string getImageName(void) const;

		/// @brief Returns the bounding box of the currently loaded information
		Image::bbox_t getBoundingBox(void) const;

		//////////////////////////////////////////////////////
		//													//
		//           GETTERS FOR GRID TRANSFORM(S)	        //
		//													//
		//////////////////////////////////////////////////////

		/// @brief Returns the current transform stack, with all used transforms.
		TransformStack::Ptr getTransformStack() const;

		/// @brief Returns the transform stack's precomputed matrix
		MatrixTransform::Ptr getPrecomputedMatrix() const;

		/// @brief Add a transform to the transform stack.
		void addTransform(ITransform::Ptr _transform_to_add);

		//////////////////////////////////////////////////////
		//													//
		//			  READ GRID DATA INTO A BUFFER			//
		//													//
		//////////////////////////////////////////////////////

		/// @brief Template to return the minimum and maximum values stored in the file, if given.
		/// @note By default, returns the internal data type's min and max values. Not all filetypes carry the
		/// information about the min and max sample values.
		template <typename data_t>
		bool getRangeValues(std::size_t channel, glm::vec<2, data_t, glm::defaultp>& _range);

		/// @brief Template to read a single pixel's value(s) in the image.
		template <typename data_t>
		bool readPixel(svec3 index, std::vector<data_t>& values);

		/// @brief Template to read a single line of voxels in ihe image.
		template <typename data_t>
		bool readLine(svec2 line_idx, std::vector<data_t>& values);

		/// @brief Template to read a whole slice of voxels in the image at once.
		template <typename data_t>
		bool readSlice(std::size_t slice_idx, std::vector<data_t>& values);

		/// @brief Template to read a subregion of voxels in the image.
		template <typename data_t>
		bool readSubRegion(svec3 origin, svec3 size, std::vector<data_t>& values);

	protected:
		/// @brief The opaque pointer which will perform all the logic in regards to the reading of data.
		std::experimental::propagate_const<GenericImageReader::Ptr> pImpl;
		/// @brief The grid from which this one is either a downsampled version or a sub-region.
		Grid::Ptr parentGrid;
		/// @brief If the grid has a parent, stores the size of this current grid (whether sub-region, or downsampled).
		svec3 imageSize;
		/// @brief If the grid is a sub-region of a bigger grid, this will be its starting point in the parent.
		svec3 voxelOffset;
		/// @brief Transform stack for the grid
		TransformStack::Ptr grid_transforms;
	};

}	 // namespace Image

// Template definitions of the grid class :
#include "./grid.impl.hpp"

#endif	  // VISUALIZATION_IMAGE_API_GRID_HPP_
