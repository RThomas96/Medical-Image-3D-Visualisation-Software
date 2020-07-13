#ifndef VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
#define VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_

#include "./scene.hpp"

#include <QGLViewer/qglviewer.h>
#include <QTimer>

#include <memory>

/// @brief This enumeration defines multiple possible focus states of the viewer.
enum FocusStates {
	NoFocus,
	TextureFocus,
	NeighborFocus,
	DefaultFocus = TextureFocus,
};

/// @brief A viewer that displays a scene, either in real space or in initial space
class Viewer : public QGLViewer {
		Q_OBJECT
	public:
		/// @brief Default constructor for the viewer.
		Viewer(Scene* const scene, bool _isRealSpace, QWidget* parent = nullptr);
		/// @brief Multiplier to apply to scene radii for the scene's view.
		static float sceneRadiusMultiplier;
	protected:
		/// @brief Initializes the scene, and the viewer's variables.
		virtual void init() override;
		/// @brief Draws the scene, in the space the viewer is supposed to show.
		virtual void draw() override;
		/// @brief Handles key events from the user.
		virtual void keyPressEvent(QKeyEvent* e) override;
	private:
		/// @brief The scene to control.
		Scene* const scene;
		/// @brief Determines if the viewer show real space, or initial space.
		bool isRealSpace;
		/// @brief The focus this viewer is supposed to have.
		FocusStates focusType;
		/// @brief A refresh timer for the viewer, to update in real time
		QTimer* refreshTimer;

		/// @brief Sets the focus the viewer has to be the texture focus.
		void updateTextureFocus();
		/// @brief Sets the focus the viewer has to be the neighbor focus.
		void updateNeighborFocus();
	public slots:
		/// @brief Set the focus state from elsewhere in the program.
		void setFocusState(int state);

		/// @brief Update the view, as a slot without any arguments (currently only used by QTimer)
		void updateView() { this->update(); }
};

#endif // VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
