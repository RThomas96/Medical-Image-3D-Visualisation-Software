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

	this->button_selection = new QPushButton("Toggle selection mode");
	this->button_update = new QPushButton("Update camera");
	this->button_alignARAP = new QPushButton("Align ARAP");
	this->button_launchARAP = new QPushButton("Launch ARAP");
	this->button_vaoState = new QPushButton("VAO GL info (stderr)");
	this->button_select_all = new QPushButton("Select all vertices");
	this->button_unselect_all = new QPushButton("Unselect all vertices");
	this->button_reset_arap = new QPushButton("Reset ARAP constraints");
	this->button_enable_def = new QPushButton("Enable deformation");

	QDoubleSpinBox* sphere_size_slider = new QDoubleSpinBox;
	sphere_size_slider->setMaximum(1.e15);
	sphere_size_slider->setMinimum(1.e-3);

	QObject::connect(this->button_selection, &QPushButton::pressed, this->viewer, &Viewer::toggleSelectionMode);
	QObject::connect(this->button_update, &QPushButton::pressed, this->viewer, &Viewer::updateCameraPosition);
	QObject::connect(this->button_alignARAP, &QPushButton::pressed, this->viewer, &Viewer::alignARAP);
	QObject::connect(this->button_launchARAP, &QPushButton::pressed, this->viewer, &Viewer::launchARAP);
	QObject::connect(this->button_vaoState, &QPushButton::pressed, this->viewer, &Viewer::printVAOStateNext);
	QObject::connect(this->button_select_all, &QPushButton::pressed, this->viewer, &Viewer::mesh_select_all);
	QObject::connect(this->button_unselect_all, &QPushButton::pressed, this->viewer, &Viewer::mesh_unselect_all);
	QObject::connect(this->button_reset_arap, &QPushButton::pressed, this->viewer, &Viewer::resetARAPConstraints);
	QObject::connect(this->button_enable_def, &QPushButton::pressed, this->viewer, &Viewer::toggleDeformation);
	QObject::connect(sphere_size_slider, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this->viewer, &Viewer::setSphereSize);
	// Connect the button press of 'Enable deformation' with the actual action of enabling the deformation :
	QObject::connect(this->viewer, &Viewer::enableDeformationPanel, this, &ViewerHelper::toggleDeformationButtons);

	buttons->addWidget(header);
	buttons->addWidget(this->button_selection);
	buttons->addWidget(this->button_update);
	buttons->addWidget(scene);
	buttons->addWidget(this->button_vaoState);
	buttons->addWidget(sphere_size_slider);
	buttons->addWidget(arap);
	buttons->addWidget(this->button_enable_def);
	buttons->addWidget(this->button_select_all);
	buttons->addWidget(this->button_unselect_all);
	buttons->addWidget(this->button_alignARAP);
	buttons->addWidget(this->button_launchARAP);
	buttons->addWidget(this->button_reset_arap);

	this->setLayout(buttons);

	this->toggleDeformationButtons(false);
}

void ViewerHelper::toggleDeformationButtons(bool should_enable) {
	this->button_select_all->setEnabled(should_enable);
	this->button_unselect_all->setEnabled(should_enable);
	this->button_alignARAP->setEnabled(should_enable);
	this->button_launchARAP->setEnabled(should_enable);
	this->button_reset_arap->setEnabled(should_enable);
}

void ViewerHelper::initSignals() {
}
