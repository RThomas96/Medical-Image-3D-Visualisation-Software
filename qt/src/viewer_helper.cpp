#include "../include/viewer_helper.hpp"

#include <QDoubleSpinBox>
#include <QFrame>
#include <iostream>

ViewerHelper::ViewerHelper(Viewer* _v, Scene* _s, QWidget* parent) : viewer(_v), scene(_s), QWidget(parent) {
	std::cerr << "Initializing viewer helper ...\n";
	this->init();
	this->initSignals();
	std::cerr << "Initialized viewer helper.\n";
}

void ViewerHelper::init() {
	QVBoxLayout* buttons = new QVBoxLayout;
	QLabel* header = new QLabel("Viewer helper");
	QLabel* scene = new QLabel("Scene");
	QLabel* arap = new QLabel("ARAP");

	QPushButton* button_selection = new QPushButton("Toggle selection mode");
	QPushButton* button_update = new QPushButton("Update camera");
	QPushButton* button_alignARAP = new QPushButton("Align ARAP");
	QPushButton* button_launchARAP = new QPushButton("Launch ARAP");
	QPushButton* button_vaoState = new QPushButton("VAO GL info (stderr)");
	QPushButton* button_draw_arap = new QPushButton("Draw ARAP manipulators");
	QPushButton* button_select_all = new QPushButton("Select all vertices");
	QPushButton* button_unselect_all = new QPushButton("Unselect all vertices");
	QPushButton* button_reset_arap = new QPushButton("Reset ARAP constraints");
	QPushButton* button_enable_def = new QPushButton("Enable deformation");
	QPushButton* button_save_mesh = new QPushButton("Save mesh to file (OFF)");
	QPushButton* button_save_curve = new QPushButton("Save curve to file (OBJ)");

	QDoubleSpinBox* sphere_size_slider = new QDoubleSpinBox;
	sphere_size_slider->setMaximum(1.e15);
	sphere_size_slider->setMinimum(1.e-3);
	sphere_size_slider->setSingleStep(1e-2);

	QFrame* sep0 = new QFrame;
	sep0->setFrameShape(QFrame::Shape::HLine);
	sep0->setFrameShadow(QFrame::Shadow::Sunken);
	sep0->setLineWidth(1);

	QFrame* sep1 = new QFrame;
	sep1->setFrameShape(QFrame::Shape::HLine);
	sep1->setFrameShadow(QFrame::Shadow::Sunken);
	sep1->setLineWidth(1);

	QObject::connect(button_selection, &QPushButton::pressed, this->viewer, &Viewer::toggleSelectionMode);
	QObject::connect(button_update, &QPushButton::pressed, this->viewer, &Viewer::updateCameraPosition);
	QObject::connect(button_alignARAP, &QPushButton::pressed, this->viewer, &Viewer::alignARAP);
	QObject::connect(button_launchARAP, &QPushButton::pressed, this->viewer, &Viewer::launchARAP);
	QObject::connect(button_vaoState, &QPushButton::pressed, this->viewer, &Viewer::printVAOStateNext);
	QObject::connect(button_draw_arap, &QPushButton::pressed, this->viewer, &Viewer::toggleDrawARAP);
	QObject::connect(button_select_all, &QPushButton::pressed, this->viewer, &Viewer::mesh_select_all);
	QObject::connect(button_unselect_all, &QPushButton::pressed, this->viewer, &Viewer::mesh_unselect_all);
	QObject::connect(sphere_size_slider, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this->viewer, &Viewer::setSphereSize);
	QObject::connect(button_reset_arap, &QPushButton::pressed, this->viewer, &Viewer::resetARAPConstraints);
	QObject::connect(button_enable_def, &QPushButton::pressed, this->viewer, &Viewer::toggleDeformation);
	QObject::connect(button_save_mesh, &QPushButton::pressed, this->scene, &Scene::dummy_save_mesh_to_file);
	QObject::connect(button_save_curve, &QPushButton::pressed, this->scene, &Scene::dummy_save_curve_to_file);

	buttons->addWidget(header);
	buttons->addWidget(button_selection);
	buttons->addWidget(button_update);
	buttons->addWidget(sep0);
	buttons->addWidget(scene);
	buttons->addWidget(button_vaoState);
	buttons->addWidget(button_draw_arap);
	buttons->addWidget(button_save_mesh);
	buttons->addWidget(button_save_curve);
	buttons->addWidget(sphere_size_slider);
	buttons->addWidget(sep1);
	buttons->addWidget(arap);
	buttons->addWidget(button_select_all);
	buttons->addWidget(button_unselect_all);
	buttons->addWidget(button_alignARAP);
	buttons->addWidget(button_launchARAP);
	buttons->addWidget(button_reset_arap);
	buttons->addWidget(button_enable_def);

	this->setLayout(buttons);
}

void ViewerHelper::initSignals() {
}
