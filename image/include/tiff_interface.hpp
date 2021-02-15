#ifndef IMAGE_INCLUDE_TIFF_INTERFACE_HPP_
#define IMAGE_INCLUDE_TIFF_INTERFACE_HPP_

#include "./disk_interface.hpp"

#include "../../TinyTIFF/src/tinytiffreader.h"
#include "../../TinyTIFF/src/tinytiffwriter.h"

namespace IO {

	template <typename T>
	class StackedTIFFInterface : public DiskInterface<T> {
		public:
			StackedTIFFInterface(void);
			~StackedTIFFInterface(void);
			/// @brief Sets the filenames for the interface, to either read or write the files.
			virtual StackedTIFFInterface<T>& setFilenames(const std::vector<std::string>& filenames) override;
		protected:
			TinyTIFFReaderFile* reader;
			TinyTIFF
	};

}

#endif // IMAGE_INCLUDE_TIFF_INTERFACE_HPP_
