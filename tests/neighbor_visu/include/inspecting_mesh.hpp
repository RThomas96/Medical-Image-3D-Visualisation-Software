#ifndef TESTS_NEIGHBOR_VISU_INSPECTING_MESH_HPP_
#define TESTS_NEIGHBOR_VISU_INSPECTING_MESH_HPP_

#include <glm/glm.hpp>

#include <iostream>
#include <cstdint>
#include <vector>

enum NeighborDirection {
	Center   = 0b00000000u,
	Bottom   = 0b00000001u,
	Top      = 0b00000010u,
	Left     = 0b00000100u,
	Right    = 0b00001000u,
	Backward = 0b00010000u,
	Forward  = 0b00100000u
};

///! @brief Returns lhs & rhs
inline bool operator& (const NeighborDirection lhs, const NeighborDirection rhs) {
	return (static_cast<uint_fast16_t>(lhs) & static_cast<uint_fast16_t>(rhs));
}

///! @brief Returns lhs | rhs, only applied once (checks if already applied)
inline NeighborDirection operator| (const NeighborDirection lhs, const NeighborDirection rhs) {
	return (lhs & rhs) ? lhs : static_cast<NeighborDirection>(static_cast<uint_fast16_t>(lhs) | static_cast<uint_fast16_t>(rhs));
}

class InspectingMesh {
	public:
		InspectingMesh(void);
		InspectingMesh(const glm::vec3 origin);
		InspectingMesh(const glm::vec4 origin);
		InspectingMesh(const glm::vec3 origin, const glm::mat4 transformationMatrix);
		InspectingMesh(const glm::vec4 origin, const glm::mat4 transformationMatrix);
		~InspectingMesh(void);
		// Returns any point directly connected to the queried point
		glm::vec4 operator[](const NeighborDirection dir) const;
		// Return any point at any distance I,J,K (measured in voxels on all axes)
		glm::vec4 operator()(const int i, const int j, const int k) const;
		InspectingMesh& setOrigin(glm::vec3 nO);
		InspectingMesh& setTransformationMatrix(const glm::mat4 tM);
	private:
		std::vector<glm::vec4> grid; ///< statically assigned grid coordinates for the neighbor inspector
		glm::vec4 origin; ///< origin of the grid (will be used to compute the data at a given point
		glm::mat4 inversetransformMatrix; ///< To go from the real space to the initial space
};

#endif // TESTS_NEIGHBOR_VISU_INSPECTING_MESH_HPP_
