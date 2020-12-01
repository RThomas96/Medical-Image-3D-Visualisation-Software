#ifndef QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
#define QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_

#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "../../viewer/include/scene.hpp"
#include "./scene_control.hpp"
#include "./grid_control.hpp"

#include <QWidget>
#include <QGLViewer/qglviewer.h>

#define ENABLE_DUAL_VISU

class MainWidget : public QWidget {
		Q_OBJECT
	public:
		MainWidget();
		~MainWidget();
	protected:
		void setupWidgets();
		bool eventFilter(QObject* obj, QEvent* e) override;
	private:
		Viewer* leftViewer;
		#ifdef ENABLE_DUAL_VISU
		Viewer* rightViewer;
		#endif
		Scene* scene;
		ControlPanel* controlPanel;
		GridControl* gridController;
		bool widgetSizeSet; ///< checks if the widget size has been set before
};

#endif // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
