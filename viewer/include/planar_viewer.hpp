#ifndef VISUALISATION_VIEWER_INCLUDE_PLANAR_VIEWER_HPP_
#define VISUALISATION_VIEWER_INCLUDE_PLANAR_VIEWER_HPP_

#include "../../features.hpp"
#include "./scene.hpp"

#include <QGLViewer/qglviewer.h>

#include <memory>

class PlanarViewer : public QGLViewer {
	public :
		/// @brief Default constructor for the viewer.
		PlanarViewer(Scene* const _scene, planes _p, QWidget* parent = nullptr);
		/// @brief Default destructor for the viewer.
		~PlanarViewer(void);
	protected :
		/// @brief Initializes the scene, and the viewer's variables.
		virtual void init(void) override;
		/// @brief Draws the plane the viewer is supposed to show.
		virtual void draw(void) override;
		/// @brief Handles key events from the user.
		virtual void keyPressEvent(QKeyEvent* _e) override;
		/// @brief Handles mouse events from the user.
		virtual void mousePressEvent(QMouseEvent* _m) override;
	private :
		Scene* sceneToShow; ///< The scene to draw.
		planes planeToShow; ///< The plane to show.
	public slots:
		/// @brief Update the view, as a slot without any arguments
		void updateView(void) { this->update(); }
};

#endif // VISUALISATION_VIEWER_INCLUDE_PLANAR_VIEWER_HPP_
