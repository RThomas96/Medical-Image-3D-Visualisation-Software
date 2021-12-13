#include "../include/generic_image_downsampler.hpp"

#include "../include/generic_image_downsampler_templated.hpp"

namespace Image {

	GenericImageDownsampler::GenericImageDownsampler(const svec3 desired_resolution, Grid::Ptr parent) :
		target_resolution(desired_resolution), parent_grid(parent) {
		// manually set the internal data type.
		if (parent != nullptr) {
			this->custom_name			= this->parent_grid->getImageName() + std::string("_downsampled");
			this->source_resolution		= this->parent_grid->getResolution();
			this->internal_data_type	= this->parent_grid->getInternalDataType();
			this->voxel_dimensionality	= this->parent_grid->getVoxelDimensionality();
			// Compute voxel sizes of this downsampled version of the grid :
			glm::vec3 size_factor		= glm::convert_to<float>(this->source_resolution) / glm::convert_to<float>(this->target_resolution);
			this->voxel_sizes			= this->parent_grid->getVoxelDimensions() * size_factor;
		} else {
			throw std::runtime_error("Error : cannot downsample a grid without a valid grid as parent !");
		}
	}

	ImageDataType GenericImageDownsampler::getInternalDataType() const {
		return this->internal_data_type;
	}

	bool GenericImageDownsampler::presentOnDisk() const { return false; }

	std::size_t GenericImageDownsampler::getVoxelDimensionality() const { return this->parent_grid->getVoxelDimensionality(); }

	glm::vec3 GenericImageDownsampler::getVoxelDimensions() const {
		// precomputed in the ctor for this class !
		return this->voxel_sizes;
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

			/* Huge and dirty switch-like method for returning the right instance of a downsampler given the parent's type : */

			// Floating-point types :
			if (parent_data_type & ImageDataType::Floating) {
				if (parent_data_type & ImageDataType::Bit_32) {
					return GenericImageDownsamplerTemplated<float, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<float>(method));
				}
				if (parent_data_type & ImageDataType::Bit_64) {
					return GenericImageDownsamplerTemplated<double, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<double>(method));
				}
				throw std::runtime_error("Error : trying to create a subregion of a floating-point grid with bit widths different from 32/64");
			}

			// Unsigned types :
			if (parent_data_type & ImageDataType::Unsigned) {
				if (parent_data_type & ImageDataType::Bit_8) {
					return GenericImageDownsamplerTemplated<std::uint8_t, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<std::uint8_t>(method));
				}
				if (parent_data_type & ImageDataType::Bit_16) {
					return GenericImageDownsamplerTemplated<std::uint16_t, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<std::uint16_t>(method));
				}
				if (parent_data_type & ImageDataType::Bit_32) {
					return GenericImageDownsamplerTemplated<std::uint32_t, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<std::uint32_t>(method));
				}
				if (parent_data_type & ImageDataType::Bit_64) {
					return GenericImageDownsamplerTemplated<std::uint64_t, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<std::uint64_t>(method));
				}
				throw std::runtime_error("Error : trying to create a subregion of an unsigned grid with bit widths different from 8/16/32/64");
			}

			// Signed types :
			if (parent_data_type & ImageDataType::Signed) {
				if (parent_data_type & ImageDataType::Bit_8) {
					return GenericImageDownsamplerTemplated<std::int8_t, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<std::int8_t>(method));
				}
				if (parent_data_type & ImageDataType::Bit_16) {
					return GenericImageDownsamplerTemplated<std::int16_t, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<std::int16_t>(method));
				}
				if (parent_data_type & ImageDataType::Bit_32) {
					return GenericImageDownsamplerTemplated<std::int32_t, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<std::int32_t>(method));
				}
				if (parent_data_type & ImageDataType::Bit_64) {
					return GenericImageDownsamplerTemplated<std::int64_t, resampler_functor>::createBackend(size, parent, findRightInterpolatorType<std::int64_t>(method));
				}
				throw std::runtime_error("Error : trying to create a subregion of a signed grid with bit widths different from 8/16/32/64");
			}
			throw std::runtime_error("Error : trying to create a subregion of an unknown pixel type (not floating or [un]signed)");

		}

	} // namespace Downsampled

}
