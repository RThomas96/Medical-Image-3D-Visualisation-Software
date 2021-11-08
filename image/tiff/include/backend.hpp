#ifndef VISUALIAZTION_IMAGE_TIFF_INCLUDE_BACKEND_HPP_
#define VISUALIAZTION_IMAGE_TIFF_INCLUDE_BACKEND_HPP_

#include "../../interface/backend.hpp"
#include "./tiff_frame.hpp"

/// @defgroup tiff TIFF file handling
/// @brief All classes that aim to interact with TIFF files.
/// @details This group contains all classes that must interact in any way, shape or form with a TIFF file. Those
/// include the image backend implementations, the writer backend implementations, and utility functions to handle files
/// written in the TIFF format.

namespace Image {

/// @brief Encloses all classes interacting with the TIFF file format.
namespace Tiff {

	/// @ingroup tiff newgrid
	/// @brief The TIFFBackendImpl class is the base class that implements the necessary logic to read TIFF images.
	/// @details This base class is then derived into template versions of this backend, which can read values directly
	/// from the images on disk into memory.
	class TIFFBackendImpl : public ImageBackendImpl {
		public:
			/// @brief A simple typedef which restricts the class to be owned by one object only
			typedef std::unique_ptr<TIFFBackendImpl> Ptr;

			/// @brief TIFFImage represents a series of TIFF directories which make up a single image in the grid.
			typedef std::vector<Tiff::Frame::Ptr> TIFFImage;

		protected:
			/// @brief Default ctor, only initializes the top-level class.
			TIFFBackendImpl(void);

		public:
			/// @brief Default dtor for the	class.
			/// @note Since no allocation or 'special' logic takes place in this class, left as `default`.
			virtual ~TIFFBackendImpl(void) = default;

			/// @brief Returns true if the TIFF file provided can be read by this image implementation.
			/// @note This static function only works for _basic TIFF files_. The OME-TIFF backend implementation which
			/// inherits from this class has its own implementation of this.
			static bool canReadImage(std::string& image_name);

			/// @brief Simple call to parse images given in the ctor.
			virtual ThreadedTask::Ptr parseImageInfo(ThreadedTask::Ptr pre_existing_task,
													const std::vector<std::vector<std::string>>& filenames
													) noexcept(false) override;

			/// @brief Get the number of elements present in each voxel.
			virtual std::size_t getVoxelDimensionality(void) const override;

			/// @brief Returns the image's defined voxel resolutions, if applicable.
			virtual glm::vec3 getVoxelDimensions(void) const override;

			/// @brief Returns the dimensions of the image.
			virtual svec3 getResolution(void) const override;

			/// @brief Returns the internal data type represented in the images.
			virtual ImageDataType getInternalDataType(void) const override;

			/// @brief Checks if the implementation is not only valid, but the data source is from the user disk (not RAM)
			virtual bool presentOnDisk(void) const override { return true; }

			/// @brief Returns the name of this image, determined from the files taken as input.
			virtual std::string getImageName(void) const override;

			/// @brief Allows to set a user-defined name for this acquisition.
			virtual void setImageName(std::string& _user_defined_name_) override;

			/// @brief Returns the bounding box surrounding the image in its own space.
			virtual BoundingBox_General<float> getBoundingBox(void) const override;

		protected:
			/// @brief Parse the filename's information into a separate thread.
			/// @param t the progress tracker for this operation
			/// @param _f the filenames to parse
			virtual void parse_info_in_separate_thread(ThreadedTask::Ptr t, const std::vector<std::vector<std::string>>& _f) = 0;

			/// @brief Cleans up the allocated resources after an error has occured.
			/// @note Can also be called in the dtor.
			virtual void cleanResources(void) = 0;

		protected:
			/// @brief The filenames, as provided by the parsing function
			std::vector<std::vector<std::string>> filenames;

			/// @brief The images loaded from the disk
			std::vector<TIFFImage> images;

			/// @brief The voxel dimensions. In regular TIFF, it will always be a unit vector.
			glm::vec3 voxel_dimensions;

			/// @brief The image's resolution, as stored on disk
			svec3 resolution;

			/// @brief The number of elements per pixel
			std::size_t voxel_dimensionality;

			/// @brief The basename of the stack, computed when parsing the stack.
			/// @note Can be either user-defined or derived from the stack's file names.
			std::string stack_base_name;

			/// @brief The number of bits per sample in the internal representation of the file.
			uint16_t bitsPerSample;

			/// @brief The sample format used (from libTIFF macros)
			uint16_t sampleFormat;

			/// @brief The number of samples per pixel
			uint16_t samplesPerPixel;
	};

	/// @brief Creates a suitable backend for the given files.
	/// @details Tries to open a reference TIFF directory (the first directory of the first file in the first stack) and
	/// attempts to create a suitable TIFFBackendImpl-derived class that can read the data from disk.
	/// @param filenames the filenames to open
	/// @returns A suitable TIFF backend for the files given.
	TIFFBackendImpl::Ptr createBackend(std::string reference_filename);

} // namespace Tiff

}

#endif // VISUALIAZTION_IMAGE_TIFF_INCLUDE_BACKEND_HPP_
