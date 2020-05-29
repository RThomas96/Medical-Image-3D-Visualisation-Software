#include "../include/image_storage.hpp"

ImageStorage::ImageStorage() {
	this->imageData = nullptr;
	this->imageWidth = 0;
	this->imageHeight = 0;
	this->imageDepth = 0;
}

ImageStorage::ImageStorage(size_t stW, size_t stH, size_t stD, const unsigned char* pData) {
	this->imageData = pData;
	this->imageWidth = stW;
	this->imageHeight = stH;
	this->imageWidth = stD;
}

ImageStorage::~ImageStorage() {
	//
}

unsigned char ImageStorage::getImageDataAtPosition(glm::vec4 position) {
	assert(position.x > .0f && position.y > .0f && position.z > .0f);

	std::size_t x = static_cast<std::size_t>(std::round(position.x));
	std::size_t y = static_cast<std::size_t>(std::round(position.y));
	std::size_t z = static_cast<std::size_t>(std::round(position.z));

	std::size_t index = x + y * this->imageWidth + z * this->imageWidth * this->imageHeight;

	return this->imageData[index];
}

ImageStorage& ImageStorage::setImageData(const unsigned char* pData, size_t stW, size_t stH, size_t stD) {
	this->imageData = pData;
	this->imageWidth = stW;
	this->imageHeight = stH;
	this->imageWidth = stD;
	return *this;
}
