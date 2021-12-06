#include "./mesh.hpp"

DrawableMesh::DrawableMesh(std::shared_ptr<Mesh> _mesh) : mesh(_mesh), DrawableBase() {
	this->vao = 0;
	this->vbo_vertices = 0;
	this->vbo_normals = 0;
	this->vbo_texture = 0;
}

void DrawableMesh::draw(GLfloat *proj_mat, GLfloat *view_mat, glm::vec4 camera) {
	//
}