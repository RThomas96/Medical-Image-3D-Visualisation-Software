#ifndef VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_HPP_
#define VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_HPP_

#include "./generic_image_downsampler.hpp"
#include "./generic_image_reader.hpp"

#include "../../../new_grid/include/grid.hpp"

#include "../../utils/include/local_cache.hpp"
#include "../../utils/include/read_cache.hpp"

namespace Image {

	/// @brief The GenericImageDownsamplerTemplated class is the templated instance of the GenericImageDownsampler class.
	/// @details When instanciating a variable of this class, two template arguments are required : the pixel data type and the downsampling method
	/// as a functor defined elsewhere. Since the user is really unlikely to require changing the downsampling method after loading the grid (and
	/// since it would require a new scan of the original grid anyway), this is passed as a template argument.
	/// @tparam element_t The pixel data type. Determined by the grid we are sampling from
	/// @tparam resampler_t The functor responsible for the resampling algorithm.
	/// @todo Define functors for some interpolation methods : nearest neighbor, [bi/tri]linear, [bi/tri]cubic ...
	template <typename element_t, template <typename, class> class resampler_t>
	class GenericImageDownsamplerTemplated : public GenericImageDownsampler {

	public:
		/// @brief Public typedef to the internal pixel type.
		using pixel_t = element_t;

		/// @brief Public typedef to the internal up/downsampler type (useful when referencing member functions/variables).
		using sampler_t = resampler_t<pixel_t, Grid>;

		/// @brief Public typedef to the current instanciated type.
		using self_t = GenericImageDownsamplerTemplated<pixel_t, resampler_t>;

		/// @brief Pointer type to the current instanciation of GenericImageDownsamplerTemplated.
		using Ptr = std::unique_ptr<self_t>;

		/// @brief The type of the associated read cache.
		using cache_t = ReadCache<std::size_t, std::vector<pixel_t>>;

		/// @brief The type of locally-stored slices.
		using storage_t = Image::LocalCache<std::size_t, std::vector<pixel_t>>;

	protected:
		/// @brief The default ctor for this template, setting the right variables for the instance.
		GenericImageDownsamplerTemplated(svec3 desired_dimension, Grid::Ptr parentgrid, sampler_t resampler);

	public:
		/// @brief Default dtor for the class, set to default for now.
		virtual ~GenericImageDownsamplerTemplated() = default;

		/// @brief Returns a suitable backend for the given grid at the given resolution with the given resampler.
		static Ptr createBackend(svec3 size, Grid::Ptr parentgrid, sampler_t resampler);

		/// @brief Should parse images, but since we're sampling a known grid this just initializes the right variables.
		virtual ThreadedTask::Ptr parseImageInfo(ThreadedTask::Ptr pre_existing_task,
		  const std::vector<std::vector<std::string>>& _filenames) noexcept(false) override;

	protected:
		/// @brief Loads a slice from the parent grid, and cache it for later use.
		/// @note This is not supposed to be called anywhere else than in the parsing/loading function ! All other read
		/// operations are supposed to directly read from the local cache.
		std::size_t load_slice_from_parent_grid(std::size_t slice_idx);

		/// @brief Downsampling operation upon grid loading.
		virtual void downsample_in_separate_thread(ThreadedTask::Ptr progress_tracker) override;

		/// @brief Single function to call when trying to read a sub-region of the parent grid.
		template <typename out_t>
		bool templated_read_region(svec3 read_origin, svec3 read_size, std::vector<out_t>& return_values);

		/// @brief Single function to call when trying to read the sample value ranges of the parent grid.
		template <typename range_t>
		bool templated_read_ranges(std::size_t channel, glm::tvec2<range_t>& ranges);

		//////////////////////////////////////////////////////
		//													//
		//             SUB-REGION READ FUNCTIONS            //
		//													//
		//////////////////////////////////////////////////////

		/// @brief Read a sub-region of the image, implemented in the derived classes.  8-bit unsigned version.
		virtual bool internal_readSubRegion(tag<std::uint8_t> tag, svec3 origin, svec3 size,
		  std::vector<std::uint8_t>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}
		/// @brief Read a sub-region of the image, implemented in the derived classes. 16-bit unsigned version.
		virtual bool internal_readSubRegion(tag<std::uint16_t> tag, svec3 origin, svec3 size,
		  std::vector<std::uint16_t>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}
		/// @brief Read a sub-region of the image, implemented in the derived classes. 32-bit unsigned version.
		virtual bool internal_readSubRegion(tag<std::uint32_t> tag, svec3 origin, svec3 size,
		  std::vector<std::uint32_t>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}
		/// @brief Read a sub-region of the image, implemented in the derived classes. 64-bit unsigned version.
		virtual bool internal_readSubRegion(tag<std::uint64_t> tag, svec3 origin, svec3 size,
		  std::vector<std::uint64_t>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}
		/// @brief Read a sub-region of the image, implemented in the derived classes.  8-bit signed version.
		virtual bool internal_readSubRegion(tag<std::int8_t> tag, svec3 origin, svec3 size,
		  std::vector<std::int8_t>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}
		/// @brief Read a sub-region of the image, implemented in the derived classes. 16-bit signed version.
		virtual bool internal_readSubRegion(tag<std::int16_t> tag, svec3 origin, svec3 size,
		  std::vector<std::int16_t>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}
		/// @brief Read a sub-region of the image, implemented in the derived classes. 32-bit signed version.
		virtual bool internal_readSubRegion(tag<std::int32_t> tag, svec3 origin, svec3 size,
		  std::vector<std::int32_t>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}
		/// @brief Read a sub-region of the image, implemented in the derived classes. 64-bit signed version.
		virtual bool internal_readSubRegion(tag<std::int64_t> tag, svec3 origin, svec3 size,
		  std::vector<std::int64_t>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}
		/// @brief Read a sub-region of the image, implemented in the derived classes, single precision floating point.
		virtual bool internal_readSubRegion(tag<float> tag, svec3 origin, svec3 size,
		  std::vector<float>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}
		/// @brief Read a sub-region of the image, implemented in the derived classes, double precision floating point.
		virtual bool internal_readSubRegion(tag<double> tag, svec3 origin, svec3 size,
		  std::vector<double>& data) override {
			UNUSED(tag);
			return this->templated_read_region(origin, size, data);
		}

		//////////////////////////////////////////////////////
		//													//
		//          VALUE RANGE GETTER FUNCTIONS            //
		//													//
		//////////////////////////////////////////////////////

		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<std::int8_t> tag, std::size_t channel,
		  glm::tvec2<std::int8_t>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}
		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<std::int16_t> tag, std::size_t channel,
		  glm::tvec2<std::int16_t>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}
		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<std::int32_t> tag, std::size_t channel,
		  glm::tvec2<std::int32_t>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}
		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<std::int64_t> tag, std::size_t channel,
		  glm::tvec2<std::int64_t>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}
		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<std::uint8_t> tag, std::size_t channel,
		  glm::tvec2<std::uint8_t>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}
		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<std::uint16_t> tag, std::size_t channel,
		  glm::tvec2<std::uint16_t>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}
		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<std::uint32_t> tag, std::size_t channel,
		  glm::tvec2<std::uint32_t>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}
		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<std::uint64_t> tag, std::size_t channel,
		  glm::tvec2<std::uint64_t>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}
		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<float> tag, std::size_t channel,
		  glm::tvec2<float>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}
		/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
		virtual bool internal_getRangeValues(tag<double> tag, std::size_t channel,
		  glm::tvec2<double>& _values) override {
			UNUSED(tag);
			return this->templated_read_ranges(channel, _values);
		}

	private:
		/// @brief The cached slices, kept in memory
		cache_t read_cache;

		/// @brief The downsampled slices.
		storage_t downsampled_slices;

		/// @brief The resampling method the user required.
		sampler_t resampling_method;
	};

}

#include "./generic_image_downsampler_templated.impl.hpp"

#endif // VISUALISATION_IMAGE_GENERIC_INCLUDE_GENERIC_IMAGE_DOWNSAMPLER_TEMPLATED_HPP_
