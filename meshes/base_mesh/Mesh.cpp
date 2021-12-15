#include "Mesh.hpp"
#include <QOpenGLFunctions>
#include <algorithm>
#include <cfloat>
#include <memory>

Mesh::Mesh() :
	vertices(), normals(), verticesNormals(), triangles(),
	BBMin(), BBMax(), radius(1.f), normalDirection(1.),
	kdtree(nullptr), kdtree_adaptor(nullptr) {
	glm::vec3::value_type min = std::numeric_limits<glm::vec3::value_type>::lowest();
	glm::vec3::value_type max = std::numeric_limits<glm::vec3::value_type>::max();

	this->BBMin = glm::vec3(max, max, max);
	this->BBMax = glm::vec3(min, min, min);
}

Mesh::Mesh(std::vector<glm::vec3>& _vertex, std::vector<Triangle>& _tris) :
	vertices(_vertex), normals(), verticesNormals(), triangles(_tris),
	BBMin(), BBMax(), radius(1.f), normalDirection(1.),
	kdtree(nullptr), kdtree_adaptor(nullptr) {
	glm::vec3::value_type min = std::numeric_limits<glm::vec3::value_type>::lowest();
	glm::vec3::value_type max = std::numeric_limits<glm::vec3::value_type>::max();

	this->BBMin = glm::vec3(max, max, max);
	this->BBMax = glm::vec3(min, min, min);

	this->update();
}

void Mesh::computeBB() {
	BBMin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	BBMax = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const auto& point : this->vertices) {
		for (int v = 0; v < 3; v++) {
			float value = point[v];
			if (BBMin[v] > value)
				BBMin[v] = value;
			if (BBMax[v] < value)
				BBMax[v] = value;
		}
	}

	radius = glm::length(BBMax - BBMin);
}

std::vector<glm::vec3> Mesh::getBB() {
	computeBB();

	std::vector<glm::vec3> bb;
	bb.push_back(BBMin);
	bb.push_back(BBMax);

	return bb;
}

void Mesh::applyTransformation(glm::mat4 transformation) {
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		glm::vec3 vertex  = this->vertices[i];
		this->vertices[i] = glm::vec3(transformation * (glm::vec4(vertex, 1.f)));
	}

	this->update();
}

void Mesh::updateQuick() {
	computeBB();
	recomputeNormals();
}

void Mesh::update() {
	// first, update BB :
	this->updateQuick();

	// then update kdtree :
	if (this->kdtree_adaptor == nullptr) {
		this->kdtree_adaptor = std::make_shared<mesh_kdtree_adaptor_t>(this->vertices);
	}
	if (this->kdtree == nullptr) {
		this->kdtree = std::make_shared<mesh_kdtree_t>(3, *this->kdtree_adaptor.get(), 10);
	}
	// kdtree should be initialized now :
	this->kdtree->buildIndex();
}

void Mesh::clear() {
	vertices.clear();

	triangles.clear();

	normals.clear();
	verticesNormals.clear();
}

glm::vec3 Mesh::closestPointTo(glm::vec3 query, std::size_t& vertex_idx) const {
	// WARNING : This is ripped straight from one of nanoflann's examples [1]. Should be fine to use it legally.
	// [1] : https://github.com/jlblancoc/nanoflann/blob/master/examples/pointcloud_adaptor_example.cpp#L131
	// do a knn search
	const size_t num_results = 1;
	size_t ret_index;
	float out_dist_sqr;
	nanoflann::KNNResultSet<float> resultSet(num_results);
	resultSet.init(&ret_index, &out_dist_sqr);
	this->kdtree->findNeighbors(resultSet, &query[0], nanoflann::SearchParams(10));
	// return the index of the closes vertex
	vertex_idx = ret_index;
	return this->vertices[ret_index];
}

void Mesh::recomputeNormals() {
	computeTriangleNormals();
	computeVerticesNormals();
}

void Mesh::computeTriangleNormals() {
	normals.clear();

	for (unsigned int i = 0; i < triangles.size(); i++) {
		normals.push_back(computeTriangleNormal(i));
	}
}

glm::vec3 Mesh::computeTriangleNormal(int id) {
	const Triangle& t = triangles[id];
	glm::vec3 normal  = glm::cross(vertices[t.getVertex(1)] - vertices[t.getVertex(0)], vertices[t.getVertex(2)] - vertices[t.getVertex(0)]);
	normal			  = glm::normalize(normal);
	return normal;
}

void Mesh::computeVerticesNormals() {
	verticesNormals.clear();
	verticesNormals.resize(vertices.size(), glm::vec3(0., 0., 0.));

	for (unsigned int t = 0; t < triangles.size(); ++t)
	{
		glm::vec3 const& tri_normal = normals[t];

		verticesNormals[triangles[t].getVertex(0)] += tri_normal;
		verticesNormals[triangles[t].getVertex(1)] += tri_normal;
		verticesNormals[triangles[t].getVertex(2)] += tri_normal;
	}

	for (unsigned int v = 0; v < verticesNormals.size(); ++v)
	{
		verticesNormals[v] = glm::normalize(verticesNormals[v]);
	}
}

void Mesh::collectOneRing(std::vector<std::vector<unsigned int>>& oneRing) const {
	oneRing.resize(vertices.size());
	for (unsigned int i = 0; i < triangles.size(); i++) {
		const Triangle& ti = triangles[i];
		for (unsigned int j = 0; j < 3; j++) {
			unsigned int vj = ti.getVertex(j);
			for (unsigned int k = 1; k < 3; k++) {
				unsigned int vk = ti.getVertex((j + k) % 3);
				if (std::find(oneRing[vj].begin(), oneRing[vj].end(), vk) == oneRing[vj].end())
					oneRing[vj].push_back(vk);
			}
		}
	}
}

void Mesh::glTriangle(unsigned int i) {
	const Triangle& t = triangles[i];
	for (int j = 0; j < 3; j++) {
		glm::vec3 n = verticesNormals[t.getVertex(j)] * static_cast<glm::vec3::value_type>(normalDirection);
		glm::vec3 v = vertices[t.getVertex(j)];

		glNormal3f(n.x, n.y, n.z);
		glVertex3f(v.x, v.y, v.z);
	}
}

void Mesh::sortFaces(FacesQueue& facesQueue) {
	float modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

	for (unsigned int t = 0; t < triangles.size(); ++t)
	{
		glm::vec3 _center = (vertices[triangles[t].getVertex(0)] +
							  vertices[triangles[t].getVertex(1)] +
							  vertices[triangles[t].getVertex(2)]) /
							3.f;
		facesQueue.push(std::make_pair(modelview[2] * _center[0] + modelview[6] * _center[1] + modelview[10] * _center[2] + modelview[14], t));
	}
}

void Mesh::draw(std::vector<bool>& selected, std::vector<bool>& fixed) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH);

	glBegin(GL_TRIANGLES);

	for (unsigned int i = 0; i < triangles.size(); i++)
	{
		unsigned int vi[3] = {triangles[i].getVertex(0), triangles[i].getVertex(1), triangles[i].getVertex(2)};
		if (selected[vi[0]] && selected[vi[1]] && selected[vi[2]])
			glColor3f(0.8, 0., 0.);
		if (fixed[vi[0]] && fixed[vi[1]] && fixed[vi[2]])
			glColor3f(0., 0.8, 0.);
		else
			glColor3f(0.37, 0.55, 0.82);
		glTriangle(i);
	}

	glEnd();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH);
}

void Mesh::draw() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH);

	glBegin(GL_TRIANGLES);

	for (unsigned int i = 0; i < triangles.size(); i++) {
		glTriangle(i);
	}

	glEnd();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH);
}
