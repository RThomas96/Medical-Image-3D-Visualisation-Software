#include "base_mesh.hpp"
#include "../deformation/mesh_deformer.hpp"
#include "../ui/manipulator.hpp"
#include <map>
#include <algorithm>
#include <math.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp> 


BaseMesh::BaseMesh(): bbMin(glm::vec3(0., 0., 0.)), bbMax(glm::vec3(0., 0., 0.)), meshDeformer(new NormalMethod(this)), transformation(glm::mat4(1.f)), scale(glm::vec3(1., 1., 1.)) {
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
    if(this->meshDeformer->hasSelectedPts()) {
        this->meshDeformer->movePoint(origin, target);
        this->computeNormals();
        this->updatebbox();
    } else {
        std::cout << "WARNING: try to move points when there is no point in the point to move queue" << std::endl;
    }
}

void BaseMesh::setNormalDeformationMethod() {
    if(this->meshDeformer->deformMethod != DeformMethod::NORMAL) {
        delete this->meshDeformer;
        this->meshDeformer = new NormalMethod(this);
    }
}

void BaseMesh::setWeightedDeformationMethod(float radius) {
    if(this->meshDeformer->deformMethod != DeformMethod::WEIGHTED) {
        delete this->meshDeformer;
        this->meshDeformer = new WeightedMethod(this, radius);
    }
}

void BaseMesh::selectPts(const glm::vec3& pt) {
    this->meshDeformer->selectPts(pt);
}

void BaseMesh::deselectPts(const glm::vec3& pt) {
    this->meshDeformer->deselectPts(pt);
}

void BaseMesh::deselectAllPts() {
    this->meshDeformer->deselectAllPts();
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

void BaseMesh::setOrigin(const glm::vec3& origin, bool modifyPoints) {
    if(modifyPoints) {
        glm::vec3 move = origin / this->scale;
        for(int i = 0; i < this->getNbVertices(); ++i) {
            this->vertices[i] += move;
        }
        this->computeNormals();
        this->updatebbox();
    } else {
        this->transformation[3][0] = origin[0];
        this->transformation[3][1] = origin[1];
        this->transformation[3][2] = origin[2];
    }
}

void BaseMesh::translate(const glm::vec3& vec) {
    this->transformation[3][0] += vec[0];
    this->transformation[3][1] += vec[1];
    this->transformation[3][2] += vec[2];
}

void BaseMesh::setScale(glm::vec3 scale) {
    this->scale = scale;
}

std::vector<glm::vec3> BaseMesh::getWorldMeshPositions() {
    std::vector<glm::vec3> worldPos;
    for(int i = 0; i < this->vertices.size(); ++i) {
        glm::vec4 pt = this->getModelMatrix() * glm::vec4(this->vertices[i], 1.);
        worldPos.push_back(glm::vec3(pt[0], pt[1], pt[2]));
    }
    return worldPos;
}

glm::vec3 BaseMesh::toWorld(const glm::vec3& pt) const {
    return glm::vec3(this->getModelMatrix() * glm::vec4(pt, 1.));
}

glm::vec3 BaseMesh::toModel(const glm::vec3& pt) const {
    return glm::vec3(glm::inverse(this->getModelMatrix()) * glm::vec4(pt, 1.));
}

const glm::vec3& BaseMesh::getVertice(int i) const {
    return this->vertices[i];
}

const glm::vec3& BaseMesh::getVerticeNormal(int i) const {
    return this->verticesNormals[i];
}

const glm::vec3 BaseMesh::getWorldVertice(int i) const {
    return glm::vec3(this->getModelMatrix() * glm::vec4(this->vertices[i], 1.));
}

const glm::vec3 BaseMesh::getWorldVerticeNormal(int i) const {
    return glm::normalize(glm::vec3(this->getModelMatrix() * glm::vec4(this->verticesNormals[i], 1.)));
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

void BaseMesh::rotate(const float angle, const glm::vec3 axis, bool modifyPoints) {
    if(modifyPoints) {
        for(int i = 0; i < this->getNbVertices(); ++i) {
            this->vertices[i] = glm::vec3(glm::rotate(glm::radians(angle), axis) * glm::vec4(this->vertices[i], 1.));
        }
        this->computeNormals();
        this->updatebbox();
    } else {
        this->transformation = glm::rotate(this->transformation, glm::radians(angle), axis);
    }
}

glm::mat4 BaseMesh::getModelMatrix() const {
    glm::mat4 res = this->transformation;
    for(int i = 0; i < 3; ++i)
        res[i][i] *= this->scale[i];
    return res;
}

void BaseMesh::setTransformation(const glm::mat4& transf) {
    this->transformation = transf;
}

void BaseMesh::drawNormals() const {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH);

    float normalSize = 0.02*100. * 10;

    for(int i = 0; i < this->vertices.size(); ++i) {
        glm::vec3 p = this->getWorldVertice(i);
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
