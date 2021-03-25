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

OutputGrid::OutputGrid(const std::shared_ptr<OutputGrid>& _og) {
	this->modifiable = _og->modifiable;
	this->isOffline = _og->isOffline;
	if (not this->isOffline) {
		this->data = _og->data;
	} else {
		this->data.clear();
	}

	this->gridDimensions = _og->gridDimensions;
	this->gridReader = _og->gridReader;
	this->gridWriter = _og->gridWriter;
	this->gridName = _og->gridName;
	this->voxelDimensions = _og->voxelDimensions;
	this->transform_gridToWorld = _og->transform_gridToWorld;
	this->transform_worldToGrid = _og->transform_worldToGrid;
	this->boundingBox = _og->boundingBox;
	this->dataThreshold = _og->dataThreshold;
	this->dataBoundingBox = _og->dataBoundingBox;
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
	if (this->isOffline == false) {
		std::cerr << "Grid was offline ? no";
		return *this;
	}
	if (this->gridWriter == nullptr) {
		std::cerr << "[ERROR] Requested to write slice " << this->currentSlice << " but no writer was set.\n";
		return *this;
	}
	if (this->data.size() == 0) { this->preallocateData(); }
	std::cerr << "[LOG] Writing slice " << this->currentSlice << "/" << this->gridDimensions.z << " ... ";
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
