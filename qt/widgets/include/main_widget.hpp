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
#include <QMenuBar>
#include <QToolButton>
#include <QLineEdit>
#include <QFormLayout>
#include <QTextEdit>
#include <QTextBlock>

class ColorBoundWidget;

enum ObjectToChoose {
    ALL,
    GRID
};

class ObjectChooser : public QComboBox {
    Q_OBJECT

public:
    ObjectToChoose objectToChoose;

    ObjectChooser(QWidget *parent = nullptr):QComboBox(parent){}

    void fillChoices(const ObjectToChoose& objectToChoose, Scene * scene) {
        this->clear();
        this->objectToChoose = objectToChoose;
        std::vector<std::string> all;
        if(objectToChoose == ObjectToChoose::ALL)
            all = scene->getAllBaseMeshesName();

        if(objectToChoose == ObjectToChoose::GRID)
            all = scene->getAllGridsName();

        for(std::string name : all) {
            this->addItem(QString(name.c_str()));
        }
    }

    void fillChoices(Scene * scene) {
        this->fillChoices(this->objectToChoose, scene);
    }
};

class Form : public QWidget {
    Q_OBJECT

public:
    QFormLayout * layout;

    QStringList names;

    std::map<QString, QLabel*> labels;
    std::map<QString, QLineEdit*> lineEdits;
    std::map<QString, ObjectChooser*> objectChoosers;
    std::map<QString, QTextEdit*> textEdits;
    std::map<QString, QPushButton*> buttons;

    Form(QWidget *parent = nullptr):QWidget(parent){init();}

public slots:

    void init() {
        this->setWindowFlags(Qt::WindowStaysOnTopHint);
        this->layout = new QFormLayout();
        this->setLayout(this->layout);
    }

    void addLabel(const QString& name) {
        names += name;
        labels[name] = new QLabel(name);
        layout->addRow(labels[name]);
    }

    void addLineEdit(const QString& name) {
        names += name;
        labels[name] = new QLabel(name);
        lineEdits[name] = new QLineEdit();
        layout->addRow(labels[name], lineEdits[name]);
    }

    void addButton(const QString& name) {
        names += name;
        buttons[name] = new QPushButton(name);
        layout->addRow(buttons[name]);
    }

    void addTextEdit(const QString& name, const QString& label, bool editable = true) {
        names += name;
        labels[name] = new QLabel(label);
        textEdits[name] = new QTextEdit();
        textEdits[name]->setReadOnly(!editable);
        layout->addRow(labels[name], textEdits[name]);
    }

    void addTextEdit(const QString& name, bool editable = true) {
        this->addTextEdit(name, name, editable);
    }

    void addMeshChooser(const QString& name, const ObjectToChoose& objectToChoose = ObjectToChoose::ALL) {
        names += name;
        labels[name] = new QLabel(name);
        objectChoosers[name] = new ObjectChooser();
        objectChoosers[name]->objectToChoose = objectToChoose;
        layout->addRow(labels[name], objectChoosers[name]);
    }

    void update(Scene * scene) {
        for(auto& chooser : objectChoosers) {
            chooser.second->fillChoices(scene);
        }
    }
};

class DeformationForm : Form {
    Q_OBJECT

public:

    std::vector<glm::vec3> origins;
    std::vector<glm::vec3> results;
    DeformationForm(Scene * scene, QWidget *parent = nullptr):Form(parent){init();connect(scene);}

public slots:

    void init() {
        this->addMeshChooser("From", ObjectToChoose::GRID);
        this->addMeshChooser("To", ObjectToChoose::GRID);
        this->addTextEdit("PtToDeform", "Points to deform", true);
        this->addTextEdit("Result", "Result", false);
        this->addButton("Deform");
        this->addButton("Preview");
        this->addButton("Save images");
    }

    void update(Scene * scene) {
        Form::update(scene);
    }

    void show() {
        Form::show();
    }

    void extractPointsFromText(std::vector<glm::vec3>& points) {
        QTextDocument * doc = this->textEdits["PtToDeform"]->document();
        int nbLine = doc->lineCount();
        for(int i = 0; i < nbLine; ++i) {
            QString line = doc->findBlockByLineNumber(i).text();
            line.replace(".", ",");
            QStringList toRemove{"[", "]", "(", ")"};
            for(const QString& stringToRemove : toRemove) {
                line.remove(stringToRemove);
            }
            QStringList separators{";", " ", "; "};
            for(const QString& separator : separators) {
                QStringList parsed = line.split(separator);
                if(parsed.length() == 3) {
                    glm::vec3 point;
                    point.x = std::atof(parsed[0].toStdString().c_str());
                    point.y = std::atof(parsed[1].toStdString().c_str());
                    point.z = std::atof(parsed[2].toStdString().c_str());
                    points.push_back(point);
                    break;
                }
            }
        }
    }

    void convertPoints(Scene * scene) {
        this->results.clear();
        this->origins.clear();
        this->extractPointsFromText(origins);
        QString result;
        for(auto& pt : this->origins) {
            glm::vec3 newPt = scene->getTransformedPoint(pt, this->objectChoosers["From"]->currentText().toStdString(), this->objectChoosers["To"]->currentText().toStdString());
            QString line;
            line += "[ ";
            line += std::to_string(newPt.x).c_str();
            line += " ";
            line += std::to_string(newPt.y).c_str();
            line += " ";
            line += std::to_string(newPt.z).c_str();
            line += " ]\n";
            result += line;
            this->results.push_back(newPt);
        }
        this->textEdits["Result"]->clear();
        this->textEdits["Result"]->setPlainText(result);
    }

    std::string getFromGridName() {
        return this->objectChoosers["From"]->currentText().toStdString();
    }

    std::string getToGridName() {
        return this->objectChoosers["To"]->currentText().toStdString();
    }

    void connect(Scene * scene) {
        QObject::connect(this->buttons["Deform"], &QPushButton::clicked, [this, scene](){this->convertPoints(scene);});
        QObject::connect(this->buttons["Preview"], &QPushButton::clicked, [this, scene](){
                scene->writeImageWithPoints("previewFrom.tiff", this->getFromGridName(), this->origins);
                scene->writeImageWithPoints("previewTo.tiff", this->getToGridName(), this->results);
        });
        QObject::connect(this->buttons["Save images"], &QPushButton::clicked, [this, scene](){
            scene->writeDeformation(this->getFromGridName(), this->getToGridName());
        });
    }
};

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
        //this->info_position_data->setStyleSheet("font: 9pt;");

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

class QActionManager : QWidget {
    Q_OBJECT
public:
    std::map<std::string, QAction *> actions;
    std::map<std::string, QActionGroup *> actionExclusiveGroups;

    std::map<std::string, QStringList> actionGroups;

    QAction * getAction(const QString& name) {
        return actions[name.toStdString()];
    }

    void activateGroup(const QString& name) {
        this->showGroup(name);
        this->enableGroup(name);
    }

    void deactivateGroup(const QString& name) {
        this->hideGroup(name);
        this->disableGroup(name);
    }

    void hideGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setVisible(false);
        }
    }

    void showGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setVisible(true);
        }
    }

    void disableGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setEnabled(false);
        }
    }

    void enableGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setEnabled(true);
        }
    }

    void createQActionGroup(const QString& name, const QStringList& actionNames, bool exclusive = false) {
        this->actionGroups[name.toStdString()] = actionNames;
    }

    void createQExclusiveActionGroup(const QString& name, const QStringList& actionNames) {
        this->actionExclusiveGroups[name.toStdString()] = new QActionGroup(this);
        this->actionExclusiveGroups[name.toStdString()]->setExclusive(true);
        for(int i = 0; i < actionNames.size(); ++i) {
            if(this->actions.count(actionNames.at(i).toStdString())) {
                this->actionExclusiveGroups[name.toStdString()]->addAction(actions[actionNames.at(i).toStdString()]);
            } else {
                std::cout << "ERROR: the action [" << actionNames.at(i).toStdString() << "] doesn't exist !" << std::endl;
                throw std::runtime_error("[ERROR] wrong action name in the function createQActionGroup.");
            }
        }
    }

    QAction * createQAction(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon, bool checkable, bool checked) {
        QIcon icon;
        QSize size(80, 80);
        if(!defaultIcon.isEmpty())
            icon.addFile(QString("../resources/" + defaultIcon + QString(".svg")), size, QIcon::Normal, QIcon::Off);
        if(!pressedIcon.isEmpty())
            icon.addFile(QString("../resources/" + pressedIcon + QString(".svg")), size, QIcon::Normal, QIcon::On);
    
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

class DisplayPannel : public QGroupBox {
    Q_OBJECT
public:
    DisplayPannel(const QString &title, QActionManager& actionManager, QWidget *parent = nullptr): QGroupBox(title, parent){init();connect(actionManager);}

    QVBoxLayout * mainLayout;
    QToolBar * toolBar;

public slots:
    void init() {
        this->mainLayout = new QVBoxLayout(this);
        this->mainLayout->setAlignment(Qt::AlignHCenter);
        //QFont font = this->font();
        //font.setPointSize(8);
        //this->setFont(font);

        this->toolBar = new QToolBar(this);
        this->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        //this->toolBar->setIconSize(QSize(150, 150));

        this->mainLayout->addWidget(this->toolBar);
        //this->mainLayout->addWidget(this->toolBarRow2);
    };
    void connect(QActionManager& actionManager) {
        toolBar->addAction(actionManager.getAction("ToggleDisplayMesh"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayGrid"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayPlanarViewers"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayWireframe"));
    };
};

class ToolPannel : public QGroupBox {
    Q_OBJECT

public:
    
    ToolPannel(QWidget *parent = nullptr):QGroupBox(parent){init();}
    ToolPannel(const QString &title, QActionManager& actionManager, QWidget *parent = nullptr): QGroupBox(title, parent){init();connect(actionManager);}

    QVBoxLayout * main_layout;

    QToolBar * toolbar;

    QToolBar * move_toolbar;
    QToolBar * arap_toolbar;
    QToolBar * fixedRegistration_toolbar;

public slots:
    void init(){
        this->main_layout = new QVBoxLayout(this);
        //this->main_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        this->toolbar = new QToolBar(this);
        this->toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        this->main_layout->addWidget(this->toolbar);
        this->main_layout->setAlignment(Qt::AlignHCenter);
        this->main_layout->addStretch();
    };

    void connect(QActionManager& actionManager) {
        // Spacer
        this->toolbar->addAction(actionManager.getAction("MoveTool_toggleEvenMode"));
        this->toolbar->addAction(actionManager.getAction("MoveTool_toggleMoveCage"));
        this->toolbar->addAction(actionManager.getAction("MoveTool_reset"));

        this->toolbar->addAction(actionManager.getAction("ARAPTool_moveMode"));
        this->toolbar->addAction(actionManager.getAction("ARAPTool_handleMode"));
        this->toolbar->addAction(actionManager.getAction("ARAPTool_toggleEvenMode"));
        
        this->toolbar->addAction(actionManager.getAction("FixedTool_apply"));
        this->toolbar->addAction(actionManager.getAction("FixedTool_clear"));
    };
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

    CutPlaneGroupBox* cutPlane_pannel;
    DisplayPannel* display_pannel;
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

    DeformationForm * deformationForm;

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

    void changeCurrentTool(UITool::MeshManipulatorType newTool) {
        this->actionManager->deactivateGroup("MoveTool");
        this->actionManager->deactivateGroup("ARAPTool");
        this->actionManager->deactivateGroup("FixedTool");
        switch(newTool) {
            case UITool::MeshManipulatorType::POSITION:
                this->actionManager->activateGroup("MoveTool");
                break;
            case UITool::MeshManipulatorType::DIRECT:
                break;
            case UITool::MeshManipulatorType::ARAP:
                this->actionManager->activateGroup("ARAPTool");
                break;
            case UITool::MeshManipulatorType::FIXED_REGISTRATION:
                this->actionManager->activateGroup("FixedTool");
                break;
        }
    }

    void changeActiveMesh() {
        this->actionManager->getAction("ToggleNoneTool")->activate(QAction::Trigger);
    }

    void initialize() {
        if(this->scene->demos.isDemo) {
            this->scene->changeActiveMesh(this->combo_mesh->itemText(this->combo_mesh->currentIndex()).toStdString());
            this->changeActiveMesh();
        }
    }
};

#endif	  // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
