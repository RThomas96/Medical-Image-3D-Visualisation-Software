#ifndef VISUALIZATION_VIEWER_INCLUDE_MANIPULATED_PLANE_HPP_
#define VISUALIZATION_VIEWER_INCLUDE_MANIPULATED_PLANE_HPP_

#include <QGLViewer/qglviewer.h>
#include <QGLViewer/mouseGrabber.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QMouseEvent>

#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_0_Core>

#include <glm/glm.hpp>

class ManipulatedPlane : public qglviewer::MouseGrabber {
	protected:
		ManipulatedPlane(void);
	public:
		/// @b Create a manipulated plane from a normal and a distance from the origin.
		static std::shared_ptr<ManipulatedPlane> createPlane(glm::vec3 normal, float distance);
		/// @b Create a manipulated plane from a plane equation.
		static std::shared_ptr<ManipulatedPlane> createPlane(glm::vec4 plane_equation);
		/// @b Default dtor for a manipulated plane.
		~ManipulatedPlane(void);

	public: // Getters, setters

	public:
		/// @b Inverts the plane's orientation
		void invertPlane(void);

	protected:
		/// @b The plane equation
		glm::vec4 planeEq;
		/// @b The plane's position, for drawing the fresnel guides
		glm::vec4 planePosition;
		/// @b The min point of the "viewport" on this plane
		glm::vec2 viewportMin;
		/// @b The max point of the "viewport" on this plane
		glm::vec2 viewportMax;
};

#endif // VISUALIZATION_VIEWER_INCLUDE_MANIPULATED_PLANE_HPP_
