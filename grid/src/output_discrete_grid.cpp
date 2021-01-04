#include "../include/output_discrete_grid.hpp"

#include <iomanip>

OutputGrid::OutputGrid(void) : DiscreteGrid() {
	this->setModifiable(true);
	this->data.clear();
	this->gridName = "defaultOutputGrid";
	this->boundingBox = bbox_t();
	this->transform_gridToWorld = glm::mat4(1.f);
	this->transform_worldToGrid = glm::mat4(1.f);
}

OutputGrid::~OutputGrid() {}

void OutputGrid::setVoxelData(sizevec3 idx, DataType val) {
	// early check :
	if (idx.x >= this->gridDimensions.x || idx.y >= this->gridDimensions.y || idx.z >= this->gridDimensions.z) { return; }
	// set value :
	this->data[idx.x+idx.y*this->gridDimensions.x+idx.z*this->gridDimensions.x*this->gridDimensions.y] = val;
	return;
}

OutputGrid& OutputGrid::preallocateData() {
	return this->preallocateData(this->gridDimensions);
}

OutputGrid& OutputGrid::preallocateData(sizevec3 dims) {
	this->data.clear();
	this->data.resize(dims.x*dims.y*dims.z);
	return *this;
}

OutputGrid& OutputGrid::setBoundingBox(bbox_t renderWindow) {
	// Warning : assumes the bounding box given is in world space

	// get bb in this stack's space :
	this->boundingBox = renderWindow;

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
