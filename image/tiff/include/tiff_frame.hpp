#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_FRAME_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_FRAME_HPP_

#include "../../macros.hpp"

#include <tiff.h>
#include <tiffio.h>

#include <string>
#include <memory>
#include <vector>

namespace Image {

namespace Tiff {

	/// @b Redirects the error messages from the TIFF files.
	void tiff_error_redirection(const char* module, const char* fmt, va_list _va_) {
		UNUSED_PARAMETER(module)
		UNUSED_PARAMETER(fmt)
		UNUSED_PARAMETER(_va_)
	}

	/// @b Redirects the warning messages from the TIFF files.
	void tiff_warning_redirection(const char* module, const char* fmt, va_list _va_) {
		UNUSED_PARAMETER(module)
		UNUSED_PARAMETER(fmt)
		UNUSED_PARAMETER(_va_)
	}

	struct Frame {
		public:
			typedef std::shared_ptr<Frame> Ptr;

		public:
			/// @b Default ctor, which constructs the frame, but doesn't parse the information in it.
			/// @warning If the frame cannot be parsed, throws an exception.
			Frame(const std::string& _v, const tdir_t _cur_directory) noexcept(false);
			/// @b Default dtor, which releases the information required by this frame
			~Frame(void) = default;

			/// @b Loads information from the TIFF file, in order to parse it efficiently later.
			/// @warning This TIFF frame implementation only supports one sample per pixel, for now.
			void loadTIFFInfo(std::string_view sourceFile) noexcept(false);

			/// @b Returns true if the two frames are 'compatible'.
			/// @details Two frames are considered compatible if and only if they have the same width & the same height.
			/// Anything else is not taken into account, for example if we have one frame which has two samples per
			/// pixel and another which has only one, they are still considered compatible if they have the same width
			/// and height.
			bool isCompatibleWith(const Frame& f);

		public:
			/// @b The filename of the TIFF file this frame is located in.
			const std::string sourceFile;
			/// @b The directory offset into the file
			const tdir_t directoryOffset;
			/// @b The width of the frame
			uint32_t width;
			/// @b The height of the frame
			uint32_t height;
			/// @b The number of rows per strip
			uint32_t rowsPerStrip;
			/// @b The number of samples per pixel. Per the TIFF spec, is contained in [1, 3]
			uint16_t samplesPerPixel;
			/// @b The number of bits per sample. Per the TIFF spec, can be 8, 16, 32, or 64.
			uint16_t bitsPerSample;
			/// @b The number of strips per image.
			uint64_t stripsPerImage;

	};

} // namespace TIFF

} // namespace Image

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_FRAME_HPP_
