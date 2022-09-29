#ifndef GRAPHMESH_HPP_
#define GRAPHMESH_HPP_

#include "base_mesh.hpp"

struct MeshDeformer;

//! \addtogroup geometry
//! @{

struct GraphEdge {
    int pointsIdx[2];

    GraphEdge(int a, int b) {
        pointsIdx[0] = a;
        pointsIdx[1] = b;
    }
};

class GraphMesh : public BaseMesh {

public:
    std::vector<GraphEdge> mesh;

    GraphMesh(std::string const &filename) {
        this->loadMESH(filename);
    }

    void setARAPDeformationMethod() override {};
    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos, glm::vec3& res) const override { return false;};
    void computeNeighborhood() override {};
    void computeNormals() override {};

    void loadMESH(std::string const &filename);

    ~GraphMesh(){};
};

//! @}
#endif
