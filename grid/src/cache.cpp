#include "../include/cache.hpp"

Cache::Cache(glm::vec3 imageSize): img(CImg<uint16_t>(imageSize[0], imageSize[1], imageSize[2], 1, 0)) {}

void Cache::reset() {
    //this->img.assign(this->img.width(), this->img.height(), this->img.depth(), 0);
    this->img = CImg<uint16_t>(this->img.width(), this->img.height(), this->img.depth(), 1, 0);
}

void Cache::storeImage(int imageIdx, const std::vector<uint16_t>& data) {
    this->img.get_shared_slice(imageIdx).assign(data.data(), this->img.width(), this->img.height(), 1.);
}

uint16_t Cache::getValue(const glm::vec3& coord, InterpolationMethod interpolationMethod) {
    if(interpolationMethod == InterpolationMethod::Linear) {
        return static_cast<uint16_t>(this->img.linear_atXYZ(coord[0], coord[1], coord[2], 0, static_cast<uint8_t>(0)));
    } else if (interpolationMethod == InterpolationMethod::Cubic) {
        return static_cast<uint16_t>(this->img.cubic_atXYZ_c(coord[0], coord[1], coord[2], 0, static_cast<uint8_t>(0)));
    } else {
        return this->img.atXYZ(coord[0], coord[1], coord[2]);
    }
}

/************************************/

//Cache::Cache(TIFF * tiff, glm::vec3 imageSize, Image::ImageDataType imageDataType, int capacity = 3): tif(tiff), imageSize(imageSize), capacity(capacity), imgDataType(imageDataType), nbInsertion(0), data(std::vector<std::vector<std::vector<uint16_t>>>(this->capacity, std::vector<std::vector<uint16_t>>(this->imageSize[0], std::vector<uint16_t>(this->imageSize[1], 0)))), indices(std::vector<int>(this->capacity, -1)) {}
UnsortedCache::UnsortedCache(glm::vec3 imageSize, int capacity = 3): imageSize(imageSize), capacity(capacity), nbInsertion(0), data(std::vector<std::vector<uint16_t>>(this->capacity, std::vector<uint16_t>())), indices(std::vector<int>(this->capacity, -1)) {}

uint16_t UnsortedCache::getValue(const glm::vec3& coord) {
    return this->data[this->getCachedIdx(coord[2])][coord[1]*this->imageSize[0]+coord[0]];
}

int UnsortedCache::getCachedIdx(int imageIdx) const {
    if(!this->isCached(imageIdx)) {
        std::cout << "ERROR: try to load an imahe that is not in cache !" << std::endl;
        return -1;
    }
    auto it = std::find(this->indices.begin(), this->indices.end(), imageIdx);
    return std::distance(this->indices.begin(), it);
}

bool UnsortedCache::isCached(int imageIdx) const {
    return (std::find(this->indices.begin(), this->indices.end(), imageIdx) != this->indices.end());
}

std::vector<uint16_t> * UnsortedCache::storeImage(int imageIdx) {
    std::pair<glm::vec3, glm::vec3> bboxes{glm::vec3(0., 0., 0.), glm::vec3(this->imageSize)};

    int nextImageToReplace = this->getNextCachedImageToReplace();
    indices[nextImageToReplace] = imageIdx;
    this->data[nextImageToReplace].clear();
    this->nbInsertion += 1;
    return &this->data[nextImageToReplace];
}

int UnsortedCache::getNextCachedImageToReplace() const {
    return this->nbInsertion % this->capacity;
}

void UnsortedCache::setCapacity(int capacity) {
    this->capacity = capacity;
    this->data = std::vector<std::vector<uint16_t>>(this->capacity, std::vector<uint16_t>());
    this->indices = std::vector<int>(this->capacity, -1);
}

