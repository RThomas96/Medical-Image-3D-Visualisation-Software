#include "../include/scene_control.hpp"
#include "../include/scene.hpp"
#include "../include/viewer.hpp"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

ControlPanel::ControlPanel(Scene* const scene, Viewer* lv, Viewer* rv, QWidget* parent) : QWidget(parent), sceneToControl(scene), leftViewer(lv), rightViewer(rv) {
	// Create sliders :
	this->xPicker = new QDoubleSpinBox();
	this->yPicker = new QDoubleSpinBox();
	this->zPicker = new QDoubleSpinBox();
	this->xTex = new QSlider(Qt::Horizontal);
	this->yTex = new QSlider(Qt::Horizontal);
	this->zTex = new QSlider(Qt::Horizontal);
	// Create checkbox :
	this->toggleTexCubeCheckbox = new QCheckBox("Show texture cube");
	this->toggleTexCubeCheckbox->setCheckState(Qt::Unchecked);
	// Create the container widget :
	this->controlContainer = new QWidget();


	// Create labels for sliders :
	QLabel* xSliderLabel = new QLabel("X coordinate");
	QLabel* ySliderLabel = new QLabel("Y coordinate");
	QLabel* zSliderLabel = new QLabel("Z coordinate");
	QLabel* xTexLabel = new QLabel("X texture");
	QLabel* yTexLabel = new QLabel("Y texture");
	QLabel* zTexLabel = new QLabel("Z texture");

	// Create containers layouts :
	QVBoxLayout* xContainer = new QVBoxLayout();
	QVBoxLayout* yContainer = new QVBoxLayout();
	QVBoxLayout* zContainer = new QVBoxLayout();
	QHBoxLayout* topContainer = new QHBoxLayout(this->controlContainer);
	xContainer->addWidget(xSliderLabel);
	xContainer->addWidget(this->xPicker);
	xContainer->addWidget(xTexLabel);
	xContainer->addWidget(this->xTex);

	yContainer->addWidget(ySliderLabel);
	yContainer->addWidget(this->yPicker);
	yContainer->addWidget(yTexLabel);
	yContainer->addWidget(this->yTex);

	zContainer->addWidget(zSliderLabel);
	zContainer->addWidget(this->zPicker);
	zContainer->addWidget(zTexLabel);
	zContainer->addWidget(this->zTex);

	topContainer->addLayout(xContainer);
	topContainer->addLayout(yContainer);
	topContainer->addLayout(zContainer);
	topContainer->addWidget(this->toggleTexCubeCheckbox);

	// Disable by default the top level container :
	this->controlContainer->setEnabled(false);

	this->initSignals();

	this->xPicker->setMinimum(0);
	this->xPicker->setMaximum(1e9);
	this->xPicker->setValue(0);
	this->xPicker->setDecimals(0);
	this->xPicker->setStepType(QAbstractSpinBox::StepType::DefaultStepType);
	this->xPicker->setSingleStep(1.0);

	this->yPicker->setMinimum(0);
	this->yPicker->setMaximum(1e9);
	this->yPicker->setValue(0);
	this->yPicker->setDecimals(0);
	this->yPicker->setStepType(QAbstractSpinBox::StepType::DefaultStepType);
	this->yPicker->setSingleStep(1.0);

	this->zPicker->setMinimum(0);
	this->zPicker->setMaximum(1e9);
	this->zPicker->setValue(0);
	this->zPicker->setDecimals(0);
	this->zPicker->setStepType(QAbstractSpinBox::StepType::DefaultStepType);
	this->zPicker->setSingleStep(1.0);

	this->xTex->setMinimum(0);
	this->xTex->setMaximum(100);
	this->xTex->setValue(0);

	this->yTex->setMinimum(0);
	this->yTex->setMaximum(100);
	this->yTex->setValue(0);

	this->zTex->setMinimum(0);
	this->zTex->setMaximum(100);
	this->zTex->setValue(0);

	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(this->controlContainer);

	this->setLayout(mainLayout);

	this->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
}

void ControlPanel::initSignals() {
	connect(this->xPicker, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControlPanel::setXCoord);
	connect(this->yPicker, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControlPanel::setYCoord);
	connect(this->zPicker, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControlPanel::setZCoord);
	connect(this->xTex, &QSlider::valueChanged, this, &ControlPanel::setXTexCoord);
	connect(this->yTex, &QSlider::valueChanged, this, &ControlPanel::setYTexCoord);
	connect(this->zTex, &QSlider::valueChanged, this, &ControlPanel::setZTexCoord);
	connect(this->toggleTexCubeCheckbox, &QCheckBox::clicked, this, &ControlPanel::setTexCube);
}

void ControlPanel::activatePanels(bool activeStatus) {
	this->controlContainer->setEnabled(activeStatus);
	if (activeStatus) {
		this->controlContainer->show();
	} else {
		this->controlContainer->hide();
	}
	this->update();
}

void ControlPanel::setImageBoundaries(int bounds[6]) {
	// get old slider values :
	int oldXValue = this->xTex->value();
	int oldYValue = this->yTex->value();
	int oldZValue = this->zTex->value();

	if (bounds[1] == 0) {
		this->controlContainer->setEnabled(false);
		this->controlContainer->hide();
	} else {
		this->controlContainer->setEnabled(true);
		this->controlContainer->show();
	}

	// Sets the range of the sliders based on image size :
	// (also, set the value to min or max if it goes out of bounds)

	this->xTex->setMinimum(bounds[0]);
	this->xTex->setMaximum(bounds[1]);
	if (oldXValue < this->xTex->minimum()) { this->xTex->setValue(this->xTex->minimum()); }
	else if (oldXValue > this->xTex->maximum()) { this->xTex->setValue(this->xTex->maximum()); }

	this->yTex->setMinimum(bounds[2]);
	this->yTex->setMaximum(bounds[3]);
	if (oldYValue < this->yTex->minimum()) { this->yTex->setValue(this->yTex->minimum()); }
	else if (oldYValue > this->yTex->maximum()) { this->yTex->setValue(this->yTex->maximum()); }

	this->zTex->setMinimum(bounds[4]);
	this->zTex->setMaximum(bounds[5]);
	if (oldZValue < this->zTex->minimum()) { this->zTex->setValue(this->zTex->minimum()); }
	else if (oldZValue > this->zTex->maximum()) { this->zTex->setValue(this->zTex->maximum()); }
}

void ControlPanel::setXCoord(double coordX) {
	int co = static_cast<float>(coordX);
	this->sceneToControl->slotSetNeighborXCoord(co);
	this->leftViewer->update();
	this->rightViewer->update();
}

void ControlPanel::setYCoord(double coordY) {
	int co = static_cast<float>(coordY);
	this->sceneToControl->slotSetNeighborYCoord(co);
	this->leftViewer->update();
	this->rightViewer->update();
}

void ControlPanel::setZCoord(double coordZ) {
	int co = static_cast<float>(coordZ);
	this->sceneToControl->slotSetNeighborZCoord(co);
	this->leftViewer->update();
	this->rightViewer->update();
}

void ControlPanel::setXTexCoord(int coordX) {
	uint co = static_cast<uint>(coordX);
	this->sceneToControl->slotSetTextureXCoord(co);
	this->leftViewer->update();
	this->rightViewer->update();
}

void ControlPanel::setYTexCoord(int coordY) {
	uint co = static_cast<uint>(coordY);
	this->sceneToControl->slotSetTextureYCoord(co);
	this->leftViewer->update();
	this->rightViewer->update();
}

void ControlPanel::setZTexCoord(int coordZ) {
	uint co = static_cast<uint>(coordZ);
	this->sceneToControl->slotSetTextureZCoord(co);
	this->leftViewer->update();
	this->rightViewer->update();
}

void ControlPanel::setTexCube(bool show) {
	this->sceneToControl->slotToggleShowTextureCube(show);
	this->leftViewer->update();
	this->rightViewer->update();
}
