#ifndef SURFACE_MESH_HPP_
#define SURFACE_MESH_HPP_

#include "../utils/Triangle.h"
#include "base_mesh.hpp"
#include <iostream>
#include "glm/gtx/string_cast.hpp"

//! \addtogroup geometry
//! @{

class SurfaceMesh : public BaseMesh {

public:
	SurfaceMesh(const std::vector<glm::vec3>& vertices, const std::vector<Triangle>& triangles);
    SurfaceMesh(std::string const &filename);

    void computeTriangleNormal();
    void computeVerticesNormal();
    void glTriangle(unsigned int i);
    void draw();

    void loadOBJ(std::string const &filename);
    void loadOFF(std::string const &filename);

    //*********/

	std::vector<Triangle> triangles;
	std::vector<glm::vec3> normals;

    void setARAPDeformationMethod() override;
    void computeNeighborhood() override;
    void computeNormals() override;
    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos, glm::vec3& res) const override;

    ~SurfaceMesh();

	std::vector<glm::vec3>& getVertices() { return vertices; }
	const std::vector<glm::vec3>& getVertices() const { return vertices; }

	std::vector<glm::vec3>& getVertexNormals() { return this->verticesNormals; }
	const std::vector<glm::vec3>& getVertexNormals() const { return this->verticesNormals; }

	std::vector<Triangle>& getTriangles() { return triangles; }
	const std::vector<Triangle>& getTriangles() const { return triangles; }

	std::vector<glm::vec3>& getNormals() { return normals; }
	const std::vector<glm::vec3>& getNormals() const { return normals; }
};

//! @}
#endif
