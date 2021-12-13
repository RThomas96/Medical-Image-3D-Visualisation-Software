#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HELPERS_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HELPERS_HPP_

#include "./tiff_include_common.hpp"

namespace Image {

	namespace Tiff {

		/// @brief Redirects the error messages from the TIFF files to nothing.
		void tiff_error_redirection(const char* module, const char* fmt, va_list _va_);

		/// @brief Redirects the warning messages from the TIFF files to nothing.
		void tiff_warning_redirection(const char* module, const char* fmt, va_list _va_);

		/// @brief Counts the available IFDs in a TIFF file of given 'name'
		tdir_t countDirectories(std::string_view file_name);

	}	 // namespace Tiff

}	 // namespace Image

#endif	  // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HELPERS_HPP_
