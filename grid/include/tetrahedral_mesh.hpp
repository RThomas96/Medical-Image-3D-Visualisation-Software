#ifndef TETRAHEDRALMESH_HPP_
#define TETRAHEDRALMESH_HPP_

#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <vector>

enum DeformMethod {
    NORMAL,
    WEIGHTED
};

struct TetMesh;
struct MeshDeformator {
    DeformMethod deformMethod;
    TetMesh * tetmesh;

    MeshDeformator(TetMesh * tetmesh, DeformMethod deformMethod) : tetmesh(tetmesh), deformMethod(deformMethod) {}

    // Here origin is basically the clicked point
    virtual bool hasSelectedPts() = 0;
    virtual void selectPts(const glm::vec3& pt) = 0;
    virtual void deselectPts(const glm::vec3& pt) = 0;
    virtual void deselectAllPts() = 0;

    virtual void movePoint(const glm::vec3& origin, const glm::vec3& target) = 0;

    virtual ~MeshDeformator() = default;// To make MeshDeformator virtual
};

struct WeightedMethod : MeshDeformator {
    float radius;
    glm::vec3 originalPoint;
    std::vector<int> selectedPts;

    WeightedMethod(TetMesh * tetmesh, float radius) : MeshDeformator(tetmesh, DeformMethod::WEIGHTED), radius(radius) {}

    bool hasSelectedPts() override;
    void selectPts(const glm::vec3& pt) override;
    void deselectPts(const glm::vec3& pt) override;
    void deselectAllPts() override;

    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
};

struct NormalMethod : MeshDeformator {
    std::vector<int> selectedPts;

    NormalMethod(TetMesh * tetmesh) : MeshDeformator(tetmesh, DeformMethod::NORMAL) {}

    bool hasSelectedPts() override;
    void selectPts(const glm::vec3& pt) override;
    void deselectPts(const glm::vec3& pt) override;
    void deselectAllPts() override;

    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
};

/***/

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
};

/***/

struct TetMesh {

    std::vector<Tetrahedron> mesh;
    std::vector<glm::vec3> ptGrid;
    std::vector<glm::vec3> texCoordGrid;// These are normalised coordinates

    glm::vec3 nbTetra;

    glm::vec3 bbMin;
    glm::vec3 bbMax;

    MeshDeformator * meshDeformator;

    TetMesh(): nbTetra(glm::vec3(0., 0., 0.)), bbMin(glm::vec3(0., 0., 0.)), bbMax(glm::vec3(0., 0., 0.)), meshDeformator(new NormalMethod(this)) {}

    void buildGrid(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin);

    bool isEmpty() const;

    Tetrahedron getTetra(int idx) const;
    int inTetraIdx(const glm::vec3& p) const;

    int getIdxOfClosestPoint(const glm::vec3& p) const;

    glm::vec3 getDimensions() const;
    int from3DTo1D(const glm::vec3& p) const;

    // Temporary function to map to current GL
    void replaceAllPoints(const std::vector<glm::vec3>& pts);

    void computeNeighborhood();
    void computeNormals();
    void updatebbox();

    void movePoint(const glm::vec3& origin, const glm::vec3& target);
    void setNormalDeformationMethod();
    void setWeightedDeformationMethod(float radius);

private:
    // This function is private because it doesn't update fields nbTetra, bbMin and bbMax
    // Thus it can only be used in buildGrid function
    void decomposeAndAddCube(std::vector<glm::vec3*> pts, const std::vector<int>& ptsIdx);
    std::vector<glm::vec3*> insertCubeIntoPtGrid(std::vector<glm::vec3> cubePts, glm::vec3 indices, std::vector<glm::vec3>& ptGrid, std::vector<int>& ptIndices);
};

/***/

/***/

#endif
