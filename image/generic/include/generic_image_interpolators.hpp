#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_
#define VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_

#include <memory>
#include <vector>

namespace Image {

	// Interpolator default signature :
	//	template <typename element_t, class grid_t>
	//	using resampler_functor = std::function<
	//		element_t(const glm::vec3 position, const glm::vec3 element_size, const svec3 index, const std::shared_ptr<grid_t> grid)
	//	>;
	//

}

#endif // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_INTERPOLATORS_HPP_
