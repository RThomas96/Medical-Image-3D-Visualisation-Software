#ifndef VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
#define VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_

#include "../../features.hpp"

#include "./scene.hpp"

#include <QGLViewer/qglviewer.h>
#include <QTimer>

// #include <renderdoc_app.h>

#include <memory>

/// @brief A viewer that displays a scene, either in real space or in initial space
class Viewer : public QGLViewer {
		Q_OBJECT
	public:
		/// @brief Default constructor for the viewer.
		Viewer(Scene* const scene, QWidget* parent = nullptr);
		~Viewer();
		/// @brief Multiplier to apply to scene radii for the scene's view.
		static float sceneRadiusMultiplier;
	protected:
		/// @brief Initializes the scene, and the viewer's variables.
		virtual void init() override;
		/// @brief Draws the scene, in the space the viewer is supposed to show.
		virtual void draw() override;
		/// @brief Handles key events from the user.
		virtual void keyPressEvent(QKeyEvent* e) override;
		/// @b Wheel event for the mouse.
		virtual void wheelEvent(QWheelEvent* _w) override;
		/// @b Defines the 'Help'/'About' string defined for this viewer.
		virtual QString helpString(void) const override;
		/// @b Defines the 'Help'/'About' string for the keyboard for this viewer.
		virtual QString keyboardString(void) const override;
		/// @b Defines the 'Help'/'About' string for the mouse for this viewer.
		virtual QString mouseString(void) const override;
	private:
		/// @brief The scene to control.
		Scene* const scene;
		/// @brief Should we draw it in volumetric mode ?
		bool drawVolumetric;
		/// @brief A refresh timer for the viewer, to update in real time.
		QTimer* refreshTimer;
		/// @b Should we capture a frame ?
		bool shouldCapture;
		/// @b Is the Ctrl key pressed down ?
		bool keyboard_CtrlDown;
	public slots:
		/// @brief Update the view, as a slot without any arguments (currently only used by QTimer)
		void updateView() { this->update(); }
		/// @brief Adds a grid to the scene
		//void addGrid();
		/// @brief Adds one grid, composed of two grid's metadata to the scene
		//void addTwoGrids();
		void loadGrid(std::shared_ptr<InputGrid>& g);
		void loadTwoGrids(std::shared_ptr<InputGrid>& g1, std::shared_ptr<InputGrid>& g2);
		/// @b Re-centers the camera around the scene-defined center point
		void centerScene(void) ;
};

#endif // VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
