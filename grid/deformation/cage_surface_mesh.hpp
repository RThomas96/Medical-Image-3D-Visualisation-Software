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

struct Cage : SurfaceMesh {

    bool moveMeshToDeform;// Indicate if translation, rotation, etc, move the mesh to deform too

    BaseMesh * meshToDeform;
    std::vector<glm::vec3> originalVertices;

    Cage(std::string const &filename, BaseMesh * meshToDeform) : SurfaceMesh(filename), meshToDeform(meshToDeform), moveMeshToDeform(true) {
        this->originalVertices = this->meshToDeform->vertices;
    };

    virtual void reInitialize() = 0;
    virtual void computeCoordinates() = 0;

    void translate(const glm::vec3& vec) override {
        SurfaceMesh::translate(vec);
        if(moveMeshToDeform) {
            this->meshToDeform->translate(vec);
            this->reInitialize();
        }
    }

    void rotate(const glm::mat3& transf) override {
        SurfaceMesh::rotate(transf);
        if(moveMeshToDeform) {
            this->meshToDeform->rotate(transf);
            this->reInitialize();
        }
    }

    void scale(const glm::vec3& scale) override {
        SurfaceMesh::scale(scale);
        if(moveMeshToDeform) {
            this->meshToDeform->scale(scale);
            this->reInitialize();
        }
    }

    void setOrigin(const glm::vec3& origin) override {
        SurfaceMesh::setOrigin(origin);
        if(moveMeshToDeform) {
            this->meshToDeform->setOrigin(origin);
            this->reInitialize();
        }
    }

    void bindMovementWithDeformedMesh() {
        this->moveMeshToDeform = true;
        this->reInitialize();
    }

    void unbindMovementWithDeformedMesh() {
        this->moveMeshToDeform = false;
    }

    virtual ~Cage(){};
};

struct CageMVC : Cage {

    std::vector<std::vector<std::pair<unsigned int , float>>> MVCCoordinates;

    CageMVC(std::string const &filename, BaseMesh * meshToDeform) : Cage(filename, meshToDeform) {
        this->reInitialize();
    };

    void reInitialize() override;
    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
    void computeCoordinates() override;
};

struct CageGreen : Cage {

    std::vector<std::vector<double>> phiCoordinates;
    std::vector<std::vector<double>> psiCoordinates;
    std::vector<GreenCoords::GreenScalingFactor<BasicPoint>> scalingFactors;

    CageGreen(std::string const &filename, BaseMesh * meshToDeform) : Cage(filename, meshToDeform) {
        this->reInitialize();
    };

    void reInitialize() override;
    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
    void computeCoordinates() override;

    void update_cage_triangle_scalingFactors();
};

//! @}
#endif
