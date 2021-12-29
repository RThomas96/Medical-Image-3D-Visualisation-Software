#ifndef VISUALISATION_QT_INCLUDE_ARAP_MAIN_WIDGET_HPP_
#define VISUALISATION_QT_INCLUDE_ARAP_MAIN_WIDGET_HPP_

#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "../../viewer/include/planar_viewer.hpp"
#include "../../viewer/include/scene.hpp"
#include "../../qt/include/viewer_helper.hpp"
#include "../../qt/include/grid_control.hpp"
#include "./loader_widget.hpp"
#include "../../qt/include/opengl_debug_log.hpp"
#include "./scene_control.hpp"
#include "../../qt/include/user_settings_widget.hpp"
#include "../../qt/include/arap_controller.hpp"

#include <QGLViewer/qglviewer.h>
#include <QMainWindow>
#include <QWidget>

class ColorBoundWidget;

/// @ingroup qtwidgets
/// @brief The ARAPMainWidget class represents the top-level widget, created when the deformation program is run.
/// @details It holds references to the top menu bar, the status bar of the program, the button to show the GL log and
/// all of the possible actions available for the program (in Qt lingo, actions = events from keyboard, mouse, other).
/// In addition, it also holds references to the Scene class, the Viewer (s) and their ViewerHeader (s), as well as the
/// ControlPanel that controls many aspects of the scene, and the GL debug log.
class ARAPMainWidget : public QMainWindow {
Q_OBJECT
public:
	ARAPMainWidget();
	virtual ~ARAPMainWidget();
	Viewer* getViewer3D() const { return this->viewer; }

protected:
	/// @brief Setup all widgets, and connect their signals.
	void setupWidgets();
	/// @brief Setup the widget's signals.
	void setupSignals();
	/// @brief Allow to run code on any widget event
	/// @details In this case, set the minimum width and height of widgets in order to
	/// have them both square, and not too small.
	bool eventFilter(QObject* obj, QEvent* e) override;

	void showHelper();

protected slots:
	void showLoader();

private:
	Scene* scene;	 ///< The underlying scene, with the data to display

	ViewerHeader3D* header3d;	 ///< The header for the 3D widget
	Viewer* viewer;	   ///< The visualisation panel, drawing elements from the scene

	ViewerHeader* headerX;	  ///< Header for the X plane viewer
	PlanarViewer* viewer_planeX;	///< The visualisation of the grid on plane X

	ViewerHeader* headerY;	  ///< Header for the Y plane viewer
	PlanarViewer* viewer_planeY;	///< The visualisation of the grid on plane Y

	ViewerHeader* headerZ;	  ///< Header for the Z plane viewer
	PlanarViewer* viewer_planeZ;	///< The visualisation of the grid on plane Z

	OpenGLDebugLog* glDebug;	///< Output for the OpenGL debug messages.

	UserSettingsWidget* usettings;	  ///< User settings dialog.
	GridLoaderWidget* loaderWidget;	   ///< Loader widget

	ARAPController* arap_controller;

	VisuBoxController* boxController;	 ///< The visu box controller

	ControlPanel* controlPanel;	   ///< The control panel at the bottom of the grid
	bool widgetSizeSet;	   ///< Checks if the widget size has been set before
	std::vector<QObject*> strayObj;	   ///< Pointers to all temporary objects allocated in the setup process

	// UI Stuff :

	QMenu* fileMenu;				///< The 'File' menu of the application
	QMenu* viewMenu;				///< The 'View' menu of the application
	QMenu* helpMenu;				///< The 'Help' menu of the application
	QAction* action_addGrid;		///< Action to launch the grid loader.
	QAction* action_exitProgram;	///< Action to exit the program.
	QAction* action_showVisuBox;	///< Action to show the visualization box controller
	QAction* action_drawModeS;		///< Action to set the scene's draw mode to 'Solid'
	QAction* action_drawModeV;		///< Action to set the scene's draw mode to 'Volumetric'
	QAction* action_drawModeVB;		///< Action to set the scene's draw mode to 'Volumetric boxed'
	QAction* action_showHelp3D;		///< Action to show the help dialog for the 3D viewer
	QAction* action_showHelpPlane;	///< Action to show the help dialog for the planar viewers
	QAction* action_showSettings;	///< Action to show user settings
	QStatusBar* statusBar;			///< Status bar
	QPushButton* showGLLog;			///< Button to open the QOpenGLDebugLog message box.
	QPushButton* deform;			///< The 'Deform' button for debug purpose
};

#endif // VISUALISATION_QT_INCLUDE_ARAP_MAIN_WIDGET_HPP_
