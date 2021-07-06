#ifndef VISUALIAZTION_IMAGE_TIFF_INCLUDE_BACKEND_HPP_
#define VISUALIAZTION_IMAGE_TIFF_INCLUDE_BACKEND_HPP_

#include "../../api/include/backend.hpp"
#include "./tiff_frame.hpp"

namespace Image {

namespace Tiff {

	/// @b The base class for the TIFF template
	class TIFFBackendImpl : public ImageBackendImpl {
		public:
			/// @b A simple typedef which restricts the class to be owned by one object only
			typedef std::unique_ptr<TIFFBackendImpl> Ptr;

			/// @b A simple typedef which restricts the class to be owned by one object only
			typedef std::vector<Tiff::Frame::Ptr> TIFFImage;

		protected:
			/// @b Default ctor, initializes the needed variables from the
			TIFFBackendImpl(void);

		public:
			/// @b Default dtor for the	class.
			virtual ~TIFFBackendImpl(void) = default;

			/// @b Returns true if the TIFF file provided can be read by this image implementation.
			/// @note This static function only works for _basic TIFF files_. The OME-TIFF backend implementation which
			/// inherits from this class has its own implementation of this.
			static bool canReadImage(std::string& image_name);

			/// @b Simple call to parse images given in the ctor.
			virtual ThreadedTask::Ptr parseImageInfo(ThreadedTask::Ptr pre_existing_task,
													const std::vector<std::vector<std::string>>& filenames
													) noexcept(false) override;

			/// @b Get the number of elements present in each voxel.
			virtual std::size_t getVoxelDimensionality(void) const override;

			/// @b Returns the image's defined voxel resolutions, if applicable.
			virtual glm::vec3 getVoxelDimensions(void) const override;

			/// @b Returns the dimensions of the image.
			virtual svec3 getResolution(void) const override;

			/// @b Returns the internal data type represented in the images.
			virtual ImageDataType getInternalDataType(void) const override;

			/// @b Checks if the implementation is not only valid, but the data source is from the user disk (not RAM)
			virtual bool presentOnDisk(void) const override;

			/// @b Returns the name of this image, determined from the files taken as input.
			virtual std::string getImageName(void) const override;

			/// @b Allows to set a user-defined name for this acquisition.
			virtual void setImageName(std::string& _user_defined_name_) override;

			/// @b Returns the bounding box surrounding the image in its own space.
			virtual BoundingBox_General<float> getBoundingBox(void) const override;

		protected:
			/// @b Parse the filename's information into a separate thread.
			/// @note The ThreadedTask pointer is a shared resource that allows to know the progress of the task.
			virtual void parse_info_in_separate_thread(ThreadedTask::Ptr t, const std::vector<std::vector<std::string>>& _f) = 0;

			/// @b Cleans up the allocated resources after an error has occured.
			/// @note Can also be called in the dtor.
			virtual void cleanResources(void) = 0;

		protected:
			/// @b The filenames, as provided by the parsing function
			std::vector<std::string> filenames;

			/// @b The images loaded from the disk
			std::vector<TIFFImage> images;

			/// @b The voxel dimensions. In regular TIFF, it will always be a unit vector.
			glm::vec3 voxel_dimensions;

			/// @b The image's resolution, as stored on disk
			svec3 resolution;

			/// @b The number of elements per pixel
			std::size_t voxel_dimensionality;

			/// @b The basename of the stack, computed when parsing the stack.
			/// @note Can be either user-defined or derived from the stack's file names.
			std::string stack_base_name;

			/// @b The number of bits per sample in the internal representation of the file.
			uint16_t bitsPerSample;

			/// @b The sample format used (from libTIFF macros)
			uint16_t sampleFormat;

			/// @b The number of samples per pixel
			uint16_t samplesPerPixel;
	};
}

}

#endif // VISUALIAZTION_IMAGE_TIFF_INCLUDE_BACKEND_HPP_
