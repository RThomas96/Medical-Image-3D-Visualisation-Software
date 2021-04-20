#ifndef VISUALIZATION_IMAGE_API_IMAGE_API_INCLUDE_COMMON_HPP_
#define VISUALIZATION_IMAGE_API_IMAGE_API_INCLUDE_COMMON_HPP_

#include <glm/glm.hpp>

#include <type_traits>
#include <memory>

namespace Image {
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

	/// @b Simple typedef for a 3-component GLM vector, useful for image resolution
	typedef glm::vec<3, std::size_t, glm::defaultp> svec3;

}

#endif // VISUALIZATION_IMAGE_API_IMAGE_API_INCLUDE_COMMON_HPP_
