#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_DOUBLE_VIEWER_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_DOUBLE_VIEWER_HPP_

#include <QWidget>

#include "./simple_viewer.hpp"
#include "./control_panel.hpp"

// TODO : MAke this the main widget the window will be showing
// TODO : add a scene class in order to get a single scene in mutliple viewers
// TODO : Import a KPF-like GLSL interface in order to load shaders into the program
// TODO : Furnish the singleViewer class

class DoubleViewerWidget {
		Q_OBJECT
	protected:
		SimpleViewer* leftViewer;
		SimpleViewer* rightViewer;
		ControlPanel* controlPanel;
	public:
		explicit DoubleViewerWidget(QWidget* parent = nullptr);
	public slots:
		void controlPanelXCoordChanged(int x);
		void controlPanelYCoordChanged(int y);
		void controlPanelZCoordChanged(int z);
		void requestPositionUpdate();
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_DOUBLE_VIEWER_HPP_
