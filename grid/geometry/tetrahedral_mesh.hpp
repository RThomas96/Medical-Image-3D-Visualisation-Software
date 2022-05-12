#ifndef TETRAHEDRALMESH_HPP_
#define TETRAHEDRALMESH_HPP_

#include "base_mesh.hpp"
//#include "../include/mesh_deformer.hpp"

struct MeshDeformer;

//! \addtogroup geometry
//! @{

struct Tetrahedron {
    glm::vec3 * points[4];// Optionnal, this data can be deleted and computeBary, isInTet and baryToWord function moved out in the TetMesh class
    glm::vec4 normals[4];

    int pointsIdx[4];
    int neighbors[4];

    Tetrahedron();

    Tetrahedron(glm::vec3* a, glm::vec3* b, glm::vec3* c, glm::vec3* d);

    void setIndices(int a, int b, int c, int d);

    glm::vec4 computeBaryCoord(const glm::vec3& p);

    bool isInTetrahedron(const glm::vec3& p) const;

    glm::vec3 baryToWorldCoord(const glm::vec4& coord);

    void computeNormals();

    int getPointIndex(int faceIdx, int ptIdxInFace) const;

    glm::vec3 * getPoint(int faceIdx, int ptIdxInFace) const;

    void getCentroid(glm::vec3& centroid) const;

    //bool faceIntersect(const glm::vec3& p1, const glm::vec3& p2, int faceIdx) const;

    bool faceIntersect(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) const;
    bool planeIntersect(const glm::vec3& origin, const glm::vec3& direction) const;

    glm::vec3 getBBMax() const;
    glm::vec3 getBBMin() const;
};

class TetMesh : public BaseMesh {

public:
    std::vector<Tetrahedron> mesh;
    glm::vec3 nbTetra;

    TetMesh();

    virtual void loadMESH(std::string const &filename);

    bool isEmpty() const;
    void buildGrid(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin);

    void computeNeighborhood() override;
    void computeNormals() override;

    // Specific to Tethrahedal mesh
    Tetrahedron getTetra(int idx) const;
    int inTetraIdx(const glm::vec3& p) const;
    //glm::vec3 getCoordInInitial(const TetMesh& initial, glm::vec3 p) const;
    bool getCoordInInitial(const TetMesh& initial, const glm::vec3& p, glm::vec3& out, int tetraIdx = -1) const;

    void setARAPDeformationMethod() override;
    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos, glm::vec3& res) const override;

    ~TetMesh();

private:
    // This function is private because it doesn't update fields nbTetra, bbMin and bbMax
    // Thus it can only be used in buildGrid function
    void decomposeAndAddCube(std::vector<glm::vec3*> pts, const std::vector<int>& ptsIdx);
    std::vector<glm::vec3*> insertCubeIntoPtGrid(std::vector<glm::vec3> cubePts, glm::vec3 indices, std::vector<glm::vec3>& ptGrid, std::vector<int>& ptIndices);
    int from3DTo1D(const glm::vec3& p) const;
};

//! @}
#endif
