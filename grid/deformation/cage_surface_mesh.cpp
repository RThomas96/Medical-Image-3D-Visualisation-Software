#include "cage_surface_mesh.hpp"

void toBasicPoint(const std::vector<glm::vec3>& points, std::vector<BasicPoint>& res) {
    res.clear();
    for(int i = 0; i < points.size(); ++i)
        res.push_back(BasicPoint(points[i][0], points[i][1], points[i][2]));
}

BasicPoint toBasicPoint(const glm::vec3& point) {
    return BasicPoint(point[0], point[1], point[2]);
}

void toRawTriangleFormat(const std::vector<Triangle>& triangles, std::vector<std::vector<int>>& res) {
    res.clear();
    for(int i = 0; i < triangles.size(); ++i) {
        const Triangle& triangle = triangles[i];
        const int v1 = triangle[0];
        const int v2 = triangle[1];
        const int v3 = triangle[2];
        res.push_back(std::vector<int>{v1, v2, v3});
    }
}

void Cage::applyCage(const std::vector<glm::vec3>& cage) {
    for(int i = 0; i < cage.size(); ++i) {
        if(i < this->vertices.size())
            this->vertices[i] = cage[i];
    }
    // Artificially move a point to update the vertices position of the mesh to deform
    this->movePoint(this->vertices[0], this->vertices[0]);
}

void CageMVC::reInitialize() {
    this->originalVertices = this->meshToDeform->vertices;
    this->computeNormals();
    this->MVCCoordinates.clear();
    this->computeCoordinates();
}

void CageMVC::movePoint(const glm::vec3& origin, const glm::vec3& target) {
    BaseMesh::movePoint(origin, target);// Update the cage positions
    // Update positions of the mesh to deform
    for( unsigned int v = 0 ; v < this->meshToDeform->getNbVertices() ; ++v ) {
        glm::vec3 pos(0,0,0);
        for( unsigned int vc = 0 ; vc < this->MVCCoordinates[v].size() ; ++vc )
            pos += this->MVCCoordinates[v][vc].second * this->getVertice(this->MVCCoordinates[v][vc].first);
        this->meshToDeform->vertices[v] = pos;
    }
    this->meshToDeform->computeNormals();
    this->meshToDeform->updatebbox();
}

void CageMVC::movePoints(const std::vector<glm::vec3>& origins, const std::vector<glm::vec3>& targets) {
    BaseMesh::movePoints(origins, targets);
    this->movePoint(this->vertices[0], this->vertices[0]);// Artificially move a point to move the deformed mesh
}

void CageMVC::computeCoordinates() {
    MVCCoordinates.clear();
    MVCCoordinates.resize(this->meshToDeform->getNbVertices());

    std::vector<std::vector<int>> trianglesRawFormat;
    std::vector<BasicPoint> verticesRawFormat;
    std::vector<BasicPoint> normalsRawFormat;

    toRawTriangleFormat(this->triangles, trianglesRawFormat);
    toBasicPoint(this->vertices, verticesRawFormat);
    toBasicPoint(this->normals, normalsRawFormat);

    for( unsigned int p_idx = 0 ; p_idx < this->meshToDeform->getNbVertices(); ++p_idx )
    {
        MVCCoords::computeMVCCoordinatesOf3dPoint(
                toBasicPoint(this->originalVertices[p_idx]),
                trianglesRawFormat,
                verticesRawFormat,
                normalsRawFormat,
                this->MVCCoordinates[p_idx]);
    }
}

void CageGreen::reInitialize() {
    this->originalVertices.clear();
    this->originalVertices = this->meshToDeform->vertices;
    this->computeNormals();

    this->initial_cage_vertices.clear();
    this->initial_cage_triangle_normals.clear();
    this->initial_cage_vertices = this->vertices;
    this->initial_cage_triangle_normals = this->normals;

    this->phiCoordinates.clear();
    this->psiCoordinates.clear();
    this->scalingFactors.clear();

    for(int i = 0; i < this->triangles.size(); ++i) {
        const Triangle& triangle = this->triangles[i];
        const int v1 = triangle[0];
        const int v2 = triangle[1];
        const int v3 = triangle[2];
        scalingFactors.push_back(GreenCoords::GreenScalingFactor<BasicPoint>(toBasicPoint(this->vertices[v2] - this->vertices[v1]) ,toBasicPoint(this->vertices[v3] - this->vertices[v1])));
    }
    this->computeCoordinates();
}

void CageGreen::movePoint(const glm::vec3& origin, const glm::vec3& target) {
    BaseMesh::movePoint(origin, target);// Update the cage positions
    // Update positions of the mesh to deform
    this->computeNormals();
    this->update_cage_triangle_scalingFactors();
    for(unsigned int v = 0; v < this->meshToDeform->getNbVertices(); ++v) {
        glm::vec3 pos(0,0,0);
        for(unsigned int vc = 0; vc < this->getNbVertices(); ++vc)
            pos += static_cast<float>(phiCoordinates[v][vc]) * this->vertices[vc];
        for(unsigned int tc = 0; tc < this->triangles.size(); ++tc)
            pos += static_cast<float>(psiCoordinates[v][tc]) * static_cast<float>(scalingFactors[tc].scalingFactor()) * glm::normalize(this->normals[tc]);

        this->meshToDeform->vertices[v] = pos;
    }
    this->meshToDeform->computeNormals();
    this->meshToDeform->updatebbox();
}

void CageGreen::movePoints(const std::vector<glm::vec3>& origins, const std::vector<glm::vec3>& targets) {
    BaseMesh::movePoints(origins, targets);
    this->movePoint(this->vertices[0], this->vertices[0]);// Artificially move a point to move the deformed mesh
}

void CageGreen::update_cage_triangle_scalingFactors() {
    for(unsigned int t = 0; t < this->triangles.size(); ++t) {
        this->scalingFactors[t].computeScalingFactor(toBasicPoint(this->vertices[this->triangles[t].getVertex(1)] - this->vertices[this->triangles[t].getVertex(0)]),
                toBasicPoint(this->vertices[this->triangles[t].getVertex(2)] - this->vertices[this->triangles[t].getVertex(0)]));
    }
}

void CageGreen::computeCoordinates() {
    if(this->meshToDeform->getNbVertices() == 0)
        return ;
    this->phiCoordinates.clear();
    this->psiCoordinates.clear();
    this->phiCoordinates.resize(this->meshToDeform->getNbVertices());
    this->psiCoordinates.resize(this->meshToDeform->getNbVertices());

    for (unsigned int i =0; i < this->meshToDeform->getNbVertices(); i++) {
        this->phiCoordinates[i].clear();
        this->psiCoordinates[i].clear();
        this->phiCoordinates[i].resize(this->getNbVertices(), 0.f);
        this->psiCoordinates[i].resize(this->getNbVertices(), 0.f);
    }

    std::vector<std::vector<int>> trianglesRawFormat;
    std::vector<BasicPoint> verticesRawFormat;
    std::vector<BasicPoint> normalsRawFormat;

    toRawTriangleFormat(this->triangles, trianglesRawFormat);
    toBasicPoint(this->vertices, verticesRawFormat);
    toBasicPoint(this->normals, normalsRawFormat);

    for( unsigned int p_idx = 0 ; p_idx < this->meshToDeform->getNbVertices(); ++p_idx )
    {
        URAGO::computeCoordinatesOf3dPoint(
                toBasicPoint(this->originalVertices[p_idx]),
                trianglesRawFormat,
                verticesRawFormat,
                normalsRawFormat,
                this->phiCoordinates[p_idx],
                this->psiCoordinates[p_idx]);
    }
}
