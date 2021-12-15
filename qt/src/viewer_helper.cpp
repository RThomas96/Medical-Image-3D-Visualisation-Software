#include "../include/viewer_helper.hpp"

#include <QDoubleSpinBox>
#include <iostream>

ViewerHelper::ViewerHelper(Viewer* _v, QWidget* parent) : viewer(_v), QWidget(parent) {
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
	QPushButton* button_select_all = new QPushButton("Select all vertices");
	QPushButton* button_unselect_all = new QPushButton("Unselect all vertices");
	QPushButton* button_reset_arap = new QPushButton("Reset ARAP constraints");
	QPushButton* button_enable_def = new QPushButton("Enable deformation");

	QDoubleSpinBox* sphere_size_slider = new QDoubleSpinBox;
	sphere_size_slider->setMaximum(1.e15);
	sphere_size_slider->setMinimum(1.e-3);

	QObject::connect(button_selection, &QPushButton::pressed, this->viewer, &Viewer::toggleSelectionMode);
	QObject::connect(button_update, &QPushButton::pressed, this->viewer, &Viewer::updateCameraPosition);
	QObject::connect(button_alignARAP, &QPushButton::pressed, this->viewer, &Viewer::alignARAP);
	QObject::connect(button_launchARAP, &QPushButton::pressed, this->viewer, &Viewer::launchARAP);
	QObject::connect(button_vaoState, &QPushButton::pressed, this->viewer, &Viewer::printVAOStateNext);
	QObject::connect(button_select_all, &QPushButton::pressed, this->viewer, &Viewer::mesh_select_all);
	QObject::connect(button_unselect_all, &QPushButton::pressed, this->viewer, &Viewer::mesh_unselect_all);
	QObject::connect(sphere_size_slider, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this->viewer, &Viewer::setSphereSize);
	QObject::connect(button_reset_arap, &QPushButton::pressed, this->viewer, &Viewer::resetARAPConstraints);
	QObject::connect(button_enable_def, &QPushButton::pressed, this->viewer, &Viewer::toggleDeformation);

	buttons->addWidget(header);
	buttons->addWidget(button_selection);
	buttons->addWidget(button_update);
	buttons->addWidget(scene);
	buttons->addWidget(button_vaoState);
	buttons->addWidget(sphere_size_slider);
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
