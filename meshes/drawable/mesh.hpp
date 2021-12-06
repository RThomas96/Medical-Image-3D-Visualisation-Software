#ifndef VISUALISATION_MESHES_DRAWABLE_MESH_HPP_
#define VISUALISATION_MESHES_DRAWABLE_MESH_HPP_

#include "./base.hpp"
#include "../base_mesh/Mesh.hpp"

#include <QOpenGLFunctions>

#include <vector>
#include <memory>

class DrawableMesh : public DrawableBase {
public:
	DrawableMesh(std::shared_ptr<Mesh> _mesh);
	~DrawableMesh() = default;

	virtual void initialize(QOpenGLContext* _context) override;

	virtual void draw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) override;

	virtual void fastDraw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) override;
protected:
	GLuint program_handle_draw;
	GLuint program_handle_fastdraw;

	GLuint vao;
	GLuint vbo_vertices;
	GLuint vbo_normals;
	GLuint vbo_texture;

	std::shared_ptr<Mesh> mesh;
};

#endif // VISUALISATION_MESHES_DRAWABLE_MESH_HPP_
