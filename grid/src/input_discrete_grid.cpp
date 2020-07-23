#include "../include/input_discrete_grid.hpp"

InputGrid::InputGrid(void){
	this->gridName = "InputGrid";
	this->setModifiable(false);
}

InputGrid& InputGrid::preAllocateImageData(sizevec3 dimensions) {
	this->gridDimensions = dimensions;
	std::size_t datasize = dimensions.x * dimensions.y * dimensions.z;
	this->data.resize(datasize);
	// Input grids have voxel sizes of 1, and their bounding box is equal to their dimensions :
	this->voxelDimensions = glm::vec3(1.f, 1.f, 1.f);
	this->boundingBox.setMin(glm::vec3(.0f));
	this->boundingBox.setMax(glm::vec3(static_cast<float>(dimensions.x), static_cast<float>(dimensions.y), static_cast<float>(dimensions.z)));
}

InputGrid& InputGrid::addImage(std::vector<DataType> imgData, std::size_t imgIndex)  {
	// Overrides any data previously here, and assumes the image is the same
	// resolution as the grid it's inserted into :
	std::size_t startIdx = this->gridDimensions.x * this->gridDimensions.y * imgIndex;
	// Copy the data :
	std::copy(imgData.begin(), imgData.end(), this->data.begin()+startIdx);
}

InputGrid& InputGrid::setModifiable(bool b) { return *this; }

InputGrid& InputGrid::setBoundingBox(glm::vec4 min, glm::vec4 max) { return *this; }

InputGrid& InputGrid::setResolution(sizevec3 newRes) { return *this; }
