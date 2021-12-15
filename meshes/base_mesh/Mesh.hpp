#ifndef MESH_H
#define MESH_H

#include "Triangle.h"
#include <glm/glm.hpp>
#include <memory>
#include <nanoflann.hpp>
#include <queue>

template <typename vec_t>
struct MeshVerticesNanoFLANNAdaptor
{
public:
	typedef typename vec_t::value_type vertex_t;	///< In our case, std::vector<glm::vec3<<::value_type == glm::vec3
	typedef typename vertex_t::value_type coord_t;	  ///< In our case, glm::vec3::value_type == float

	/// @brief The reference to the vertices array, a std::vector !
	const vec_t& vertex_array;

	MeshVerticesNanoFLANNAdaptor(const vec_t& _data) :
		vertex_array(_data) {}

	inline const vec_t& derived() const { return this->vertex_array; }

	/// @brief Returns the number of data points
	inline size_t kdtree_get_point_count() const { return derived().size(); }

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline coord_t kdtree_get_pt(const size_t idx, const size_t dim) const {
		if (dim == 0)
			return derived()[idx].x;
		else if (dim == 1)
			return derived()[idx].y;
		else
			return derived()[idx].z;
	}

	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
};

typedef MeshVerticesNanoFLANNAdaptor<std::vector<glm::vec3>> mesh_kdtree_adaptor_t;
typedef nanoflann::KDTreeSingleIndexAdaptor<
  nanoflann::L2_Simple_Adaptor<glm::vec3::value_type, mesh_kdtree_adaptor_t>,
  mesh_kdtree_adaptor_t, 3>
  mesh_kdtree_t;

class Mesh {
public:
	using Ptr = std::shared_ptr<Mesh>;

public:
	Mesh();
	Mesh(std::vector<glm::vec3>& vertices, std::vector<Triangle>& triangles);
	~Mesh() = default;

	std::vector<glm::vec3>& getVertices() { return vertices; }
	const std::vector<glm::vec3>& getVertices() const { return vertices; }

	std::vector<glm::vec3>& getVertexNormals() { return this->verticesNormals; }
	const std::vector<glm::vec3>& getVertexNormals() const { return this->verticesNormals; }

	std::vector<Triangle>& getTriangles() { return triangles; }
	const std::vector<Triangle>& getTriangles() const { return triangles; }

	std::vector<glm::vec3>& getNormals() { return normals; }
	const std::vector<glm::vec3>& getNormals() const { return normals; }

	void applyTransformation(glm::mat4 transformation);

	glm::vec3 closestPointTo(glm::vec3 query, std::size_t& vertex_idx) const;

	std::vector<glm::vec3> getBB();

	// modify vertices
	void setVertices(unsigned int index, glm::vec3 vertex) {
		vertices[index] = vertex;
		this->update();
	}
	void setNewVertexPositions(std::vector<glm::vec3>& new_positions) {
		this->vertices = new_positions;
		this->update();
	}

	void draw();
	void draw(std::vector<bool>& selected, std::vector<bool>& fixed);

	void recomputeNormals();
	void update();
	void updateQuick();

	void clear();

	typedef std::priority_queue<std::pair<float, int>, std::deque<std::pair<float, int>>, std::greater<std::pair<float, int>>> FacesQueue;

	void invertNormal() { normalDirection *= -1; }

protected:
	void computeBB();

	void collectOneRing(std::vector<std::vector<unsigned int>>& oneRing) const;

	void computeTriangleNormals();
	glm::vec3 computeTriangleNormal(int t);

	void computeVerticesNormals();

	void glTriangle(unsigned int i);

	void sortFaces(FacesQueue& facesQueue);

	std::vector<glm::vec3> vertices;
	std::vector<Triangle> triangles;

	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> verticesNormals;

	glm::vec3 BBMin;
	glm::vec3 BBMax;
	float radius;

	int normalDirection;

	/// @brief The kd-tree responsible for fast spatial queries on the mesh
	std::shared_ptr<mesh_kdtree_t> kdtree;
	/// @brief The adaptor between nanoFLANN and the vertices vector.
	std::shared_ptr<mesh_kdtree_adaptor_t> kdtree_adaptor;
};

#endif	  // MESH_H
