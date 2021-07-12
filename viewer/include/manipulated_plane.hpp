#ifndef VISUALIZATION_VIEWER_INCLUDE_MANIPULATED_PLANE_HPP_
#define VISUALIZATION_VIEWER_INCLUDE_MANIPULATED_PLANE_HPP_

#include <QGLViewer/qglviewer.h>
#include <QGLViewer/mouseGrabber.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QMouseEvent>

#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_0_Core>

#include <glm/glm.hpp>

/// @ingroup graphpipe
/// @brief Supposed to be a manipulable plane in 3D using QGLViewer.
/// @note Not tested in any way, shape or form.
class ManipulatedPlane : public qglviewer::MouseGrabber {
	protected:
		ManipulatedPlane(void);
	public:
		/// @brief Create a manipulated plane from a normal and a distance from the origin.
		static std::shared_ptr<ManipulatedPlane> createPlane(glm::vec3 normal, float distance);
		/// @brief Create a manipulated plane from a plane equation.
		static std::shared_ptr<ManipulatedPlane> createPlane(glm::vec4 plane_equation);
		/// @brief Default dtor for a manipulated plane.
		~ManipulatedPlane(void);

	public: // Getters, setters

	public:
		/// @brief Inverts the plane's orientation
		void invertPlane(void);

	protected:
		/// @brief The plane equation
		glm::vec4 planeEq;
		/// @brief The plane's position, for drawing the fresnel guides
		glm::vec4 planePosition;
		/// @brief The min point of the "viewport" on this plane
		glm::vec2 viewportMin;
		/// @brief The max point of the "viewport" on this plane
		glm::vec2 viewportMax;
};

#endif // VISUALIZATION_VIEWER_INCLUDE_MANIPULATED_PLANE_HPP_
