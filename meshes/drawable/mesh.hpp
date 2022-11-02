#ifndef VISUALISATION_MESHES_DRAWABLE_MESH_HPP_
#define VISUALISATION_MESHES_DRAWABLE_MESH_HPP_

#include "../base_mesh/Mesh.hpp"
#include "./base.hpp"

#include <memory>
#include <vector>

/// @brief The DrawableMesh class represents the necessary structure around a Mesh object that enables to draw it using OpenGL.
/// @details It uses the ShaderCompiler class in order to compile and link shaders, and it uses the 'QOpenGLFunctions_X_Y_Z'
/// classes from Qt providing access to OpenGL functions (where X = major version, Y = minor version, Z = Core/Compatibility).
class DrawableMesh : public DrawableBase {
public:
	typedef std::shared_ptr<DrawableMesh> Ptr;
	/// @brief Default ctor for the class, initializing the pointer to the mesh and the GL names.
	DrawableMesh(Mesh::Ptr& _mesh);
	/// @brief Default dtor for the mesh, default-defined.
	virtual ~DrawableMesh();

	/// @brief Initializes the mesh, creates the program and uploads the vertex data to the scene.
	virtual void initialize(QOpenGLContext* _context, ShaderCompiler::GLFunctions* functions) override;

	/// @brief Draws the mesh to the screen.
	virtual void draw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) override;

	/// @brief Draws the mesh when the camera is moving.
	/// @note For now, calls draw().
	virtual void fastDraw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) override;

	/// @brief Returns a reference to the mesh pointer.
	virtual Mesh::Ptr& getMesh() { return this->mesh; }

	virtual void updateBoundingBox() override;

	void setIvoryColor(bool _sbI);

protected:
	/// @brief Create the VAO, the VBOs and upload data
	void makeVAO(void);

	/// @brief Re-samples the data in the mesh and uploads it to the GL.
	void updateData(void);

protected:
	GLuint program_handle_draw;		///< The program name used in the regular drawing method.
	GLuint program_handle_fastdraw;	///< The program name used in the 'fast' drawing method.

	GLuint vao;				///< The VAO name to use in order to draw the mesh.
	GLuint vbo_vertices;	///< The VBO name for the vertex data.
	GLuint vbo_normals;		///< The VBO name for the normal data.
	GLuint vbo_texture;		///< The VBO name for the texture data.
	GLuint vbo_indices;		///< The VBO for the draw order.

	bool color_Ivory; ///< Should the mesh be drawn in an ivory color (patient-specific) or a blue color (reference) ?

	Mesh::Ptr mesh;	   ///< The mesh to render.
};

#endif	  // VISUALISATION_MESHES_DRAWABLE_MESH_HPP_
