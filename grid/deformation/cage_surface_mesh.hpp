#ifndef CAGE_HPP_
#define CAGE_HPP_

#include <vector>
#include <queue>
#include <map>
#include <set>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <cmath>

#include "../geometry/surface_mesh.hpp"
#include "CageCoordinates.h"
#include "../utils/BasicPoint.h"

//! \addtogroup deformation
//! @{

struct CageMVC : SurfaceMesh {

    BaseMesh * meshToDeform;
    std::vector<glm::vec3> originalVertices;

    std::vector<std::vector<std::pair<unsigned int , float>>> MVCCoordinates;

    CageMVC(std::string const &filename, BaseMesh * meshToDeform) : SurfaceMesh(filename), meshToDeform(meshToDeform) {
        this->reInitialize();
    }

    void reInitialize();
    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
    void computeCoordinates();
};

struct CageGreen : SurfaceMesh {
    std::vector<glm::vec3> initial_cage_vertices;
    std::vector<std::vector<int>> initial_cage_triangles;
    std::vector<glm::vec3> initial_cage_triangle_normals;

    std::vector<std::vector<double>> phiCoordinates;
    std::vector<std::vector<double>> psiCoordinates;
    std::vector<GreenCoords::GreenScalingFactor<BasicPoint>> scalingFactors;

    BaseMesh * meshToDeform;

    CageGreen(std::string const &filename, BaseMesh * meshToDeform) : SurfaceMesh(filename), meshToDeform(meshToDeform) {
        this->reInitialize();
    }

    void reInitialize();
    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
    void update_cage_triangle_scalingFactors();
    void computeCoordinates();
};

//! @}
#endif
