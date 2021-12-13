#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_HPP_
#define VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_HPP_

#include "../../../new_grid/include/grid.hpp"
#include "../include/generic_image_interpolators.hpp"
#include "./generic_image_reader.hpp"

namespace Image {

	/// @ingroup newgrid
	/// @brief The GenericImageDownsampler class provides a downsampled version of a parent grid to the user.
	/// @details Its design follows the GenericImageReaderSubRegion class, in that it takes as input a single variable: the desired size of the grid.
	/// The downsampling method will be defined by the template argument passed in the specific instanciation of a downsampled grid.
	class GenericImageDownsampler : public GenericImageReader {
	public:
		/// @brief Pointer type for this backend image implementation.
		typedef std::unique_ptr<GenericImageDownsampler> Ptr;

	protected:
		/// @brief Default ctor for the class. Constructs a grid representation at the given resolution.
		GenericImageDownsampler(const svec3 desired_resolution, Grid::Ptr parent_grid);

	public:
		/// @brief Default dtor for this backend image representation.
		virtual ~GenericImageDownsampler() = default;

		/// @brief Returns the internal type of the backend, based on the internal type of the grid sampled.
		virtual ImageDataType getInternalDataType() const override;

		/// @brief Checks if the information present in this implementation is present on disk, or in memory.
		virtual bool presentOnDisk(void) const override;

		/// @brief Returns the number of channels of the image
		virtual std::size_t getVoxelDimensionality(void) const override;

		/// @brief Returns the image's defined voxel resolutions, if applicable.
		virtual glm::vec3 getVoxelDimensions(void) const override;

		/// @brief Returns the dimensions of the image.
		virtual svec3 getResolution(void) const override;

		/// @brief Allows to get the name of the loaded image(s).
		/// @details If the file format does not support defining the name of the grid in its files or metadata
		/// (like the TIFF format for example), then the name returned is either a previously user-defined name, or
		/// the name of the first image/file loaded.
		virtual std::string getImageName(void) const override;

		/// @brief Allows for the user to specify a custom name for the grid.
		virtual void setImageName(std::string& _user_defined_name_) override;

		/// @brief Returns the image bounding box, either as computed (voxel sizes x res), or defined in file.
		virtual BoundingBox_General<float> getBoundingBox(void) const override;

	protected:
		/// @brief Downsamples all source images in a separate thread and fills the local cache.
		virtual void downsample_in_separate_thread(ThreadedTask::Ptr progress_tracker) = 0;

	protected:
		/// @brief Target resolution desired by the user.
		svec3 target_resolution;

		/// @brief Source resolution of the grid
		svec3 source_resolution;

		/// @brief Voxel sizes, computed at creation time.
		glm::vec3 voxel_sizes;

		/// @brief Voxel element size (1/2/3D, more ?)
		std::size_t voxel_dimensionality;

		/// @brief Parent grid to sample from
		Grid::Ptr parent_grid;

		/// @brief The image's name. By default, is set to <parent grid name>_downsampled but can be user-defined.
		std::string custom_name;
	};

	namespace Downsampled {

		/// @brief Create a backend of the desired type for a downsampled region of an image.
		GenericImageDownsampler::Ptr createBackend(Grid::Ptr parent, svec3 size, ImageResamplingTechnique method);

		/// @brief Return an instance of interpolator with the right type and resampling technique.
		/// @details This function is used to get the right instance of a resampling technique with a given grid type (always Image::Grid in this
		/// context) and a given pixel data type (float, u16, i32 ...). The functor returned will be used in the constructor of a
		/// GenericImageDownsampledTemplated<> instance.
		/// @tparam element_t The type of the pixel data contained in the grid (float, u16, i32 ...).
		/// @param technique An enum value (of type ImageResamplingTechnique) that determines which resampling algorithm will be applied to the grid.
		/// @returns A functor suitable for use within a ctor of a GenericImageDownsampledTemplated<> instance.
		template <typename element_t>
		resampler_functor<element_t, Image::Grid> findRightInterpolatorType(ImageResamplingTechnique technique) {
			if (technique & ImageResamplingTechnique::None) {
				return Interpolators::null_interpolator<element_t, Grid>;
			}
			if (technique & ImageResamplingTechnique::NearestNeighbor) {
				return Interpolators::nearest_neighbor_interpolator<element_t>;
			}
			if (technique & ImageResamplingTechnique::Linear) {
				return Interpolators::linear_interpolator<element_t>;
			}
			if (technique & ImageResamplingTechnique::Cubic) {
				return Interpolators::null_interpolator<element_t, Grid>;
			}

			// No matching value, print an error and return a default interpolator type :
			std::cerr << "Error : trying to find a right interpolator with no valid enum values given (value : "
					  << std::hex << (int) technique << std::dec
					  << ")\n";
			return Interpolators::null_interpolator<element_t, Grid>;
		}

	}	 // namespace Downsampled

}	 // namespace Image

#endif	  // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_HPP_
