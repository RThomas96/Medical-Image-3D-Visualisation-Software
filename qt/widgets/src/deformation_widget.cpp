#include "../include/deformation_widget.hpp"

#include <glm/gtx/io.hpp>

#include <QCoreApplication>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

#include <iomanip>

GridDeformationWidget::GridDeformationWidget(Scene* scene, QWidget* parent) :
	QWidget(parent) {

    this->setWindowFlags(Qt::WindowStaysOnTopHint);

	this->group_mesh = new QGroupBox("Objects");
	this->radio_mesh_grid_1 = new QRadioButton("Grid 1");
	this->radio_mesh_grid_1->setChecked(true);
	this->radio_mesh_grid_2 = new QRadioButton("Grid 2");
	this->radio_mesh_grid_2->setChecked(false);
    this->combo_mesh = new QComboBox();

	this->group_selector = new QGroupBox("Tools");
	this->radio_selector_direct = new QRadioButton("Direct");
	this->radio_selector_direct->setChecked(true);
	this->radio_selector_free = new QRadioButton("Free");
	this->radio_selector_free->setChecked(false);
	this->radio_selector_position = new QRadioButton("Position");
	this->radio_selector_position->setChecked(false);
    this->bindMove = new QPushButton("Link to mesh");
    this->bindMove->setCheckable(true);
    this->bindMove->setChecked(true);
    this->bindMove->hide();
	this->radio_selector_comp = new QRadioButton("Registration");
	this->radio_selector_comp->setChecked(false);
    this->combo_mesh_register = new QComboBox();
    this->selection_mode_register = new QPushButton("Start");
    this->validate = new QPushButton("Validate");
    this->undo = new QPushButton("Undo");
    this->clear = new QPushButton("Clear");
    this->apply = new QPushButton("Apply");

	this->radio_selector_ARAP = new QRadioButton("ARAP");
	this->radio_selector_ARAP->setChecked(false);

	this->group_move = new QGroupBox("Deformation");
	this->radio_move_normal = new QRadioButton("Normal");
	this->radio_move_normal->setChecked(true);
	this->radio_move_weighted = new QRadioButton("Weighted");
	this->radio_move_weighted->setChecked(false);
	this->radio_move_ARAP = new QRadioButton("ARAP");
	this->radio_move_ARAP->setChecked(false);

	this->label_radius_selection = new QLabel("Selection radius");
	this->spinbox_radius_selection = new QDoubleSpinBox;
    this->spinbox_radius_selection->setValue(30.);
	this->spinbox_radius_selection->setRange(0., 20000.);
	this->spinbox_radius_selection->setSingleStep(10.);
	this->label_radius_selection->hide();
	this->spinbox_radius_selection->hide();

	this->group_visu = new QGroupBox("Options");
	this->label_radius_sphere = new QLabel("Radius");
	this->spinbox_radius_sphere = new QDoubleSpinBox;
    this->spinbox_radius_sphere->setValue(5.);
	this->spinbox_radius_sphere->setRange(0., 9999.);
	this->spinbox_radius_sphere->setSingleStep(1.);

	this->checkbox_wireframe = new QCheckBox;
    this->checkbox_wireframe->setChecked(true);
    this->checkbox_wireframe->setText("Wireframe");

    this->handleMode = new QPushButton("Handle mode", this);
    this->handleMode->setCheckable(true);
    //this->debug_it = new QPushButton("&ITERATION", this);
    //this->debug_it->setAutoRepeat(true);
    //this->debug_init = new QPushButton("&INIT", this);

	//this->spinbox_l_selection = new QDoubleSpinBox;
    //this->spinbox_l_selection->setValue(0.);
	//this->spinbox_l_selection->setRange(0., 20000.);
	//this->spinbox_l_selection->setSingleStep(1.);

	//this->spinbox_N_selection = new QDoubleSpinBox;
    //this->spinbox_N_selection->setValue(0.);
	//this->spinbox_N_selection->setRange(0., 20000.);
	//this->spinbox_N_selection->setSingleStep(1.);

	//this->spinbox_S_selection = new QDoubleSpinBox;
    //this->spinbox_S_selection->setValue(0.);
	//this->spinbox_S_selection->setRange(0., 20000.);
	//this->spinbox_S_selection->setSingleStep(1.);

    this->setupLayouts();
	this->setupSignals(scene);
}

void GridDeformationWidget::setupLayouts() {

    this->mainLayout = new QVBoxLayout;

	layout_mesh     = new QVBoxLayout;
	layout_selector = new QVBoxLayout;
	layout_move     = new QVBoxLayout;
	layout_visu     = new QVBoxLayout;

	this->group_mesh->setLayout(this->layout_mesh);
	this->group_selector->setLayout(this->layout_selector);
	this->group_move->setLayout(this->layout_move);
	this->group_visu->setLayout(this->layout_visu);

	this->layout_mesh->addWidget(this->combo_mesh, 1);
	//this->layout_mesh->addWidget(this->radio_mesh_grid_1, 1);
	//this->layout_mesh->addWidget(this->radio_mesh_grid_2, 2);

	this->layout_selector->addWidget(this->radio_selector_direct, 1);
	this->layout_selector->addWidget(this->radio_selector_free, 2);
	this->layout_selector->addWidget(this->radio_selector_position, 3);
	this->layout_selector->addWidget(this->bindMove, 4);
	this->layout_selector->addWidget(this->radio_selector_comp, 5);
	this->layout_selector->addWidget(this->combo_mesh_register, 6);
	this->layout_selector->addWidget(this->selection_mode_register, 7);
	this->layout_selector->addWidget(this->validate, 8);
	this->layout_selector->addWidget(this->undo, 9);
	this->layout_selector->addWidget(this->clear, 10);
	this->layout_selector->addWidget(this->apply, 11);
	this->layout_selector->addWidget(this->radio_selector_ARAP, 12);
    this->layout_selector->addWidget(this->handleMode, 13);
    this->handleMode->hide();
    this->bindMove->hide();

    this->combo_mesh_register->hide();
    this->selection_mode_register->hide();
    this->validate->hide();
    this->undo->hide();
    this->clear->hide();
    this->apply->hide();

	this->layout_move->addWidget(this->radio_move_normal, 1);
	this->layout_move->addWidget(this->radio_move_weighted, 2);
    this->layout_move->addWidget(this->label_radius_selection);
    this->layout_move->addWidget(this->spinbox_radius_selection);
	this->layout_move->addWidget(this->radio_move_ARAP, 3);

    this->layout_visu->addWidget(this->label_radius_sphere, 1);
    this->layout_visu->addWidget(this->spinbox_radius_sphere, 2);
    this->layout_visu->addWidget(this->checkbox_wireframe, 3);

    this->mainLayout->addWidget(this->group_mesh);
    this->mainLayout->addWidget(this->group_selector);
    this->mainLayout->addWidget(this->group_move);
    this->mainLayout->addWidget(this->group_visu);

    this->setLayout(this->mainLayout);
}

GridDeformationWidget::~GridDeformationWidget() {
}

void GridDeformationWidget::updateScene(Scene * scene, int meshTool, int moveMethod, bool activeMeshChanged) {
    if(this->combo_mesh->count() <= 0)
        return;
    
    std::string currentMeshName = std::string((this->combo_mesh->itemText(this->combo_mesh->currentIndex())).toStdString());

    bool isGrid = this->gridOrCage[this->combo_mesh->currentIndex()].first;
    bool isMesh = !isGrid;
    bool isCage = this->gridOrCage[this->combo_mesh->currentIndex()].second;

    this->registrationInitialize = false;

    if(activeMeshChanged) {
        //Get back to reset position
        this->radio_selector_direct->setChecked(true);
        this->radio_selector_free->setChecked(false);
        this->radio_selector_position->setChecked(false);
        this->radio_selector_comp->setChecked(false);
        this->radio_selector_ARAP->setChecked(false);
        this->radio_move_normal->setChecked(true);
        this->radio_move_weighted->setChecked(false);
        this->radio_move_ARAP->setChecked(false);

        meshTool = 0;
        moveMethod = 0;
    }

    // Lock/Unlock features
    this->radio_mesh_grid_1->setEnabled(true);
    this->radio_mesh_grid_2->setEnabled(true);
    this->radio_selector_direct->setEnabled(true);
    this->radio_selector_free->setEnabled(true);
    this->radio_selector_position->setEnabled(true);
    this->radio_selector_comp->setEnabled(true);
    this->radio_selector_ARAP->setEnabled(true);
    this->radio_move_normal->setEnabled(true);
    this->radio_move_weighted->setEnabled(true);
    this->radio_move_ARAP->setEnabled(true);
    this->handleMode->hide();
    this->bindMove->hide();
	this->label_radius_selection->hide();
	this->spinbox_radius_selection->hide();
    this->combo_mesh_register->hide();
    this->selection_mode_register->hide();
    this->validate->hide();
    this->undo->hide();
    this->clear->hide();
    this->apply->hide();

    if(isCage) {
        this->bindMove->show();
        this->bindMove->setChecked(true);
        scene->setBindMeshToCageMove(currentMeshName, true);
    }
    if(isMesh) {
        this->radio_selector_free->setEnabled(false);
        this->radio_selector_comp->setEnabled(false);
        if(moveMethod == 2 || (this->currentMeshTool != 4 && meshTool == 4)) {
            this->currentMeshTool = 4;
            this->currentMoveMethod = 2;
            this->radio_move_ARAP->setEnabled(true);
            this->radio_selector_ARAP->setEnabled(true);
            this->handleMode->show();

            this->radio_move_ARAP->setChecked(true);
            this->radio_selector_ARAP->setChecked(true);
            this->radio_move_normal->setEnabled(false);
            this->radio_move_weighted->setEnabled(false);
            this->radio_move_ARAP->setEnabled(false);
        }
        if(this->currentMeshTool == 4 && meshTool != 4 && moveMethod == -1) {
            this->currentMoveMethod = 0;
            this->radio_move_normal->setEnabled(true);
            this->radio_move_weighted->setEnabled(true);
            this->radio_move_ARAP->setEnabled(true);
            this->radio_move_normal->setChecked(true);
            this->radio_move_weighted->setChecked(false);
            this->radio_move_ARAP->setChecked(false);
        }
    } else {
        this->radio_selector_ARAP->setEnabled(false);
	    //this->radio_selector_position->setEnabled(false);

        if(meshTool == 3) {
            this->combo_mesh_register->show();
            this->selection_mode_register->show();
            this->validate->show();
            this->undo->show();
            this->clear->show();
            this->apply->show();
        }
    }

    scene->changeActiveMesh(currentMeshName);

    if(meshTool >= 0) {
        scene->updateTools(meshTool);
        this->currentMeshTool = meshTool;
    }
    if(moveMethod >= 0) {
        switch(moveMethod) {
            case 0:
                scene->setNormalDeformationMethod(currentMeshName);
                break;

            case 1:
                this->label_radius_selection->show(); 
                this->spinbox_radius_selection->show(); 
                scene->setWeightedDeformationMethod(currentMeshName, this->spinbox_radius_selection->value());
                break;

            case 2:
                scene->setARAPDeformationMethod(currentMeshName);
                break;

            default:
                std::cout << "Unkown move method" << std::endl;
        }
        this->currentMoveMethod = moveMethod;
    }
}

void GridDeformationWidget::setupSignals(Scene * scene) {

    QObject::connect(this->combo_mesh, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){this->updateScene(scene, -1, -1, true);});

	QObject::connect(this->radio_selector_direct, &QPushButton::clicked, this, [this, scene]() {this->updateScene(scene, 0, -1);});

	QObject::connect(this->radio_selector_free, &QPushButton::clicked, this, [this, scene]() {this->updateScene(scene, 1, -1);});

	QObject::connect(this->radio_selector_position, &QPushButton::clicked, this, [this, scene]() {this->updateScene(scene, 2, -1);});

	QObject::connect(this->radio_selector_comp, &QPushButton::clicked, this, [this, scene]() {this->updateScene(scene, 3, -1);});

	QObject::connect(this->radio_selector_ARAP, &QPushButton::clicked, this, [this, scene]() {this->updateScene(scene, 4, 2);});

	QObject::connect(this->radio_move_normal, &QPushButton::clicked, this, [this, scene]() {this->updateScene(scene, -1, 0);});

	QObject::connect(this->radio_move_weighted, &QPushButton::clicked, this, [this, scene]() {this->updateScene(scene, -1, 1);});

	QObject::connect(this->radio_move_ARAP, &QPushButton::clicked, this, [this, scene]() {this->updateScene(scene, -1, 2);});

    /***/

    QObject::connect(this->handleMode, &QPushButton::released, this, [this, scene]() {scene->toggleARAPManipulatorMode();});

    QObject::connect(this->bindMove, &QPushButton::released, this, [this, scene]() {scene->toggleBindMeshToCageMove(std::string((this->combo_mesh->itemText(this->combo_mesh->currentIndex())).toStdString()));});

	QObject::connect(this->spinbox_radius_sphere, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double i){ scene->setManipulatorRadius(i);}); 

	QObject::connect(this->checkbox_wireframe, &QPushButton::clicked, this, [this, scene]() {scene->toggleWireframe();});

    /***/

	QObject::connect(scene, &Scene::meshAdded, this, &GridDeformationWidget::addNewMesh);

    /***/

    QObject::connect(this->combo_mesh_register, QOverload<int>::of(&QComboBox::highlighted), [=](int index){scene->assignMeshToRegisterRegistrationTool(std::string((this->combo_mesh_register->itemText(this->combo_mesh_register->currentIndex())).toStdString()));});
    //QObject::connect(this->combo_mesh_register, &QComboBox::textActivated, [this, scene](const QString& text){scene->assignMeshToRegisterRegistrationTool(text.toStdString());});

	QObject::connect(this->selection_mode_register, &QPushButton::clicked, this, [this, scene]() {
            if(!this->registrationInitialize) {
                scene->assignMeshToRegisterRegistrationTool(std::string((this->combo_mesh_register->itemText(this->combo_mesh_register->currentIndex())).toStdString()));
                this->registrationInitialize = true;
            }
                scene->switchToSelectionModeRegistrationTool();
    });

	QObject::connect(this->validate, &QPushButton::clicked, this, [this, scene]() {scene->validateRegistrationTool();});

	QObject::connect(this->undo, &QPushButton::clicked, this, [this, scene]() {scene->undoRegistrationTool();});

	QObject::connect(this->clear, &QPushButton::clicked, this, [this, scene]() {scene->clearRegistrationTool();});

	QObject::connect(this->apply, &QPushButton::clicked, this, [this, scene]() {
            scene->applyRegistrationTool();
            this->updateScene(scene, 3, -1);
            scene->assignMeshToRegisterRegistrationTool(std::string((this->combo_mesh_register->itemText(this->combo_mesh_register->currentIndex())).toStdString()));
            this->registrationInitialize = true;
    });
}
