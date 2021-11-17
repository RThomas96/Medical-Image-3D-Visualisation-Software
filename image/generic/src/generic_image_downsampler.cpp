#include "../include/generic_image_downsampler.hpp"

namespace Image {

	GenericImageDownsampler::GenericImageDownsampler(const svec3 desired_resolution, Grid::Ptr parent) :
		target_resolution(desired_resolution), parent_grid(parent) {
		// manually set the internal data type.
		if (parent != nullptr) {
			this->internal_data_type = this->parent_grid->getInternalDataType();
			this->custom_name		 = this->parent_grid->getImageName() + std::string("_downsampled");
		} else {
			throw std::runtime_error("Error : cannot create a grid subregion without a valid grid !");
		}
	}

	ImageDataType GenericImageDownsampler::getInternalDataType() const {
		return this->internal_data_type;
	}

	ThreadedTask::Ptr GenericImageDownsampler::parseImageInfo(ThreadedTask::Ptr pre_existing_task,
	  const std::vector<std::vector<std::string>> &_filenames) {
		UNUSED(_filenames);
		// Ignores filenames entirely. This is simply a grid subregion.

		if (pre_existing_task == nullptr) {
			pre_existing_task = std::make_shared<ThreadedTask>();
			pre_existing_task->end();
		}
		return pre_existing_task;
	}

	bool GenericImageDownsampler::presentOnDisk() const { return false; }

	std::size_t GenericImageDownsampler::getVoxelDimensionality() const { return this->parent_grid->getVoxelDimensionality(); }

	glm::vec3 GenericImageDownsampler::getVoxelDimensions() const {
		// Get scaling factor for all dimensions :
		glm::vec3 float_target_resolution = glm::convert_to<float>(this->target_resolution);
		glm::vec3 float_source_resolution = glm::convert_to<float>(this->source_resolution);
		// Scale voxel dimensions by that much :
		return this->parent_grid->getVoxelDimensions() * (float_source_resolution / float_target_resolution);
	}

	svec3 GenericImageDownsampler::getResolution() const { return this->target_resolution; }

	std::string GenericImageDownsampler::getImageName() const { return this->custom_name; }

	void GenericImageDownsampler::setImageName(std::string &_user_defined_name_) {
		if (not _user_defined_name_.empty()) {
			this->custom_name = _user_defined_name_;
		}
	}

	BoundingBox_General<float> GenericImageDownsampler::getBoundingBox() const {
		// This class only downsamples, so the BB is the same as the parent :
		return this->parent_grid->getBoundingBox();
	}

}
