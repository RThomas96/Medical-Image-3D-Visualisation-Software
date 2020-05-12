#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_MAIN_WIDGET_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_MAIN_WIDGET_HPP_

#include "./viewer.hpp"
#include "./scene.hpp"
#include "./control_panel.hpp"

#include <QWidget>
#include <QGLViewer/qglviewer.h>

class MainWidget : public QWidget {
		Q_OBJECT
	public:
		MainWidget();
		~MainWidget(){}
	protected:
		void setupWidgets();
		bool eventFilter(QObject* obj, QEvent* e) override;
	private:
		Viewer* leftViewer;
		//Viewer* rightViewer;
		Scene* scene;
		ControlPanel* controlPanel;
		bool widgetSizeSet; ///< checks if the widget size has been set before
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_MAIN_WIDGET_HPP_
