#include "../include/backend.hpp"

namespace Image {

	// By default, the data type is unknown for those files.
	ImageBackendImpl::ImageBackendImpl(const std::vector<std::vector<std::string>>& fns) {
		this->voxelDimensions = glm::vec3(0.f, 0.f, 0.f);
		this->imageResolution = svec3(0, 0, 0);
		this->dimensionality = 0;
		this->internal_data_type = ImageDataType::Unknown;
	}

	// By default, the base class for image backend cannot read anything. Always return false.
	bool ImageBackendImpl::canReadImage(const std::string& image_name) {
		UNUSED_PARAMETER(image_name);
		return false;
	}

	ImageDataType ImageBackendImpl::getInternalDataType() const { return this->internal_data_type; }

	std::size_t ImageBackendImpl::getVoxelDimensionality() const { return this->dimensionality; }

	glm::vec3 ImageBackendImpl::getVoxelDimensions() const { return this->voxelDimensions; }

	svec3 ImageBackendImpl::getResolution() const { return this->imageResolution; }

}
