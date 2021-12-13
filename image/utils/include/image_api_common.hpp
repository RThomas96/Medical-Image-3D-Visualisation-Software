#ifndef VISUALIZATION_IMAGE_API_IMAGE_API_INCLUDE_COMMON_HPP_
#define VISUALIZATION_IMAGE_API_IMAGE_API_INCLUDE_COMMON_HPP_

#include "../../macros.hpp"

//#include "../../grid/include/bounding_box.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <ostream>
#include <type_traits>
#include <typeinfo>

// A compiler-specific header, used to un-mangle names returned by typeid<>::name()
#include <cxxabi.h>

#define PRINT_FN_ENTRY
//#define PRINT_FN_ENTRY std::cerr << "Entry point : " << __PRETTY_FUNCTION__ << "\n"

namespace Image {

	/// @brief Simple typedef for a 3-component GLM vector, useful for image resolution
	typedef glm::tvec2<std::size_t> svec2;

	/// @brief Simple typedef for a 3-component GLM vector, useful for image resolution
	typedef glm::tvec3<std::size_t> svec3;

	/// @brief The ImageDataType enum allows to know what bit-width and signed-ness an image implementation really holds.
	/// @note This enum is OR-able, in order to do things like : `imgType = (Bit_8 | Unsigned); // ==> uint8_t | uchar`
	enum ImageDataType {
		Unknown	 = 0b00000000,
		Bit_8	 = 0b00000001,
		Bit_16	 = 0b00000010,
		Bit_32	 = 0b00000100,
		Bit_64	 = 0b00001000,
		Unsigned = 0b00010000,
		Floating = 0b00100000,
		Signed	 = 0b01000000,
	};

	/// @brief The ImageResamplingTechnique enum allows to pick a sampling method for re-sampled grids at runtime for a given instance.
	/// @note This is only for information purposes, it allows to pick the right functor when instanciating the downsampled grid interface and to
	/// query it later from elsewhere in the program.
	enum ImageResamplingTechnique {
		None			= 0x00,
		NearestNeighbor = 0x01,
		Linear			= 0x02,
		Cubic			= 0x04,
	};

	/// @brief Allows to check for the status of a particular flag in the image type enum.
	inline bool operator&(ImageDataType lhs, ImageDataType rhs) {
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	/// @brief Makes the ImageDataType enumeration OR-able, to have multiple flags.
	/// @details Disallows to set the same flag multiple times.
	inline ImageDataType operator|(ImageDataType lhs, ImageDataType rhs) {
		if (lhs & rhs) {
			return lhs;
		}
		return static_cast<ImageDataType>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
	}

	/// @brief Boolean operation to check if a certain enum value has been OR-ed into a variable.
	inline bool operator&(ImageResamplingTechnique lhs, ImageResamplingTechnique rhs) {
		// Enum is only defined in 8 bits :
		using data_t = unsigned char;
		return (static_cast<data_t>(lhs)) && (static_cast<data_t>(rhs));
	}

	/// @brief Handy typedef to a bounding box type
	//typedef BoundingBox_General<float> bbox_t;

	inline std::ostream& operator<<(std::ostream& _os, ImageDataType d) {
		if (d & ImageDataType::Unknown) {
			_os << "<unknown>";
			return _os;
		}
		_os << "type : ";
		if (d & ImageDataType::Unsigned) {
			_os << "unsigned ";
		}
		if (d & ImageDataType::Floating) {
			_os << "floating ";
		}
		if (d & ImageDataType::Signed) {
			_os << "signed ";
		}
		if (d & ImageDataType::Bit_8) {
			_os << "width : 8b ";
		}
		if (d & ImageDataType::Bit_16) {
			_os << "width : 16b ";
		}
		if (d & ImageDataType::Bit_32) {
			_os << "width : 32b ";
		}
		if (d & ImageDataType::Bit_64) {
			_os << "width : 64b ";
		}
		return _os;
	}

	/// @brief A simple tagging system, allowing for explicit concrete function calls from a template interface.
	template <typename restict_t>
	struct tag
	{};

	/// @brief Simple extraction operator to print the type of the tag to something like std::cout, std::cerr ...
	template <typename tag_t>
	inline std::ostream& operator<<(std::ostream& _os, tag<tag_t> t) {
		UNUSED(t);
		int status			 = 0;
		char* unmangled_type = abi::__cxa_demangle(typeid(tag_t).name(), nullptr, nullptr, &status);
		if (status == -1) {
			unmangled_type = (char*) "<malloc_error>";
		}
		if (status == -2) {
			unmangled_type = (char*) "<invalid_typename>";
		}
		if (status == -3) {
			unmangled_type = (char*) "<invalid_arg>";
		}
		return _os << "{tag_t: " << unmangled_type << "}";
	}

	inline double getMinNumericLimit(ImageDataType imageDataType) {
		if (imageDataType == (Bit_8 | Signed))
			return static_cast<double>(std::numeric_limits<int8_t>::lowest());

		if (imageDataType == (Bit_16 | Signed))
			return static_cast<double>(std::numeric_limits<int16_t>::lowest());

		if (imageDataType == (Bit_32 | Signed))
			return static_cast<double>(std::numeric_limits<int32_t>::lowest());

		if (imageDataType == (Bit_64 | Signed))
			return static_cast<double>(std::numeric_limits<int64_t>::lowest());

		if (imageDataType == (Bit_8 | Unsigned))
			return static_cast<double>(std::numeric_limits<uint8_t>::lowest());

		if (imageDataType == (Bit_16 | Unsigned))
			return static_cast<double>(std::numeric_limits<uint16_t>::lowest());

		if (imageDataType == (Bit_32 | Unsigned))
			return static_cast<double>(std::numeric_limits<uint32_t>::lowest());

		if (imageDataType == (Bit_64 | Unsigned))
			return static_cast<double>(std::numeric_limits<uint64_t>::lowest());

		if (imageDataType == (Bit_32 | Floating))
			return static_cast<double>(std::numeric_limits<float>::lowest());

		if (imageDataType == (Bit_64 | Floating))
			return static_cast<double>(std::numeric_limits<double>::lowest());

		if (imageDataType == Unknown)
			throw std::runtime_error("[ERROR] cannot query min numeric limit of an unknown data type.");
	}

	inline double getMaxNumericLimit(ImageDataType imageDataType) {
		if (imageDataType == (Bit_8 | Signed))
			return static_cast<double>(std::numeric_limits<int8_t>::max());

		if (imageDataType == (Bit_16 | Signed))
			return static_cast<double>(std::numeric_limits<int16_t>::max());

		if (imageDataType == (Bit_32 | Signed))
			return static_cast<double>(std::numeric_limits<int32_t>::max());

		if (imageDataType == (Bit_64 | Signed))
			return static_cast<double>(std::numeric_limits<int64_t>::max());

		if (imageDataType == (Bit_8 | Unsigned))
			return static_cast<double>(std::numeric_limits<uint8_t>::max());

		if (imageDataType == (Bit_16 | Unsigned))
			return static_cast<double>(std::numeric_limits<uint16_t>::max());

		if (imageDataType == (Bit_32 | Unsigned))
			return static_cast<double>(std::numeric_limits<uint32_t>::max());

		if (imageDataType == (Bit_64 | Unsigned))
			return static_cast<double>(std::numeric_limits<uint64_t>::max());

		if (imageDataType == (Bit_32 | Floating))
			return static_cast<double>(std::numeric_limits<float>::max());

		if (imageDataType == (Bit_64 | Floating))
			return static_cast<double>(std::numeric_limits<double>::max());

		if (imageDataType == Unknown)
			throw std::runtime_error("[ERROR] cannot query max numeric limit of an unknown data type.");
	}

}	 // namespace Image

#endif	  // VISUALIZATION_IMAGE_API_IMAGE_API_INCLUDE_COMMON_HPP_
