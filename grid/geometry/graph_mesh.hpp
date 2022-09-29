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

    void loadMESH(std::string const &filename);

    ~GraphMesh();
};

//! @}
#endif
