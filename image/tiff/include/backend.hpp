#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HPP_

#include "./tiff_frame.hpp"
#include "./tiff_private.hpp"

#include "../../api/include/image_api_common.hpp"
#include "../../api/include/backend.hpp"
#include "../../api/include/read_cache.hpp"
#include "../../api/include/threaded_task.hpp"

namespace Image {

	/**
	 * @brief The TIFFBackend class allows to read information from one or more TIFF (virtual) stacks directly.
	 * @details This class will parse the TIFF files, generate adequate frame descrptions for each of the detected
	 * frames and allow random access to data, no matter its dimensionality, number of slices or resolution.
	 * @warning This class supports only reading data from single-channel TIFF frames. If more channels are requested,
	 * those additional channels will have to be provided using another TIFF frames (multiple _frames_ can live inside
	 * a single TIFF _file_).
	 */
	class TIFFBackend : public ImageBackendImpl {
		protected:
			/// @b Default ctor for the implementation, simply calling its superclass ctor.
			TIFFBackend(std::vector<std::vector<std::string>> fns) : ImageBackendImpl(fns) {}

		public:
			/// @b Default dtor for the class
			virtual ~TIFFBackend(void) = default;

			/// @b Returns true if the TIFF file provided can be read by this image implementation.
			static bool canReadImage(const std::string& image_name);

			/// @b Creates a backend implementation which can read the data, if possible. Returns nullptr otherwise.
			static ImageBackendImpl::Ptr createBackend(std::vector<std::vector<std::string>> fns);

			/// @b Simple call to parse images given in the ctor.
			virtual ThreadedTask::Ptr parseImageInfo(void) noexcept(false) override;

			/// @b Get the number of elements present in each voxel.
			virtual std::size_t getVoxelDimensionality(void) override;

			/// @b Returns the image's internal representation for data.
			virtual ImageDataType getVoxelDataType(void) override;

			/// @b Returns the image's defined voxel resolutions, if applicable.
			virtual glm::vec3 getVoxelDimensions(void) override;

			/// @b Returns the dimensions of the image.
			virtual svec3 getResolution(void) override;

			/// @b Returns the image bounding box, either as computed (voxel sizes x res), or defined in file.
			virtual BoundingBox_General<float> getBoundingBox(void) override;

		protected:
			/// @b The pointer which can interface directly with the files on disk.
			Tiff::TIFFPrivate::Ptr pImpl;
	};

} // namespace Image

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HPP_
