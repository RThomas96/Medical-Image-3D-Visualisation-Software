#ifndef VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_TEMPLATE_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_TEMPLATE_HPP_

#include "./generic_image_reader.hpp"
#include "./generic_image_reader_subregion.hpp"
#include "../../utils/include/read_cache.hpp"

#include "../../new_grid/include/grid.hpp"

namespace Image {

	template <typename element_t>
	class GenericImageReaderSubregionTemplated : public GenericImageReaderSubregion {
	public:
		/// @brief Public typedef to the internal pixel type.
		typedef element_t pixel_t;

		/// @brief Pointer type to reference this backend implementation detail.
		typedef std::unique_ptr<GenericImageReaderSubregionTemplated<pixel_t>> Ptr;

		/// @brief The type of a simple read cache, keeping the last few slices in memory.
		typedef ReadCache<std::size_t, std::vector<pixel_t>> cache_t;

	protected:
		/// @brief Sets the right variables when creating a grid.
		GenericImageReaderSubregionTemplated(svec3 origin, svec3 size, Grid::Ptr parent_grid);

	public:
		virtual ~GenericImageReaderSubregionTemplated(void);

		/// @brief Create an appropriate backend for this class.
		static Ptr createBackend(svec3 origin, svec3 size, Grid::Ptr parent);

	protected:
		/// @brief Loads a slice from the parent grid, and cache it for later use.
		std::size_t load_slice_from_parent_grid(std::size_t slice_idx);

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

	protected:
		/// @brief The simple read cache, keeping some slices in memory.
		cache_t read_cache;
	};

}	 // namespace Image

#include "./generic_image_reader_subregion_templated.impl.hpp"

#endif	  // VISUALIZATION_IMAGE_API_INCLUDE_GRID_SUBREGION_TEMPLATE_HPP_
