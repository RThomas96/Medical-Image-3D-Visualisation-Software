#ifndef IMAGE_INCLUDE_DATA_INTERFACE_HPP_
#define IMAGE_INCLUDE_DATA_INTERFACE_HPP_

#include "../../macros.hpp"
#include "./interpolator.hpp"

#include <memory>
#include <vector>

namespace IO {

	template <typename T>
	class DataInterface : public std::enable_shared_from_this<DataInterface<T>> {
		public:
			/// @brief Simple typedef for the current template parameter typename.
			/// @details need to be re-defined in derived classes.
			using data_t = T;
			/// @brief Pixel type, used for getPixel().
			/// @details Similar to OpenGL, whatever the dimensionnality of the data, queries return a vec4.
			using pixel_t = glm::vec<4, T, glm::defaultp>;
		public:
			DataInterface() = default;
			~DataInterface(void) = default;
		public:
			/// @brief Common interface to load information from the disk, overriden in derived classes.
			virtual void loadInformation(void);
			/// @brief Common interface to write information to the disk, overriden in derived classes.
			virtual void writeInformation(void);

			/// @brief Sets the data threshold for the class.
			virtual DataInterface<T>& setDataThreshold(const T _thresh) = 0;

			/// @brief Sets the filenames for the class.
			virtual DataInterface<T>& setFilenames(const std::vector<std::string>& fnames) = 0;
			/// @brief Sets the file basename for file formats that are written in multiple files.
			virtual DataInterface<T>& setFileBasename(const std::string& name) = 0;

			/// @brief Sets the interpolation structure used to downsample images.
			virtual DataInterface<T>& setInterpolator(const Interpolators::genericInterpolator<T>& i) = 0;

			/// @brief Returns a pixel's data, in the grid.
			virtual pixel_t getPixel(std::size_t i, std::size_t j, std::size_t k) = 0;
			/// @brief Set the pixel at coordinates (I,J,K) of channel 'C' to data 'D'
			virtual DataInterface<T>& setPixel(std::size_t i, std::size_t j, std::size_t k, std::size_t c, data_t d) = 0;
	};

}

#endif // IMAGE_INCLUDE_DATA_INTERFACE_HPP_
