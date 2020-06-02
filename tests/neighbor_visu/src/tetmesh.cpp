#include "../include/tetmesh.hpp"

TetMesh::TetMesh(const TextureStorage* const texL) : texLoader(texL) {
	this->vertices.clear();
	this->vertexValues.clear();
	this->tetrahedra.clear();
	this->origin = glm::vec4(.0, .0, .0, .0);
	this->transformationMatrix = glm::mat4(1.f);
	this->inverseTransformationMatrix = this->transformationMatrix;

	this->makeTetrahedra();
}

TetMesh::TetMesh(const TextureStorage* const texL, const glm::vec4 pos) : texLoader(texL) {
	this->origin = pos;
	this->transformationMatrix = glm::mat4(1.f);
	this->inverseTransformationMatrix = this->transformationMatrix;

	this->vertices.clear();
	this->vertexValues.clear();
	this->tetrahedra.clear();

	this->makeTetrahedra();
	this->updatePositions();
}

TetMesh::~TetMesh() {
	this->vertices.clear();
	this->vertexValues.clear();
	this->tetrahedra.clear();
}

TetMesh& TetMesh::setOrigin(const glm::vec4 position) {
	this->resetPositions();
	this->origin = position;
	this->origin.w = 1.;
	this->updatePositions();

	return *this;
}

TetMesh& TetMesh::resetPositions() {
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		this->vertices[i] -= this->origin;
		this->vertices[i].w = 1.;
	}
	return *this;
}

TetMesh& TetMesh::setOrigin(std::size_t i, std::size_t j, std::size_t k) {
	return this->setOrigin(glm::vec4(static_cast<std::size_t>(i), static_cast<std::size_t>(j), static_cast<std::size_t>(k), 1.));
}

TetMesh& TetMesh::setTransformationMatrix(const glm::mat4& transfoMat) {
	this->transformationMatrix = transfoMat;
	this->inverseTransformationMatrix = glm::inverse(transfoMat);
	this->updateValues();

	return *this;
}

TetMesh& TetMesh::updateValues() {
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		this->vertexValues[i] = this->texLoader->getTexelValue(inverseTransformationMatrix * this->vertices[i]);
	}
	return *this;
}

TetMesh& TetMesh::updatePositions() {
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		this->vertices[i] += this->origin;
		this->vertices[i].w = 1.;
	}
	this->updateValues();

	return *this;
}

void TetMesh::makeTetrahedra() {
	// For now, a mesh of side 1, centered at the origin

	float m = -0.5f; // 'minus' coordinate
	float p =  0.5f; // 'plus' coordinate

	glm::vec4 a = glm::vec4(m, m, m, 1.); this->vertices.push_back(a); this->vertexValues.push_back(0);
	glm::vec4 b = glm::vec4(m, m, p, 1.); this->vertices.push_back(b); this->vertexValues.push_back(0);
	glm::vec4 c = glm::vec4(m, p, m, 1.); this->vertices.push_back(c); this->vertexValues.push_back(0);
	glm::vec4 d = glm::vec4(m, p, p, 1.); this->vertices.push_back(d); this->vertexValues.push_back(0);
	glm::vec4 e = glm::vec4(p, m, m, 1.); this->vertices.push_back(e); this->vertexValues.push_back(0);
	glm::vec4 f = glm::vec4(p, m, p, 1.); this->vertices.push_back(f); this->vertexValues.push_back(0);
	glm::vec4 g = glm::vec4(p, p, m, 1.); this->vertices.push_back(g); this->vertexValues.push_back(0);
	glm::vec4 h = glm::vec4(p, p, p, 1.); this->vertices.push_back(h); this->vertexValues.push_back(0);

	// For now the tetrahedra initialization is hard-coded, but shouldn't be too hard to paramterize for
	// other shapes using CGAL for example. Here, a 6-tetrahedra decomposition of a cube (not the single
	// possible 5-tetrehedra one) :
	std::vector<std::size_t> tet0 = std::vector<std::size_t>({4, 6, 2, 7}); this->tetrahedra.push_back(tet0);
	std::vector<std::size_t> tet1 = std::vector<std::size_t>({1, 0, 3, 5}); this->tetrahedra.push_back(tet1);
	std::vector<std::size_t> tet2 = std::vector<std::size_t>({3, 4, 7, 5}); this->tetrahedra.push_back(tet2);
	std::vector<std::size_t> tet3 = std::vector<std::size_t>({0, 4, 3, 5}); this->tetrahedra.push_back(tet3);
	std::vector<std::size_t> tet4 = std::vector<std::size_t>({0, 4, 2, 3}); this->tetrahedra.push_back(tet4);
	std::vector<std::size_t> tet5 = std::vector<std::size_t>({2, 4, 7, 3}); this->tetrahedra.push_back(tet5);
}
