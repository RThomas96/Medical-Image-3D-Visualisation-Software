#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_IMPL_HPP_
#define VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_IMPL_HPP_

#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_
#include "./generic_image_interpolators.hpp"
#endif

namespace Image {

namespace Interpolators {

	template <typename element_t, class grid_t>
	std::vector<element_t> null_interpolator(
		const std::shared_ptr<Image::Grid> sampled_grid,
		const svec3 index,
		const std::size_t channels_to_sample,
		const svec3 source_resolution,
		const glm::vec3 source_position,
		const glm::vec3 source_voxel_sizes
	) {
		UNUSED(sampled_grid);
		UNUSED(index);
		UNUSED(source_resolution);
		UNUSED(source_position);
		UNUSED(source_voxel_sizes);
		return std::vector<element_t>(channels_to_sample, 0);
	}

	template <typename element_t>
	std::vector<element_t> nearest_neighbor_interpolator(
		const Grid::Ptr sampled_grid,
		const svec3 index,
		const std::size_t channels_to_sample,
		const svec3 source_resolution,
		const glm::vec3 source_position,
		const glm::vec3 source_voxel_sizes
	) {
		if (source_position.x < .0f || source_position.y < .0f || source_position.z < .0f) {
			// Should not happen ? The grid as loaded from memory should have its origin at (0,0,0) and be defined in Z+ ...
			// The sample will have values of 0
			return std::vector<element_t>(channels_to_sample, static_cast<element_t>(0));
		}

		// Prepare the vector of interpolated values :
		std::vector<element_t> sample_values(channels_to_sample);
		// Compute the index within the source grid from the position of the sample (will always be positive here because of check above) :
		svec3 grid_position = glm::convert_to<svec3::value_type>(
					glm::floor((glm::convert_to<float>(index) * source_voxel_sizes) / sampled_grid->getVoxelDimensions())
					);
		// Fetch the value at the position computed earlier :
		if (sampled_grid->readPixel(grid_position, sample_values) == false) {
			std::cerr << "Error trying to read the grid at position (" << index << "). Setting to 0.\n";
			for (std::size_t c = 0; c < channels_to_sample; ++c) { sample_values[c] = static_cast<element_t>(0); }
		}
		return sample_values;
	}

}

}

#endif // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_IMPL_HPP_
