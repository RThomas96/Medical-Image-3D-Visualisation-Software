#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_

#include "./tiff_private.hpp"
#include "../../api/include/read_cache.hpp"

namespace Image {

	template <typename img_t>
	class TIFFReader : public Tiff::TIFFPrivate {
		protected:
			TIFFReader();
		public:
			/// @b Default dtor for the backend, which releases the cached data.
			virtual ~TIFFReader(void) { this->cachedSlices.clearCache(); };

			template <typename target_t>
			void getRangeValues(glm::vec<2, target_t, glm::defaultp>& _range);

		protected:
			/// @b The min and max of the image (if specified in the header)
			/// @details If specified in the header, those values will be returned. Instead, returns the min/max of the
			/// data type the image is saved as.
			glm::vec<2, img_t, glm::defaultp> dataRange;

			/// @b A cache of recently loaded slices.
			ReadCache<std::size_t, std::vector<img_t>> cachedSlices;
	};

	template <>
	TIFFReader<float>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_32;
	}

	template <>
	TIFFReader<double>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_64;
	}

	template <>
	TIFFReader<std::uint8_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_8;
	}

	template <>
	TIFFReader<std::uint16_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_16;
	}

	template <>
	TIFFReader<std::uint32_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_32;
	}

	template <>
	TIFFReader<std::uint64_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_64;
	}

	template <>
	TIFFReader<std::int8_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_8;
	}

	template <>
	TIFFReader<std::int16_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_16;
	}

	template <>
	TIFFReader<std::int32_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_32;
	}

	template <>
	TIFFReader<std::int64_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_64;
	}
}

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_
