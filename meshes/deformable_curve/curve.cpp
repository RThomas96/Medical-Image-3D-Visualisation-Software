#include "./curve.hpp"
#include "../base_mesh/Triangle.h"

#include <array>
#include <glm/gtx/io.hpp>

Curve::Ptr openCurveFromOBJ(std::string& filename, std::shared_ptr<Mesh>& mesh) {
	std::vector<glm::vec3> vertex_positions;
	std::vector<Triangle> triangles;
	// Load the OBJ file :
	FileIO::objLoader(filename, vertex_positions, triangles);

	// The cage weights :
	std::vector<std::vector<std::pair<std::size_t, float>>> phi;

	// Convert mesh triangles to a series of vectors :
	std::vector<std::vector<std::size_t>> cageTriangles;
	std::vector<std::vector<int>> cageTrianglesGC;

	for (std::size_t i = 0; i < mesh->getTriangles().size(); i++)
	{
		std::vector<std::size_t> currentTriangle;
		std::vector<int> currentTriangleGC;

		currentTriangle.push_back(mesh->getTriangles()[i].getVertex(0));
		currentTriangle.push_back(mesh->getTriangles()[i].getVertex(1));
		currentTriangle.push_back(mesh->getTriangles()[i].getVertex(2));
		currentTriangleGC.push_back(static_cast<int>(mesh->getTriangles()[i].getVertex(0)));
		currentTriangleGC.push_back(static_cast<int>(mesh->getTriangles()[i].getVertex(1)));
		currentTriangleGC.push_back(static_cast<int>(mesh->getTriangles()[i].getVertex(2)));

		cageTriangles.push_back(currentTriangle);
		cageTrianglesGC.push_back(currentTriangleGC);
	}

	std::cout << "Triangles container modified" << std::endl;

	// containers to stock the coordinates
	std::vector<std::pair<std::size_t, float>> _phi;
	std::vector<std::vector<float>> gc_phi(vertex_positions.size());
	std::vector<std::vector<float>> gc_psi(vertex_positions.size());

	for (std::size_t i = 0; i < vertex_positions.size(); i++)
	{
		// FOR MVC :
		MVCCoords::computeMVCCoordinatesOf3dPoint(vertex_positions[i], cageTriangles, mesh->getVertices(), mesh->getNormals(), _phi);
		// FOR GREEN COORDS :
		//GreenCoords::computeGreenCoordinatesOf3dPoint(vertex_positions[i], cageTrianglesGC, mesh->getVertices(), mesh->getNormals(), gc_phi[i], gc_psi[i]);
		phi.push_back(_phi);
	}

	std::cout << "Cage coordinates calculated" << std::endl;

	// Check the curve
	glm::vec3 vertexSum{.0f};
	glm::vec3 normalSum{.0f};
	glm::vec3 sum{.0f};

	// FOR MVC :
	for (std::size_t j = 0; j < phi[0].size(); j++)
	{
		vertexSum = vertexSum + phi[0][j].second * mesh->getVertices()[phi[0][j].first];
	}
	// FOR GREEN :
//	for (std::size_t j = 0; j < gc_phi[0].size(); ++j) {
//		vertexSum += gc_phi[0][j] * vertex_positions[j];
//	}
//	for (std::size_t f = 0; f < gc_psi[0].size(); ++f) {
//		normalSum += gc_psi[0][f] * mesh->getNormals()[f];
//	}

	sum = vertexSum; // FOR MVC
	//sum = vertexSum + normalSum; // FOR GREEN

	std::cout << "Coordonees du premier point sans tranformation de la cage : " << sum << std::endl;

	// FOR MVC :
	return std::make_shared<Curve>(mesh, vertex_positions, phi);
	// FOR GREEN COORDS :
	//return std::make_shared<Curve>(mesh, vertex_positions, gc_phi, gc_psi);
}

void Curve::setPositions(std::vector<glm::vec3>& newpositions) {
	this->positions = newpositions;
	this->computeWeightsFromMeshData();
	this->update();
}

void Curve::deformFromMeshData() {
	std::cerr << "Updating data from Mesh to curve ...\n";

	const auto& mesh_vertices = this->mesh_cage->getVertices();
	const auto& mesh_normals = this->mesh_cage->getNormals();
	std::vector<glm::vec3> deformed_curve(this->positions.size(), glm::vec3{});

	for (std::size_t v = 0; v < this->positions.size(); ++v) {
		glm::vec3 new_v_position{.0f};
		std::cerr << "Updating vertex " << v << " from the curve ...\n";
		// compute position based on the weights of all vertices that define the original curve position
		// FOR MVC :
		if (not this->is_green) {
			for (auto& vertex_weights : this->phi[v]) {
				new_v_position += vertex_weights.second * mesh_vertices[vertex_weights.first];
			}
		} else {
			// FOR GREEN :
			for (size_t vv = 0; vv < this->gc_phi[v].size(); ++vv) {
				new_v_position += this->gc_phi[v][vv] * mesh_vertices[v];
			}
			for (size_t f = 0; f < this->gc_psi[v].size(); ++f) {
				new_v_position += this->gc_psi[v][f] * mesh_normals[f]; // FIXME : scaling weights ?
			}
		}
		deformed_curve[v] = new_v_position;
	}

	this->positions = deformed_curve;

	this->update();
	std::cerr << "Updated data from the Mesh.\n";
}

void Curve::update() {
	glm::vec3::value_type min = std::numeric_limits<glm::vec3::value_type>::lowest();
	glm::vec3::value_type max = std::numeric_limits<glm::vec3::value_type>::max();
	this->bb_min			  = glm::vec3(max, max, max);
	this->bb_max			  = glm::vec3(min, min, min);

	for (const auto& vertex : this->positions) {
		this->bb_min.x = std::min(vertex.x, this->bb_min.x);
		this->bb_min.y = std::min(vertex.y, this->bb_min.y);
		this->bb_min.z = std::min(vertex.z, this->bb_min.z);

		this->bb_max.x = std::max(vertex.x, this->bb_max.x);
		this->bb_max.y = std::max(vertex.y, this->bb_max.y);
		this->bb_max.z = std::max(vertex.z, this->bb_max.z);
	}

	return;
}

void Curve::computeWeightsFromMeshData() {
	std::vector<std::vector<std::pair<std::size_t, float>>> new_phi;

	// Convert mesh triangles to a series of vectors :
	std::vector<std::vector<std::size_t>> cageTriangles;
	std::vector<std::vector<int>> cageTrianglesGC;

	for (std::size_t i = 0; i < this->mesh_cage->getTriangles().size(); i++)
	{
		std::vector<std::size_t> currentTriangle;
		std::vector<int> currentTriangleGC;

		currentTriangle.push_back(this->mesh_cage->getTriangles()[i].getVertex(0));
		currentTriangle.push_back(this->mesh_cage->getTriangles()[i].getVertex(1));
		currentTriangle.push_back(this->mesh_cage->getTriangles()[i].getVertex(2));
		currentTriangleGC.push_back(static_cast<int>(this->mesh_cage->getTriangles()[i].getVertex(0)));
		currentTriangleGC.push_back(static_cast<int>(this->mesh_cage->getTriangles()[i].getVertex(1)));
		currentTriangleGC.push_back(static_cast<int>(this->mesh_cage->getTriangles()[i].getVertex(2)));

		cageTriangles.push_back(currentTriangle);
		cageTrianglesGC.push_back(currentTriangleGC);
	}

	std::cout << "Triangles container modified" << std::endl;

	// containers to stock the coordinates
	std::vector<std::pair<std::size_t, float>> _phi;
	const auto& vertex_positions = this->positions;
	std::vector<std::vector<float>> _gc_phi(vertex_positions.size());
	std::vector<std::vector<float>> _gc_psi(vertex_positions.size());

	for (std::size_t i = 0; i < vertex_positions.size(); i++)
	{
		if (not this->is_green) {
			MVCCoords::computeMVCCoordinatesOf3dPoint(vertex_positions[i], cageTriangles, this->mesh_cage->getVertices(), this->mesh_cage->getNormals(), _phi);
			new_phi.push_back(_phi);
		} else {
			GreenCoords::computeGreenCoordinatesOf3dPoint(vertex_positions[i], cageTrianglesGC, this->mesh_cage->getVertices(), this->mesh_cage->getNormals(), _gc_phi[i], _gc_psi[i]);
			// FIXME : scaling weights ?
		}
	}

	std::cout << "Cage coordinates calculated" << std::endl;

	// Check the curve
	glm::vec3 vertexSum{.0f};
	glm::vec3 normalSum{.0f};
	glm::vec3 sum;

	if (not this->is_green) {
		// FOR MVC :
		for (std::size_t j = 0; j < phi[0].size(); j++) {
			vertexSum = vertexSum + phi[0][j].second * this->mesh_cage->getVertices()[phi[0][j].first];
		}
		sum = vertexSum;
	} else {
		// FOR GREEN :
		for (std::size_t j = 0; j < gc_phi[0].size(); ++j) {
			vertexSum += gc_phi[0][j] * vertex_positions[j];
		}
		for (std::size_t f = 0; f < gc_psi[0].size(); ++f) {
			normalSum += gc_psi[0][f] * this->mesh_cage->getNormals()[f];
		}
		sum = vertexSum + normalSum;
	}


	std::cout << "Coordonees du premier point sans tranformation de la cage : " << sum << std::endl;

	if (this->is_green) {
		this->gc_phi = _gc_phi;
		this->gc_psi = _gc_psi;
	} else {
		this->phi = new_phi;
	}
}
