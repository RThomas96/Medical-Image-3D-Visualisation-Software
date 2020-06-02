#include "../include/image_storage.hpp"

TextureStorage::TextureStorage() {
	this->data.clear();
	this->texLoader = nullptr;
	this->imageSpecs.clear();
	this->downsampleImages = false;

	this->resetImageSpecs();
}

TextureStorage::~TextureStorage() {
	this->resetTexture();
}

TextureStorage& TextureStorage::loadImages() {
	this->resetTexture();

	// Create a new image loader, as the texture has been reset
	this->texLoader = new bulk_texture_loader();
	this->texLoader->enable_downsampling(this->downsampleImages);

	// Load images from the bulk_texture_loader (handles everything);
	this->texLoader->load_stack_from_folder();
	const unsigned char* texData = this->texLoader->get_data();

	// If any data was loaded (the loading went smoothly) :
	if (texData != nullptr) {
		this->imageSpecs[0][0] = this->texLoader->get_image_width();
		this->imageSpecs[0][1] = this->texLoader->get_image_height();
		this->imageSpecs[0][2] = this->texLoader->get_image_depth();
		std::size_t imageSize = this->imageSpecs[0][0] * this->imageSpecs[0][1] * this->imageSpecs[0][2];
		this->data.insert(this->data.end(), texData, texData+imageSize);

		std::cerr << "Data is " << data.size() << " elements long" << '\n';

		this->loadImageSpecs();
	}

	return *this;
}

std::vector<std::vector<std::size_t>> TextureStorage::getImageSpecs() const {
	return this->imageSpecs;
}

std::vector<std::size_t> TextureStorage::getImageSize() const {
	return this->imageSpecs[0];
}

std::vector<std::size_t> TextureStorage::getImageBoundingBoxMin() const {
	return this->imageSpecs[1];
}

std::vector<std::size_t> TextureStorage::getImageBoundingBoxMax() const {
	return this->imageSpecs[2];
}

const std::vector<unsigned char>& TextureStorage::getData() const {
	return this->data;
}

unsigned char TextureStorage::getTexelValue(const glm::vec3& position) const {
	if (this->data.empty()) {
		std::cerr << __PRETTY_FUNCTION__ << " : Data was not loaded !" << '\n';
		return '\0';
	}

	// For now, we don't interpolate the value of the texel at the position given :
	std::size_t x = static_cast<std::size_t>(std::roundf(position.x));
	std::size_t y = static_cast<std::size_t>(std::roundf(position.y));
	std::size_t z = static_cast<std::size_t>(std::roundf(position.z));

	std::size_t imageWidth = this->imageSpecs[0][0];
	std::size_t imageHeight = this->imageSpecs[0][1];

	std::size_t index = x + y * imageWidth + z * imageWidth * imageHeight;
	if (index > this->data.size()) {
		std::cerr << __PRETTY_FUNCTION__ << " : The position asked for was OOB." << '\n';
		return '\0';
	}

	return this->data[index];
}

unsigned char TextureStorage::getTexelValue(const glm::vec4& position) const {
	return this->getTexelValue(glm::vec3(position.x, position.y, position.z));
}

TextureStorage& TextureStorage::resetTexture() {
	this->data.clear();
	this->resetImageSpecs();

	if (this->texLoader != nullptr) {
		delete this->texLoader;
		this->texLoader = nullptr;
	}

	return *this;
}

void TextureStorage::loadImageSpecs() {
	this->resetImageSpecs();

	this->imageSpecs[0][0] = this->texLoader->get_image_width();
	this->imageSpecs[0][1] = this->texLoader->get_image_height();
	this->imageSpecs[0][2] = this->texLoader->get_image_depth();

	std::size_t& imageWidth = this->imageSpecs[0][0];
	std::size_t& imageHeight = this->imageSpecs[0][1];
	std::size_t& imageDepth = this->imageSpecs[0][2];

	std::size_t imageSize = imageWidth * imageHeight;

	std::cerr << "Updating texture bounding box ...";

	for (std::size_t z = 0; z < imageDepth; ++z) {
		for (std::size_t y = 0; y < imageHeight; ++y) {
			for (std::size_t x = 0; x < imageWidth; ++x) {
				std::size_t index = x + y * imageWidth + z * imageSize;
				if (this->data[index] > MIN_DATA_VALUE) {
					this->imageSpecs[1][0] = (x < this->imageSpecs[1][0]) ? x : this->imageSpecs[1][0];
					this->imageSpecs[1][1] = (y < this->imageSpecs[1][1]) ? y : this->imageSpecs[1][1];
					this->imageSpecs[1][2] = (z < this->imageSpecs[1][2]) ? z : this->imageSpecs[1][2];

					this->imageSpecs[2][0] = (x > this->imageSpecs[2][0]) ? x : this->imageSpecs[2][0];
					this->imageSpecs[2][1] = (y > this->imageSpecs[2][1]) ? y : this->imageSpecs[2][1];
					this->imageSpecs[2][2] = (z > this->imageSpecs[2][2]) ? z : this->imageSpecs[2][2];
				}
			}
		}
	}

	std::cerr << "Texture bounding box updated.\n";
}

void TextureStorage::resetImageSpecs() {
	this->imageSpecs.clear();
	this->imageSpecs.resize(3);
	this->imageSpecs[0].resize(3);
	this->imageSpecs[1].resize(3);
	this->imageSpecs[2].resize(3);

	std::size_t maxVal = std::numeric_limits<std::size_t>::max();

	this->imageSpecs[0][0] = 0;
	this->imageSpecs[0][1] = 0;
	this->imageSpecs[0][2] = 0;
	this->imageSpecs[1][0] = maxVal;
	this->imageSpecs[1][1] = maxVal;
	this->imageSpecs[1][2] = maxVal;
	this->imageSpecs[2][0] = 0;
	this->imageSpecs[2][1] = 0;
	this->imageSpecs[2][2] = 0;
}
