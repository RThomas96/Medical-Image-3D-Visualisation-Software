#ifndef VISUALISATION_VIEWER_INCLUDE_PLANAR_VIEWER_HPP_
#define VISUALISATION_VIEWER_INCLUDE_PLANAR_VIEWER_HPP_

#include "../../features.hpp"
#include "../../qt/widgets/include/viewer_header.hpp"
#include "./scene.hpp"

#include <QGLViewer/qglviewer.h>
#include <QTimer>

#include <memory>

/// @ingroup graphpipe
/// @brief The PlanarViewer class handles the OpenGL widget responsible for showing the contents present at the cutting
/// plane's positions.
/// @details While this is inheriting from QGLViewer, it does not use any of the camera settings. It uses a completely
/// custom rendering pipeline.
class PlanarViewer : public QGLViewer {
	Q_OBJECT
protected:
	friend class ViewerHeader;

public:
	/// @brief Default constructor for the viewer.
	PlanarViewer(Scene* const _scene, planes _p, QStatusBar* status_bar, planeHeading _h = North, QWidget* parent = nullptr);
	/// @brief Default destructor for the viewer.
	~PlanarViewer(void);

	/// @brief Add a status bar pointer to the viewer, in order to show messages
	virtual void addParentStatusBar(QStatusBar* _sb);

protected:
	virtual void init(void) override;
	virtual void draw(void) override;
	virtual void keyPressEvent(QKeyEvent* _e) override;
	virtual void mousePressEvent(QMouseEvent* _m) override;
	virtual void mouseMoveEvent(QMouseEvent* _m) override;
	virtual void mouseReleaseEvent(QMouseEvent* _m) override;
	virtual void wheelEvent(QWheelEvent* _w) override;
	virtual void resizeGL(int w, int h) override;
	virtual QString helpString(void) const override;
	virtual QString keyboardString(void) const override;
	virtual QString mouseString(void) const override;
	virtual void guessScenePosition(void);

protected:
	void setController(ViewerHeader* _header);

protected:
	Scene* sceneToShow;	   ///< The scene to draw.
	planes planeToShow;	   ///< The plane to show.
	QTimer* refreshTimer;	 ///< Triggers a scene reload
	planeHeading planeOrientation;	  ///< This plane's orientation
	ViewerHeader* viewerController;	   ///< The widget that controls this widget

	glm::ivec2 posRequest;	  ///< A texture position request for the render
	GLuint renderTarget;	///< The texture to render to for additional info
	float minZoomRatio;	   ///< minimum value of the zoom applied to the image
	float maxZoomRatio;	   ///< maximum value of the zoom applied to the image
	float zoomRatio;	///< The current zoom level applied to the image
	float planeDepth;	 ///< The normalized plane depth, set from the viewer's header slider
	glm::vec2 offset;	 ///< Normalized offset from the window's origin
	glm::vec2 tempOffset;	 ///< Temporary offset applied to the rendered image while moving around
	std::size_t mouse_isPressed;	///< Is the mouse pressed ? Counted in number of frames
	bool ctrl_pressed;	  ///< Is the Ctrl keymod pressed ?
	QPoint cursorPosition_last;	   ///< Last known position, relative to window coordinates
	QPoint cursorPosition_current;	  ///< Current mouse position, relative to window coordinates
	QStatusBar* status_bar;	   ///< The status bar in which to show the positions extracted from the mesh
public slots:
	void updateView(void);
	void updatePlaneDepth(int newVal);
	void flipPlaneDirection(void);
	void rotatePlaneClockwise(void);
	void rotatePlaneCounterClockwise(void);
	void togglePlaneVisibility(void);

public:
    glm::vec3 getPositionFromMouse();

signals:
    void pointIsClickedInPlanarViewer(const glm::vec3& position);
};

#endif	  // VISUALISATION_VIEWER_INCLUDE_PLANAR_VIEWER_HPP_
