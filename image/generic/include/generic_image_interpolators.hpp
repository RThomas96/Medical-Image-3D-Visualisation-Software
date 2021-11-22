#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_
#define VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_

#include "./generic_image_reader.hpp"

#include "../../utils/include/image_api_common.hpp"
#include "../../../new_grid/include/grid.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace Image {

namespace Interpolators {

	template <typename element_t , class grid_t>
	std::vector<element_t> null_interpolator(
		const std::shared_ptr<grid_t> sampled_grid,
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

	template <typename element_t , class grid_t>
	std::vector<element_t> nearest_neighbor_interpolator(
		const std::shared_ptr<grid_t> sampled_grid,
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

}

}

#endif // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_
