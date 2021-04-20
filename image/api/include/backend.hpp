#ifndef VISUALIZATION_IMAGE_API_BACKEND_HPP_
#define VISUALIZATION_IMAGE_API_BACKEND_HPP_

#include "image_api_common.hpp"
// Needed for bounding box :
#include "../../../grid/include/bounding_box.hpp"

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace Image {

	/**
	 * @brief The ImageBackendImpl class aims to provide a simple, yet usable API to query data from a set of files.
	 * @details This is only a base class, all its functionnality will be implemented in derived classes. As such, we
	 * can provide support for a wide range of data types, while having a stable API.
	 */
	class ImageBackendImpl {
		public:
			/// @b Simple typedef for a unique_ptr of an image backend.
			typedef std::unique_ptr<ImageBackendImpl> ptr;

		protected:
			/// @b Default ctor of an image backend. Declared protected to not be instanciated alone.
			ImageBackendImpl(std::vector<std::string> fns);

		public:
			/// @b Default dtor of the class : frees up all allocated resources, and returns.
			virtual ~ImageBackendImpl(void) = default;

			/// @b Simple call to parse images, the functionnality will be added in derived classes.
			virtual void parseImages(void) noexcept(false) = 0;

			/// @b Returns the number of channels of the image
			virtual std::size_t getImageDimensionality(void) = 0;

			/// @b Returns the dimensions of the image, for each channel
			virtual glm::vec<3, std::size_t, glm::defaultp> getImageResolution(void) = 0;

			/// @b Returns the image's internal representation for data.
			virtual ImageDataType getImageDataType(void) = 0;

			/// @b Returns the image's defined voxel resolutions, if applicable.
			virtual glm::vec3 getImageVoxelDimensions(void) = 0;

			/// @b Returns the image bounding box, either as computed (voxel sizes x res), or defined in file.
			virtual BoundingBox_General<float> getImageBoundingBox(void) = 0;

			/// @b Template to return the minimum and maximum values stored in the file, if given.
			/// @note By default, returns the internal data type's min and max values.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> void getImageRangeValues(glm::vec<2, data_t, glm::defaultp>& _range);

			/// @b Template to return the minimum and maximum values stored in the file, if given.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> void readPixel(svec3 index, std::vector<data_t>& values);

			/// @b Template to return the minimum and maximum values stored in the file, if given.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> void readSlice(std::size_t slice_idx, std::vector<data_t>& values);

		protected:
			/// @b The filenames which the backends must parse in order to load the image.
			std::vector<std::string> filenames;
	};

} // namespace Image

#endif // VISUALIZATION_IMAGE_API_BACKEND_HPP_
