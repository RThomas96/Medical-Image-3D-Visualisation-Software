#ifndef TETRAHEDRALMESH_HPP_
#define TETRAHEDRALMESH_HPP_

#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <vector>

struct Tetrahedron {
    glm::vec3 * points[4];

    Tetrahedron();

    Tetrahedron(glm::vec3* a, glm::vec3* b, glm::vec3* c, glm::vec3* d);

    glm::vec4 computeBaryCoord(const glm::vec3& p);

    bool isInTetrahedron(const glm::vec3& p);

    glm::vec3 baryToWorldCoord(const glm::vec4& coord);
};

struct TetMesh {

    std::vector<Tetrahedron> mesh;
    std::vector<glm::vec3> ptGrid;
    glm::vec3 nbTetra;

    glm::vec3 bbMin;
    glm::vec3 bbMax;

    TetMesh(): nbTetra(glm::vec3(0., 0., 0.)), bbMin(glm::vec3(0., 0., 0.)), bbMax(glm::vec3(0., 0., 0.)) {}

    void buildGrid(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin);

    void movePoint(const glm::vec3& indices, const glm::vec3& position);

    bool isEmpty() const;

    Tetrahedron getTetra(int idx) const;

    int inTetraIdx(const glm::vec3& p);

    glm::vec3 getDimensions() const;

    int from3DTo1D(const glm::vec3& p) const;

    // Temporary function to map to current GL
    void replaceAllPoints(const std::vector<glm::vec3>& pts);

private:
    // This function is private because it doesn't update fields nbTetra, bbMin and bbMax
    // Thus it can only be used in buildGrid function
    void addCube(std::vector<glm::vec3*> pts);
    std::vector<glm::vec3*> insertCube(std::vector<glm::vec3> cubePts, glm::vec3 indices, std::vector<glm::vec3>& ptGrid) const;
};

#endif
