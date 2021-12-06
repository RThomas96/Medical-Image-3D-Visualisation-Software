#ifndef MESH_H
#define MESH_H

#include "Triangle.h"
#include <glm/glm.hpp>
#include <queue>

class Mesh {

public:
	Mesh():normalDirection(1.){}

	Mesh(std::vector<glm::vec3> & vertices, std::vector<Triangle> & triangles): vertices(vertices), triangles(triangles), normalDirection(1.){
		update();
	}
	~Mesh() = default;

	std::vector<glm::vec3> & getVertices(){return vertices;}
	const std::vector<glm::vec3> & getVertices()const {return vertices;}

	std::vector<Triangle> & getTriangles(){return triangles;}
	const std::vector<Triangle> & getTriangles()const {return triangles;}

	std::vector<glm::vec3> & getNormals(){return normals;}
	const std::vector<glm::vec3> & getNormals()const {return normals;}

	std::vector<glm::vec3> getBB();

	// modify vertices
	void setVertices(unsigned int index, glm::vec3 vertex){vertices[index] = vertex;}

	void draw();
	void draw( std::vector<bool> & selected, std::vector<bool> & fixed);

	void recomputeNormals();
	void update();

	void clear();

	typedef std::priority_queue< std::pair< float , int > , std::deque< std::pair< float , int > > , std::greater< std::pair< float , int > > > FacesQueue;

	void invertNormal(){normalDirection *= -1;}
protected:

	void computeBB();

	void collectOneRing (std::vector<std::vector<unsigned int> > & oneRing) const;

	void computeTriangleNormals();
	glm::vec3 computeTriangleNormal(int t);

	void computeVerticesNormals();

	void glTriangle(unsigned int i);

	void sortFaces( FacesQueue & facesQueue );

	std::vector <glm::vec3> vertices;
	std::vector <Triangle> triangles;

	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> verticesNormals;

	glm::vec3 BBMin;
	glm::vec3 BBMax;
	float radius;

	int normalDirection;
};

#endif // MESH_H
