#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_

#include "./tiff_private.hpp"

namespace Image {
namespace Tiff {

	template <typename img_t>
	class TIFFReader : public TIFFPrivate {
		public:
			typedef ReadCache<std::size_t, std::vector<img_t>> cache_t;
		protected:
			TIFFReader(uint32_t w, uint32_t h, std::size_t _dim);
		public:
			/// @b Default dtor for the backend, which releases the cached data.
			virtual ~TIFFReader(void) { this->cachedSlices.clearCache(); };

			static TIFFPrivate::Ptr createTIFFBackend(uint32_t width, uint32_t height, std::size_t dim) {
				return TIFFPrivate::Ptr(new TIFFReader<img_t>(width, height, dim));
			}

			virtual svec3 getResolution() const override { return svec3(this->width, this->height, this->images.size()); }
			virtual glm::vec3 getVoxelDimensions() const override { return glm::vec3(1.f); }
			virtual std::size_t getVoxelDimensionality() const override { return this->voxel_dimensionalty; }

			/// @b Returns the range of values present (or representable) in the file, converted to the 'target_t' type.
			template <typename source_t> TIFFReader& setRangeValues(glm::vec<2, source_t, glm::defaultp>& _range);

			/// @b Template to read a single pixel's value(s) in the image.
			/// @note This will call `pImpl->readPixel<>();`.
			template <typename target_t> void readPixel(svec3 index, std::vector<target_t>& values);

			/// @b Template to read a single line of voxels in ihe image.
			/// @note This will call `pImpl->readLine<>();`.
			template <typename target_t> void readLine(svec2 line_idx, std::vector<target_t>& values);

			/// @b Template to read a whole slice of voxels in the image at once.
			/// @note This will call `pImpl->readSlice<>();`.
			template <typename target_t> void readSlice(std::size_t slice_idx, std::vector<target_t>& values);

		protected:
			/// @b Loads and puts into the cache the slice requested. Returns its cache index if loaded previously.
			/// @details The loaded slice will be not only loaded, but converted and put into the cache encoded in
			/// this class' internal type.
			/// @note This function will be the main source of reading procedures for the class
			virtual std::size_t loadAndCacheSlice(std::size_t);

			/// @b Reads the given subregion and converts it to the output value type into the given vector.
			template <typename out_t>
			bool template_tiff_read_sub_region(svec3 origin, svec3 size, std::vector<out_t>& values);

			template <typename range_t>
			bool template_tiff_get_sub_range_values(std::size_t channel, glm::vec<2, range_t, glm::defaultp>& _values);

			//////////////////////////////////////////////////////
			//													//
			//             SUB-REGION READ FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @b Read a sub-region of the image, implemented in the derived classes.  8-bit unsigned version.
			virtual bool tiff_readSubRegion(tag<std::uint8_t> tag, svec3 origin, svec3 size,
											std::vector<std::uint8_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit unsigned version.
			virtual bool tiff_readSubRegion(tag<std::uint16_t> tag, svec3 origin, svec3 size,
											std::vector<std::uint16_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit unsigned version.
			virtual bool tiff_readSubRegion(tag<std::uint32_t> tag, svec3 origin, svec3 size,
											std::vector<std::uint32_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit unsigned version.
			virtual bool tiff_readSubRegion(tag<std::uint64_t> tag, svec3 origin, svec3 size,
											std::vector<std::uint64_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes.  8-bit   signed version.
			virtual bool tiff_readSubRegion(tag<std::int8_t> tag, svec3 origin, svec3 size,
											std::vector<std::int8_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit   signed version.
			virtual bool tiff_readSubRegion(tag<std::int16_t> tag, svec3 origin, svec3 size,
											std::vector<std::int16_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit   signed version.
			virtual bool tiff_readSubRegion(tag<std::int32_t> tag, svec3 origin, svec3 size,
											std::vector<std::int32_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit   signed version.
			virtual bool tiff_readSubRegion(tag<std::int64_t> tag, svec3 origin, svec3 size,
											std::vector<std::int64_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes, single precision floating point.
			virtual bool tiff_readSubRegion(tag<float> tag, svec3 origin, svec3 size,
											std::vector<float>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @b Read a sub-region of the image, implemented in the derived classes, double precision floating point.
			virtual bool tiff_readSubRegion(tag<double> tag, svec3 origin, svec3 size,
											std::vector<double>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}

			//////////////////////////////////////////////////////
			//													//
			//          VALUE RANGE GETTER FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			virtual bool tiff_getRangeSubValues(tag<std::int8_t> tag, std::size_t channel,
												 glm::vec<2, std::int8_t, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}
			virtual bool tiff_getRangeSubValues(tag<std::int16_t> tag, std::size_t channel,
												 glm::vec<2, std::int16_t, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}
			virtual bool tiff_getRangeSubValues(tag<std::int32_t> tag, std::size_t channel,
												 glm::vec<2, std::int32_t, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}
			virtual bool tiff_getRangeSubValues(tag<std::int64_t> tag, std::size_t channel,
												 glm::vec<2, std::int64_t, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}
			virtual bool tiff_getRangeSubValues(tag<std::uint8_t> tag, std::size_t channel,
												 glm::vec<2, std::uint8_t, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}
			virtual bool tiff_getRangeSubValues(tag<std::uint16_t> tag, std::size_t channel,
												 glm::vec<2, std::uint16_t, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}
			virtual bool tiff_getRangeSubValues(tag<std::uint32_t> tag, std::size_t channel,
												 glm::vec<2, std::uint32_t, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}
			virtual bool tiff_getRangeSubValues(tag<std::uint64_t> tag, std::size_t channel,
												 glm::vec<2, std::uint64_t, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}
			virtual bool tiff_getRangeSubValues(tag<float> tag, std::size_t channel,
												 glm::vec<2, float, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}
			virtual bool tiff_getRangeSubValues(tag<double> tag, std::size_t channel,
												 glm::vec<2, double, glm::defaultp>& values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, values);
			}

		protected:
			/// @b The min and max of the image (if specified in the header)
			/// @details If specified in the header, those values will be returned. Instead, returns the min/max of the
			/// data type the image is saved as.
			std::vector<glm::vec<2, img_t, glm::defaultp>> dataRange;

			/// @b A cache of recently loaded slices.
			cache_t cachedSlices;
	};

}
}

#include "./tiff_template.impl.hpp"

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_
