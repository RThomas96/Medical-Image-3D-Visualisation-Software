#include "../include/input_discrete_grid.hpp"

InputGrid::InputGrid(void){
	this->gridName = "defaultInputGrid";
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

	return *this;
}

InputGrid& InputGrid::addImage(std::vector<DataType> imgData, std::size_t imgIndex)  {
	if (imgIndex > this->gridDimensions.z) { std::cerr << "[ERROR] Tried to access image at depth of "
	<< imgIndex << " out of " << this->gridDimensions.z << '\n'; return *this; }
	// Overrides any data previously here, and assumes the image is the same
	// resolution as the grid it's inserted into :
	std::size_t startIdx = this->gridDimensions.x * this->gridDimensions.y * imgIndex;
	// Copy the data :
	std::copy(imgData.begin(), imgData.end(), this->data.begin()+startIdx);
	// data copied, but we should not update the data bounding box.
	return *this;
}

InputGrid& InputGrid::setGrid(std::vector<DataType> imgData, sizevec3 dimensions) {
	this->data.clear();
	this->data.resize(dimensions.x*dimensions.y*dimensions.z);
	std::copy(imgData.begin(), imgData.end(), this->data.begin());
	// Set the grid's dimensions :
	this->gridDimensions = dimensions;
	// Set the bounding box's dimensions :
	this->boundingBox.setMin(glm::vec3(.0f));
	this->boundingBox.setMax(glm::vec3(static_cast<float>(dimensions.x), static_cast<float>(dimensions.y), static_cast<float>(dimensions.z)));
	// Set the bounding box for data loaded :
	this->recomputeBoundingBox(this->dataThreshold);

	return *this;
}

InputGrid& InputGrid::setModifiable(bool b) { return *this; }

InputGrid& InputGrid::setResolution(sizevec3 newRes) { return *this; }

InputGrid& InputGrid::setBoundingBox(bbox_t renderWindow) { return *this; }

OfflineInputGrid::OfflineInputGrid(void){
	this->gridName = "defaultOfflineInputGrid";
	this->setModifiable(false);
	this->dimFile = nullptr;
	this->imaFile = nullptr;
}

OfflineInputGrid::~OfflineInputGrid() {
	if (this->dimFile) { this->dimFile->close(); }
	if (this->imaFile) { this->imaFile->close(); }
}

OfflineInputGrid& OfflineInputGrid::fromInputGrid(const std::shared_ptr<InputGrid>& igrid) {
	this->filenames = igrid->getFilenames();
	this->boundingBox = igrid->getBoundingBox();
	this->setTransform_GridToWorld(igrid->getTransform_GridToWorld());

	char* bname = IO::FileBaseName(this->filenames[0].c_str());
	const char* dname = IO::AppendExtension(bname, "dim");
	const char* iname = IO::AppendExtension(bname, "ima");

	this->dimFile = new std::ifstream(dname);
	this->imaFile = new std::ifstream(iname);

	// read number of voxels into image dimensions :
	(*this->dimFile) >> this->gridDimensions.x;
	(*this->dimFile) >> this->gridDimensions.y;
	(*this->dimFile) >> this->gridDimensions.z;

	// read info from the DIM file (extended with our properties)
	std::string token, type;
	do {
		(*this->dimFile) >> token;
		if (token.find("-type") != std::string::npos) { (*this->dimFile) >> type; }
		else if (token.find("-dx") != std::string::npos) {(*this->dimFile) >> this->voxelDimensions.x;}
		else if (token.find("-dy") != std::string::npos) {(*this->dimFile) >> this->voxelDimensions.y;}
		else if (token.find("-dz") != std::string::npos) {(*this->dimFile) >> this->voxelDimensions.z;}
		else {
			std::cerr << "[ERROR] DIMReader - token "<< token <<" did not represent anything"<<'\n';
		}
	} while (not this->dimFile->eof());

	return *this;

}

OfflineInputGrid::DataType OfflineInputGrid::getPixel(std::size_t x, std::size_t y, std::size_t z) const {
	std::size_t idx = x + y * this->gridDimensions.x + z * this->gridDimensions.x * this->gridDimensions.y;

	this->imaFile->seekg(idx*sizeof(DataType));
	DataType storage = 0;
	this->imaFile->read(reinterpret_cast<char*>(&storage), sizeof(DataType));

	return storage;
}
