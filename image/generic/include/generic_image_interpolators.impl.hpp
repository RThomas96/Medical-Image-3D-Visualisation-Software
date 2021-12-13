#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_IMPL_HPP_
#define VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_IMPL_HPP_

#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_
#include "./generic_image_interpolators.hpp"
#endif

namespace Image {

namespace Interpolators {

	template <typename element_t>
	auto vectorized_mult_op = [](std::vector<element_t>& result, float factor) -> void {
		for (std::size_t i = 0; i < result.size(); ++i) { result[i] *= factor;}
	};
	template <typename element_t>
	auto vectorized_add_op = [](std::vector<element_t>& result, std::vector<element_t> adder) -> void {
		for (std::size_t i = 0; i < result.size(); ++i) { result[i] += adder[i]; }
	};

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

	template <typename element_t>
	std::vector<element_t> linear_interpolator(
		const Grid::Ptr sampled_grid,
		const svec3 index,
		const std::size_t channels_to_sample,
		const svec3 source_resolution,
		const glm::vec3 source_position,
		const glm::vec3 source_voxel_sizes
	) {
		// Sample the values of the neighboring voxels from the source_position voxel.
		std::array<glm::vec3, 8> neighbors {
			source_position + (source_voxel_sizes * glm::vec3{-1.f, -1.f, -1.f}),
			source_position + (source_voxel_sizes * glm::vec3{-1.f, -1.f,  1.f}),
			source_position + (source_voxel_sizes * glm::vec3{-1.f,  1.f, -1.f}),
			source_position + (source_voxel_sizes * glm::vec3{-1.f,  1.f,  1.f}),
			source_position + (source_voxel_sizes * glm::vec3{ 1.f, -1.f, -1.f}),
			source_position + (source_voxel_sizes * glm::vec3{ 1.f, -1.f,  1.f}),
			source_position + (source_voxel_sizes * glm::vec3{ 1.f,  1.f, -1.f}),
			source_position + (source_voxel_sizes * glm::vec3{ 1.f,  1.f,  1.f})
		};
		// The sampled values from the grid :
		std::array<std::vector<element_t>, 8> sampled_values;
		// The sampled grid's voxel sizes :
		glm::vec3 sampled_voxel_sizes = sampled_grid->getVoxelDimensions();
		// The 'default' value in case a sample point goes outside the grid :
		std::vector<element_t> default_grid_value(sampled_grid->getVoxelDimensionality());
		sampled_grid->readPixel(glm::convert_to<svec3::value_type>(glm::floor(source_position / sampled_voxel_sizes)), default_grid_value);
		// Final sample value :
		std::vector<element_t> final_pixel_value(sampled_grid->getVoxelDimensionality());

		// Simple average over all neighbors :
		for (std::size_t v = 0; v < 8; ++v) {
			// Voxel index is the floor()-ed value of the position div-ed by the sampled voxel sizes :
			svec3 current_position = glm::convert_to<svec3::value_type>(glm::floor(neighbors[v] / sampled_voxel_sizes));
			// attempt to read pixel value, and set to middle pixel value in case not valid :
			if (sampled_grid->readPixel(current_position, sampled_values[v]) == false) {
				sampled_values[v].resize(sampled_grid->getVoxelDimensionality());
				std::copy(std::cbegin(default_grid_value), std::cend(default_grid_value), std::begin(sampled_values[v]));
				// should be performed by std::vector<>::operator=() but I'm not sure, so make it verbose here ...
			}
			// Simple averaging of the value in order to determine which is correct :
			vectorized_mult_op<element_t>(sampled_values[v], 1.f/8.f);
			vectorized_add_op<element_t>(final_pixel_value, sampled_values[v]);
		}

		return final_pixel_value;
	}

}

}

#endif // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_IMPL_HPP_
