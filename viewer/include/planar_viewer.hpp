#ifndef VISUALISATION_VIEWER_INCLUDE_PLANAR_VIEWER_HPP_
#define VISUALISATION_VIEWER_INCLUDE_PLANAR_VIEWER_HPP_

#include "../../features.hpp"
#include "./scene.hpp"
#include "../../qt/include/viewer_header.hpp"

#include <QGLViewer/qglviewer.h>
#include <QTimer>

#include <memory>

class PlanarViewer : public QGLViewer {
	protected:
		friend class ViewerHeader;
	public:
		/// @brief Default constructor for the viewer.
		PlanarViewer(Scene* const _scene, planes _p, planeHeading _h = North, QWidget* parent = nullptr);
		/// @brief Default destructor for the viewer.
		~PlanarViewer(void);
	protected:
		/// @brief Initializes the scene, and the viewer's variables.
		virtual void init(void) override;
		/// @brief Draws the plane the viewer is supposed to show.
		virtual void draw(void) override;
		/// @brief Handles key events from the user.
		virtual void keyPressEvent(QKeyEvent* _e) override;
		/// @brief Handles mouse events from the user.
		virtual void mousePressEvent(QMouseEvent* _m) override;
		/// @brief Overrides the mouse wheel events from the user.
		virtual void wheelEvent(QWheelEvent* _w) override;
	protected:
		/// @b Sets the widget in charge of controlling the viewer
		void setController(ViewerHeader* _header);
	protected:
		Scene* sceneToShow; ///< The scene to draw.
		planes planeToShow; ///< The plane to show.
		planeHeading planeOrientation; ///< This plane's orientation
		QTimer* refreshTimer; ///< Triggers a scene reload
		ViewerHeader* viewerController; ///< The widget that controls this widget
		float minZoomRatio; ///< minimum value of the zoom applied to the image
		float maxZoomRatio; ///< maximum value of the zoom applied to the image
		float zoomRatio; ///< The current zoom level applied to the image
	public slots:
		/// @brief Update the view, as a slot without any arguments
		void updateView(void);
		/// @b Signal from a slider to update the value of the cutting plane.
		void updatePlaneDepth(int newVal);
		/// @b Signal from a push button to flip the plane's cutting direction.
		void flipPlaneDirection(void);
		/// @b Rotates a plane in a clockwise fashion.
		void rotatePlaneClockwise(void);
		/// @b Rotates a plane in a counter-clockwise fashion.
		void rotatePlaneCounterClockwise(void);
		/// @b Toggles the corresponding plane's visibility in the scene
		void togglePlaneVisibility(void);
};

#endif // VISUALISATION_VIEWER_INCLUDE_PLANAR_VIEWER_HPP_
