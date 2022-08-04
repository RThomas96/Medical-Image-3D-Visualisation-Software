#include "base_mesh.hpp"
#include "../deformation/mesh_deformer.hpp"
#include "../ui/manipulator.hpp"
#include <map>
#include <algorithm>
#include <math.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp> 
#include <numeric>


BaseMesh::BaseMesh(): bbMin(glm::vec3(0., 0., 0.)), bbMax(glm::vec3(0., 0., 0.)), meshDeformer(new NormalMethod(this)), normalDeformer(new NormalMethod(this)), history(nullptr), coordinate_system({glm::vec3(1., 0., 0.), glm::vec3(0., 1., 0.), glm::vec3(0., 0., 1.)}) {
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
    if(this->useNormal) {
        this->normalDeformer->movePoint(origin, target);
    } else {
        this->meshDeformer->movePoint(origin, target);
    }
    this->computeNormals();
    this->updatebbox();
    //this->history->addStep(this->vertices);
}

void BaseMesh::movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) {
    if(this->useNormal) {
        this->normalDeformer->movePoints(origins, targets);
    } else {
        this->meshDeformer->movePoints(origins, targets);
    }
    this->computeNormals();
    this->updatebbox();
    //this->history->addStep(this->vertices);
}

void BaseMesh::movePoints(const std::vector<glm::vec3>& targets) {
    std::vector<int> indices(this->vertices.size());
    std::iota(std::begin(indices), std::end(indices), 0); // Fill with 0, 1, ..., vertices.size
    this->movePoints(indices, targets);
}

void BaseMesh::setNormalDeformationMethod() {
    if(this->meshDeformer->deformMethod != DeformMethod::NORMAL) {
        delete this->meshDeformer;
        this->meshDeformer = new NormalMethod(this);
    }
}

std::vector<glm::vec3>& BaseMesh::getMeshPositions() {
    return this->vertices;
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

void BaseMesh::drawNormals() const {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH);

    float normalSize = 0.02*100. * 10;

    for(int i = 0; i < this->vertices.size(); ++i) {
        glm::vec3 p = this->getVertice(i);
        glm::vec3 p2 = p + this->getVerticeNormal(i) * normalSize;
        glBegin(GL_LINES);
        glVertex3f(p[0], p[1], p[2]);
        glColor3f(1., 0., 0.);
        glVertex3f(p2[0], p2[1], p2[2]);
        glColor3f(1., 1., 0.);
        glEnd();
    }

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH);
}

void BaseMesh::scaleToBBox(const glm::vec3& bbMin, const glm::vec3& bbMax) {
    float targetLength = glm::length(bbMax - bbMin);
    float currentLength = glm::length(this->bbMax - this->bbMin);
    float scaleFactor = targetLength / currentLength;
    this->scale(glm::vec3(scaleFactor, scaleFactor, scaleFactor));
}

bool BaseMesh::isInBBox(const glm::vec3& p) const {
    return (p.x > this->bbMin.x && p.y > this->bbMin.y && p.z > this->bbMin.z &&
            p.x < this->bbMax.x && p.y < this->bbMax.y && p.z < this->bbMax.z);
}

void BaseMesh::addStateToHistory(bool useTimer) {
    this->history->addStep(this->vertices, useTimer);
}
