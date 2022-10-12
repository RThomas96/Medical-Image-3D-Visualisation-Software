#ifndef GRAPHMESH_HPP_
#define GRAPHMESH_HPP_

#include "base_mesh.hpp"
#include "grid/drawable/drawable.hpp"

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

class GraphMesh : public BaseMesh, public UITool::GL::DrawableUI {

public:
    std::vector<GraphEdge> mesh;

    GraphMesh(std::string const &filename) {
        this->loadOFF(filename);
    }

    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos, glm::vec3& res) const override { return false;};
    void computeNormals() override {};

    void loadOFF(std::string const &filename);
    void saveOFF(std::string const &filename);
    void draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat);

    ~GraphMesh(){};
};

//! @}
#endif
