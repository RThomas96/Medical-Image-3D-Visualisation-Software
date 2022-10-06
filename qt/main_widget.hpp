#ifndef QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
#define QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_

#include <iomanip>
#include <sstream>
#include "3D_viewer.hpp"
//#include "planar_viewer.hpp"
#include "scene.hpp"
//#include "./grid_control.hpp"
//#include "./loader_widget.hpp"
//#include "../deformation_widget.hpp"
#include "legacy/openMeshWidget.hpp"
#include "legacy/applyCageWidget.hpp"
#include "cutplane_groupbox.hpp"
//#include "./opengl_debug_log.hpp"
#include "scene_control.hpp"
//#include "./user_settings_widget.hpp"
#include "glm/fwd.hpp"
#include "qboxlayout.h"
#include "qbuttongroup.h"
#include "qjsonarray.h"
#include "qnamespace.h"
#include "qobjectdefs.h"
#include "legacy/viewer_structs.hpp"
#include <random>

#include <map>

#include <QMessageBox>
#include <QPainter>
#include <QGLViewer/qglviewer.h>
#include <QMainWindow>
#include <QWidget>
#include <QToolBar>
#include <QFrame>
#include <QSizePolicy>
#include <QTabWidget>
#include <QShortcut>
#include <QMenuBar>
#include <QToolButton>
#include <QLineEdit>
#include <QFormLayout>
#include <QTextEdit>
#include <QTextBlock>
#include <QMessageBox>
#include <QImage>
#include <QSlider>
#include <QSignalMapper>
#include <QSplitter>
#include <QScrollArea>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <string>
#include <vector>

#include "UI/chooser.hpp"
#include "UI/form.hpp"
#include "UI/deformation_form.hpp"
#include "UI/color_control.hpp"
#include "UI/quicksave_mesh.hpp"
#include "UI/tool_pannel.hpp"
#include "UI/display_pannel.hpp"
#include "UI/info_pannel.hpp"
#include "UI/open_image_form.hpp"
#include "UI/save_image_form.hpp"
#include "helper/QActionManager.hpp"

#include "image3D_viewer.hpp"

class ColorBoundWidget;

class MainWidget : public QMainWindow {
	Q_OBJECT
public:
	MainWidget();
	virtual ~MainWidget();
	Viewer* getViewer3D() const { return this->viewer; }

protected:
	void setupWidgets();
    void setupActions();
    void setupForms();
    void updateForms();
	/// @brief Allow to run code on any widget event
	/// @details In this case, set the minimum width and height of widgets in order to
	/// have them both square, and not too small.
	bool eventFilter(QObject* obj, QEvent* e) override;

private:
	Scene* scene;

    QActionManager* actionManager;

    QFrame* viewerFrame;

    QSplitter* hSplit;
    QSplitter* vSplit1;
    QSplitter* vSplit2;

	Viewer* viewer;

    RangeControl * range;
	ControlPanel* controlPanel;

	bool widgetSizeSet;

	QMenu* fileMenu;
    QMenu* editMenu;
    QMenu* windowsMenu;
    QMenu* otherMenu;

    QToolBar * toolbar;

    QComboBox* combo_mesh;

    CutPlaneGroupBox* cutPlane_pannel;
    DisplayPannel* display_pannel;
    InfoPannel* info_pannel;
    ToolPannel* tool_pannel;

	QStatusBar* statusBar;

    // Legacy widget, that don't use the Form class
    OpenMeshWidget * openMeshWidget;
    ApplyCageWidget * applyCageWidget;

    // New widget, that use the Form class
    DeformationForm * deformationForm;
    SaveImageForm * saveImageForm;
    QuickSaveMesh * quickSaveCage;
    OpenImageForm * openImageForm;
    PlanarViewer2D * planarViewer;

public slots:
    void addNewMesh(const std::string& name, bool grid, bool cage) {
        this->combo_mesh->insertItem(this->combo_mesh->count(), QString(name.c_str()));
        this->actionManager->getAction("SaveImage")->setDisabled(false);
        this->actionManager->getAction("SaveImageColormap")->setDisabled(false);
        if(this->scene->hasTwoOrMoreGrids()) {
            this->actionManager->getAction("ToggleDisplayMultiView")->setDisabled(false);
            this->actionManager->getAction("Transform")->setDisabled(false);
            this->actionManager->getAction("Boundaries")->setVisible(true);
            this->cutPlane_pannel->setDisabledAlpha(false);
        }
    }

    // *************** //
    // Connected to UI //
    // *************** //

    void changeCurrentTool(UITool::MeshManipulatorType newTool) {
        this->actionManager->deactivateGroup("MoveTool");
        this->actionManager->deactivateGroup("ARAPTool");
        this->actionManager->deactivateGroup("FixedTool");
        this->actionManager->deactivateGroup("SliceTool");
        switch(newTool) {
            case UITool::MeshManipulatorType::NONE:
                break;
            case UITool::MeshManipulatorType::POSITION:
                this->actionManager->activateGroup("MoveTool");
                break;
            case UITool::MeshManipulatorType::DIRECT:
                break;
            case UITool::MeshManipulatorType::ARAP:
                this->actionManager->activateGroup("ARAPTool");
                break;
            case UITool::MeshManipulatorType::SLICE:
                this->actionManager->activateGroup("SliceTool");
                break;
            case UITool::MeshManipulatorType::MARKER:
                break;
        }
    }

    void changeActiveMesh() {
        this->actionManager->getAction("ToggleNoneTool")->activate(QAction::Trigger);
        this->scene->toggleDisplayTetmesh(!this->actionManager->getAction("ToggleDisplayWireframe")->isChecked());
        if(this->scene->isGrid(this->combo_mesh->itemText(this->combo_mesh->currentIndex()).toStdString())) {
            Q_EMIT(this->gridSelected());
        } else {
            Q_EMIT(this->meshSelected());
        }
    }

    void initialize() {}
signals:
    void gridSelected();
    void meshSelected();
};

#endif	  // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
