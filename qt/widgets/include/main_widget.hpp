#ifndef QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
#define QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_

#include "../../qt/viewers/include/neighbor_visu_viewer.hpp"
#include "../../qt/viewers/include/planar_viewer.hpp"
#include "../../qt/viewers/include/scene.hpp"
//#include "./grid_control.hpp"
#include "./loader_widget.hpp"
#include "../deformation_widget.hpp"
#include "../openMeshWidget.hpp"
#include "../saveMeshWidget.hpp"
#include "./opengl_debug_log.hpp"
#include "./scene_control.hpp"
#include "./user_settings_widget.hpp"

#include <QGLViewer/qglviewer.h>
#include <QMainWindow>
#include <QWidget>

#define ENABLE_QUAD_VIEW

/// @defgroup qtwidgets Qt Widgets
/// @brief All Qt widget/object classes that don't yet have a group.
///
/// The Widgets namespace groups all in-use Qt widgets in the program. Some of them will be denoted as 'legacy', meaning
/// they implement features that use the legacy `DiscreteGrid` implementation of a voxel grid.
///
/// Otherwise, those Qt widgets are all still in use in the program.
///
/// However, please note that not all classes deriving from Qt object classes are here. Some might be more suited to
/// another module.

class ColorBoundWidget;

/// @ingroup qtwidgets
/// @brief The MainWidget class represents the top-level widget, created when the program is run.
/// @details It holds references to the top menu bar, the status bar of the program, the button to show the GL log and
/// all of the possible actions available for the program (in Qt lingo, actions = events from keyboard, mouse, other).
/// In addition, it also holds references to the Scene class, the Viewer (s) and their ViewerHeader (s), as well as the
/// ControlPanel that controls many aspects of the scene, and the GL debug log.
class MainWidget : public QMainWindow {
	Q_OBJECT
public:
	MainWidget();
	virtual ~MainWidget();
	Viewer* getViewer3D() const { return this->viewer; }

protected:
	/// @brief Setup all widgets, and connect their signals.
	void setupWidgets();
	/// @brief Allow to run code on any widget event
	/// @details In this case, set the minimum width and height of widgets in order to
	/// have them both square, and not too small.
	bool eventFilter(QObject* obj, QEvent* e) override;

private:
	Scene* scene;	 ///< The underlying scene, with the data to display

    QWidget* _ViewerCapsule;
    QWidget* xViewerCapsule;
    QWidget* yViewerCapsule;
    QWidget* zViewerCapsule;

	ViewerHeader3D* header3d;
	Viewer* viewer;

	ViewerHeader* headerX;
	PlanarViewer* viewer_planeX;

	ViewerHeader* headerY;
	PlanarViewer* viewer_planeY;

	ViewerHeader* headerZ;
	PlanarViewer* viewer_planeZ;

	OpenGLDebugLog* glDebug;

	UserSettingsWidget* usettings;
	GridLoaderWidget* loaderWidget;

	GridDeformationWidget* deformationWidget;

	ControlPanel* controlPanel;
	bool widgetSizeSet;
	std::vector<QObject*> strayObj;

	QMenu* fileMenu;
	QMenu* viewMenu;
	QMenu* helpMenu;
	QAction* action_addGrid;
	QAction* action_saveGrid;
	QAction* action_exitProgram;
	QAction* action_showVisuBox;
	QAction* action_showPlanarViewers;
	QAction* action_drawModeS;
	QAction* action_drawModeV;
	QAction* action_drawModeVB;
	QAction* action_showHelp3D;
	QAction* action_showHelpPlane;
	QAction* action_showSettings;
	QAction* action_loadMesh;
	QAction* action_saveMesh;
	QStatusBar* statusBar;
	QPushButton* showGLLog;
	QPushButton* deform_menu;

    OpenMeshWidget * openMeshWidget;
    SaveMeshWidget * saveMeshWidget;

    bool isShiftPressed = false;
};

#endif	  // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
