#include "../include/generic_image_downsampler.hpp"

#include "../include/generic_image_downsampler_templated.hpp"

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

	namespace Downsampled {

		GenericImageDownsampler::Ptr createBackend(Grid::Ptr parent, svec3 size, ImageResamplingTechnique method) {
			ImageDataType parent_data_type = parent->getInternalDataType();

#define DEFAULT_SAMPLER_FUNC(type) \
	resampler_functor<type, Grid> sampler = [](const glm::vec3 p, const glm::vec3 s, const svec3 i, const Grid::Ptr g) -> type {return type(0);};


			if (parent_data_type & ImageDataType::Floating) {
				if (parent_data_type & ImageDataType::Bit_32) {
					DEFAULT_SAMPLER_FUNC(float);
					return GenericImageDownsamplerTemplated<float, resampler_functor>::createBackend(size, parent, sampler);
				}
				if (parent_data_type & ImageDataType::Bit_64) {
					DEFAULT_SAMPLER_FUNC(double);
					return GenericImageDownsamplerTemplated<double, resampler_functor>::createBackend(size, parent, sampler);
				}
				throw std::runtime_error("Error : trying to create a subregion of a floating-point grid with bit widths different from 32/64");
			}

			if (parent_data_type && ImageDataType::Unsigned) {
				if (parent_data_type & ImageDataType::Bit_8) {
					DEFAULT_SAMPLER_FUNC(std::uint8_t);
					return GenericImageDownsamplerTemplated<std::uint8_t, resampler_functor>::createBackend(size, parent, sampler);
				}
				if (parent_data_type & ImageDataType::Bit_16) {
					DEFAULT_SAMPLER_FUNC(std::uint16_t);
					return GenericImageDownsamplerTemplated<std::uint16_t, resampler_functor>::createBackend(size, parent, sampler);
				}
				if (parent_data_type & ImageDataType::Bit_32) {
					DEFAULT_SAMPLER_FUNC(std::uint32_t);
					return GenericImageDownsamplerTemplated<std::uint32_t, resampler_functor>::createBackend(size, parent, sampler);
				}
				if (parent_data_type & ImageDataType::Bit_64) {
					DEFAULT_SAMPLER_FUNC(std::uint64_t);
					return GenericImageDownsamplerTemplated<std::uint64_t, resampler_functor>::createBackend(size, parent, sampler);
				}
				throw std::runtime_error("Error : trying to create a subregion of an unsigned grid with bit widths different from 8/16/32/64");
			}

			if (parent_data_type && ImageDataType::Signed) {
				if (parent_data_type & ImageDataType::Bit_8) {
					DEFAULT_SAMPLER_FUNC(std::int8_t);
					return GenericImageDownsamplerTemplated<std::int8_t, resampler_functor>::createBackend(size, parent, sampler);
				}
				if (parent_data_type & ImageDataType::Bit_16) {
					DEFAULT_SAMPLER_FUNC(std::int16_t);
					return GenericImageDownsamplerTemplated<std::int16_t, resampler_functor>::createBackend(size, parent, sampler);
				}
				if (parent_data_type & ImageDataType::Bit_32) {
					DEFAULT_SAMPLER_FUNC(std::int32_t);
					return GenericImageDownsamplerTemplated<std::int32_t, resampler_functor>::createBackend(size, parent, sampler);
				}
				if (parent_data_type & ImageDataType::Bit_64) {
					DEFAULT_SAMPLER_FUNC(std::int64_t);
					return GenericImageDownsamplerTemplated<std::int64_t, resampler_functor>::createBackend(size, parent, sampler);
				}
				throw std::runtime_error("Error : trying to create a subregion of a signed grid with bit widths different from 8/16/32/64");
			}
			throw std::runtime_error("Error : trying to create a subregion of an unknown pixel type (not floating or [un]signed)");

#undef DEFAULT_SAMPLER_FUNC

		}

	} // namespace Downsampled

}
