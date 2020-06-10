#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_VIEWER_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_VIEWER_HPP_

#include "./scene.hpp"

#include <QGLViewer/qglviewer.h>

enum FocusStates {
	NoFocus,
	TextureFocus,
	NeighborFocus,
	DefaultFocus = TextureFocus,
};

class Viewer : public QGLViewer {
		Q_OBJECT
	public:
		Viewer(Scene* const scene, bool _isRealSpace, QWidget* parent = nullptr);
		static float sceneRadiusMultiplier;
	protected:
		virtual void init() override;
		virtual void draw() override;
		virtual void keyPressEvent(QKeyEvent* e) override;
	private:
		Scene* const scene;
		bool isRealSpace;
		bool applyMatrix;
		FocusStates focusType;

		void updateTextureFocus();
		void updateNeighborFocus();
	public slots:
		void setFocusState(int state);
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_VIEWER_HPP_
