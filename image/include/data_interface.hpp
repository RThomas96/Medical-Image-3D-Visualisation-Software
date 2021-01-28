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
			/// @brief Position and type within the image.
			/// @details Allows to get a position (X,Y,Z). The color channel will be separate.
			using pos_t = glm::vec<3, std::size_t, glm::defaultp>;
			/// @brief Size type within the image.
			/// @details Allows to define a size (X,Y,Z), with the number of color channels as 'A'.
			using size_t = glm::vec<3, std::size_t, glm::defaultp>;

		protected:
			/// @brief Default constructor. Made protected so as not to call it directly.
			DataInterface() = default;

		public:
			/// @brief Default deletor. Will release the resources allocated in this class.
			~DataInterface(void) = default;

			/// @brief Common interface to load information from the disk, overriden in derived classes.
			virtual void loadInformation(void) = 0;
			/// @brief Common interface to write information to the disk, overriden in derived classes.
			virtual void writeInformation(void) = 0;

			/// @brief Get the currently set data threshold.
			virtual data_t getDataThreshold(void) const = 0;
			/// @brief Sets the data threshold for the class.
			virtual DataInterface<T>& setDataThreshold(const data_t _thresh) = 0;

			/// @brief Get the image's resolution.
			virtual size_t getResolution(void) const = 0;
			/// @brief Sets the resolution of the image.
			virtual DataInterface<data_t>& setResolution(size_t s) = 0;

			/// @brief Get the image's voxel sizes. If it cannot be read, this will return (1,1,1).
			virtual glm::vec3 getVoxelDimensions(void) const = 0;
			/// @brief Sets the voxel resolution for the image.
			virtual DataInterface<data_t>& setVoxelDimensions(glm::vec3 dims) = 0;

			/// @brief Returns a pixel's data, in the grid.
			virtual pixel_t getPixel(std::size_t i, std::size_t j, std::size_t k) const = 0;
			/// @brief Set the pixel at coordinates 'pos' of channel 'C' to data 'D'
			virtual DataInterface<data_t>& setPixel(pos_t pos, std::size_t c, data_t d) = 0;

			/// @brief Get the image that lives on disk/in memory, downsampled to 'target_size'.
			virtual std::vector<pixel_t> getImageDownsampled(pos_t target_size) = 0;
			/// @brief Get a sub-image of the currently loaded image.
			virtual std::vector<pixel_t> getSubImage(pos_t begin, pos_t size) const = 0;

			/**********************************/
			/* File names, paths and basename */
			/**********************************/

			/// @brief Get the filenames associated with the data interface.
			virtual std::vector<std::string> getFilenames(void) = 0;
			/// @brief Sets the filenames for the class.
			virtual DataInterface<data_t>& setFilenames(const std::vector<std::string>& fnames) = 0;
			/// @brief Get the file's basename, if it exists.
			virtual std::string getFileBasename(void) = 0;
			/// @brief Sets the file basename for file formats that are written in multiple files.
			virtual DataInterface<data_t>& setFileBasename(const std::string& name) = 0;
			/// @brief Gets the base file path for the image save directory.
			virtual std::string getFilePath(void) = 0;
			/// @brief Sets the base file path for the image save directory.
			virtual DataInterface<data_t>& setFilePath(const std::string _path) = 0;
	};

}

#endif // IMAGE_INCLUDE_DATA_INTERFACE_HPP_
