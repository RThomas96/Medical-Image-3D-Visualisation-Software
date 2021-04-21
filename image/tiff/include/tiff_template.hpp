#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_

#include "./backend.hpp"
#include "../../api/include/read_cache.hpp"

namespace Image {

	template <typename img_t>
	class TIFFReader : public TIFFBackend {
		protected:
			TIFFReader(std::vector<std::vector<std::string>> fns) ;
		public:
			/// @b Default dtor for the backend, which releases the cached data.
			virtual ~TIFFReader(void) {
				this->cachedSlices.clearCache();
			};

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
	TIFFReader<float>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_32;
	}

	template <>
	TIFFReader<double>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_64;
	}

	template <>
	TIFFReader<std::uint8_t>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_8;
	}

	template <>
	TIFFReader<std::uint16_t>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_16;
	}

	template <>
	TIFFReader<std::uint32_t>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_32;
	}

	template <>
	TIFFReader<std::uint64_t>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_64;
	}

	template <>
	TIFFReader<std::int8_t>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_8;
	}

	template <>
	TIFFReader<std::int16_t>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_16;
	}

	template <>
	TIFFReader<std::int32_t>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_32;
	}

	template <>
	TIFFReader<std::int64_t>::TIFFReader(std::vector<std::vector<std::string>> fns) : TIFFBackend(fns) {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_64;
	}
}

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_
