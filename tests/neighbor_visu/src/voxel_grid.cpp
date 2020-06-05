#include "../include/voxel_grid.hpp"

#ifndef NDEBUG
#define OUT std::cerr
#else
#define OUT std::cout
#endif

VoxelGrid::VoxelGrid() {
	// No data is provided for now
	this->gridDimensions = svec3(0, 0, 0);
	this->voxelDimensions = glm::vec3(.0, .0, .0);
	this->renderBB = BoundingBox_General<float>();
	this->data.clear();
	this->imageStack.reset();
	this->inspectorMesh.reset();
}

VoxelGrid::VoxelGrid(std::size_t width, std::size_t height, std::size_t depth) : VoxelGrid() {
	// Local-scope definition of the render bounding box's vector type.
	using renderBBvec = BoundingBox_General<float>::vec;

	// Update fields that need to be updated :
	this->gridDimensions = svec3(width, height, depth);
	this->voxelDimensions = glm::vec3(1.f, 1.f, 1.f);

	// Set the bounding box to cover [origin -> gridSize] :
	this->renderBB.setMax(renderBBvec(static_cast<float>(width), static_cast<float>(height), static_cast<float>(depth)));
}

VoxelGrid::~VoxelGrid() {
	this->data.clear();
	this->imageStack.reset();
	this->inspectorMesh.reset();
}

VoxelGrid& VoxelGrid::setGridSize(svec3 newDimensions) {
	this->gridDimensions = newDimensions;
	return *this;
}

VoxelGrid& VoxelGrid::setRenderBoundingBox(glm::vec4 minPoint, glm::vec4 maxPoint) {
	this->renderBB.setMax(glm::vec3(maxPoint.x, maxPoint.y, maxPoint.z));
	this->renderBB.setMin(glm::vec3(minPoint.x, minPoint.y, minPoint.z));
	return *this;
}

VoxelGrid& VoxelGrid::setImageStack(std::shared_ptr<TextureStorage> _stack) {
	this->imageStack = _stack;
	return *this;
}

VoxelGrid& VoxelGrid::setInspector(std::shared_ptr<TetMesh> _mesh) {
	this->inspectorMesh = _mesh;
	return *this;
}

VoxelGrid& VoxelGrid::populateGrid() {
	if (this->imageStack == nullptr || this->inspectorMesh == nullptr) {
		OUT << "VoxelGrid : No image stack or inspecting mesh was set !" << '\n';
	}

	this->reserveSpace();

	// TODO : add a progress tracker called upon at the end of each depth level
	this->computeData();
}

void VoxelGrid::reserveSpace() {
	this->data.clear();

	std::size_t dataSize = this->gridDimensions.x * this->gridDimensions.y * this->gridDimensions.z;
	this->data.reserve(dataSize);
}

void VoxelGrid::computeData() {
	//
}
