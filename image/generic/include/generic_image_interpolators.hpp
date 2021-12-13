#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_
#define VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_

#include "./generic_image_reader.hpp"

#include "../../../new_grid/include/grid.hpp"
#include "../../utils/include/image_api_common.hpp"

#include <glm/gtx/io.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace Image {

	namespace Interpolators {

		/// @brief Default 'black-ole' interpolator (sets everything to 0).
		/// @details If no suitable interpolator can be found in Image::Downsampled::findRightInterpolatorType() based on the given enum value, this is
		/// the functor returned. Since no suitable interpolation could be found, then no interpolator will be returned.
		template <typename element_t, class grid_t>
		std::vector<element_t> null_interpolator(
		  const std::shared_ptr<grid_t> sampled_grid,
		  const svec3 index,
		  const std::size_t channels_to_sample,
		  const svec3 source_resolution,
		  const glm::vec3 source_position,
		  const glm::vec3 source_voxel_sizes);

		/// @brief A nearest-neighbor interpolator for the downsampled image reader.
		/// @details Performs a nearest neighbor interpolation by computing the index corresponsing to the given position within the grid to sample, and
		/// calling Grid::readPixel() in the position.
		template <typename element_t>
		std::vector<element_t> nearest_neighbor_interpolator(
		  const Grid::Ptr sampled_grid,
		  const svec3 index,
		  const std::size_t channels_to_sample,
		  const svec3 source_resolution,
		  const glm::vec3 source_position,
		  const glm::vec3 source_voxel_sizes);

		/// @brief A trilinear interpolator for the downsampled image reader.
		template <typename element_t>
		std::vector<element_t> linear_interpolator(
		  const Grid::Ptr sampled_grid,
		  const svec3 index,
		  const std::size_t channels_to_sample,
		  const svec3 source_resolution,
		  const glm::vec3 source_position,
		  const glm::vec3 source_voxel_sizes);

	}	 // namespace Interpolators

}	 // namespace Image

#include "./generic_image_interpolators.impl.hpp"

#endif	  // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_
