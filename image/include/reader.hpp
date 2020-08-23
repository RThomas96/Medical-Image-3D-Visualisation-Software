#ifndef IMAGE_INCLUDE_READER_HPP_
#define IMAGE_INCLUDE_READER_HPP_

#include "../../grid/include/bounding_box.hpp"
#include "../../TinyTIFF/tinytiffreader.h"

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

class DiscreteGrid; // Fwd-declaration

namespace IO {

	/// \brief Checks if the file given in argument exists
	/// \param filename The name of the file to check
	bool FileExists(const char* filename);

	/// \brief Returns the file base name (the name, without the extension at the end).
	/// \param filename The full name of the file, possibly with an extension or leading paths
	/// \warning If the file doesn't have an extension, returns nullptr.
	char* FileBaseName(const char* filename);

	/// \brief Appends the required extension to the provided base name
	/// \param basename The base name of the file
	/// \param extension The extension to append to it.
	/// \return A new char array containing <basename>.<extension>, or nullptr if an error occured
	char* AppendExtension(char* basename, const char* extension);

	/// @brief This class implements a basic reader to load data from disk
	/// directly into a voxel grid.
	class GenericGridReader {
		public:
			/// @brief Vector to store grid dimensions.
			typedef glm::vec<3, std::size_t, glm::defaultp> sizevec3;
			/// @brief Data type to be loaded from disk into memory.
			typedef unsigned char data_t;
			/// @brief Type of bounding box used in this class.
			typedef BoundingBox_General<float> bbox_t;

		protected:
			/// @brief Constructor for the class.
			/// @details Declared as protected in order not to have any instances of this class
			/// created, instead of instance based on derived classes.
			GenericGridReader(data_t threshold);

		public:
			/// @brief Destructor of the class. Closes files and frees up memory taken by the loaded data.
			/// @details Closes all filestreams, and clears all data associated with the loader.
			virtual ~GenericGridReader(void);

			/// @brief Set the threshold at which a voxel is considered data.
			virtual GenericGridReader& setDataThreshold(data_t _thresh);

			/// @brief Sets the filenames to load.
			virtual GenericGridReader& setFilenames(std::vector<std::string>& names);

			/// @brief Starts the image loading process.
			virtual GenericGridReader& loadImage();

			/// @brief Returns the whole data loaded, at once.
			virtual const std::vector<data_t>& getGrid(void) const;

			/// @brief Get the grid dimensions, once loaded.
			virtual sizevec3 getGridDimensions(void) const;

			/// @brief Get the grid's voxel dimensions, once loaded.
			virtual glm::vec3 getVoxelDimensions(void) const;

			/// @brief Get the associated transform of the grid, if any is provided.
			/// @note If none are provided, this will return an identity matrix.
			virtual glm::mat4 getTransform(void) const;

			/// @brief Get the bounding box of the grid.
			virtual bbox_t getBoundingBox(void) const;

			/// @brief Get the grid's data bounding box.
			/// @note Computed according to the value set in `GenericGridReader::threshold`.
			virtual bbox_t getDataBoundingBox(void) const;

			/// @brief Returns the threshold from which data is considered information.
			virtual data_t getDataThreshold(void) const;

			/// @brief Swaps the contents from this grid's data to the target vector.
			virtual GenericGridReader& swapData(std::vector<data_t>& target);

		protected:
			/// @brief Open the file with the given name, and load its contents into memory.
			virtual GenericGridReader& openFile(std::string& name);

			/// @brief Load an image from the filesystem, for slice-based image formats.
			virtual GenericGridReader& loadImageIndexed(std::size_t idx);

			/// @brief Load the whole grid into memory at once.
			virtual GenericGridReader& loadGrid();

		protected:
			/// @brief Filenames to open images from.
			std::vector<std::string> filenames;
			/// @brief Data loaded from images.
			std::vector<data_t> data;
			/// @brief Grid dimensions (resolution of the grid).
			sizevec3 gridDimensions;
			/// @brief Dimensions of voxels, if provided. Otherwise, set to unit volume.
			glm::vec3 voxelDimensions;
			/// @brief Transform to apply to the grid, if any is provided. Identity matrix otherwise.
			glm::mat4 transform;
			/// @brief Bounding box of the grid, in the grid's space.
			bbox_t boundingBox;
			/// @brief Bounding box of the data in the grid, dictated by the grid threshold.
			bbox_t dataBoundingBox;
			/// @brief Minimum value from which data is considered valuable.
			data_t threshold;
			/// @brief Name of the grid, if provided. Otherwise, empty string.
			std::string name;
			/// @brief Tracks if the data needs to be downsampled upon loading.
			bool downsampled;
	};

	class DIMReader : public GenericGridReader {
		public:
			DIMReader(data_t _thresh);
			virtual ~DIMReader(void);

			/// @brief Loads the image from disk. If no filenames are provided, loads nothing.
			virtual DIMReader& loadImage() override;
		protected:
			/// @brief Open the DIM and IMA files to read later.
			virtual DIMReader& openFile(std::string& name) override;
			virtual DIMReader& loadGrid() override;

		protected:
			/// @brief The DIM file
			std::ifstream* dimFile;
			/// @brief The IMA file
			std::ifstream* imaFile;
	};

	class StackedTIFFReader : public GenericGridReader {
		public:
			StackedTIFFReader(data_t thresh);
			virtual ~StackedTIFFReader(void);

			/// @brief Loads the image from disk. If no filenames are provided, does nothing.
			virtual StackedTIFFReader& loadImage() override;

			/// @brief Enables downsampling upon image loading or not.
			virtual StackedTIFFReader& enableDownsampling(bool enabled = true);

		protected:
			/// @brief Preallocates the data vector and updates some data we can gather before loading the images.
			/// @details Loads the first image, takes its dimensions and preallocates the whole grid to load the images faster.
			/// also updates the grid's bounding box, as well as the grid dimensions, the voxel dimensions and
			virtual StackedTIFFReader& preAllocateStorage();
			/// @brief Opens the specified file to be able to read it later.
			virtual StackedTIFFReader& openFile(std::string& filename) override;
			/// @brief Loads the image at index 'idx' in the filenames in memory.
			virtual StackedTIFFReader& loadImageIndexed(std::size_t idx) override;

		protected:
			TinyTIFFReaderFile* tiffFile;
	};

	namespace Reader {
		/// @brief Alias for the IO::DIMReader class.
		typedef ::IO::DIMReader DIM;
		/// @brief Alias for the IO::StackedTIFFReader class.
		typedef ::IO::StackedTIFFReader TIFF;
	}
}

#endif // IMAGE_INCLUDE_READER_HPP_
