#ifndef VISUALISATION_MESHES_DRAWABLE_BASE_HPP_
#define VISUALISATION_MESHES_DRAWABLE_BASE_HPP_

#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

/// @brief The DrawableBase class represents a base structure for all drawable objects in the program.
/// @details It provides virtual functions for initializing an object, and drawing it one of two ways:
/// either a 'complete' draw or a 'fast' draw.
class DrawableBase {
	using Ptr = std::shared_ptr<DrawableBase>;
protected:
	DrawableBase() : bound_context(nullptr) {}
public:
	~DrawableBase() = default;

	/// @brief Checks if the object has been initialized
	virtual bool isInitialized() const { return this->bound_context != nullptr; }

	/// @brief Initializes the object.
	virtual void initialize(QOpenGLContext* context) { this->bound_context = context; }

	/// @brief Draws the object.
	virtual void draw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) = 0;
	/// @brief Draws the object (fast version).
	virtual void fastDraw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) = 0;

protected:
	/// @brief The context in which the drawable was initialized.
	QOpenGLContext* bound_context;
};

#endif // VISUALISATION_MESHES_DRAWABLE_BASE_HPP_
