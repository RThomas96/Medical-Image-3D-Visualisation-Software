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
			TIFFReader(uint32_t w, uint32_t h);
		public:
			/// @b Default dtor for the backend, which releases the cached data.
			virtual ~TIFFReader(void) { this->cachedSlices.clearCache(); };

			static TIFFPrivate::Ptr createTIFFBackend(uint32_t width, uint32_t height) {
				return TIFFPrivate::Ptr(new TIFFReader<img_t>(width, height));
			}

			/// @b Returns the range of values present (or representable) in the file, converted to the 'target_t' type.
			template <typename source_t> TIFFReader& setRangeValues(glm::vec<2, source_t, glm::defaultp>& _range);

			/// @b Returns the range of values present (or representable) in the file, converted to the 'target_t' type.
			template <typename target_t> bool getRangeValues(glm::vec<2, target_t, glm::defaultp>& _range) const;

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
			virtual std::size_t loadAndCacheSlice(std::size_t) ;

		protected:
			/// @b The min and max of the image (if specified in the header)
			/// @details If specified in the header, those values will be returned. Instead, returns the min/max of the
			/// data type the image is saved as.
			glm::vec<2, img_t, glm::defaultp> dataRange;

			/// @b A cache of recently loaded slices.
			ReadCache<std::size_t, std::vector<img_t>> cachedSlices;
	};

}
}

#include "./tiff_template.impl.hpp"

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_
