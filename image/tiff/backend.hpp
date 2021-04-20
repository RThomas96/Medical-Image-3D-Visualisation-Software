#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HPP_

#include "./tiff_frame.hpp"

#include "../api/include/image_api_common.hpp"
#include "../api/include/backend.hpp"
#include "../api/include/threaded_task.hpp"

namespace Image {

	class TIFFBackend : public ::Image::ImageBackendImpl {
		protected:
			TIFFBackend(std::vector<std::string> fns) : ImageBackendImpl(fns) {}
		public:
			virtual ~TIFFBackend(void) = default;
	};

} // namespace Image

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HPP_
