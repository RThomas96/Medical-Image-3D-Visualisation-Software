#include "../include/output_discrete_grid.hpp"

#include <iomanip>
#include <fstream>

OutputGrid::OutputGrid(void) : DiscreteGrid() {
	this->setModifiable(true);
	this->data.clear();
	this->gridName = "defaultOutputGrid";
	this->boundingBox = bbox_t();
	this->transform_gridToWorld = glm::mat4(1.f);
	this->transform_worldToGrid = glm::mat4(1.f);
}

OutputGrid::~OutputGrid() {}

OutputGrid& OutputGrid::preallocateData() {
	return this->preallocateData(this->gridDimensions);
}

OutputGrid& OutputGrid::preallocateData(sizevec3 dims) {
	this->data.clear();
	if (this->isOffline) {
		this->data.resize(dims.x*dims.y);
	} else {
		this->data.resize(dims.x*dims.y*dims.z);
	}
	return *this;
}

OutputGrid& OutputGrid::updateRenderBox(const bbox_t& newbox) {
	// Get input grid render box :
	std::vector<bbox_t::vec> corners = newbox.transformTo(this->transform_worldToGrid).getAllCorners();

	// Add all points to this render bounding box :
	this->boundingBox.addPoints(corners);
	bbox_t::vec min = newbox.getMin();
	bbox_t::vec max = newbox.getMax();
	sizevec3::value_type x = static_cast<sizevec3::value_type>(max.x - min.x);
	sizevec3::value_type y = static_cast<sizevec3::value_type>(max.y - min.y);
	sizevec3::value_type z = static_cast<sizevec3::value_type>(max.z - min.z);
	this->setResolution(sizevec3(x, y, z));

	return *this;
}

OutputGrid& OutputGrid::writeSlice() {
	if (this->isOffline == false) { return *this; }
	if (this->gridWriter == nullptr) {
		std::cerr << "[ERROR] Requested to write slice " << this->currentSlice << " but no writer was set.\n";
		return *this;
	}
	if (this->data.size() == 0) { this->preallocateData(); }
	std::cerr << "[LOG] Writing slice " << this->currentSlice << "/" << this->gridDimensions.z << " ...\n";
	this->gridWriter->writeSlice(this->data, this->currentSlice);
	return *this;
}

OutputGrid& OutputGrid::setPixel(std::size_t i, std::size_t j, std::size_t k, DataType _data) {
	if (this->isOffline) {
		DiscreteGrid::setPixel(i, j, 0, _data);
	} else {
		DiscreteGrid::setPixel(i, j, k, _data);
	}
	return *this;
}

OutputGrid& OutputGrid::setCurrentSlice(std::size_t cs) { this->currentSlice = cs; return *this; }

OfflineOutputGrid::OfflineOutputGrid(void) : OutputGrid() {
	this->setModifiable(true);
	this->data.clear();
	this->gridName = "defaultOfflineOutputGrid";
	this->boundingBox = bbox_t();
	this->transform_gridToWorld = glm::mat4(1.f);
	this->transform_worldToGrid = glm::mat4(1.f);
	this->outputDIM = nullptr;
	this->outputIMA = nullptr;
}

OfflineOutputGrid::~OfflineOutputGrid() {}

bool OfflineOutputGrid::hasData() const { return false; } // Those grids don't technically have data you can write to a file or upload to OpenGL.

OfflineOutputGrid& OfflineOutputGrid::preallocateData() {
	return this->preallocateData(this->gridDimensions);
}

OfflineOutputGrid& OfflineOutputGrid::preallocateData(sizevec3 dims) {
	this->data.clear();
	std::cerr << "Filenames check.\n\n\n";
	if (this->filenames.empty()) { return *this; }

	std::cerr << "WARNING : Untested code ahead. Writing directly to a file.\n";
	std::cerr << "          Filename chosen : \"" << this->filenames[0] << "\"";

	std::string dimName = this->filenames[0] + ".dim";
	std::string imaName = this->filenames[0] + ".ima";

	std::ios::openmode openingMode = std::ios::out | std::ios::trunc;

	this->outputDIM = new std::ofstream(dimName, openingMode);
	this->outputIMA = new std::ofstream(imaName, openingMode | std::ios::binary);

	if (not this->outputDIM->is_open()) {
		std::cerr << __FUNCTION__ << " : Warning, couldn't open " << dimName << '\n';
		this->outputDIM->close();
		this->outputIMA->close();
		this->outputDIM = nullptr;
		this->outputIMA = nullptr;
		return *this;
	}
	if (not this->outputIMA->is_open()) {
		std::cerr << __FUNCTION__ << " : Warning, couldn't open " << imaName << '\n';
		this->outputDIM->close();
		this->outputIMA->close();
		this->outputDIM = nullptr;
		this->outputIMA = nullptr;
		return *this;
	}

	// Both files are now open. We can write the grid dimensions
	// to the DIM and empty data to the IMA.

	*this->outputDIM << dims.x << " " << dims.y << " " << dims.z << '\n';
	*this->outputDIM << "-type U8\n";

	// Writes the voxel's dimensions within the grid :
	glm::vec3 vxDim	= this->getVoxelDimensions();
	*this->outputDIM << "-dx " << vxDim.x << '\n';
	*this->outputDIM << "-dy " << vxDim.y << '\n';
	*this->outputDIM << "-dz " << vxDim.z << '\n';

	std::size_t dataSize = dims.x*dims.y*dims.z;
	DataType* emptyData = new DataType[dataSize];
	this->outputIMA->write((const char*)emptyData, (dataSize * sizeof(DataType)));

	this->outputDIM->flush();
	this->outputIMA->flush();
	delete[] emptyData;

	// return to the beginning of the file, and check it has :
	this->outputIMA->seekp(0);
	if (this->outputIMA->bad()) {
		std::cerr << "[ERROR] : OutputIMA file did not go to beginning position.\n";
	} else {
		std::cerr << "[LOG] Returned to beggining of the file.\n";
	}

	return *this;
}

OfflineOutputGrid& OfflineOutputGrid::setPixel(std::size_t x, std::size_t y, std::size_t z, DataType data) {
	std::size_t idx = x + y * this->gridDimensions.x + z * this->gridDimensions.x * this->gridDimensions.y;
	if (this->outputIMA == nullptr || this->outputIMA->good() == false) { return *this; }

	this->outputIMA->seekp(idx*sizeof(DataType));
	DataType toWrite = data;
	this->outputIMA->write(reinterpret_cast<const char*>(&toWrite), sizeof(DataType));

	return *this;
}
