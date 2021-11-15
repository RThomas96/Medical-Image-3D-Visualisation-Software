#ifndef VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_TEMPLATE_IMPL_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_TEMPLATE_IMPL_HPP_

#ifndef VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_TEMPLATE_HPP_
#include "./grid_subregion_template.hpp"
#endif

namespace Image {

template <typename unsupported_element_type>
GenericImageReaderSubregionTemplated<unsupported_element_type>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) {
	throw std::runtime_error("Error : trying to create a grid subregion backend with an unsupported type.");
}

template <>
GenericImageReaderSubregionTemplated<std::uint8_t>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Unsigned | ImageDataType::Bit_8;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <>
GenericImageReaderSubregionTemplated<std::uint16_t>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Unsigned | ImageDataType::Bit_16;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <>
GenericImageReaderSubregionTemplated<std::uint32_t>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Unsigned | ImageDataType::Bit_32;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <>
GenericImageReaderSubregionTemplated<std::uint64_t>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Unsigned | ImageDataType::Bit_64;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <>
GenericImageReaderSubregionTemplated<std::int8_t>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Signed | ImageDataType::Bit_8;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <>
GenericImageReaderSubregionTemplated<std::int16_t>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Signed | ImageDataType::Bit_16;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <>
GenericImageReaderSubregionTemplated<std::int32_t>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Signed | ImageDataType::Bit_32;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <>
GenericImageReaderSubregionTemplated<std::int64_t>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Signed | ImageDataType::Bit_64;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <>
GenericImageReaderSubregionTemplated<float>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Floating | ImageDataType::Bit_32;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <>
GenericImageReaderSubregionTemplated<double>::GenericImageReaderSubregionTemplated(svec3 _o, svec3 _s, Grid::Ptr _g) : GenericImageReaderSubregion(_o, _s, _g) {
	ImageDataType reference_type= ImageDataType::Floating | ImageDataType::Bit_64;
	if (this->internal_data_type != reference_type) {
		throw std::runtime_error("Error : subregion does not contain the same internal type as the parent grid.");
	}
}

template <typename element_t>
GenericImageReaderSubregionTemplated<element_t>::~GenericImageReaderSubregionTemplated() {
	this->read_cache.clearCache();
	this->parent_grid.reset();
}

template <typename element_t>
std::size_t GenericImageReaderSubregionTemplated<element_t>::load_slice_from_parent_grid(std::size_t slice_idx) {
	// typedef for cached data :
	using ImagePtr = typename cache_t::data_t_ptr; // So : std::shared_ptr< std::vector<element_t> >
	// resolution of current image :
	svec3 resolution = this->sampling_region_size;
	std::size_t dimensionality = this->getVoxelDimensionality();

	// Sanity check :
	if (slice_idx >= resolution.z) { throw std::runtime_error("Error : tried to load past-the end of the image."); }

	// if already cached previously :
	if (this->read_cache.hasData(slice_idx)) { return this->read_cache.findIndex(slice_idx); }

	// source and size to read :
	svec3 origin = this->sampling_region_origin;
	origin.z = slice_idx;
	svec3 size = this->sampling_region_size;
	size.z = 1;

	// If got here, should load image directly from source. Create target vector for all data channels :
	ImagePtr full_image_data = std::make_shared<std::vector<pixel_t>>(resolution.x * resolution.y * dimensionality);
	// Read data directly from parent grid :
	bool could_read_data = this->parent_grid->readSubregion(origin, size, *full_image_data);
	if (not could_read_data) { throw std::runtime_error("Error : could not read data from the parent grid."); }

	// Vector is now fully loaded with the data from the parent grid. Cache and return index :
	return this->read_cache.loadData(slice_idx, full_image_data);
}

template <typename element_t>
template <typename out_t>
bool GenericImageReaderSubregionTemplated<element_t>::templated_read_region(svec3 origin, svec3 size, std::vector<out_t>& values) {
	// Reminder : the availability of the grid coordinates have been checked before.
	// As such, we can read the data directly.

	/// @brief Const iterator type for the cached data, which does not modify the data itself
	using cache_iterator_t = typename cache_t::data_t_ptr::element_type::const_iterator;
	/// @brief Iterator type for the target data
	using target_iterator_t = typename std::vector<out_t>::iterator;

	std::size_t voxel_dimensionality = this->getVoxelDimensionality();
	svec3 resolution = this->getResolution();

	// ensure we have the right size for the buffer, fill it with 0s for now :
	values.resize(size.x * size.y * size.z * voxel_dimensionality);
	std::fill(values.begin(), values.end(), pixel_t(0));

	//
	// Check for outliers :
	// If origin's coordinates are bigger than this stack's dimensions, the whole subregion will be outside.
	// Simply fill the vector with null values and return
	//
	if (origin.x >= resolution.x || origin.y >= resolution.y || origin.z >= resolution.z){ return true; }

	/// @brief Beginning of slices to load and cache
	std::size_t src_slice_begin = origin.z;
	/// @brief end of slices to cache or end of slices available
	std::size_t src_slice_end = (src_slice_begin + size.z >= resolution.z) ?
					resolution.z : src_slice_begin + size.z;

	// the number of slices which will be read by the first for-loop :
	std::size_t tgt_slices_readable = src_slice_end - src_slice_begin;

	/// @brief Index of the last line we can read from the source buffer
	std::size_t src_height_idx_end= (origin.y + size.y >= resolution.y) ?
					resolution.y - origin.y : size.y;

	/// @brief the number of lines that can be read from the source buffer :
	std::size_t src_height_readable = src_height_idx_end - origin.y;

	/// @brief total length of a line in the source
	std::size_t src_line_size = resolution.x * voxel_dimensionality;
	/// @brief beginning of a line to read from the source buffer
	std::size_t src_line_idx_begin = origin.x * voxel_dimensionality;
	/// @brief amount of values to read into the target buffer from the source
	std::size_t src_line_idx_end = (src_line_idx_begin + size.x * voxel_dimensionality >= src_line_size) ?
				src_line_size : src_line_idx_begin + size.x * voxel_dimensionality;

	/// @brief Line length in the buffer to write to
	std::size_t target_line_length = size.x * voxel_dimensionality;
	/// @brief image length in the buffer to write to
	std::size_t target_image_length = size.y * target_line_length;

	std::size_t y = 0, z = 0;

	// Iterate on slices :
	for (z = 0;	z < tgt_slices_readable ; ++z) {
		// load and cache the slice to load in memory (or fetch it directly if already cached) :
		std::size_t cache_idx = this->load_slice_from_parent_grid(src_slice_begin + z);
		// get img data from cache :
		typename cache_t::data_t_ptr image_data = this->cachedSlices.getDataIndexed(cache_idx);
		// read all lines we _can_ from the source :
		for (y = 0; y < src_height_readable; ++y) {
			// Figure out the right location in the source buffer :
			cache_iterator_t begin = image_data->cbegin() + (origin.y + y) * src_line_size + src_line_idx_begin;
			// Read <source begin> + [size to read | rest of source line] :
			cache_iterator_t end   = image_data->cbegin() + (origin.y + y) * src_line_size + src_line_idx_end;
			// beginning in target buffer :
			target_iterator_t target_begin = values.begin() + z * target_image_length + y * target_line_length;
			// Copy a whole line at once :
			std::copy(begin, end, target_begin);
		}
	}
	// don't need to pad the remaining slices (if any) because of the first call to fill()
	return true;
}

} // namespace Image

#endif // VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_TEMPLATE_IMPL_HPP_
