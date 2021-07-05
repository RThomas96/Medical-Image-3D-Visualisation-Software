#ifndef VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_HPP_
#define VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_HPP_

#include "./backend.hpp"

namespace Image {

	/// @b The implementation detail
	template <typename element_type>
	class TIFFBackendDetail : public TIFFBackendImpl {
		public:
			/// @b The type of the data loaded in.
			typedef element_type pixel_t;
			/// @b Redefinition of the pointer type for this particular class.
			typedef std::unique_ptr<TIFFBackendDetail<element_type>> Ptr;
			/// @b Typedef for the cache structure which holds recently accessed data.
			typedef ReadCache<std::size_t, std::vector<element_type>> cache_t;

		protected:
			/// @b Default ctor for the class, initializing the right variables.
			TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim);

		public:
			/// @b Default dtor for the class, removing all allocated data from the program's memory pool.
			~TIFFBackendDetail(void);

			/// @b Static function, creating a backend with the given specs.
			static Ptr createBackend(uint32_t width, uint32_t height, std::size_t dim);

		protected:
			/// @b Asks the cache for a given slice 's'. If already cached, return the data but load it otherwise.
			/// @note Asking to load slices this way loads the _whole slice_. If it's made up of 10 samples across
			/// multiple images, it's gonna load that and cache the result.
			virtual std::size_t loadSlice(std::size_t s);

			/// @b Reads data, converted in the right type into the given vector from a specific subregion of the image.
			/// @returns True if the region was successfully read, and false otherwise.
			template <typename out_t>
			bool template_tiff_read_sub_region(svec3 origin, svec3 size, std::vector<out_t>& values);

			/// @b Reads the min and max ranges of the image, if available.
			/// @note If not available in the metadata, returns the internal type's min and max values.
			template <typename range_t>
			bool template_tiff_get_sub_range_values(std::size_t channel, glm::vec<2, range_t, glm::defaultp>& _values);

			//////////////////////////////////////////////////////
			//													//
			//             SUB-REGION READ FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @b Read a sub-region of the image, implemented in the derived classes.  8-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint8_t> tag, svec3 origin, svec3 size,
									std::vector<std::uint8_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint16_t> tag, svec3 origin, svec3 size,
									std::vector<std::uint16_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint32_t> tag, svec3 origin, svec3 size,
									std::vector<std::uint32_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint64_t> tag, svec3 origin, svec3 size,
									std::vector<std::uint64_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes.  8-bit   signed version.
			virtual bool internal_readSubRegion(tag<std::int8_t> tag, svec3 origin, svec3 size,
									std::vector<std::int8_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit   signed version.
			virtual bool internal_readSubRegion(tag<std::int16_t> tag, svec3 origin, svec3 size,
									std::vector<std::int16_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit   signed version.
			virtual bool internal_readSubRegion(tag<std::int32_t> tag, svec3 origin, svec3 size,
									std::vector<std::int32_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit   signed version.
			virtual bool internal_readSubRegion(tag<std::int64_t> tag, svec3 origin, svec3 size,
									std::vector<std::int64_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes, single precision floating point.
			virtual bool internal_readSubRegion(tag<float> tag, svec3 origin, svec3 size,
									std::vector<float>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes, double precision floating point.
			virtual bool internal_readSubRegion(tag<double> tag, svec3 origin, svec3 size,
									std::vector<double>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}

			//////////////////////////////////////////////////////
			//													//
			//          VALUE RANGE GETTER FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int8_t> tag, std::size_t channel,
												glm::vec<2, std::int8_t, glm::defaultp>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int16_t> tag, std::size_t channel,
												glm::vec<2, std::int16_t, glm::defaultp>& _values) override	{
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int32_t> tag, std::size_t channel,
												glm::vec<2, std::int32_t, glm::defaultp>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int64_t> tag, std::size_t channel,
												glm::vec<2, std::int64_t, glm::defaultp>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint8_t> tag, std::size_t channel,
												glm::vec<2, std::uint8_t, glm::defaultp>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint16_t> tag, std::size_t channel,
												glm::vec<2, std::uint16_t, glm::defaultp>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint32_t> tag, std::size_t channel,
												glm::vec<2, std::uint32_t, glm::defaultp>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint64_t> tag, std::size_t channel,
												glm::vec<2, std::uint64_t, glm::defaultp>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<float> tag, std::size_t channel,
												glm::vec<2, float, glm::defaultp>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<double> tag, std::size_t channel,
												glm::vec<2, double, glm::defaultp>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}

		protected:
			/// @b The cached slices, in order to speed up offline computation.
			cache_t cachedSlices;
	};

}

#endif // VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_HPP_
