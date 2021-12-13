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
	std::vector<std::vector<std::pair< std::size_t , float >>> phi;

	// Convert mesh triangles to a series of vectors :
	std::vector<std::vector<std::size_t>> cageTriangles;

	for(std::size_t i=0; i< mesh->getTriangles().size() ; i++ )
	{
		std::vector<std::size_t> currentTriangle;

		currentTriangle.push_back(mesh->getTriangles()[i].getVertex(0));
		currentTriangle.push_back(mesh->getTriangles()[i].getVertex(1));
		currentTriangle.push_back(mesh->getTriangles()[i].getVertex(2));

		cageTriangles.push_back(currentTriangle);
	}

	std::cout << "Triangles container modified" << std::endl;

	// containers to stock the coordinates
	std::vector<std::pair< std::size_t , float >> _phi;

	for(std::size_t i=0 ; i<vertex_positions.size() ; i++)
	{
		MVCCoords::computeMVCCoordinatesOf3dPoint(vertex_positions[i],cageTriangles,mesh->getVertices(),mesh->getNormals(),_phi);
		phi.push_back(_phi);
	}

	std::cout << "Cage coordinates calculated" << std::endl;

	// Check the curve
	glm::vec3 vertexSum;
	glm::vec3 normalSum;
	glm::vec3 sum;

	for(std::size_t j=0; j<phi[0].size(); j++)
	{
		vertexSum = vertexSum + phi[0][j].second*mesh->getVertices()[phi[0][j].first];
	}

	sum = vertexSum;

	std::cout << "Coordonees du premier point sans tranformation de la cage : " << sum << std::endl;

	return std::make_shared<Curve>(mesh, vertex_positions, phi);
}

void Curve::deformFromMeshData() {
	std::cerr << "Updating data from Mesh to curve ...\n";

	const auto& mesh_vertices = this->mesh_cage->getVertices();
	std::vector<glm::vec3> deformed_curve(this->positions.size(), glm::vec3{});

	for (std::size_t v = 0; v < this->positions.size(); ++v) {
		glm::vec3 new_v_position{};
		std::cerr << "Updating vertex " << v << " from the curve ...\n";
		// compute position based on the weights of all vertices that define the original curve position
		for (auto& vertex_weights : this->phi[v]) {
			new_v_position += vertex_weights.second * mesh_vertices[vertex_weights.first];
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
	this->bb_min = glm::vec3(max, max, max);
	this->bb_max = glm::vec3(min, min, min);

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