#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_IMPL_HPP_
#define VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_IMPL_HPP_

// For linters, include the header file between header guards :
#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_HPP_
#include "./generic_image_downsampler_templated.hpp"
#endif // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_HPP_

namespace Image {

	template <typename element_t, template <typename, class> typename resampler_t>
	GenericImageDownsamplerTemplated<element_t, resampler_t>::GenericImageDownsamplerTemplated(svec3 size, Grid::Ptr parent, sampler_t resampler) :
		resampling_method(resampler), GenericImageDownsampler(size, parent) {
		// reset cache
		this->read_cache.clearCache();
	}

	template <typename element_t, template <typename, class> typename resampler_t>
	typename GenericImageDownsamplerTemplated<element_t, resampler_t>::Ptr GenericImageDownsamplerTemplated<element_t, resampler_t>::createBackend(
			const svec3 size, Grid::Ptr parent, const sampler_t resampler)
	{
		return Ptr(new GenericImageDownsamplerTemplated<element_t, resampler_t>(size, parent, resampler));
	}

	template <typename element_t, template <typename, class> typename resampler_t>
	std::size_t GenericImageDownsamplerTemplated<element_t, resampler_t>::load_slice_from_parent_grid(std::size_t slice_idx) {
		//
	}

	template <typename element_t, template <typename, class> typename resampler_t>
	template <typename out_t>
	bool GenericImageDownsamplerTemplated<element_t, resampler_t>::templated_read_region(svec3 read_origin, svec3 read_size, std::vector<out_t>& return_values) {
#warning Does not actually fill values for now.
		return true;
	}

	template <typename element_t, template <typename, class> typename resampler_t>
	template <typename range_t>
	bool GenericImageDownsamplerTemplated<element_t, resampler_t>::templated_read_ranges(std::size_t channel, glm::tvec2<range_t>& ranges) {
#warning Does not actually fill values for now.
		return true;
	}

}

#endif // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_IMPL_HPP_
