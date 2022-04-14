#ifndef QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
#define QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_

#include <iomanip>
#include <sstream>
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

#include <map>

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
    
    InfoPannel(Scene * scene, QWidget *parent = nullptr):QGroupBox(parent){init();connect(scene);}
    InfoPannel(const QString &title, Scene * scene, QWidget *parent = nullptr): QGroupBox(title, parent){init();connect(scene);}

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
        this->info_position = new QLabel("Pos:");

        this->info_id_data = new QLabel("-");
        this->info_position_data = new QLabel("[]");
        this->info_position_data->setStyleSheet("font: 9pt;");

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

    void connect(Scene * scene) {
        QObject::connect(scene, &Scene::selectedPointChanged, this, &InfoPannel::updatePointInfo);
    }

    void updatePointInfo(std::pair<int, glm::vec3> selectedPoint) {
        std::string idx = std::to_string(selectedPoint.first);
        //std::string pt = std::string("[") + 
        //                 std::to_string(selectedPoint.second[0]) + 
        //                 std::string(", ") +
        //                 std::to_string(selectedPoint.second[1]) + 
        //                 std::string(", ") +
        //                 std::to_string(selectedPoint.second[2]) +
        //                 std::string("]");
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << "[" << selectedPoint.second[0] << ", " << selectedPoint.second[1] << ", " << selectedPoint.second[2] << "]";
        std::string pt = stream.str();

        if(selectedPoint.first >= 0) {
            this->info_id_data->setText(QString(idx.c_str()));
            this->info_position_data->setText(QString(pt.c_str()));
        } else {
            this->info_id_data->setText(QString("-"));
            this->info_position_data->setText(QString("[]"));
        }
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

class QActionManager : QWidget {
    Q_OBJECT
public:
    std::map<std::string, QAction *> actions;
    std::map<std::string, QActionGroup *> actionGroups;

    QAction * getAction(const QString& name) {
        return actions[name.toStdString()];
    }

    void createQExclusiveActionGroup(const QString& name, const QStringList& actionNames) {
        this->actionGroups[name.toStdString()] = new QActionGroup(this);
        for(int i = 0; i < actionNames.size(); ++i) {
            this->actionGroups[name.toStdString()]->addAction(actions[actionNames.at(i).toStdString()]);
        }
        this->actionGroups[name.toStdString()]->setExclusive(true);
    }

    QAction * createQAction(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon, bool checkable, bool checked) {
        QIcon icon;
        if(!defaultIcon.isEmpty())
            icon.addPixmap(QPixmap(QString("../resources/") + defaultIcon + QString(".svg")), QIcon::Normal, QIcon::Off);
        if(!pressedIcon.isEmpty())
            icon.addPixmap(QPixmap(QString("../resources/") + pressedIcon + QString(".svg")), QIcon::Normal, QIcon::On);
    
        QAction * action = new QAction(icon, text);
        if(checkable)
            action->setCheckable(true);
        action->setStatusTip(statusTip + QString(" - ") + keySequence);
        action->setToolTip(statusTip + QString(" - ") + keySequence);
    
        action->setShortcut(QKeySequence(keySequence));
        action->setIconVisibleInMenu(true);

        actions[name.toStdString()] = action;

        if(checked) {
            action->setChecked(true);
        }
    
        return action;
    }
    
    QAction * createQActionButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip) {
        return createQAction(name, text, keySequence, statusTip, QString(), QString(), false, false);
    }

    QAction * createQActionButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, QString(), false, false);
    }

    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon, bool checked) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, pressedIcon, true, checked);
    }
    
    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, pressedIcon, true, false);
    }

    QAction * createQActionToggledButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, pressedIcon, true, true);
    }
    
    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon) {
        return createQActionToggleButton(name, text, keySequence, statusTip, defaultIcon, QString());
    }
    
    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip) {
        return createQActionToggleButton(name, text, keySequence, statusTip, QString(), QString());
    }

    QAction * createQActionToggledButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon) {
        return createQActionToggledButton(name, text, keySequence, statusTip, defaultIcon, QString());
    }
    
    QAction * createQActionToggledButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip) {
        return createQActionToggledButton(name, text, keySequence, statusTip, QString(), QString());
    }
};

class Display : public QGroupBox {
    Q_OBJECT
public:
    Display(const QString &title, QActionManager& actionManager, QWidget *parent = nullptr): QGroupBox(title, parent){init();connect(actionManager);}

    QVBoxLayout * mainLayout;
    QToolBar * toolBar;

public slots:
    void init() {
        this->mainLayout = new QVBoxLayout(this);

        this->toolBar = new QToolBar(this);
        this->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        this->mainLayout->addWidget(this->toolBar);
    };
    void connect(QActionManager& actionManager) {
        toolBar->addAction(actionManager.getAction("ToggleDisplayMesh"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayGrid"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayPlanarViewers"));
        toolBar->addAction(actionManager.getAction("MoveTool_toggleEvenMode"));
    };
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
    void setupActions();
	/// @brief Allow to run code on any widget event
	/// @details In this case, set the minimum width and height of widgets in order to
	/// have them both square, and not too small.
	bool eventFilter(QObject* obj, QEvent* e) override;

private:
	Scene* scene;

    QActionManager* actionManager;

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

    CutPlaneGroupBox* cutPlaneDisplay;
    Display* display;
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

	QAction* tool_none;
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

    // *************** //
    // Connected to UI //
    // *************** //

    void toggleDisplayPlanarViewers();
};

#endif	  // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
