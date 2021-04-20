#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_FRAME_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_FRAME_HPP_

#include <tiff.h>
#include <tiffio.h>

#include <string>

namespace Image {

namespace TIFF {

	struct Frame {
		public:
			Frame(std::string_view _v, tdir_t _cur_directory);
			~Frame(void) = default;
		protected:
			tdir_t directoryOffset;
	};

} // namespace TIFF

} // namespace Image

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_FRAME_HPP_
