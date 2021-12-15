#ifndef VISUALISATION_MESHES_DRAWABLE_BASE_HPP_
#define VISUALISATION_MESHES_DRAWABLE_BASE_HPP_

#include "./shaders.hpp"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QOpenGLFunctions_3_2_Core>

#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>

#include <iostream>
#include <memory>
#include <vector>

/// @brief The DrawableBase class represents a base structure for all drawable objects in the program.
/// @details It provides virtual functions for initializing an object, and drawing it one of two ways:
/// either a 'complete' draw or a 'fast' draw.
class DrawableBase {
public:
	using Ptr = std::shared_ptr<DrawableBase>;

protected:
	DrawableBase() :
		bound_context(nullptr), gl(nullptr),
		should_update_on_next_draw(false),
		bounding_box_min(), bounding_box_max(), transformation_matrix(1.f) {
		glm::vec3::value_type min = std::numeric_limits<glm::vec3::value_type>::lowest();
		glm::vec3::value_type max = std::numeric_limits<glm::vec3::value_type>::max();
		this->bounding_box_min	  = glm::vec3{max, max, max};
		this->bounding_box_max	  = glm::vec3{min, min, min};
	}

public:
	~DrawableBase() = default;

	/// @brief Checks if the object has been initialized
	[[nodiscard]] virtual bool isInitialized() const {
		return this->bound_context != nullptr && (glm::lessThan(this->bounding_box_min, this->bounding_box_max) == glm::vec3::bool_type{true, true, true});
	}

	/// @brief Initializes the object.
	virtual void initialize(QOpenGLContext* context, ShaderCompiler::GLFunctions* functions) {
		this->bound_context = context;
		this->gl			= functions;
	}

	/// @brief Draws the object.
	virtual void draw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) {}

	/// @brief Draws the object (fast version).
	virtual void fastDraw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera){};

	/// @brief Returns the min and max coordinates of an axis-aligned bounding box around the object.
	[[nodiscard]] virtual std::pair<glm::vec3, glm::vec3> getBoundingBox() const {
		glm::vec4 min_raw = glm::vec4(this->bounding_box_min, 1.f), max_raw = glm::vec4(this->bounding_box_max, 1.f);
		glm::vec4 min = this->transformation_matrix * min_raw;
		glm::vec4 max = this->transformation_matrix * max_raw;
		return std::make_pair(glm::vec3(min), glm::vec3(max));
	}

	/// @brief Forces an update on the next draw call.
	virtual void updateOnNextDraw() { this->should_update_on_next_draw = true; }

	/// @brief Sets the transformation to apply to the mesh
	virtual void setTransformation(glm::mat4 transformation_to_apply) { this->transformation_matrix = transformation_to_apply; }

	/// @brief Retrieves the currently applied transformation
	virtual glm::mat4 getTransformation() { return this->transformation_matrix; }

	virtual void updateBoundingBox() {}

protected:
	/// @brief The context in which the drawable was initialized.
	QOpenGLContext* bound_context;
	/// @brief A pointer to a struct of functors to call OpenGL functions through Qt.
	ShaderCompiler::GLFunctions* gl;

	bool should_update_on_next_draw;

	glm::mat4 transformation_matrix;	// The transformation matrix to apply to the mesh.

	glm::vec3 bounding_box_min;	   ///< The min point of the bounding box of the element to draw.
	glm::vec3 bounding_box_max;	   ///< The max point of the bounding box of the element to draw.
};

#endif	  // VISUALISATION_MESHES_DRAWABLE_BASE_HPP_
