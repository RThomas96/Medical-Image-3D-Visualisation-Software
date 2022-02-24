#include "mesh_deformator.hpp"
#include "../geometry/tetrahedral_mesh.hpp"
#include "../geometry/surface_mesh.hpp"
#include <algorithm>

bool WeightedMethod::hasSelectedPts() {
    return !this->selectedPts.empty();
}

void WeightedMethod::selectPts(const glm::vec3& pt) {
    for(int i = 0; i < this->baseMesh->vertices.size(); ++i) {
        const glm::vec3& pt2 = this->baseMesh->getWorldVertice(i);
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
        const glm::vec3& pt2 = this->baseMesh->getWorldVertice(this->selectedPts[i]);
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

ARAPMethod::ARAPMethod(SurfaceMesh * surfaceMesh) : MeshDeformator(dynamic_cast<BaseMesh*>(surfaceMesh), DeformMethod::ARAP){
    this->onSurfaceMesh = true;
    for(int i = 0; i < this->baseMesh->getNbVertices(); ++i)
        this->handles.push_back(false);

    //this->arap.clear();
    //this->arap.init(surfaceMesh->getWorldMeshPositions(), surfaceMesh->getTriangles());
    //this->arap.setHandles(this->handles);
}

ARAPMethod::ARAPMethod(BaseMesh * baseMesh) : MeshDeformator(baseMesh, DeformMethod::ARAP) {
    this->onSurfaceMesh = false;
    //this->arap.clear();
    std::cout << "WARNING: trying to use ARAP deformation on GenericMesh, but arap only work on SurfaceMesh, thus the operation will be a [DIRECT] deformation" << std::endl;
}

bool ARAPMethod::hasSelectedPts() {
    return !this->selectedPts.empty();
}

void ARAPMethod::selectPts(const glm::vec3& pt) {
    this->selectedPts.push_back(this->baseMesh->getIdxOfClosestPoint(pt));
}

void ARAPMethod::deselectPts(const glm::vec3& pt) {
    int ptIdx = this->baseMesh->getIdxOfClosestPoint(pt);
    auto ptIdxPos = std::find(this->selectedPts.begin(), this->selectedPts.end(), ptIdx);
    if(ptIdxPos != this->selectedPts.end()) {
        this->selectedPts.erase(ptIdxPos);
    }
}

void ARAPMethod::deselectAllPts() {
    this->selectedPts.clear();
}

void ARAPMethod::movePoint(const glm::vec3& origin, const glm::vec3& target) {
    const glm::vec3 deplacement = target - origin;
    for(int i = 0; i < this->selectedPts.size(); ++i)
        this->baseMesh->vertices[this->selectedPts[i]] += deplacement;

    if(this->onSurfaceMesh) {
        //this->arap.compute_deformation(this->baseMesh->getWorldMeshPositions());
    }
}
