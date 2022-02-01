#include "../include/mesh_deformator.hpp"
#include "../include/tetrahedral_mesh.hpp"
#include <algorithm>

bool WeightedMethod::hasSelectedPts() {
    return !this->selectedPts.empty();
}

void WeightedMethod::selectPts(const glm::vec3& pt) {
    for(int i = 0; i < this->tetmesh->vertices.size(); ++i) {
        glm::vec3& pt2 = this->tetmesh->vertices[i];
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
        glm::vec3& pt2 = this->tetmesh->vertices[this->selectedPts[i]];
        float distance = glm::distance(target, pt2);
        if(distance < this->radius) {
            float coeff = 1 - std::pow((distance / this->radius), 2);
            pt2 += (deplacement * coeff);
        }
    }
}

/***/

bool NormalMethod::hasSelectedPts() {
    return !this->selectedPts.empty();
}

void NormalMethod::selectPts(const glm::vec3& pt) {
    this->selectedPts.push_back(this->tetmesh->getIdxOfClosestPoint(pt));
}

void NormalMethod::deselectPts(const glm::vec3& pt) {
    int ptIdx = this->tetmesh->getIdxOfClosestPoint(pt);
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
        this->tetmesh->vertices[this->selectedPts[i]] += deplacement;
    }
}


