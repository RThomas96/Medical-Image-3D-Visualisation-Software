#include "../include/tiff_helpers.hpp"

namespace Image {

	namespace Tiff {

		void tiff_error_redirection(const char* module, const char* fmt, va_list _va_) {
			UNUSED(module)
			UNUSED(fmt)
			UNUSED(_va_)
		}

		void tiff_warning_redirection(const char* module, const char* fmt, va_list _va_){
		  UNUSED(module)
			UNUSED(fmt)
			  UNUSED(_va_)}

		tdir_t countDirectories(std::string_view _file_name) {
			TIFF* file = TIFFOpen(_file_name.data(), "r");
			tdir_t nb  = TIFFNumberOfDirectories(file);
			TIFFClose(file);
			return nb;
		}

	}	 // namespace Tiff

}	 // namespace Image
