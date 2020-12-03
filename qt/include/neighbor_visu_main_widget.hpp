#ifndef QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
#define QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_

#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "../../viewer/include/scene.hpp"
#include "./scene_control.hpp"
#include "./grid_control.hpp"

#include <QWidget>
#include <QGLViewer/qglviewer.h>

//#define ENABLE_QUAD_VIEW

class MainWidget : public QWidget {
		Q_OBJECT
	public:
		MainWidget();
		~MainWidget();
	protected:
		/// @b Setup all widgets, and connect their signals.
		void setupWidgets();
		/// @b Allow to run code on any widget event
		/// @details In this case, set the minimum width and height of widgets in order to
		/// have them both square, and not too small.
		bool eventFilter(QObject* obj, QEvent* e) override;
	private:
		Viewer* viewer; ///< The visualisation panel, drawing elements from the scene
		#ifdef ENABLE_QUAD_VIEW
		Viewer* viewer_planeX; ///< The visualisation of the grid on plane X
		Viewer* viewer_planeY; ///< The visualisation of the grid on plane Y
		Viewer* viewer_planeZ; ///< The visualisation of the grid on plane Z
		#endif
		Scene* scene; ///< The underlying scene, with the data to display
		ControlPanel* controlPanel; ///< The control panel at the bottom of the grid
		GridControl* gridController; ///< The control panel for the grid to generate
		bool widgetSizeSet; ///< Checks if the widget size has been set before
		std::vector<QObject*> strayObj; ///< Pointers to all temporary objects allocated in the setup process
};

#endif // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
