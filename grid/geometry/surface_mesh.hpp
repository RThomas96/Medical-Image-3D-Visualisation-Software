#ifndef SURFACE_MESH_HPP_
#define SURFACE_MESH_HPP_

#include "base_mesh.hpp"
#include <iostream>

class Triangle2 {
public:
	inline Triangle2() {}
	inline Triangle2(unsigned int v0, unsigned int v1, unsigned int v2) { init(v0, v1, v2); }
	inline Triangle2(unsigned int* vp) { init(vp[0], vp[1], vp[2]); }
	inline Triangle2(const Triangle2& it) { init(it.v[0], it.v[1], it.v[2]); }
	inline virtual ~Triangle2() {}
	inline Triangle2& operator=(const Triangle2& it) {
		init(it.v[0], it.v[1], it.v[2]);
		return (*this);
	}
	inline bool operator==(const Triangle2& t) const { return (v[0] == t.v[0] && v[1] == t.v[1] && v[2] == t.v[2]); }

	inline unsigned int getVertex(unsigned int i) const { return v[i]; }
	inline void setVertex(unsigned int i, unsigned int vertex) { v[i] = vertex; }
	inline bool contains(unsigned int vertex) const { return (v[0] == vertex || v[1] == vertex || v[2] == vertex); }

	float operator[](unsigned int c) const {
		return v[c];
	}

protected:
	inline void init(unsigned int v0, unsigned int v1, unsigned int v2) {
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
	}

private:
	unsigned int v[3];
};

extern std::ostream& operator<<(std::ostream& output, const Triangle2& t);

class SurfaceMesh : public BaseMesh {

public:
	SurfaceMesh(const std::vector<glm::vec3>& vertices, const std::vector<Triangle2>& triangles);
    SurfaceMesh(std::string const &filename);

    void computeTriangleNormal();
    void computeVerticesNormal();
    void glTriangle(unsigned int i);
    void draw();

    void loadOBJ(std::string const &filename);
    void loadOFF(std::string const &filename);

    //*********/

	std::vector<Triangle2> triangles;
	std::vector<glm::vec3> normals;

    void computeNeighborhood() override;
    void computeNormals() override;
    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, glm::vec3& res) const override;

    ~SurfaceMesh();

	std::vector<glm::vec3>& getVertices() { return vertices; }
	const std::vector<glm::vec3>& getVertices() const { return vertices; }

	std::vector<glm::vec3>& getVertexNormals() { return this->verticesNormals; }
	const std::vector<glm::vec3>& getVertexNormals() const { return this->verticesNormals; }

	std::vector<Triangle2>& getTriangles() { return triangles; }
	const std::vector<Triangle2>& getTriangles() const { return triangles; }

	std::vector<glm::vec3>& getNormals() { return normals; }
	const std::vector<glm::vec3>& getNormals() const { return normals; }
};

#endif
