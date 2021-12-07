#ifndef VISUALISATION_MESHES_DRAWABLE_BASE_HPP_
#define VISUALISATION_MESHES_DRAWABLE_BASE_HPP_

#include "./shaders.hpp"

#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_2_Compatibility>

#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>

#include <vector>
#include <memory>
#include <iostream>

/// @brief The DrawableBase class represents a base structure for all drawable objects in the program.
/// @details It provides virtual functions for initializing an object, and drawing it one of two ways:
/// either a 'complete' draw or a 'fast' draw.
class DrawableBase {
	using Ptr = std::shared_ptr<DrawableBase>;
protected:
	DrawableBase() : bound_context(nullptr), gl(nullptr),
		should_update_on_next_draw(false),
		bbmin(), bbmax() {
		glm::vec3::value_type min = std::numeric_limits<glm::vec3::value_type>::lowest();
		glm::vec3::value_type max = std::numeric_limits<glm::vec3::value_type>::max();
		this->bbmin = glm::vec3{max, max, max};
		this->bbmax = glm::vec3{min, min, min};
	}
public:
	~DrawableBase() = default;

	/// @brief Checks if the object has been initialized
	virtual bool isInitialized() const {
		return this->bound_context != nullptr && (glm::lessThan(this->bbmin, this->bbmax) == glm::vec3::bool_type{true, true, true});
	}

	/// @brief Initializes the object.
	virtual void initialize(QOpenGLContext* context, ShaderCompiler::GLFunctions* functions) {
		this->bound_context = context;
		this->gl = functions;
	}

	/// @brief Draws the object.
	virtual void draw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) {}

	/// @brief Draws the object (fast version).
	virtual void fastDraw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) {};

	/// @brief Returns the min and max coordinates of an axis-aligned bounding box around the object.
	virtual std::pair<glm::vec3, glm::vec3> getBoundingBox() const { return std::make_pair(this->bbmin, this->bbmax); }

	/// @brief Forces an update on the next draw call.
	virtual void updateOnNextDraw(void) { this->should_update_on_next_draw = true; }

protected:
	/// @brief The context in which the drawable was initialized.
	QOpenGLContext* bound_context;
	/// @brief A pointer to a struct of functors to call OpenGL functions through Qt.
	ShaderCompiler::GLFunctions* gl;

	bool should_update_on_next_draw;

	glm::vec3 bbmin; ///< The min point of the bounding box of the element to draw.
	glm::vec3 bbmax; ///< The max point of the bounding box of the element to draw.
};

#endif // VISUALISATION_MESHES_DRAWABLE_BASE_HPP_
