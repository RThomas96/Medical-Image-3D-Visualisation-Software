#ifndef VISUALISATION_MESHES_DRAWABLE_BASE_HPP_
#define VISUALISATION_MESHES_DRAWABLE_BASE_HPP_

#include "./shaders.hpp"

#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_2_Compatibility>

#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <iostream>

/// @brief The DrawableBase class represents a base structure for all drawable objects in the program.
/// @details It provides virtual functions for initializing an object, and drawing it one of two ways:
/// either a 'complete' draw or a 'fast' draw.
class DrawableBase {
	using Ptr = std::shared_ptr<DrawableBase>;
protected:
	DrawableBase() : bound_context(nullptr), gl(nullptr) {}
public:
	~DrawableBase() = default;

	/// @brief Checks if the object has been initialized
	virtual bool isInitialized() const { return this->bound_context != nullptr; }

	/// @brief Initializes the object.
	virtual void initialize(QOpenGLContext* context, ShaderCompiler::GLFunctions* functions) {
		this->bound_context = context;
		this->gl = functions;
	}

	/// @brief Draws the object.
	virtual void draw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) {}

	/// @brief Draws the object (fast version).
	virtual void fastDraw(GLfloat* proj_mat, GLfloat* view_mat, glm::vec4 camera) {};

protected:
	/// @brief The context in which the drawable was initialized.
	QOpenGLContext* bound_context;
	/// @brief A pointer to a struct of functors to call OpenGL functions through Qt.
	ShaderCompiler::GLFunctions* gl;
};

#endif // VISUALISATION_MESHES_DRAWABLE_BASE_HPP_
