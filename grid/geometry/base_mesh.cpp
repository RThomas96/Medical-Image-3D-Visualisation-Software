#include "base_mesh.hpp"
#include "../deformation/mesh_deformer.hpp"
#include "../ui/manipulator.hpp"
#include <map>
#include <algorithm>
#include <math.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp> 
#include <numeric>


BaseMesh::BaseMesh(): bbMin(glm::vec3(0., 0., 0.)), bbMax(glm::vec3(0., 0., 0.)), history(nullptr), coordinate_system({glm::vec3(1., 0., 0.), glm::vec3(0., 1., 0.), glm::vec3(0., 0., 1.)}) {
}

glm::vec3 BaseMesh::getDimensions() const {
    return this->bbMax - this->bbMin;
}

void BaseMesh::updatebbox() {
    auto maxX = std::max_element(vertices.begin(), vertices.end(), [](const glm::vec3& lhs, const glm::vec3& rhs) { return lhs[0] < rhs[0]; });
    auto minX = std::min_element(vertices.begin(), vertices.end(), [](const glm::vec3& lhs, const glm::vec3& rhs) { return lhs[0] < rhs[0]; });
    auto maxY = std::max_element(vertices.begin(), vertices.end(), [](const glm::vec3& lhs, const glm::vec3& rhs) { return lhs[1] < rhs[1]; });
    auto minY = std::min_element(vertices.begin(), vertices.end(), [](const glm::vec3& lhs, const glm::vec3& rhs) { return lhs[1] < rhs[1]; });
    auto maxZ = std::max_element(vertices.begin(), vertices.end(), [](const glm::vec3& lhs, const glm::vec3& rhs) { return lhs[2] < rhs[2]; });
    auto minZ = std::min_element(vertices.begin(), vertices.end(), [](const glm::vec3& lhs, const glm::vec3& rhs) { return lhs[2] < rhs[2]; });

    int maxXIdx = std::distance(vertices.begin(), maxX);
    int minXIdx = std::distance(vertices.begin(), minX);
    int maxYIdx = std::distance(vertices.begin(), maxY);
    int minYIdx = std::distance(vertices.begin(), minY);
    int maxZIdx = std::distance(vertices.begin(), maxZ);
    int minZIdx = std::distance(vertices.begin(), minZ);

    this->bbMax = glm::vec3(this->vertices[maxXIdx][0], this->vertices[maxYIdx][1], this->vertices[maxZIdx][2]);
    this->bbMin = glm::vec3(this->vertices[minXIdx][0], this->vertices[minYIdx][1], this->vertices[minZIdx][2]);
    //std::cout << "BBox updated to [" << glm::to_string(this->bbMin) << "] [" << glm::to_string(this->bbMax) << "]" << "[" << glm::to_string(this->getOrigin()) << "]" << std::endl;
}

//int BaseMesh::getIdxOfClosestPoint(const glm::vec3& p) const{
//    float distance = std::numeric_limits<float>::max();
//    int res = 0;
//    for(int i = 0; i < this->vertices.size(); ++i) {
//        float currentDistance = glm::distance(p, this->vertices[i]);
//        if(currentDistance < distance) {
//            distance = currentDistance;
//            res = i;
//        }
//    }
//    return res;
//}

void BaseMesh::movePoint(const int& origin, const glm::vec3& target) {
    this->vertices[origin] = target;
    this->computeNormals();
    this->updatebbox();
}

void BaseMesh::movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) {
    int i = 0;
    for(int id : origins) {
        this->vertices[id] = targets[i];
        ++i;
    }
    this->computeNormals();
    this->updatebbox();
}

void BaseMesh::movePoints(const std::vector<glm::vec3>& targets) {
    std::vector<int> indices(this->vertices.size());
    std::iota(std::begin(indices), std::end(indices), 0); // Fill with 0, 1, ..., vertices.size
    this->movePoints(indices, targets);
}

glm::vec3 BaseMesh::getOrigin() const {
    //return glm::vec3(this->bbMax + this->bbMin)/2.f;
    glm::vec3 origin(0., 0., 0.);
    for(int i = 0; i < this->getNbVertices(); ++i) {
        origin += this->vertices[i];
    }
    origin /= this->getNbVertices();
    return origin;
}

void BaseMesh::translate(const glm::vec3& vec) {
    for(int i = 0; i < this->getNbVertices(); ++i) {
        this->vertices[i] += vec;
    }
    this->computeNormals();
    this->updatebbox();
}

void BaseMesh::rotate(const glm::mat3& transf) {
    glm::vec3 origin = this->getOrigin();
    this->translate(-origin);
    for(int i = 0; i < this->getNbVertices(); ++i) {
        this->vertices[i] = transf * this->vertices[i];
    }
    this->translate(origin);
    this->computeNormals();
    this->updatebbox();
}

void BaseMesh::scale(const glm::vec3& scale) {
    for(int i = 0; i < this->getNbVertices(); ++i) {
        this->vertices[i] *= scale;
    }
    this->computeNormals();
    this->updatebbox();
}

void BaseMesh::setOrigin(const glm::vec3& origin) {
    glm::vec3 move = origin - this->getOrigin();
    this->translate(move);
}

const glm::vec3& BaseMesh::getVertice(int i) const {
    return this->vertices[i];
}

const glm::vec3& BaseMesh::getVerticeNormal(int i) const {
    return this->verticesNormals[i];
}

int BaseMesh::getNbVertices() const {
    return this->vertices.size();
}

void BaseMesh::addStateToHistory(bool useTimer) {
    this->history->addStep(this->vertices, useTimer);
}
