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

VoxelGrid& VoxelGrid::setGridResolution(svec3 newDimensions) {
	this->gridDimensions = newDimensions;
	this->updateVoxelSizes();
	return *this;
}

VoxelGrid& VoxelGrid::setRenderBoundingBox(glm::vec4 minPoint, glm::vec4 maxPoint) {
	this->renderBB.setMax(glm::vec3(maxPoint.x, maxPoint.y, maxPoint.z));
	this->renderBB.setMin(glm::vec3(minPoint.x, minPoint.y, minPoint.z));
	this->updateVoxelSizes();
	return *this;
}

void VoxelGrid::updateVoxelSizes() {
	BoundingBox_General<float>::vec diag = this->renderBB.getDiagonal();
	float vx = static_cast<float>(diag.x) / static_cast<float>(this->gridDimensions.x);
	float vy = static_cast<float>(diag.y) / static_cast<float>(this->gridDimensions.y);
	float vz = static_cast<float>(diag.z) / static_cast<float>(this->gridDimensions.z);
	this->voxelDimensions = glm::vec3(vx, vy, vz);
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
	/**
	 * Starts by iterating over the grid's resolution. Then, for each voxel, its position is defined
	 * on all axes by `renderBB.Minimum.Axis + (index) * voxelSize.Axis`.
	 *
	 * Once the iterations are over, we should have a fully populated data vector.
	 */

	float x = .0f, y = .0f, z = .0f;
	std::size_t index;

	for (std::size_t k = this->renderBB.getMin().z; k < this->gridDimensions.z; ++k) {
		z = this->renderBB.getMin().z + this->voxelDimensions.z * static_cast<float>(k);

		for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
			y = this->renderBB.getMin().y + this->voxelDimensions.y * static_cast<float>(j);

			for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
				x = this->renderBB.getMin().x + this->voxelDimensions.x * static_cast<float>(i);
				index = i + j * this->gridDimensions.x + k * this->gridDimensions.x * this->gridDimensions.y;

				// We now have the position of the voxel to render. Set the mesh here :
				glm::vec4 voxelPosWorldSpace = glm::vec4(x, y, z, 1.);
				// And get the interpolated value here, directly stored in the data vector :
				this->data[index] = this->inspectorMesh->getInterpolatedValue(voxelPosWorldSpace, InterpolationMethods::NearestNeighbor);
			}

		}
		std::cerr << "Finished depth level " << k << " of " << this->gridDimensions.z << '\n';

	}
}
