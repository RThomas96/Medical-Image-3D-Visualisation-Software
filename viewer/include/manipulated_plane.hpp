#ifndef VISUALIZATION_VIEWER_INCLUDE_MANIPULATED_PLANE_HPP_
#define VISUALIZATION_VIEWER_INCLUDE_MANIPULATED_PLANE_HPP_

#include <QGLViewer/qglviewer.h>
#include <QGLViewer/mouseGrabber.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QMouseEvent>

class ManipulatedPlane {
	public:
		ManipulatedPlane(void);
		~ManipulatedPlane(void);
	public:
		void draw(void);
		void drawNormal(void);
		void setPosition(void);
		void setNormal(void);
	protected:
		/// @b The frame that represents the position and orientation of the plane
		qglviewer::ManipulatedFrame frame;
};

#endif // VISUALIZATION_VIEWER_INCLUDE_MANIPULATED_PLANE_HPP_
