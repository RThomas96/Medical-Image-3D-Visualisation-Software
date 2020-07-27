#include "../include/voxel_grid.hpp"

#include "../../qt/include/grid_control.hpp"
#include "../../viewer/include/scene.hpp"

#ifndef NDEBUG
#define OUT std::cerr
#else
#define OUT std::cout
#endif

using defaultWriter = IO::DIMWriter;

VoxelGrid::VoxelGrid() {
	// No data is provided for now
	this->gridDimensions = svec3(0, 0, 0);
	this->voxelDimensions = glm::vec3(.0, .0, .0);
	this->renderBB = BoundingBox_General<float>();
	this->data.clear();
	this->imageStack.reset();
	this->inspectorMesh.reset();
	this->controller = nullptr;
	this->writer = nullptr;
	this->scene = nullptr;
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

VoxelGrid& VoxelGrid::populateGrid(InterpolationMethods method) {
	if (this->imageStack == nullptr || this->inspectorMesh == nullptr) {
		OUT << "VoxelGrid : No image stack or inspecting mesh was set !" << '\n';
	}

#ifndef NDEBUG
	std::cerr << "Attempting to populate a grid of size " << this->gridDimensions.x << ',' << this->gridDimensions.y << ',' << this->gridDimensions.z << '\n';
	std::cerr << "Voxel size is " << this->voxelDimensions.x << ',' << this->voxelDimensions.y << ',' << this->voxelDimensions.z << '\n';
#endif

	this->reserveSpace();

	switch (method) {
		case InterpolationMethods::NearestNeighbor:
			std::cerr << "Interpolating with NN" << '\n';
		break;
		case InterpolationMethods::TriLinear:
			std::cerr << "Interpolating with TL" << '\n';
		break;
		case InterpolationMethods::TriCubic:
			std::cerr << "Interpolating with TC" << '\n';
		break;
		case InterpolationMethods::Barycentric:
			std::cerr << "Interpolating with BC" << '\n';
		break;
		default:break;
	}

	this->computeData(method);

	if (this->controller == nullptr) {
		std::cerr << "To generate " << std::scientific << this->gridDimensions.x*this->gridDimensions.y*this->gridDimensions.z <<
			     " voxels, it took " << this->generationDuration.count() << " seconds\n";
	}

	if (this->scene != nullptr) {
		std::cerr << "Loading texture in scene" << '\n';
		//this->scene->loadVoxelGrid(this->getGridDimensions(), this->data.data());
	}

	return *this;
}

VoxelGrid& VoxelGrid::writeToFile(const std::string path) {
	this->writer = new defaultWriter(path);

	//this->writer->write(this);

	return *this;
}

VoxelGrid& VoxelGrid::setScene(Scene* _sc) {
	this->scene = _sc;
	return *this;
}

void VoxelGrid::reserveSpace() {
	this->data.clear();

	std::size_t dataSize = this->gridDimensions.x * this->gridDimensions.y * this->gridDimensions.z;
	this->data.resize(dataSize);
}

void VoxelGrid::computeData(InterpolationMethods method) {
	/* Starts by iterating over the grid's resolution. Then, for each voxel, its position is defined
	 * on all axes by `renderBB.Minimum.Axis + (index) * voxelSize.Axis`.
	 *
	 * Once the iterations are over, we should have a fully populated data vector.
	 */

	//std::cout << "Starting to compute data for a grid of dimensions " << this->gridDimensions.x << 'x' << this->gridDimensions.y << 'x' << this->gridDimensions.z << '\n';

	float x = .0f, y = .0f, z = .0f;
	std::size_t index;

	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<double, std::ratio<1,1>>> start_point = std::chrono::high_resolution_clock::now();
	{ // Timed portion of code :
		for (std::size_t k = 0; k < this->gridDimensions.z; ++k) {
			z = this->renderBB.getMin().z + this->voxelDimensions.z * static_cast<float>(k) + this->voxelDimensions.z/2.f;

			for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
				y = this->renderBB.getMin().y + this->voxelDimensions.y * static_cast<float>(j) + this->voxelDimensions.y/2.f;

				for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
					x = this->renderBB.getMin().x + this->voxelDimensions.x * static_cast<float>(i) + this->voxelDimensions.x/2.f;
					index = i + j * this->gridDimensions.x + k * this->gridDimensions.x * this->gridDimensions.y;

					// We now have the position of the voxel to render. Set the mesh here :
					glm::vec4 voxelPosWorldSpace = glm::vec4(x, y, z, 1.);
					//std::cout << "Voxel grid query :" << '\n';
					//std::cout << "\tPosition in real space : [" << voxelPosWorldSpace.x << " ," << voxelPosWorldSpace.y << " ," << voxelPosWorldSpace.z << "]\n";
					// And get the interpolated value here, directly stored in the data vector :
					//this->data[index] = this->inspectorMesh->getInterpolatedValue(voxelPosWorldSpace, method);
				}

			}

		}
	}
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<double, std::ratio<1,1>>> end_point = std::chrono::high_resolution_clock::now();
	this->generationDuration = end_point - start_point;
	std::cerr << "Finished populating the grid." << '\n';
}

void VoxelGrid::slotSetGridDimensionX(int newDim) {
	if (newDim >= 0) {
		float bbXDim = static_cast<float>(this->gridDimensions.x)*this->voxelDimensions.x;
		this->gridDimensions.x = static_cast<std::size_t>(newDim);
		this->voxelDimensions.x = bbXDim / static_cast<float>(this->gridDimensions.x);
	}
	if (this->controller) { this->controller->updateGridLabels(); }
}
void VoxelGrid::slotSetGridDimensionY(int newDim) {
	if (newDim >= 0) {
		float bbYDim = static_cast<float>(this->gridDimensions.y)*this->voxelDimensions.y;
		this->gridDimensions.y = static_cast<std::size_t>(newDim);
		this->voxelDimensions.y = bbYDim / static_cast<float>(this->gridDimensions.y);
	}
	if (this->controller) { this->controller->updateGridLabels(); }
}
void VoxelGrid::slotSetGridDimensionZ(int newDim) {
	if (newDim >= 0) {
		float bbZDim = static_cast<float>(this->gridDimensions.z)*this->voxelDimensions.z;
		this->gridDimensions.z = static_cast<std::size_t>(newDim);
		this->voxelDimensions.z = bbZDim / static_cast<float>(this->gridDimensions.z);
	}
	if (this->controller) { this->controller->updateGridLabels(); }
}
void VoxelGrid::slotSetGridBBMinX(double newDim) { this->renderBB.setMinX(newDim); this->updateVoxelSizes(); if (this->controller) { this->controller->updateGridLabels(); } }
void VoxelGrid::slotSetGridBBMinY(double newDim) { this->renderBB.setMinY(newDim); this->updateVoxelSizes(); if (this->controller) { this->controller->updateGridLabels(); } }
void VoxelGrid::slotSetGridBBMinZ(double newDim) { this->renderBB.setMinZ(newDim); this->updateVoxelSizes(); if (this->controller) { this->controller->updateGridLabels(); } }
void VoxelGrid::slotSetGridBBMaxX(double newDim) { this->renderBB.setMaxX(newDim); this->updateVoxelSizes(); if (this->controller) { this->controller->updateGridLabels(); } }
void VoxelGrid::slotSetGridBBMaxY(double newDim) { this->renderBB.setMaxY(newDim); this->updateVoxelSizes(); if (this->controller) { this->controller->updateGridLabels(); } }
void VoxelGrid::slotSetGridBBMaxZ(double newDim) { this->renderBB.setMaxZ(newDim); this->updateVoxelSizes(); if (this->controller) { this->controller->updateGridLabels(); } }
