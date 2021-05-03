#ifndef VISUALIZATION_IMAGE_API_IMAGE_API_INCLUDE_COMMON_HPP_
#define VISUALIZATION_IMAGE_API_IMAGE_API_INCLUDE_COMMON_HPP_

#include "../../macros.hpp"

#include "../../grid/include/bounding_box.hpp"

#include <glm/glm.hpp>

#include <type_traits>
#include <typeinfo>
#include <memory>
#include <ostream>

// A compiler-specific header, used to un-mangle names returned by typeid<>::name()
#include <cxxabi.h>

//#define PRINT_FN_ENTRY
#define PRINT_FN_ENTRY std::cerr << "Entry point : " << __PRETTY_FUNCTION__ << "\n"

namespace Image {

	/// @b Simple typedef for a 3-component GLM vector, useful for image resolution
	typedef glm::vec<2, std::size_t, glm::defaultp> svec2;

	/// @b Simple typedef for a 3-component GLM vector, useful for image resolution
	typedef glm::vec<3, std::size_t, glm::defaultp> svec3;

	/**
	 * @brief The ImageDataType enum allows to know what bit-width and signed-ness an image implementation really holds.
	 * @note This enum is OR-able, in order to do things like : `imgType = (Bit_8 | Unsigned); // ==> uint8_t | uchar`
	 */
	enum ImageDataType {
		Unknown  = 0b00000000,
		Bit_8    = 0b00000001,
		Bit_16   = 0b00000010,
		Bit_32   = 0b00000100,
		Bit_64   = 0b00001000,
		Unsigned = 0b00010000,
		Floating = 0b00100000,
		Signed   = 0b01000000,
	};

	/// @b Allows to check for the status of a particular flag in the image type enum.
	inline bool operator& (ImageDataType lhs, ImageDataType rhs) {
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	/// @b Makes the ImageDataType enumeration OR-able, to have multiple flags.
	/// @details Disallows to set the same flag multiple times.
	inline ImageDataType operator| (ImageDataType lhs, ImageDataType rhs) {
		if (lhs & rhs) { return lhs; }
		return static_cast<ImageDataType>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
	}

	/// @b Handy typedef to a bounding box type
	typedef BoundingBox_General<float> bbox_t;

	inline std::ostream& operator<<(std::ostream& _os, ImageDataType d) {
		if (d & ImageDataType::Unknown) { _os << "<unknown>"; return _os; }
		_os << "type : ";
		if (d & ImageDataType::Unsigned) { _os << "unsigned "; }
		if (d & ImageDataType::Floating) { _os << "floating "; }
		if (d & ImageDataType::Signed) { _os << "signed "; }
		if (d & ImageDataType::Bit_8) { _os << "width : 8b "; }
		if (d & ImageDataType::Bit_16) { _os << "width : 16b "; }
		if (d & ImageDataType::Bit_32) { _os << "width : 32b "; }
		if (d & ImageDataType::Bit_64) { _os << "width : 64b "; }
		return _os;
	}

	/// @b A simple tagging system, allowing for explicit concrete function calls from a template interface.
	template <typename restict_t> struct tag {};

	/// @b Simple extraction operator to print the type of the tag to something like std::cout, std::cerr ...
	template <typename tag_t> inline std::ostream& operator<<(std::ostream& _os, tag<tag_t> t) {
		UNUSED_PARAMETER(t);
		int status = 0;
		char* unmangled_type = abi::__cxa_demangle(typeid(tag_t).name(), nullptr, nullptr, &status);
		if (status == -1) { unmangled_type = (char*)"<malloc_error>"; }
		if (status == -2) { unmangled_type = (char*)"<invalid_typename>"; }
		if (status == -3) { unmangled_type = (char*)"<invalid_arg>"; }
		return _os << "{tag_t: " << unmangled_type << "}";
	}

}

#endif // VISUALIZATION_IMAGE_API_IMAGE_API_INCLUDE_COMMON_HPP_
