#ifndef SURFACE_MESH_HPP_
#define SURFACE_MESH_HPP_

#include "../utils/Triangle.h"
#include "base_mesh.hpp"
#include <iostream>
#include "glm/gtx/string_cast.hpp"
#include "grid/deformation/AsRigidAsPossible.h"
#include "grid/drawable/drawable_surface_mesh.hpp"

//! \addtogroup geometry
//! @{
//!

class AsRigidAsPossible;

class SurfaceMesh : public BaseMesh, public DrawableMesh {

public:
    AsRigidAsPossible * arapDeformer;

    SurfaceMesh(std::string const &filename);

    void computeTriangleNormal();
    void computeVerticesNormal();
    void glTriangle(unsigned int i);

    void loadOBJ(std::string const &filename);
    void loadOFF(std::string const &filename);

    void saveOFF(std::string const & filename);

    //*********/

	std::vector<Triangle> triangles;
	std::vector<glm::vec3> normals;

    void computeNormals() override;
    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos, glm::vec3& res) const override;

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
