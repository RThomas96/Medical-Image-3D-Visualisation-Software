#ifndef MACROS_HPP
#define MACROS_HPP

/**
This file exists to define some macros used through the program.
**/
#include <glm/glm.hpp>
#include <type_traits>
#include <iostream>

/// @brief Allows to restrict a template to use only floating point values for the type T.
template <typename T>
using restrict_floating_point_check = typename std::enable_if<std::is_floating_point<T>::value>::type*;

/// @brief Allows to restrict a template to use only integer values for type T
template <typename T>
using restrict_integer_check = typename std::enable_if<std::is_integral<T>::value>::type*;

namespace glm {

	template <typename out, typename in, qualifier prec>
	GLM_FUNC_DECL GLM_CONSTEXPR glm::vec<3, out, prec> convert_to(glm::vec<3, in, prec> origin) {
		return glm::vec<3, out, prec>(
			static_cast<out>(origin.x),
			static_cast<out>(origin.y),
			static_cast<out>(origin.z)
		);
	}

	template <typename out, typename in, qualifier prec>
	GLM_FUNC_DECL GLM_CONSTEXPR glm::vec<4, out, prec> convert_to(glm::vec<4, in, prec> origin) {
		return glm::vec<4, out, prec>(
			static_cast<out>(origin.x),
			static_cast<out>(origin.y),
			static_cast<out>(origin.z),
			static_cast<out>(origin.a)
		);
	}

} // glm namespace

#define FUNC_ERR(s) __attribute__((error(s)))
#define FUNC_WARN(s) __attribute__((warning(s)))

#define FUNC_FLATTEN __attribute__((flatten))
#define FUNC_ALWAYS_INLINE __attribute__((always_inline))

#if defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Do we want the shader contents to show on compilation failure ?
// Define this for yes, comment out for no :
// #define ENABLE_SHADER_CONTENTS_ON_FAILURE

// What's the size of data we want to load in the program ?
//#define VISUALISATION_USE_UINT8		// the program uses uint8_t, or uchar
#define VISUALISATION_USE_UINT16	// the program uses uint16_t, or ushort

// Prints whatever's fed as parameter, no expression evaluation
// (useful for printing function names for example, or macros as their name)
#define pnt(x) #x

#define LOG_ENTER(x) std::cerr << "[LOG] Entered function " << pnt(x) << " :\n";
#define LOG_LEAVE(x) std::cerr << "[LOG] Left function " << pnt(x) << " :\n";
#define PRINTVAL_NAMED(x, v) std::cerr << "[LOG] Value of " << pnt(x) << " : " << +v << '\n';
#define PRINTVAL_VAR(x, v) std::cerr << "[LOG] Value of " << x << " : " << +v << '\n';
#define PRINTVAL(x) PRINTVAL_NAMED(x, x)

/// @brief Template used to iterate over some const containers, to get the underlying object
template<class T>
std::remove_reference_t<T> const& as_const(T&&t){return t;}

#endif // MACROS_HPP
