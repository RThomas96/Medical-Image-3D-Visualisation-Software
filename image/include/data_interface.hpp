#ifndef IMAGE_INCLUDE_DATA_INTERFACE_HPP_
#define IMAGE_INCLUDE_DATA_INTERFACE_HPP_

#include "../../macros.hpp"
#include "./interpolator.hpp"

#include "../../grid/include/bounding_box.hpp"

#include <memory>
#include <vector>
#include <variant>
#include <typeinfo>

/// @brief Position and type within the image.
/// @details Allows to get a position (X,Y,Z). The color channel will be separate.
using size3_t = glm::vec<3, std::size_t, glm::defaultp>;
/// @brief Size type within the image.
/// @details Allows to define a size (X,Y,Z), with the number of color channels as 'A'.
using size4_t = glm::vec<4, std::size_t, glm::defaultp>;
/// @brief Floating-point size type within the image.
/// @details Allows to define a size (X,Y,Z), with the number of color channels as 'A'.
using fsize_t = glm::vec3;

namespace IO {

	template <typename T>
	class DataInterface : public std::enable_shared_from_this<DataInterface<T>> {
		public:
			/// @brief Simple typedef for the current template parameter typename.
			/// @details need to be re-defined in derived classes.
			using data_t = T;
			/// @brief Pixel type, used for getPixel().
			/// @details Similar to OpenGL, whatever the dimensionnality of the data, queries return a vec4.
			/// @note If a channel is not defined (query to a grid containing only red
			using pixel_t = glm::vec<4, data_t, glm::defaultp>;
			/// @brief Allows to define bounds for an image's channels.
			/// @details Used to get the image intensity limits from a particular channel.
			using limit_t = glm::vec<2, data_t, glm::defaultp>;

		protected:
			/// @brief Default constructor. Made protected so as not to call it directly.
			DataInterface();

		public:
			/// @brief Default deletor. Will release the resources allocated in this class.
			~DataInterface(void) = default;

			/// @brief Common function to update values from the loaded files.
			virtual void updateValues() = 0;

			/// @brief Common interface to load information from the disk, overriden in derived classes.
			virtual void loadInformation(void) = 0;
			/// @brief Common interface to write information to the disk, overriden in derived classes.
			virtual void writeInformation(void) = 0;

			/// @brief Get the currently set data threshold.
			virtual data_t getDataThreshold(void) const;
			/// @brief Sets the data threshold for the class.
			virtual DataInterface<T>& setDataThreshold(const data_t _thresh) = 0;

			/// @brief Get the image's resolution.
			virtual size4_t getResolution(void) const = 0;
			/// @brief Sets the resolution of the image.
			virtual DataInterface<data_t>& setResolution(size4_t s) = 0;

			/// @brief Get the image's voxel sizes. If it cannot be read, this will return (1,1,1).
			virtual fsize_t getVoxelDimensions(void) const;
			/// @brief Sets the voxel resolution for the image.
			virtual DataInterface<data_t>& setVoxelDimensions(fsize_t dims);

			/// @brief Returns a pixel's data, in the grid.
			virtual pixel_t getPixel(size3_t pos) = 0;
			/// @brief Returns a subpixel's data, in the grid.
			virtual data_t getSubPixel(size4_t pos) = 0;
			/// @brief Set the pixel at coordinates 'pos' to the contents of data '_pix'
			virtual DataInterface<data_t>& setPixel(size3_t pos, pixel_t _pix) = 0;
			/// @brief Set the subpixel at coordinates 'pos' to data 'D'
			virtual DataInterface<data_t>& setSubPixel(size4_t pos, data_t d) = 0;

			/// @brief Get the image intensity limits for the color channel '_channel'.
			virtual const limit_t getIntensityBounds(std::size_t _channel) const;

			/// @brief Get the image that lives on disk/in memory, downsampled to 'target_size'.
			virtual std::vector<pixel_t> getImageDownsampled(size3_t target_size) = 0;
			/// @brief Get a sub-image of the currently loaded image.
			virtual std::vector<pixel_t> getSubImage(size3_t begin, size3_t size) = 0;

			/**********************************/
			/* File names, paths and basename */
			/**********************************/

			/// @brief Get the filenames associated with the data interface.
			virtual const std::vector<std::string>& getFilenames(void) const;
			/// @brief Sets the filenames for the class.
			virtual DataInterface<data_t>& setFilenames(const std::vector<std::string>& fnames);
			/// @brief Get the file's basename, if it exists.
			virtual const std::string getFileBaseName(void) const;
			/// @brief Sets the file basename for file formats that are written in multiple files.
			virtual DataInterface<data_t>& setFileBaseName(const std::string& name);
			/// @brief Gets the base file path for the image save directory.
			virtual const std::string getFileBasePath(void) const;
			/// @brief Sets the base file path for the image save directory.
			virtual DataInterface<data_t>& setFileBasePath(const std::string& _path);

		protected:
			/// @brief Data threshold that was previously used to compute the data bounding box.
			data_t dataThreshold;
			/// @brief Contains image dimensions, and channel count.
			size4_t imageDimensions;
			/// @brief Voxel dimensions for the loaded image.
			fsize_t voxelDimensions;
			/// @brief Base path for the filetypes supporting it.
			std::string basePath;
			/// @brief Base name for the filetypes supporting it.
			std::string baseName;
			/// @brief Filenames for the data interface.
			/// @details Used in DiskInterface<> to read/write data, used in MemoryInterface<> to get
			/// the origin of the data, and to possibly write it later.
			std::vector<std::string> filenames;
			/// @brief Describes the image intensity limits for channels R, G, B, and A.
			/// @note Although all 4 values will be non-null, they will be usable only if the image has the
			/// channels in its data. A RGB image will have 4 components in its intensityBounds member
			/// variable, but only the elements 0 through 2 will have defined values. The rest will be set
			/// to { max(data_t), min(data_t) }.
			std::array<limit_t, 4> intensityBounds;
	};

	/// @brief Allowed types of data interfaces, wrapped in std::shared_ptr<>.
	/// @details This type is the type used for any class/function that needs to use a DataInterface. Since the data
	/// type of the DataInterface needs to be determined at runtime, we need to use std::variant's ability to store
	/// many different value types at compile-time. At runtime, we will then use a visitor for every operation we
	/// need to do on the DataInterface.
	/// @note This is defined for all possible numerical and character data types, but not every derived class will
	/// be able to return all those types. For example, a TIFF file can only be encoded in a subset of the available
	/// classes here (float, [u]char, [u]int{16|32}). Other filetypes might be able to return only one of the
	/// available data representations, or all of them.
	using DataInterface_ptr_t = std::variant<
		std::shared_ptr<DataInterface<uint8_t>>,
		std::shared_ptr<DataInterface<uint16_t>>,
		std::shared_ptr<DataInterface<uint32_t>>,
		std::shared_ptr<DataInterface<uint64_t>>,
		std::shared_ptr<DataInterface<int8_t>>,
		std::shared_ptr<DataInterface<int16_t>>,
		std::shared_ptr<DataInterface<int32_t>>,
		std::shared_ptr<DataInterface<int64_t>>,
		std::shared_ptr<DataInterface<float>>,
		std::shared_ptr<DataInterface<double>>,
		std::shared_ptr<DataInterface<char>>,
		std::shared_ptr<DataInterface<unsigned char>>
	>;

}

#endif // IMAGE_INCLUDE_DATA_INTERFACE_HPP_
