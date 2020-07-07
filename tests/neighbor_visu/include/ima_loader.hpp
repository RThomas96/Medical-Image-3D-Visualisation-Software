#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_IMA_LOADER_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_IMA_LOADER_HPP_

#include <vector>
#include <tuple>

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

/**
 * @brief Skeleton class, rensposible for loading a DIM/IMA file from disk.
 */
class IMALoader {
	public:
		IMALoader();
		~IMALoader();
		const std::vector<unsigned char>& loadData();
		const std::vector<unsigned char>& getData() { return this->data; }

		std::tuple<std::size_t, std::size_t, std::size_t> getDimensions(void) {
			return std::make_tuple(this->sx, this->sy, this->sz);
		}
		std::tuple<float, float, float> getVoxelDims(void) {
			return std::make_tuple(this->dx, this->dy, this->dz);
		}
	protected:
		std::vector<unsigned char> data;
		std::size_t sx;
		std::size_t sy;
		std::size_t sz;
		float dx;
		float dy;
		float dz;
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_IMA_LOADER_HPP_
