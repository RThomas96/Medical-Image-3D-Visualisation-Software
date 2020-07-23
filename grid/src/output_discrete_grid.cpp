#include "../include/output_discrete_grid.hpp"

OutputGrid::OutputGrid(void) {
	this->setModifiable(true);
	this->data.clear();
	this->boundingBox = BoundingBox_General<float>();
	this->gridName = "OutputGrid";
}

OutputGrid::~OutputGrid() {}

void OutputGrid::setVoxelData(sizevec3 idx, DataType val) {
	if (idx.x >= this->gridDimensions.x || idx.y >= this->gridDimensions.y || idx.z >= this->gridDimensions.z) { return; }
	// set value :
	this->data[idx.x+idx.y*this->gridDimensions.x+idx.z*this->gridDimensions.x*this->gridDimensions.y] = val;
	return;
}
