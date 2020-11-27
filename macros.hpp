#ifndef MACROS_HPP
#define MACROS_HPP

/**
This file exists to define some macros used through the program.
**/

#include <type_traits>

/// @brief Allows to restrict a template to use only floating point values for the type T.
template <typename T>
using restrict_floating_point_check = typename std::enable_if<std::is_floating_point<T>::value>::type*;

#define FLATTEN __attribute__((flatten))

/* Copy paste this to have trace output at debug time
#ifndef NDEBUG
	std::cerr << "[TRACE][" << __PRETTY_FUNCTION__ << "] : \n";
#endif
*/

#endif // MACROS_HPP
