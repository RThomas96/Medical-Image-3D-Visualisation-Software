#ifndef TESTS_NEIGHBOR_VISU_IMAGE_STORAGE_HPP_
#define TESTS_NEIGHBOR_VISU_IMAGE_STORAGE_HPP_

#include "image/include/bulk_texture_loader.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <iostream>

/// @brief The minimum value at which a point is considered 'data' instead of 'void'
#define MIN_DATA_VALUE 5

/// @brief This class holds a texture in memory, for easy management.
/// @details
class TextureStorage {

		friend class TetMesh;

	public:
		TextureStorage();
		~TextureStorage();
		TextureStorage& loadImages(void);
		/// @brief Enables downsampling on the texture loader
		TextureStorage& enableDownsampling(bool enabled = true) { this->downsampleImages = enabled; return *this; }
		/// @brief get the image specs.
		std::vector<std::vector<std::size_t>> getImageSpecs() const;
		/// @brief Returns the image size. First width, then height and finally depth (== number of images)
		std::vector<std::size_t> getImageSize() const;
		/// @brief Get the min point of the image's data bounding box
		std::vector<std::size_t> getImageBoundingBoxMin() const;
		/// @brief Get the max point of the image's data bounding box
		std::vector<std::size_t> getImageBoundingBoxMax() const;
		/// @brief Get the image data, in its entirety
		const std::vector<unsigned char>& getData() const;
		/// @brief Gets the value of the texel nearest of the given position (no interpolation).
		unsigned char getTexelValue(const glm::vec3& position) const;
		/// @brief Gets the value of the texel nearest of the given position (no interpolation).
		unsigned char getTexelValue(const glm::vec4& position) const;
		/// @brief Gets the value of the texel nearest of the given position with an interpolation applied to get the value.
		/// @warning NOT IMPLEMENTED YET
		/// @todo Implement it.
		/// @note Think about how we deal with the different interplation types ...
		unsigned char getInterpolatedValue(const glm::vec3 position) const { std::cerr << "Not implemented yet !" << '\n'; return 0; }
		glm::vec4 getVoxelPositionFromIndex(std::size_t i, std::size_t j, std::size_t k) {
			return glm::vec4(static_cast<float>(i) +.5f, static_cast<float>(j) +.5f, static_cast<float>(k) +.5f, 1.);
		}
		glm::uvec3 getVoxelIndexFromPosition(glm::vec4 pos) {
			return glm::uvec3(
				static_cast<uint>(std::truncf(pos.x)),
				static_cast<uint>(std::truncf(pos.y)),
				static_cast<uint>(std::truncf(pos.z))
			);
		}
		TextureStorage& resetTexture();
	protected:
		/// @brief Holds the specifications for the image.
		/// @details This is a 3x3 matrix, holding :
		///   - the image size in the first three components,
		///   - the image bounding box min point in the next three components,
		///   - the image bounding box max point in the last three components.
		/// The image bounding box is defined as the bounding box around the parts of the image that actually contain data.
		std::vector<std::vector<std::size_t>> imageSpecs;
		/// @brief a loader for the images.
		bulk_texture_loader* texLoader;
		/// @brief The image data loaded by the loader.
		std::vector<unsigned char> data;
		bool downsampleImages;

		void loadImageSpecs();
		void resetImageSpecs();
};

#endif // TESTS_NEIGHBOR_VISU_IMAGE_STORAGE_HPP_
