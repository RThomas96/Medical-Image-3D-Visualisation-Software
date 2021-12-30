#include "../include/generic_image_reader_subregion.hpp"

#include "../include/generic_image_reader_subregion_templated.hpp"

namespace Image {

	GenericImageReaderSubregion::GenericImageReaderSubregion(svec3 _o, svec3 _s, Grid::Ptr _p) :
		sampling_region_origin(_o), sampling_region_size(_s), parent_grid(_p) {
		// manually set the internal data type.
		if (_p != nullptr) {
			this->internal_data_type = this->parent_grid->getInternalDataType();
			this->custom_name		 = this->parent_grid->getImageName() + std::string("_subregion");
			// check the subregion can fit in the grid :
			auto res = this->parent_grid->getResolution();
			if (_o.x >= res.x || _o.y >= res.y || _o.z >= res.z) {
				throw std::runtime_error("Error : cannot fit the subregion in the parent grid (origin too far).");
			}
			if (_s.x >= res.x || _s.y >= res.y || _s.z >= res.z) {
				throw std::runtime_error("Error : cannot fit the subregion in the parent grid (size too big).");
			}
			if (_o.x + _s.x >= res.x || _o.y + _s.y >= res.y || _o.z + _s.z >= res.z) {
				throw std::runtime_error("Error : cannot fit the subregion in the parent grid (steps over bounds).");
			}
		} else {
			throw std::runtime_error("Error : cannot create a grid subregion without a valid grid !");
		}
	}

	ImageDataType GenericImageReaderSubregion::getInternalDataType() const {
		return this->internal_data_type;
	}

	ThreadedTask::Ptr GenericImageReaderSubregion::parseImageInfo(ThreadedTask::Ptr pre_existing_task,
	  const std::vector<std::vector<std::string>> &_filenames) {
		UNUSED(_filenames);
		// Ignores filenames entirely. This is simply a grid subregion.

		if (pre_existing_task == nullptr) {
			pre_existing_task = std::make_shared<ThreadedTask>();
			pre_existing_task->end();
		}
		return pre_existing_task;
	}

	bool GenericImageReaderSubregion::presentOnDisk() const { return false; }

	std::size_t GenericImageReaderSubregion::getVoxelDimensionality() const { return this->parent_grid->getVoxelDimensionality(); }

	glm::vec3 GenericImageReaderSubregion::getVoxelDimensions() const { return this->parent_grid->getVoxelDimensions(); }

	svec3 GenericImageReaderSubregion::getResolution() const { return this->sampling_region_size; }

	std::string GenericImageReaderSubregion::getImageName() const { return this->custom_name; }

	void GenericImageReaderSubregion::setImageName(std::string &_user_defined_name_) {
		if (not _user_defined_name_.empty()) {
			this->custom_name = _user_defined_name_;
		}
	}

	void GenericImageReaderSubregion::setVoxelDimensions(glm::vec3 new_voxel_dimensions) {
		throw std::logic_error("Error : cannot set voxel dimensions on a sub-region of a grid ! Change the voxel dimensions of the top-level parent instead.");
	}


	BoundingBox_General<float> GenericImageReaderSubregion::getBoundingBox() const {
		// get origin and its opposite corner from voxel indices to image coords :
		glm::vec3 origin_position	= glm::convert_to<float>(this->sampling_region_origin) * this->getVoxelDimensions();
		glm::vec3 physical_size		= glm::convert_to<float>(this->sampling_region_size) * this->getVoxelDimensions();
		glm::vec3 opposite_position = origin_position + physical_size;

		return BoundingBox_General<float>(glm::vec4{origin_position, 1.f}, glm::vec4{opposite_position, 1.f});
	}

	namespace SubRegion {

		GenericImageReaderSubregion::Ptr createBackend(svec3 origin, svec3 size, Grid::Ptr grid) {
			ImageDataType parent_data_type = grid->getInternalDataType();

			if (parent_data_type & ImageDataType::Floating) {
				if (parent_data_type & ImageDataType::Bit_32) {
					return GenericImageReaderSubregionTemplated<float>::createBackend(origin, size, grid);
				}
				if (parent_data_type & ImageDataType::Bit_64) {
					return GenericImageReaderSubregionTemplated<double>::createBackend(origin, size, grid);
				}
				throw std::runtime_error("Error : trying to create a subregion of a floating-point grid with bit widths different from 32/64");
			}

			if (parent_data_type && ImageDataType::Unsigned) {
				if (parent_data_type & ImageDataType::Bit_8) {
					return GenericImageReaderSubregionTemplated<std::uint8_t>::createBackend(origin, size, grid);
				}
				if (parent_data_type & ImageDataType::Bit_16) {
					return GenericImageReaderSubregionTemplated<std::uint16_t>::createBackend(origin, size, grid);
				}
				if (parent_data_type & ImageDataType::Bit_32) {
					return GenericImageReaderSubregionTemplated<std::uint32_t>::createBackend(origin, size, grid);
				}
				if (parent_data_type & ImageDataType::Bit_64) {
					return GenericImageReaderSubregionTemplated<std::uint64_t>::createBackend(origin, size, grid);
				}
				throw std::runtime_error("Error : trying to create a subregion of an unsigned grid with bit widths different from 8/16/32/64");
			}

			if (parent_data_type && ImageDataType::Signed) {
				if (parent_data_type & ImageDataType::Bit_8) {
					return GenericImageReaderSubregionTemplated<std::int8_t>::createBackend(origin, size, grid);
				}
				if (parent_data_type & ImageDataType::Bit_16) {
					return GenericImageReaderSubregionTemplated<std::int16_t>::createBackend(origin, size, grid);
				}
				if (parent_data_type & ImageDataType::Bit_32) {
					return GenericImageReaderSubregionTemplated<std::int32_t>::createBackend(origin, size, grid);
				}
				if (parent_data_type & ImageDataType::Bit_64) {
					return GenericImageReaderSubregionTemplated<std::int64_t>::createBackend(origin, size, grid);
				}
				throw std::runtime_error("Error : trying to create a subregion of a signed grid with bit widths different from 8/16/32/64");
			}
			throw std::runtime_error("Error : trying to create a subregion of an unknown pixel type (not floating or [un]signed)");
		}

	}	 // namespace SubRegion

}	 // namespace Image
