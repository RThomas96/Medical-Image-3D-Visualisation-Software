#ifndef VISUALISATION_MESHES_DRAWABLE_CURVE_HPP_
#define VISUALISATION_MESHES_DRAWABLE_CURVE_HPP_

#include "../deformable_curve/curve.hpp"
#include "./base.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

class DrawableCurve : public DrawableBase {
public:
	typedef std::shared_ptr<DrawableCurve> Ptr;
	/// @brief Default ctor for the class, initializing the pointer to the mesh and the GL names.
	DrawableCurve(Curve::Ptr& _mesh);
	/// @brief Default dtor for the mesh, default-defined.
	~DrawableCurve() = default;

	/// @brief Initializes the mesh, creates the program and uploads the vertex data to the scene.
	virtual void initialize(QOpenGLContext* _context, ShaderCompiler::GLFunctions* functions) override;

	/// @brief Draws the mesh to the screen.
	virtual void draw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) override;

	/// @brief Draws the mesh when the camera is moving.
	/// @note For now, calls draw().
	virtual void fastDraw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) override;

	Curve::Ptr getCurve() const { return this->curve; }

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

	Curve::Ptr curve;
};

#endif // VISUALISATION_MESHES_DRAWABLE_CURVE_HPP_
