#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_PRIVATE_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_PRIVATE_HPP_

#include "./tiff_frame.hpp"

#include "../../api/include/image_api_common.hpp"
#include "../../api/include/read_cache.hpp"
#include "../../api/include/threaded_task.hpp"

#include <tiff.h>
#include <tiffio.h>

#include <string>
#include <vector>
#include <memory>

namespace Image {
namespace Tiff {

	/**
	 * @brief The TIFFPrivate class is an interface to the backend reader implementation of the TIFFBackend class.
	 * @details This class is only concerned with the _reading_ of TIFF files. As such, it will not parse image
	 * files itself. This part will be handled by the superclass (Image::TIFFBackend).
	 */
	class TIFFPrivate {
		public:
			/// @b Simple typedef which only allows the TIFFPrivate class to be owned by one class only.
			typedef std::unique_ptr<TIFFPrivate> Ptr;

			/// @b Simple typedef which references an image (a collection of single-plane TIFF frames)
			typedef std::vector<Frame::Ptr> TIFFImage;

		protected:
			/// @b Default ctor.
			TIFFPrivate();

		public:
			/// @b Default dtor for the backend, which releases the cached data.
			virtual ~TIFFPrivate(void) = default;

			/// @b Resizes the internal buffer of frames to the specified amount
			TIFFPrivate& resizeImages(std::size_t imgcount);

			/// @b Sets the image 'idx' to the contents of 'img'.
			TIFFPrivate& setImage(std::size_t idx, TIFFImage img);

			/// @b Appends a frame to an existing image.
			TIFFPrivate& appendFrame(std::size_t idx, Frame::Ptr fr);

			/// @b Add a fully specified image to the stack.
			TIFFPrivate& addImage(TIFFImage& img);

			/// @b Sets the voxel dimensions, if the file format specifies it.
			TIFFPrivate& setDimensionality(std::size_t dim);

			/// @b returns the internal type represented by the images.
			ImageDataType getInternalType(void) const;

			/// @b If the value range of the image is defined elsewhere (like OME-TIFF specifies) we can set it here.
			template <typename source_t> void setRangeValues(glm::vec<2, source_t, glm::defaultp>& _range);

			/// @b Returns the min and max values in the image, or representable by the internal type.
			template <typename target_t> void getRangeValues(glm::vec<2, target_t, glm::defaultp>& _range) const;

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
			/// @b The internal data type represented by the struct.
			ImageDataType internal_data_type;

			/// @b The number of components in a voxel for this image.
			std::size_t voxel_dimensionalty;

			/// @b The loaded frames, which are created by the parsing function.
			std::vector<TIFFImage> images;
	};

}
}

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_PRIVATE_HPP_
