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
#include "../applyCageWidget.hpp"
#include "../CutPlaneGroupBox.h"
#include "./opengl_debug_log.hpp"
#include "./scene_control.hpp"
#include "./user_settings_widget.hpp"

#include <QGLViewer/qglviewer.h>
#include <QMainWindow>
#include <QWidget>
#include <QToolBar>
#include <QFrame>
#include <QSizePolicy>
#include <QTabWidget>
#include <QShortcut>

class ColorBoundWidget;

class InfoPannel : public QGroupBox {
    Q_OBJECT

public:
    
    InfoPannel(QWidget *parent = nullptr):QGroupBox(parent){init();}
    InfoPannel(const QString &title, QWidget *parent = nullptr): QGroupBox(title, parent){init();}

    QVBoxLayout * main_layout;
    QHBoxLayout * id_layout;
    QLabel      * info_id;
    QLabel      * info_position;
    QHBoxLayout * position_layout;
    QLabel      * info_id_data;
    QLabel      * info_position_data;

public slots:

    void init(){
        this->setCheckable(false);
        this->main_layout = new QVBoxLayout();
        this->main_layout->setAlignment(Qt::AlignTop);
        this->info_id = new QLabel("Id:");
        this->info_position = new QLabel("Position:");

        this->info_id_data = new QLabel("[]");
        this->info_position_data = new QLabel("[]");

        this->id_layout = new QHBoxLayout();
        this->id_layout->addWidget(this->info_id);
        this->id_layout->addWidget(this->info_id_data);
        this->position_layout = new QHBoxLayout();
        this->position_layout->addWidget(this->info_position);
        this->position_layout->addWidget(this->info_position_data);

        this->id_layout->setAlignment(this->info_id_data, Qt::AlignHCenter);
        this->position_layout->setAlignment(this->info_position_data, Qt::AlignHCenter);

        this->main_layout->addLayout(this->id_layout);
        this->main_layout->addLayout(this->position_layout);
        this->setLayout(this->main_layout);
    }
};

class MultipleRadioButton : public QGridLayout {
    Q_OBJECT
public:
    MultipleRadioButton(const std::vector<QString> title, QWidget *parent = nullptr): QGridLayout(parent){init(title);}

    std::vector<QLabel*> label;
    std::vector<QRadioButton*> button;

    void init(const std::vector<QString> title) {
        this->label.reserve(title.size());
        this->button.reserve(title.size());
        for(int i = 0; i < title.size(); ++i) {
            this->label.push_back(new QLabel(title[i]));
            this->button.push_back(new QRadioButton());
            this->addWidget(this->label.back(), i, 0, Qt::AlignRight);
            this->addWidget(this->button.back(), i, 1, Qt::AlignHCenter);
        }
        this->button[0]->setChecked(true);
    }
};

class ToolPannel : public QGroupBox {
    Q_OBJECT

public:
    
    ToolPannel(QWidget *parent = nullptr):QGroupBox(parent){init();}
    ToolPannel(const QString &title, Scene * scene, QWidget *parent = nullptr): QGroupBox(title, parent){init();connect(scene);}

    UITool::MeshManipulatorType currentTool;

    QVBoxLayout * tools_layout;

    QWidget     * fixedRegistration_tools;
    QGridLayout * fixedRegistration_layout;
    QLabel      * fixedRegistration_mode_label;
    QPushButton * fixedRegistration_apply;
    QPushButton * fixedRegistration_clear;
    QPushButton * fixedRegistration_loadPoint;

    QWidget     * move_tools;
    QGridLayout * move_layout;
    QLabel      * move_mode_label;
    QShortcut   * move_mode_shortcut;
    MultipleRadioButton* move_mode;

    QWidget     * arap_tools;
    QGridLayout * arap_layout;
    QLabel      * arap_mode_label;
    QShortcut   * arap_mode_shortcut;
    MultipleRadioButton* arap_mode;

public slots:
    void init(){
        this->setCheckable(false);
        this->tools_layout = new QVBoxLayout(this);
        this->tools_layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

        // Fixed registration tool
        this->fixedRegistration_tools = new QWidget(this);
        this->fixedRegistration_layout = new QGridLayout(this->fixedRegistration_tools);
        this->fixedRegistration_mode_label = new QLabel("Select first point");
        this->fixedRegistration_layout->addWidget(this->fixedRegistration_mode_label);
        this->fixedRegistration_loadPoint = new QPushButton("Load points");
        this->fixedRegistration_layout->addWidget(this->fixedRegistration_loadPoint);
        this->fixedRegistration_apply = new QPushButton("Register");
        this->fixedRegistration_layout->addWidget(this->fixedRegistration_apply);
        this->fixedRegistration_clear = new QPushButton("Clear");
        this->fixedRegistration_layout->addWidget(this->fixedRegistration_clear);

        this->tools_layout->addWidget(this->fixedRegistration_tools);

        // Move tool
        this->move_tools = new QWidget(this);
        this->move_layout = new QGridLayout(this->move_tools);
        this->move_mode_label = new QLabel("Mode: ");
        this->move_layout->addWidget(this->move_mode_label, 0, 0, Qt::AlignRight);
        this->move_mode = new MultipleRadioButton({"Normal", "Even"}, this->move_tools);
        this->move_layout->addLayout(this->move_mode, 0, 1, Qt::AlignHCenter);

        this->tools_layout->addWidget(this->move_tools);

        // ARAP tool
        this->arap_tools = new QWidget(this);
        this->arap_layout = new QGridLayout(this->arap_tools);
        this->arap_mode_label = new QLabel("Mode: ");
        this->arap_layout->addWidget(this->arap_mode_label, 0, 0, Qt::AlignRight);
        this->arap_mode = new MultipleRadioButton({"Normal", "Handle"});
        this->arap_layout->addLayout(this->arap_mode, 0, 1, Qt::AlignHCenter);

        this->tools_layout->addWidget(this->arap_tools);
    }

    void hideAllLayouts() {
        this->fixedRegistration_tools->hide();
        this->move_tools->hide();
        this->arap_tools->hide();
    }

    void changeCurrentTool(UITool::MeshManipulatorType newTool) {
        this->hideAllLayouts();
        switch(newTool) {
            case UITool::MeshManipulatorType::POSITION:
                this->move_tools->show();
                break;
            case UITool::MeshManipulatorType::DIRECT:
                break;
            case UITool::MeshManipulatorType::ARAP:
                this->arap_tools->show();
                break;
            case UITool::MeshManipulatorType::FIXED_REGISTRATION:
                this->fixedRegistration_tools->show();
                break;
        }
    }

    void connect(Scene * scene) {
        QObject::connect(this->fixedRegistration_apply, &QPushButton::clicked, [this, scene](){scene->applyFixedRegistrationTool();});

        QObject::connect(this->fixedRegistration_clear, &QPushButton::clicked, [this, scene](){scene->clearFixedRegistrationTool();});

        QObject::connect(this->arap_mode->button[0], &QRadioButton::toggled, [this, scene](){scene->toggleARAPManipulatorMode();});
        this->connectShortcut();
    }

    void connectShortcut() {
        this->arap_mode_shortcut = new QShortcut(QKeySequence("S"), this->arap_mode->button[0]);
        QObject::connect(arap_mode_shortcut, &QShortcut::activated, this, [this](){if(this->arap_mode->button[0]->isChecked()){this->arap_mode->button[1]->toggle();} else {this->arap_mode->button[0]->toggle();}});

        this->move_mode_shortcut = new QShortcut(QKeySequence("S"), this->move_mode->button[0]);
        //QObject::connect(move_mode_shortcut, &QShortcut::activated, this, [this](){this->move_mode->button[1]->toggle();});
        //QObject::connect(move_mode_shortcut, &QShortcut::released, this, [this](){this->move_mode->button[0]->toggle();});

    }

signals:

};


class MainWidget : public QMainWindow {
	Q_OBJECT
public:
	MainWidget();
	virtual ~MainWidget();
	Viewer* getViewer3D() const { return this->viewer; }

protected:
	void setupWidgets();
	/// @brief Allow to run code on any widget event
	/// @details In this case, set the minimum width and height of widgets in order to
	/// have them both square, and not too small.
	bool eventFilter(QObject* obj, QEvent* e) override;

private:
	Scene* scene;

    QFrame* viewerFrame;
    QWidget* _ViewerCapsule;
    QWidget* xViewerCapsule;
    QWidget* yViewerCapsule;
    QWidget* zViewerCapsule;

	Viewer* viewer;

	ViewerHeader* headerX;
	PlanarViewer* viewer_planeX;

	ViewerHeader* headerY;
	PlanarViewer* viewer_planeY;

	ViewerHeader* headerZ;
	PlanarViewer* viewer_planeZ;

	OpenGLDebugLog* glDebug;

	GridLoaderWidget* loaderWidget;
	GridDeformationWidget* deformationWidget;

    CutPlaneGroupBox* cutPlane;
	ControlPanel* controlPanel;
	bool widgetSizeSet;

	QMenu* fileMenu;
	QMenu* viewMenu;
	QMenu* otherMenu;

    QToolBar * toolbar;
	QAction* action_addGrid;
	QAction* action_saveGrid;
	QAction* action_exitProgram;
	QAction* action_showPlanarViewers;
	QAction* action_loadMesh;
	QAction* action_saveMesh;
	QAction* action_applyCage;
	QAction* action_openDevPannel;

    QComboBox* combo_mesh;

    InfoPannel* info_pannel;
    ToolPannel* tool_pannel;

	QAction* tool_open;
	QAction* tool_save;

	QAction* tool_position;
	QAction* tool_direct;
	QAction* tool_ARAP;
	QAction* tool_registration;

	QStatusBar* statusBar;
	QPushButton* showGLLog;

    OpenMeshWidget * openMeshWidget;
    SaveMeshWidget * saveMeshWidget;
    ApplyCageWidget * applyCageWidget;

    bool isShiftPressed = false;

public slots:
    void addNewMesh(const std::string& name, bool grid, bool cage) {
        //this->combo_mesh->insertItem(this->combo_mesh->count(), QString(this->meshNames.back().c_str()));
        this->combo_mesh->insertItem(this->combo_mesh->count(), QString(name.c_str()));
        //if(!this->gridOrCage.back().first)
        //    this->combo_mesh_register->insertItem(this->combo_mesh_register->count(), QString(this->meshNames.back().c_str()));
    }
};

#endif	  // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
