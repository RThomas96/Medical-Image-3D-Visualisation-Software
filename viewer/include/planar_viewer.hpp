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
		/// @b Links with the parent widget, to have access to the statusbar :)
		void addParentStatusBar(QStatusBar* main);
	protected:
		/// @brief Initializes the scene, and the viewer's variables.
		virtual void init(void) override;
		/// @brief Draws the plane the viewer is supposed to show.
		virtual void draw(void) override;
		/// @brief Handles key events from the user.
		virtual void keyPressEvent(QKeyEvent* _e) override;
		/// @brief Handles mouse events from the user.
		virtual void mousePressEvent(QMouseEvent* _m) override;
		/// @b Event raised when the mouse moves
		virtual void mouseMoveEvent(QMouseEvent* _m) override;
		/// @b Event raised when the mouse is released
		virtual void mouseReleaseEvent(QMouseEvent* _m) override;
		/// @brief Overrides the mouse wheel events from the user.
		virtual void wheelEvent(QWheelEvent* _w) override;
		/// @b Overrides the function to resize the widget.
		virtual void resizeGL(int w, int h) override;
		/// @b Defines the 'Help'/'About' string defined for this viewer.
		virtual QString helpString(void) const override;
		/// @b Defines the 'Help'/'About' string for the keyboard for this viewer.
		virtual QString keyboardString(void) const override;
		/// @b Defines the 'Help'/'About' string for the mouse for this viewer.
		virtual QString mouseString(void) const override;
		/// @b Guess the scene position from the fragment position after rendering
		virtual void guessScenePosition(void);
	protected:
		/// @b Sets the widget in charge of controlling the viewer
		void setController(ViewerHeader* _header);
	protected:
		Scene* sceneToShow;				///< The scene to draw.
		planes planeToShow;				///< The plane to show.
		QTimer* refreshTimer;			///< Triggers a scene reload
		planeHeading planeOrientation;	///< This plane's orientation
		ViewerHeader* viewerController;	///< The widget that controls this widget
		QStatusBar* statusbar;			///< The main statusbar !

		glm::ivec2 posRequest;			///< A texture position request for the render
		GLuint renderTarget;			///< The texture to render to for additional info
		float minZoomRatio;				///< minimum value of the zoom applied to the image
		float maxZoomRatio;				///< maximum value of the zoom applied to the image
		float zoomRatio;				///< The current zoom level applied to the image
		float planeDepth;				///< The normalized plane depth, set from the viewer's header slider
		glm::vec2 offset;				///< Normalized offset from the window's origin
		glm::vec2 tempOffset;			///< Temporary offset applied to the rendered image while moving around
		std::size_t mouse_isPressed;	///< Is the mouse pressed ? Counted in number of frames
		bool ctrl_pressed;				///< Is the Ctrl keymod pressed ?
		QPoint cursorPosition_last;		///< Last known position, relative to window coordinates
		QPoint cursorPosition_current;	///< Current mouse position, relative to window coordinates
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
