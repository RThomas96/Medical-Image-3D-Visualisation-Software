#include "../include/tetmesh.hpp"

TetMesh::TetMesh(const TextureStorage* const texL) : texLoader(texL) {
	this->vertices.clear();
	this->vertexValues.clear();
	this->tetrahedra.clear();
	this->origin = glm::vec4(.0, .0, .0, .0);

	this->makeTetrahedra();
}

TetMesh::~TetMesh() {
	this->vertices.clear();
	this->vertexValues.clear();
	this->tetrahedra.clear();
}

TetMesh& TetMesh::setOriginInitialSpace(const glm::vec4 position) {
	// The position given in argument is a 'free' position,
	// not constrained to the grid's voxel centers. We need
	// to set the center of the mesh to the center of the
	// nearest voxel :
	glm::vec4 newOrigin = glm::vec4(
			std::truncf(position.x),
			std::truncf(position.y),
			std::truncf(position.z),
			.0
		);

	// Set the new origin to be the center of the nearest
	// voxel relative to 'position'
	this->updatePositions(newOrigin);

	return *this;
}

TetMesh& TetMesh::resetPositions() {
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		this->vertices[i] -= this->origin;
		this->vertices[i].w = 1.;
	}
	return *this;
}

TetMesh& TetMesh::updateValues() {
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		glm::vec4 icoord = this->vertices[i];
		std::cerr << "Update :: querying for (" << icoord.x << ',' << icoord.y << ',' << icoord.z << ")\n";
		this->vertexValues[i] = this->texLoader->getTexelValue(icoord);
	}
	return *this;
}

TetMesh& TetMesh::updatePositions(glm::vec4 newOrigin) {
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		this->vertices[i] = this->vertices[i] - this->origin + newOrigin;
		this->vertices[i].w = 1.;
	}
	this->updateValues();
	this->origin = newOrigin;

	return *this;
}

std::vector<glm::vec4> TetMesh::getVertices() const {
	std::vector<glm::vec4> res(this->vertices);
	return res;
}

std::vector<unsigned char> TetMesh::getVertexValues() const {
	return this->vertexValues;
}

TetMesh& TetMesh::printInfo() {
	std::cout << "The image storage has the following specs :" << '\n';
	std::vector<svec3> specs = this->texLoader->getImageSpecs();
	std::cout << "Image size : " << specs[0][0] << 'x' << specs[0][1] << 'x' << specs[0][2] << '\n';
	std::cout << "Bounding box min value: " << specs[1][0] << 'x' << specs[1][1] << 'x' << specs[1][2] << '\n';
	std::cout << "Bounding box max value: " << specs[2][0] << 'x' << specs[2][1] << 'x' << specs[2][2] << '\n';

	std::cout << "The neighboring mesh has the following positions and values :" << '\n';
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		std::cout << "[" << this->vertices[i].x << ',' << this->vertices[i].y << ',' << this->vertices[i].z << "] : " << +(this->vertexValues[i]) << '\n';
	}

	return *this;
}

void TetMesh::makeTetrahedra() {
	// For now, a mesh of side 1, centered at the origin

	auto getIndex = [&](std::size_t i, std::size_t j, std::size_t k) {
		return i * 2u * 2u + j * 2u + k;
	};

	// At the start, the origin is at [.0, .0, .0]. But the center of
	// the first voxel is in fact at [.5, .5, .5], which means we
	// need to iterate in [-.5, 1.5] by steps of 1. each time.

	for (std::size_t i = 0; i < 3; ++i) {
		for (std::size_t j = 0; j < 3; ++j) {
			for (std::size_t k = 0; k < 3; ++k) {
				this->vertices.emplace_back(
					static_cast<float>(i) * 1.f - .5f,
					static_cast<float>(j) * 1.f - .5f,
					static_cast<float>(k) * 1.f - .5f,
					1.
				);
				this->vertexValues.push_back(0);
			}
		}
	}

	for (std::size_t i = 0; i < 2; ++i) {
		for (std::size_t j = 0; j < 2; ++j) {
			for (std::size_t k = 0; k < 2; ++k) {
				// Tetrahedra 1 :
				this->tetrahedra.push_back({
					getIndex(i+1, j+0, k+0),
					getIndex(i+1, j+1, k+0),
					getIndex(i+0, j+1, k+0),
					getIndex(i+1, j+1, k+1)
				});
				// Tetrahedra 2 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+0, k+1),
					getIndex(i+0, j+0, k+0),
					getIndex(i+0, j+1, k+1),
					getIndex(i+1, j+0, k+1)
				});
				// Tetrahedra 3 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+1, k+1),
					getIndex(i+1, j+0, k+0),
					getIndex(i+1, j+1, k+1),
					getIndex(i+1, j+0, k+1)
				});
				// Tetrahedra 4 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+0, k+0),
					getIndex(i+1, j+0, k+0),
					getIndex(i+0, j+1, k+1),
					getIndex(i+1, j+0, k+1)
				});
				// Tetrahedra 5 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+0, k+0),
					getIndex(i+1, j+0, k+0),
					getIndex(i+0, j+1, k+0),
					getIndex(i+0, j+1, k+1)
				});
				// Tetrahedra 6 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+1, k+0),
					getIndex(i+1, j+0, k+0),
					getIndex(i+1, j+1, k+1),
					getIndex(i+0, j+1, k+1)
				});
			}
		}
	}

	// The mesh is now constructed. Or at least, it should be.
}






















































