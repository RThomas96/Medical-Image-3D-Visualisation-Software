#include "../include/scene_control.hpp"
#include "../include/scene.hpp"
#include "../include/viewer.hpp"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

ControlPanel::ControlPanel(Scene* const scene, Viewer* lv, Viewer* rv, QWidget* parent) : QWidget(parent), sceneToControl(scene), leftViewer(lv), rightViewer(rv) {
	// Create sliders :
	this->xSlider = new QSlider(Qt::Horizontal);
	this->ySlider = new QSlider(Qt::Horizontal);
	this->zSlider = new QSlider(Qt::Horizontal);
	// Create checkbox :
	this->toggleTexCubeCheckbox = new QCheckBox("Toogle texture cube");
	// Create the container widget :
	this->controlContainer = new QWidget();

	// Create labels for sliders :
	QLabel* xSliderLabel = new QLabel("X coordinate");
	QLabel* ySliderLabel = new QLabel("Y coordinate");
	QLabel* zSliderLabel = new QLabel("Z coordinate");

	// Create containers layouts :
	QVBoxLayout* xContainer = new QVBoxLayout();
	QVBoxLayout* yContainer = new QVBoxLayout();
	QVBoxLayout* zContainer = new QVBoxLayout();
	QHBoxLayout* topContainer = new QHBoxLayout(this->controlContainer);
	xContainer->addWidget(xSliderLabel);
	xContainer->addWidget(this->xSlider);

	yContainer->addWidget(ySliderLabel);
	yContainer->addWidget(this->ySlider);

	zContainer->addWidget(zSliderLabel);
	zContainer->addWidget(this->zSlider);

	topContainer->addLayout(xContainer);
	topContainer->addLayout(yContainer);
	topContainer->addLayout(zContainer);
	topContainer->addWidget(this->toggleTexCubeCheckbox);

	// Disable by default the top level container :
	this->controlContainer->setEnabled(false);

	this->initSignals();

	this->xSlider->setMinimum(0);
	this->xSlider->setMaximum(1);
	this->xSlider->setValue(0);
	this->ySlider->setMinimum(0);
	this->ySlider->setMaximum(1);
	this->ySlider->setValue(0);
	this->zSlider->setMinimum(0);
	this->zSlider->setMaximum(1);
	this->zSlider->setValue(0);

	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(this->controlContainer);

	this->setLayout(mainLayout);

	this->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
}

void ControlPanel::initSignals() {
	connect(this->xSlider, &QSlider::valueChanged, this, &ControlPanel::setXCoord);
	connect(this->ySlider, &QSlider::valueChanged, this, &ControlPanel::setYCoord);
	connect(this->zSlider, &QSlider::valueChanged, this, &ControlPanel::setZCoord);
	connect(this->toggleTexCubeCheckbox, &QCheckBox::clicked, this, &ControlPanel::setTexCube);
}

void ControlPanel::activatePanels(bool activeStatus) {
	this->controlContainer->setEnabled(activeStatus);
	this->update();
}

void ControlPanel::setImageBoundaries(int bounds[6]) {
	// get old slider values :
	int oldXValue = this->xSlider->value();
	int oldYValue = this->ySlider->value();
	int oldZValue = this->zSlider->value();

	if (bounds[1] == 0) {
		this->controlContainer->setEnabled(false);
	} else {
		this->controlContainer->setEnabled(true);
	}

	// Sets the range of the sliders based on image size :
	// (also, set the value to min or max if it goes out of bounds)

	this->xSlider->setMinimum(bounds[0]);
	this->xSlider->setMaximum(bounds[1]);
	if (oldXValue < this->xSlider->minimum()) { this->xSlider->setValue(this->xSlider->minimum()); }
	else if (oldXValue > this->xSlider->maximum()) { this->xSlider->setValue(this->xSlider->maximum()); }

	this->ySlider->setMinimum(bounds[2]);
	this->ySlider->setMaximum(bounds[3]);
	if (oldYValue < this->ySlider->minimum()) { this->ySlider->setValue(this->ySlider->minimum()); }
	else if (oldYValue > this->ySlider->maximum()) { this->ySlider->setValue(this->ySlider->maximum()); }

	this->zSlider->setMinimum(bounds[4]);
	this->zSlider->setMaximum(bounds[5]);
	if (oldZValue < this->zSlider->minimum()) { this->zSlider->setValue(this->zSlider->minimum()); }
	else if (oldZValue > this->zSlider->maximum()) { this->zSlider->setValue(this->zSlider->maximum()); }
}

void ControlPanel::setXCoord(int coordX) {
	this->sceneToControl->slotSetTextureXCoord(coordX);
	this->leftViewer->update();
	this->rightViewer->update();
}

void ControlPanel::setYCoord(int coordY) {
	this->sceneToControl->slotSetTextureXCoord(coordY);
	this->leftViewer->update();
	this->rightViewer->update();
}

void ControlPanel::setZCoord(int coordZ) {
	this->sceneToControl->slotSetTextureXCoord(coordZ);
	this->leftViewer->update();
	this->rightViewer->update();
}

void ControlPanel::setTexCube(bool show) {
	this->sceneToControl->slotToggleShowTextureCube(show);
	this->leftViewer->update();
	this->rightViewer->update();
}
