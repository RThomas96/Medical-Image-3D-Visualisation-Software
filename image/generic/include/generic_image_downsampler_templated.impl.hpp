#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_IMPL_HPP_
#define VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_IMPL_HPP_

// For linters, include the header file between header guards :
#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_HPP_
#include "./generic_image_downsampler_templated.hpp"
#endif // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_HPP_

#include <string>

namespace Image {

	template <typename element_t, template <typename, class> typename resampler_t>
	GenericImageDownsamplerTemplated<element_t, resampler_t>::GenericImageDownsamplerTemplated(svec3 size, Grid::Ptr parent, sampler_t resampler) :
		resampling_method(resampler), GenericImageDownsampler(size, parent) {
		// reset cache
		this->read_cache.clearCache();
		this->downsampled_slices.reset();
	}

	template <typename element_t, template <typename, class> typename resampler_t>
	typename GenericImageDownsamplerTemplated<element_t, resampler_t>::Ptr GenericImageDownsamplerTemplated<element_t, resampler_t>::createBackend(
			const svec3 size, Grid::Ptr parent, const sampler_t resampler)
	{
		return Ptr(new GenericImageDownsamplerTemplated<element_t, resampler_t>(size, parent, resampler));
	}

	template <typename element_t, template <typename, class> typename resampler_t>
	std::size_t GenericImageDownsamplerTemplated<element_t, resampler_t>::load_slice_from_parent_grid(std::size_t slice_idx) {
		// Shouldn't happen, but it's good to check for it anyways :
		if (slice_idx >= this->target_resolution.z) {
			return nullptr;
		}
		// Attempt to find the slice :
		if (this->downsampled_slices.hasSlice(slice_idx)) {
			return this->downsampled_slices.findSlice(slice_idx);
		} else {
			// Load the slice :
			std::string error_msg = "Error : a 'valid' downsampled slice was not found ... (index given :" + std::to_string(slice_idx) + ")";
			throw std::runtime_error(error_msg);
		}
		// Else, return a value for the slice bigger than there can be.
		return this->target_resolution.z;
	}

	template <typename element_t, template <typename, class> typename resampler_t>
	void GenericImageDownsamplerTemplated<element_t, resampler_t>::downsample_in_separate_thread(ThreadedTask::Ptr progress_tracker) {
		// Sanity checks, returning and cancelling the operation immediately :

		// Load only the selected slices :
		progress_tracker->setSteps(this->target_resolution.z);

		for (std::size_t z = 0; z < this->target_resolution.z; ++z) {
			//
		}
	}

	template <typename element_t, template <typename, class> typename resampler_t>
	template <typename out_t>
	bool GenericImageDownsamplerTemplated<element_t, resampler_t>::templated_read_region(svec3 read_origin, svec3 read_size, std::vector<out_t>& return_values) {
		/// Note : This function fills the parts of the read reagion outside of the grid bounds with 0s.
		/// If you request a read size too big, it will SILENTLY fill this vector with 0s where appropriate and return true.
		/// We also remind that the images are already downsampled here (they were processed in parseImageInfo()).

		/// Warning : (very) verbose code ahead.

		if (read_origin.x > this->target_resolution.x || read_origin.y > this->target_resolution.y || read_origin.z > this->target_resolution.z) {
			std::fill(return_values.begin(), return_values.end(), 0);
			return false;
		}

		// resize the vector to fit all elements to be read :
		return_values.resize(read_size.x * read_size.y * read_size.z * this->voxel_dimensionality);

		// s_<axis> : iterator/index for the axis on the source grid
		std::size_t s_x, s_y, s_z;
		// <axis> : position of the currently read pixel in the output vector (return_values)
		std::size_t x, y, z;
		// max indices to read :
		svec3 read_end = read_origin + read_size;
		// sizes of primitives in the image :
		std::size_t line_size = read_size.x * this->voxel_dimensionality;
		std::size_t slice_size = read_size.y * line_size;

		// for all slices to be read :
		for (s_z = read_origin.z; s_z < read_end.z && s_z < this->target_resolution.z; ++s_z) {
			z = s_z - read_origin.z;
			// fetch the image from local storage :
			std::shared_ptr<std::vector<pixel_t>> slice_data = this->downsampled_slices.getSlice(s_z);

			// Fill with values in the grid :
			for (s_y = read_origin.y; s_y < read_end.y && s_y < this->target_resolution.y; ++s_y) {
				y = s_y - read_origin.y;

				// Fill the pixels in the grid :
				for (s_x = read_origin.x; s_x < read_end.x && s_x < this->target_resolution.x; ++s_x) {
					x = s_x - read_origin.x;
					// Index in the loaded slice :
					std::size_t slice_pixel_index_base = s_y * line_size + s_x * this->voxel_dimensionality;
					// Copy from the loaded slice :
					for (std::size_t s_v = 0; s_v < this->voxel_dimensionality; ++s_v) {
						return_values[z*slice_size + y * line_size + x * this->voxel_dimensionality + s_v]  = (*slice_data)[slice_pixel_index_base];
					}
				}

				// Fill the rest with 0s :
				for (s_x = this->target_resolution.x; s_x < read_end.x; ++s_x) {
					x = s_x - read_origin.x;
					for (std::size_t s_v = 0; s_v < this->voxel_dimensionality; ++s_v) {
						return_values[z*slice_size + y * line_size + x * this->voxel_dimensionality + s_v] = static_cast<pixel_t>(0.f);
					}
				}
			}

			// Fill the rest with 0s :
			for (s_y = this->target_resolution.y; s_y < read_end.y; ++s_y) {
				y = s_y - read_origin.y;
				// Fill the rest with 0s :
				for (s_x = read_origin.x; s_x < read_end.x; ++s_x) {
					x = s_x - read_origin.x;
					for (std::size_t s_v = 0; s_v < this->voxel_dimensionality; ++s_v) {
						return_values[z*slice_size + y * line_size + x * this->voxel_dimensionality + s_v] = static_cast<pixel_t>(0.f);
					}
				}
			}
		}

		// Fill the rest with 0s :
		for (s_z = this->target_resolution.z; s_z < read_end.z; ++s_z) {
			z = s_z - read_origin.z;
			// Fill the rest with 0s :
			for (s_y = read_origin.y; s_y < read_end.y; ++s_y) {
				y = s_y - read_origin.y;
				// Fill the rest with 0s :
				for (s_x = read_origin.x; s_x < read_end.x; ++s_x) {
					x = s_x - read_origin.x;
					for (std::size_t s_v = 0; s_v < this->voxel_dimensionality; ++s_v) {
						return_values[z*slice_size + y * line_size + x * this->voxel_dimensionality + s_v] = static_cast<pixel_t>(0.f);
					}
				}
			}
		}
		return true;
	}

	template <typename element_t, template <typename, class> typename resampler_t>
	template <typename range_t>
	bool GenericImageDownsamplerTemplated<element_t, resampler_t>::templated_read_ranges(std::size_t channel, glm::tvec2<range_t>& ranges) {
		return true;
	}

}

#endif // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_IMPL_HPP_
