#include "../include/backend.hpp"

namespace Image {

	ImageBackendImpl::ImageBackendImpl(std::vector<std::string> fns) {
		// Copy data from the filenames in argument, to the member filenames
		std::copy(fns.cbegin(), fns.cend(), this->filenames.begin());
	}

}
