#ifndef VISUALISATION_CURVE_HPP
#define VISUALISATION_CURVE_HPP

#include "../base_mesh/Mesh.hpp"
#include "./CageCoordinates.h"
#include "./obj_loader.hpp"

#include <QOpenGLFunctions>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Curve {
public:
	using Ptr = std::shared_ptr<Curve>;

public:
	/// @brief Fully-spec'd ctor. Initializes all fields except for the deformed curve positions, left at the origin.
	Curve(
	  Mesh::Ptr& deformer_mesh,
	  std::vector<glm::vec3>& curve_positions,
	  std::vector<std::vector<std::pair<std::size_t, float>>>& weights) :
		mesh_cage(deformer_mesh),
		positions(curve_positions),
		phi(weights) {
		this->update();
	}
	~Curve() = default;

	std::vector<glm::vec3> getBB() { return std::vector<glm::vec3>{this->bb_min, this->bb_max}; }

	void deformFromMeshData();
	void update();

	[[nodiscard]] const Mesh::Ptr getMesh() const { return this->mesh_cage; }
	[[nodiscard]] const std::vector<glm::vec3>& getPositions() const { return this->positions; }
	[[nodiscard]] const std::vector<std::vector<std::pair<std::size_t, float>>>& getWeights() const { return this->phi; }

protected:
	/// @brief Updated curve positions
	std::vector<glm::vec3> positions;
	/// @brief Cage coordinates of the curve relative to its surrounding cage
	std::vector<std::vector<std::pair<std::size_t, float>>> phi;
	/// @brief The surrounding cage (triangular mesh)
	Mesh::Ptr mesh_cage;

	glm::vec3 bb_min;
	glm::vec3 bb_max;
};

Curve::Ptr openCurveFromOBJ(std::string& filename, std::shared_ptr<Mesh>& mesh);

#endif	  // VISUALISATION_CURVE_HPP
