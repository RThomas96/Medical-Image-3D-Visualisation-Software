#include "../include/deformation_widget.hpp"

#include "../../../image/transforms/include/affine_transform.hpp"
#include "../../../image/transforms/include/transform_interface.hpp"
#include "../../../image/transforms/include/transform_stack.hpp"
#include "../../../image/transforms/include/trs_transform.hpp"
#include "../include/user_settings_widget.hpp"

#include <glm/gtx/io.hpp>

#include <QCoreApplication>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

#include <iomanip>

GridDeformationWidget::GridDeformationWidget(Scene* _scene, QWidget* parent) :
	QWidget(parent) {

	this->scene					= _scene;
    this->setWindowFlags(Qt::WindowStaysOnTopHint);

	this->group_selector = new QGroupBox("Selector mode");
	this->radio_selector_direct = new QRadioButton("Direct");
	this->radio_selector_direct->setChecked(true);
	this->radio_selector_free = new QRadioButton("Free");
	this->radio_selector_free->setChecked(false);

	this->group_move = new QGroupBox("Move method");
	this->radio_move_normal = new QRadioButton("Normal");
	this->radio_move_normal->setChecked(true);
	this->radio_move_weighted = new QRadioButton("Weighted");
	this->radio_move_weighted->setChecked(false);

	this->label_radius_selection = new QLabel("Selection radius");
	this->spinbox_radius_selection = new QDoubleSpinBox;
    this->spinbox_radius_selection->setValue(30.);
	this->spinbox_radius_selection->setRange(0., 20000.);
	this->spinbox_radius_selection->setSingleStep(10.);

	this->label_radius_sphere = new QLabel("Sphere radius");
	this->spinbox_radius_sphere = new QDoubleSpinBox;
    this->spinbox_radius_sphere->setValue(5.);
	this->spinbox_radius_sphere->setRange(0., 20000.);
	this->spinbox_radius_sphere->setSingleStep(1.);

	this->label_wireframe = new QLabel("Display wireframe");
	this->checkbox_wireframe = new QCheckBox;
    this->checkbox_wireframe->setChecked(true);

    this->setupLayouts();
	this->setupSignals();
}

void GridDeformationWidget::setupLayouts() {

    this->mainLayout = new QVBoxLayout;

	layout_selector = new QHBoxLayout;
	layout_move = new QHBoxLayout;

	this->group_selector->setLayout(this->layout_selector);
	this->group_move->setLayout(this->layout_move);

	this->layout_selector->addWidget(this->radio_selector_direct, 1);
	this->layout_selector->addWidget(this->radio_selector_free, 2);

	this->layout_move->addWidget(this->radio_move_normal, 1);
	this->layout_move->addWidget(this->radio_move_weighted, 2);

    this->mainLayout->addWidget(this->group_selector);
    this->mainLayout->addWidget(this->group_move);

    this->mainLayout->addWidget(this->label_radius_selection);
    this->mainLayout->addWidget(this->spinbox_radius_selection);

    this->mainLayout->addWidget(this->label_radius_sphere);
    this->mainLayout->addWidget(this->spinbox_radius_sphere);

    this->mainLayout->addWidget(this->label_wireframe);
    this->mainLayout->addWidget(this->checkbox_wireframe);

    this->setLayout(this->mainLayout);
}

GridDeformationWidget::~GridDeformationWidget() {
	this->scene	 = nullptr;
}

void GridDeformationWidget::setupSignals() {
	//if (this->scene == nullptr) {
	//	QMessageBox* msgBox = new QMessageBox;
	//	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	//	msgBox->critical(this, "Error : no scene associated", "An error has occured, and no scene was associated with this widget. Please retry again later.");
	//	return;
	//}

	//if (this->viewer == nullptr) {
	//	QMessageBox* msgBox = new QMessageBox;
	//	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	//	msgBox->critical(this, "Error : no viewer associated", "An error has occured, and no viewer was associated with this widget. Please retry again later.");
	//	return;
	//}

	QObject::connect(this->radio_selector_direct, &QPushButton::clicked, this, [this]() {this->scene->glMeshManipulator->createNewMeshManipulator(this->scene->grids[0]->grid->grid->tetmesh.ptGrid, 0);});

	QObject::connect(this->radio_selector_free, &QPushButton::clicked, this, [this]() {this->scene->glMeshManipulator->createNewMeshManipulator(this->scene->grids[0]->grid->grid->tetmesh.ptGrid, 1);});

	QObject::connect(this->radio_move_normal, &QPushButton::clicked, this, [this]() {this->scene->grids[0]->grid->grid->tetmesh.setNormalDeformationMethod();});

	QObject::connect(this->radio_move_weighted, &QPushButton::clicked, this, [this]() {this->scene->grids[0]->grid->grid->tetmesh.setWeightedDeformationMethod(this->spinbox_radius_selection->value());});

	//QObject::connect(this->spinbox_radius_sphere, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ this->scene->glMeshManipulator->setRadius(1.);}); 
	QObject::connect(this->spinbox_radius_sphere, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double i){ this->scene->glMeshManipulator->setRadius(i);}); 

	QObject::connect(this->checkbox_wireframe, &QPushButton::clicked, this, [this]() {this->scene->glMeshManipulator->toggleDisplayWireframe();});
}
