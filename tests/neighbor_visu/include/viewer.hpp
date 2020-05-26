#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_VIEWER_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_VIEWER_HPP_

#include "./scene.hpp"

#include <QGLViewer/qglviewer.h>

// TODO : link control panels's actions to signals/slots here
// TODO : test the class
// TODO : work on shaders

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
	public slots:
		void setFocusState(int state);
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_VIEWER_HPP_
