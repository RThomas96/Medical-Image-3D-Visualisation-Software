#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_PRIVATE_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_PRIVATE_HPP_

#include "./tiff_include_common.hpp"

#include "./tiff_frame.hpp"

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
			TIFFPrivate(uint32_t w, uint32_t h);

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
			template <typename source_t> void setRangeValues(std::size_t channel, glm::vec<2, source_t, glm::defaultp>& _range);

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

		public:
			//////////////////////////////////////////////////////
			//													//
			//             SUB-REGION READ FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @b Read a sub-region of the image, implemented in the derived classes.  8-bit unsigned version.
			virtual bool tiff_readSubRegion(::Image::tag<std::uint8_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint8_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit unsigned version.
			virtual bool tiff_readSubRegion(::Image::tag<std::uint16_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint16_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit unsigned version.
			virtual bool tiff_readSubRegion(::Image::tag<std::uint32_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint32_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit unsigned version.
			virtual bool tiff_readSubRegion(::Image::tag<std::uint64_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint64_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes.  8-bit   signed version.
			virtual bool tiff_readSubRegion(::Image::tag<std::int8_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int8_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit   signed version.
			virtual bool tiff_readSubRegion(::Image::tag<std::int16_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int16_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit   signed version.
			virtual bool tiff_readSubRegion(::Image::tag<std::int32_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int32_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit   signed version.
			virtual bool tiff_readSubRegion(::Image::tag<std::int64_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int64_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes, single precision floating point.
			virtual bool tiff_readSubRegion(::Image::tag<float> tag, svec3 origin, svec3 size,
									   std::vector<float>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes, double precision floating point.
			virtual bool tiff_readSubRegion(::Image::tag<double> tag, svec3 origin, svec3 size,
									   std::vector<double>& data) = 0;

			//////////////////////////////////////////////////////
			//													//
			//          VALUE RANGE GETTER FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<std::int8_t> tag, std::size_t channel,
												glm::vec<2, std::int8_t, glm::defaultp>& values) = 0;

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<std::int16_t> tag, std::size_t channel,
												glm::vec<2, std::int16_t, glm::defaultp>& values) = 0;

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<std::int32_t> tag, std::size_t channel,
												glm::vec<2, std::int32_t, glm::defaultp>& values) = 0;

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<std::int64_t> tag, std::size_t channel,
												glm::vec<2, std::int64_t, glm::defaultp>& values) = 0;

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<std::uint8_t> tag, std::size_t channel,
												glm::vec<2, std::uint8_t, glm::defaultp>& values) = 0;

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<std::uint16_t> tag, std::size_t channel,
												glm::vec<2, std::uint16_t, glm::defaultp>& values) = 0;

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<std::uint32_t> tag, std::size_t channel,
												glm::vec<2, std::uint32_t, glm::defaultp>& values) = 0;

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<std::uint64_t> tag, std::size_t channel,
												glm::vec<2, std::uint64_t, glm::defaultp>& values) = 0;

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<float> tag, std::size_t channel,
												glm::vec<2, float, glm::defaultp>& values) = 0;

			/// @b Read the (potentially present) range of values available in the image
			virtual bool tiff_getRangeSubValues(::Image::tag<double> tag, std::size_t channel,
												glm::vec<2, double, glm::defaultp>& values) = 0;

		protected:
			/// @b The loaded frames, which are created by the parsing function.
			std::vector<TIFFImage> images;

			/// @b The internal data type represented by the struct.
			ImageDataType internal_data_type;

			/// @b The number of bits per sample in the internal representation of the file.
			uint16_t bitsPerSample;

			/// @b The sample format used (from libTIFF macros)
			uint16_t sampleFormat;

			/// @b The width of the frames
			uint32_t width;

			/// @b The height of the frames
			uint32_t height;

			/// @b The number of components in a voxel for this image.
			std::size_t voxel_dimensionalty;
	};

}
}

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_PRIVATE_HPP_
