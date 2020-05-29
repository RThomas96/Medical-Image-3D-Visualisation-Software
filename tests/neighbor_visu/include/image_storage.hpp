#ifndef TESTS_NEIGHBOR_VISU_IMAGE_STORAGE_HPP_
#define TESTS_NEIGHBOR_VISU_IMAGE_STORAGE_HPP_

#include <glm/glm.hpp>

#include <vector>
#include <iostream>

// TODO : maybe make this a templated class, to allow for other data types than unsigned char for image data

class ImageStorage {
	public:
		ImageStorage(void);
		ImageStorage(size_t stW, size_t stH, size_t stD, const unsigned char* pData);
		~ImageStorage(void);

		unsigned char getImageDataAtPosition(glm::vec4 position);

		ImageStorage& setImageData(const unsigned char* pData, size_t stW, size_t stH, size_t stD);

		// Disallow copy, move and assignment operations on the object :
		ImageStorage(const ImageStorage&) = delete;
		ImageStorage(ImageStorage&&) = delete;
		ImageStorage& operator= (const ImageStorage&) = delete;
		ImageStorage& operator= (ImageStorage&&) = delete;

	protected:
		const unsigned char* imageData;
		std::size_t imageWidth;
		std::size_t imageHeight;
		std::size_t imageDepth;
};

#endif // TESTS_NEIGHBOR_VISU_IMAGE_STORAGE_HPP_
