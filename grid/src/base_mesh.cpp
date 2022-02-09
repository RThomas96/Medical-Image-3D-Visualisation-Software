#include "../include/base_mesh.hpp"
#include "../include/mesh_deformator.hpp"
#include "../include/manipulator.hpp"
#include <map>
#include <algorithm>
#include <math.h>
#include <glm/gtx/transform.hpp>


BaseMesh::BaseMesh(): bbMin(glm::vec3(0., 0., 0.)), bbMax(glm::vec3(0., 0., 0.)), meshDeformator(new NormalMethod(this)), transformation(glm::mat4(1.f)), scale(1.f) {
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
}

int BaseMesh::getIdxOfClosestPoint(const glm::vec3& p) const{
    float distance = std::numeric_limits<float>::max();
    int res = 0;
    for(int i = 0; i < this->vertices.size(); ++i) {
        const glm::vec3& p2 = this->toWorld(this->vertices[i]);
        float currentDistance = glm::distance(p, p2);
        if(currentDistance < distance) {
            distance = currentDistance;
            res = i;
        }
    }
    return res;
}

void BaseMesh::movePoint(const glm::vec3& origin, const glm::vec3& target) {
    if(this->meshDeformator->hasSelectedPts()) {
        this->meshDeformator->movePoint(origin, target);
        this->computeNormals();
        this->updatebbox();
    } else {
        std::cout << "WARNING: try to move points when there is no point in the point to move queue" << std::endl;
    }
}

void BaseMesh::setNormalDeformationMethod() {
    if(this->meshDeformator->deformMethod != DeformMethod::NORMAL) {
        delete this->meshDeformator;
        this->meshDeformator = new NormalMethod(this);
    }
}

void BaseMesh::setWeightedDeformationMethod(float radius) {
    if(this->meshDeformator->deformMethod != DeformMethod::WEIGHTED) {
        delete this->meshDeformator;
        this->meshDeformator = new WeightedMethod(this, radius);
    }
}

void BaseMesh::selectPts(const glm::vec3& pt) {
    this->meshDeformator->selectPts(pt);
}

void BaseMesh::deselectAllPts() {
    this->meshDeformator->deselectAllPts();
}

std::vector<glm::vec3>& BaseMesh::getMeshPositions() {
    return this->vertices;
}

glm::vec3 BaseMesh::getOrigin() {
    return glm::vec3(this->transformation[3][0], this->transformation[3][1], this->transformation[3][2]);
}

glm::mat4 BaseMesh::getModelTransformation() {
    return this->transformation;
}

void BaseMesh::setOrigin(const glm::vec3& origin) {
    this->transformation[3][0] = origin[0];
    this->transformation[3][1] = origin[1];
    this->transformation[3][2] = origin[2];
}

void BaseMesh::translate(const glm::vec3& vec) {
    this->transformation[3][0] += vec[0];
    this->transformation[3][1] += vec[1];
    this->transformation[3][2] += vec[2];
}

void BaseMesh::setScale(float scale) {
    this->transformation[0][0] = scale;
    this->transformation[1][1] = scale;
    this->transformation[2][2] = scale;
}

std::vector<glm::vec3> BaseMesh::getWorldMeshPositions() {
    std::vector<glm::vec3> worldPos;
    for(int i = 0; i < this->vertices.size(); ++i) {
        glm::vec4 pt = this->transformation * glm::vec4(this->vertices[i], 1.);
        worldPos.push_back(glm::vec3(pt[0], pt[1], pt[2]));
    }
    return worldPos;
}

glm::vec3 BaseMesh::toWorld(const glm::vec3& pt) const {
    return glm::vec3(this->transformation * glm::vec4(pt, 1.));
}

glm::vec3 BaseMesh::toModel(const glm::vec3& pt) const {
    return glm::vec3(glm::inverse(this->transformation) * glm::vec4(pt, 1.));
}

const glm::vec3& BaseMesh::getVertice(int i) const {
    return this->vertices[i];
}

const glm::vec3& BaseMesh::getVerticeNormal(int i) const {
    return this->verticesNormals[i];
}

const glm::vec3 BaseMesh::getWorldVertice(int i) const {
    return glm::vec3(this->transformation * glm::vec4(this->vertices[i], 1.));
}

const glm::vec3 BaseMesh::getWorldVerticeNormal(int i) const {
    return glm::normalize(glm::vec3(this->transformation * glm::vec4(this->verticesNormals[i], 1.)));
    //return glm::normalize(this->transformation * glm::vec4(this->verticesNormals[i], 1.));
}

int BaseMesh::getNbVertices() const {
    return this->vertices.size();
}

void BaseMesh::rotate(const glm::mat3& transf) {
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            this->transformation[i][j] += transf[i][j];
}

void BaseMesh::setTransformation(const glm::mat3& transf) {
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            this->transformation[i][j] = transf[i][j];
}

void BaseMesh::rotate(const float angle, const glm::vec3 axis) {
    this->transformation = glm::rotate(this->transformation, glm::radians(angle), axis);
}
