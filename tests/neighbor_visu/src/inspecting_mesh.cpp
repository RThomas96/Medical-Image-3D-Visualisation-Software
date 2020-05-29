#include "../include/inspecting_mesh.hpp"

InspectingMesh::InspectingMesh() {
	this->origin = glm::vec4(.5, .5, .5, .0);
	this->grid.clear();
	this->inversetransformMatrix = glm::mat4(1.);

	for (float z = -1.f; z < 1.1f; z += 1.f) {
		for (float y = -1.f; y < 1.1f; y += 1.f) {
			for (float x = -1.f; x < 1.1f; x += 1.f) {
				this->grid.emplace_back(x, y, z, 1.);
			}
		}
	}
}

InspectingMesh::InspectingMesh(const glm::vec3 o) : InspectingMesh() {
	this->origin = glm::vec4(o.x, o.y, o.z, .0);
}

InspectingMesh::InspectingMesh(const glm::vec4 o) : InspectingMesh() {
	this->origin = glm::vec4(o.x, o.y, o.z, .0);
}

InspectingMesh::InspectingMesh(const glm::vec3 o, const glm::mat4 m) : InspectingMesh(o) {
	this->inversetransformMatrix = glm::inverse(m);
}

InspectingMesh::InspectingMesh(const glm::vec4 o, const glm::mat4 m) : InspectingMesh(o) {
	this->inversetransformMatrix = glm::inverse(m);
}

InspectingMesh::~InspectingMesh() {
	this->grid.clear();
}

glm::vec4 InspectingMesh::operator[](const NeighborDirection dir) const {
	std::size_t x = 1;
	std::size_t y = 1;
	std::size_t z = 1;

	if (dir & NeighborDirection::Right) { x = 2; }
	if (dir & NeighborDirection::Left) { x = 0; }
	if (dir & NeighborDirection::Top) { y = 2; }
	if (dir & NeighborDirection::Bottom) { y = 0; }
	if (dir & NeighborDirection::Forward) { z = 2; }
	if (dir & NeighborDirection::Backward) { z = 0; }

	glm::vec4 realSpacePoint = this->origin + this->grid[x + y*3u + z*9u];

	return inversetransformMatrix * realSpacePoint;
}

glm::vec4 InspectingMesh::operator()(const int i, const int j, const int k) const {
	float x = static_cast<float>(i);
	float y = static_cast<float>(j);
	float z = static_cast<float>(k);

	glm::vec4 offset(x, y, z, .0);

	return this->inversetransformMatrix * (this->origin + offset);
}

InspectingMesh& InspectingMesh::setOrigin(glm::vec3 nO) {
	this->origin = glm::vec4(nO.x, nO.y, nO.z, .0);
	this->origin += glm::vec4(.5, .5, .5, .0); // always offset to be in the middle of a voxel
	return *this;
}

InspectingMesh& InspectingMesh::setTransformationMatrix(const glm::mat4 tM) {
	this->inversetransformMatrix = glm::inverse(tM);
	return *this;
}
