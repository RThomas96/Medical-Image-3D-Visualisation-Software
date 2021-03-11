#ifndef QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
#define QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_

#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "../../viewer/include/planar_viewer.hpp"
#include "../../viewer/include/scene.hpp"
#include "./scene_control.hpp"
#include "./grid_control.hpp"
#include "./opengl_debug_log.hpp"
#include "./user_settings_widget.hpp"
#include "./loader_widget.hpp"

#include <QWidget>
#include <QMainWindow>
#include <QGLViewer/qglviewer.h>

#define ENABLE_QUAD_VIEW

class ColorBoundControl;

class MainWidget : public QMainWindow {
		Q_OBJECT
	public:
		MainWidget();
		~MainWidget();

	public slots:
		void addColorControl();
	public:
		void removeColorControl();
	protected:
		/// @b Setup all widgets, and connect their signals.
		void setupWidgets();
		/// @b Allow to run code on any widget event
		/// @details In this case, set the minimum width and height of widgets in order to
		/// have them both square, and not too small.
		bool eventFilter(QObject* obj, QEvent* e) override;
	private:
		Scene* scene;			///< The underlying scene, with the data to display

		ViewerHeader3D* header3d;	///< The header for the 3D widget
		Viewer* viewer;			///< The visualisation panel, drawing elements from the scene

		ViewerHeader* headerX;		///< Header for the X plane viewer
		PlanarViewer* viewer_planeX;	///< The visualisation of the grid on plane X

		ViewerHeader* headerY;		///< Header for the Y plane viewer
		PlanarViewer* viewer_planeY;	///< The visualisation of the grid on plane Y

		ViewerHeader* headerZ;		///< Header for the Z plane viewer
		PlanarViewer* viewer_planeZ;	///< The visualisation of the grid on plane Z

		OpenGLDebugLog* glDebug;	///< Output for the OpenGL debug messages.

		UserSettingsWidget* usettings;	///< User settings dialog.
		GridLoaderWidget* loaderWidget;	///< Loader widget

		ColorBoundControl* colorControl; ///< The widget that controls the colors of the scene

		ControlPanel* controlPanel;	///< The control panel at the bottom of the grid
		bool widgetSizeSet;		///< Checks if the widget size has been set before
		std::vector<QObject*> strayObj;	///< Pointers to all temporary objects allocated in the setup process

		// UI Stuff :

		QMenu* fileMenu;		///< The 'File' menu of the application
		QMenu* viewMenu;		///< The 'View' menu of the application
		QMenu* helpMenu;		///< The 'Help' menu of the application
		QAction* action_addGrid;	///< Action to add grid
		QAction* action_saveGrid;	///< Action to save grid
		QAction* action_exitProgram;	///< Action to exit the program.
		QAction* action_showVisuBox;	///< Action to show the visualization box controller
		QAction* action_showColorControl;	///< Action to show the color control
		QAction* action_drawModeS;	///< Action to set the scene's draw mode to 'Solid'
		QAction* action_drawModeV;	///< Action to set the scene's draw mode to 'Volumetric'
		QAction* action_drawModeVB;	///< Action to set the scene's draw mode to 'Volumetric boxed'
		QAction* action_showHelp3D;	///< Action to show the help dialog for the 3D viewer
		QAction* action_showHelpPlane;	///< Action to show the help dialog for the planar viewers
		QAction* action_showSettings;	///< Action to show user settings
		QStatusBar* statusBar;		///< Status bar
		QPushButton* showGLLog;		///< Button to open the QOpenGLDebugLog message box.
};

#endif // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
