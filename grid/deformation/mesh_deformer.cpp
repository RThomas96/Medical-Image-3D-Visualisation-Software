#include "mesh_deformer.hpp"
#include "../geometry/tetrahedral_mesh.hpp"
#include "../geometry/surface_mesh.hpp"
#include <algorithm>
#include "glm/gtx/string_cast.hpp"

bool WeightedMethod::hasSelectedPts() {
    return !this->selectedPts.empty();
}

void WeightedMethod::selectPts(const glm::vec3& pt) {
    for(int i = 0; i < this->baseMesh->vertices.size(); ++i) {
        const glm::vec3& pt2 = this->baseMesh->getVertice(i);
        float distance = glm::distance(pt, pt2);
        if(distance < this->radius) {
            this->selectedPts.push_back(i);
        }
    }
    this->originalPoint = pt;
}

void WeightedMethod::deselectPts(const glm::vec3& pt) {
    this->selectedPts.clear();// For now you cannot move multiple points with weighted move
}

void WeightedMethod::deselectAllPts() {
    this->selectedPts.clear();
}

void WeightedMethod::movePoint(const glm::vec3& origin, const glm::vec3& target) {
    const float maxDist = std::max(glm::distance(target, this->originalPoint), this->radius);
    const glm::vec3 deplacement = target - origin;
    std::vector<int> idxToRemove;
    for(int i = 0; i < this->selectedPts.size(); ++i) {
        const glm::vec3& pt2 = this->baseMesh->getVertice(this->selectedPts[i]);
        float distance = glm::distance(target, pt2);
        if(distance < this->radius) {
            float coeff = 1 - std::pow((distance / this->radius), 2);
            // Here we move the original point !
            this->baseMesh->vertices[this->selectedPts[i]] += (deplacement * coeff);
        }
    }
}

/***/

bool NormalMethod::hasSelectedPts() {
    return !this->selectedPts.empty();
}

void NormalMethod::selectPts(const glm::vec3& pt) {
    this->selectedPts.push_back(this->baseMesh->getIdxOfClosestPoint(pt));
}

void NormalMethod::deselectPts(const glm::vec3& pt) {
    int ptIdx = this->baseMesh->getIdxOfClosestPoint(pt);
    auto ptIdxPos = std::find(this->selectedPts.begin(), this->selectedPts.end(), ptIdx);
    if(ptIdxPos != this->selectedPts.end()) {
        this->selectedPts.erase(ptIdxPos);
    }
}

void NormalMethod::deselectAllPts() {
    this->selectedPts.clear();
}

void NormalMethod::movePoint(const glm::vec3& origin, const glm::vec3& target) {
    const glm::vec3 deplacement = target - origin;
    for(int i = 0; i < this->selectedPts.size(); ++i) {
        this->baseMesh->vertices[this->selectedPts[i]] += deplacement;
    }
}

/***/

ARAPMethod::ARAPMethod(SurfaceMesh * surfaceMesh) : MeshDeformer(dynamic_cast<BaseMesh*>(surfaceMesh), DeformMethod::ARAP){
    this->onSurfaceMesh = true;
    std::vector<Vec3D<float>> ptsAsVec3D;
    for(int i = 0; i < this->baseMesh->getNbVertices(); ++i) {
        glm::vec3 pt = surfaceMesh->getVertice(i);
        ptsAsVec3D.push_back(Vec3D(pt[0], pt[1], pt[2]));
        this->handles.push_back(false);
    }

    this->arap.clear();
    this->arap.init(ptsAsVec3D, surfaceMesh->getTriangles());
    this->arap.setHandles(this->handles);
}

ARAPMethod::ARAPMethod(BaseMesh * baseMesh) : MeshDeformer(baseMesh, DeformMethod::ARAP) {
    this->onSurfaceMesh = false;
    this->arap.clear();
    std::cout << "WARNING: trying to use ARAP deformation on GenericMesh, but ARAP only works on SurfaceMesh, thus the operation will be a [DIRECT] deformation" << std::endl;
}

bool ARAPMethod::hasSelectedPts() {
    return !this->selectedPts.empty();
}

void ARAPMethod::selectPts(const glm::vec3& pt) {
    this->selectedPts.push_back(this->baseMesh->getIdxOfClosestPoint(pt));
    this->handles[this->selectedPts.back()] = true;
}

void ARAPMethod::deselectPts(const glm::vec3& pt) {
    int ptIdx = this->baseMesh->getIdxOfClosestPoint(pt);
    auto ptIdxPos = std::find(this->selectedPts.begin(), this->selectedPts.end(), ptIdx);
    if(ptIdxPos != this->selectedPts.end()) {
        this->selectedPts.erase(ptIdxPos);
    }
    this->handles[ptIdx] = false;
    std::vector<Vec3D<float>> ptsAsVec3D;
    for(int i = 0; i < this->baseMesh->getNbVertices(); ++i) {
        glm::vec3 pt = this->baseMesh->getVertice(i);
        ptsAsVec3D.push_back(Vec3D(pt[0], pt[1], pt[2]));
    }
    this->arap.init(ptsAsVec3D, dynamic_cast<SurfaceMesh*>(this->baseMesh)->getTriangles());
}

void ARAPMethod::deselectAllPts() {
    this->selectedPts.clear();
}

void ARAPMethod::setHandle(int idx) {
    this->handles[idx] = true;
}

void ARAPMethod::unsetHandle(int idx) {
    this->handles[idx] = false;
}

void ARAPMethod::movePoint(const glm::vec3& origin, const glm::vec3& target) {
    const glm::vec3 deplacement = target - origin;
    for(int i = 0; i < this->selectedPts.size(); ++i)
        this->baseMesh->vertices[this->selectedPts[i]] += deplacement;

    if(this->onSurfaceMesh) {
        std::vector<Vec3D<float>> ptsAsVec3D;
        for(int i = 0; i < this->baseMesh->getNbVertices(); ++i) {
            glm::vec3 pt = this->baseMesh->getVertice(i);
            ptsAsVec3D.push_back(Vec3D(pt[0], pt[1], pt[2]));
        }
        this->arap.setHandles(this->handles);
        this->arap.compute_deformation(ptsAsVec3D);
        for(int i = 0; i < this->baseMesh->getNbVertices(); ++i)
            this->baseMesh->vertices[i] = glm::vec3(ptsAsVec3D[i][0], ptsAsVec3D[i][1],ptsAsVec3D[i][2]);
    }
}

void ARAPMethod::fitToPointList(const std::vector<int>& vertices, const std::vector<glm::vec3>& newPositions) {
    for(int i = 0; i < vertices.size(); ++i) {
        this->handles[vertices[i]] = true;
        this->baseMesh->vertices[vertices[i]] = newPositions[i];
    }

    if(this->onSurfaceMesh) {
        std::vector<Vec3D<float>> ptsAsVec3D;
        for(int i = 0; i < this->baseMesh->getNbVertices(); ++i) {
            glm::vec3 pt = this->baseMesh->getVertice(i);
            ptsAsVec3D.push_back(Vec3D(pt[0], pt[1], pt[2]));
        }
        this->arap.setHandles(this->handles);
        this->arap.compute_deformation(ptsAsVec3D);
        for(int i = 0; i < this->baseMesh->getNbVertices(); ++i)
            this->baseMesh->vertices[i] = glm::vec3(ptsAsVec3D[i][0], ptsAsVec3D[i][1],ptsAsVec3D[i][2]);
    }

    for(int i = 0; i < vertices.size(); ++i)
        this->handles[vertices[i]] = false;
}
