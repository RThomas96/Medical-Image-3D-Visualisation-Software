#include "../include/backend.hpp"

namespace Image {

	// By default, the data type is unknown for those files.
	ImageBackendImpl::ImageBackendImpl(std::vector<std::vector<std::string>> fns)
		: filenames(fns), internal_data_type(ImageDataType::Unknown) {}

	// By default, the base class for image backend cannot read anything. Always return false.
	bool ImageBackendImpl::canReadImage(const std::string& image_name) { return false; }

}
