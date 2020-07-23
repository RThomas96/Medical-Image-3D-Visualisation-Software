#include "../include/scene_control.hpp"
#include "../../viewer/include/scene.hpp"
#include "../../viewer/include/neighbor_visu_viewer.hpp"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

ControlPanel::ControlPanel(Scene* const scene, Viewer* lv, Viewer* rv, QWidget* parent) : QWidget(parent), sceneToControl(scene), leftViewer(lv), rightViewer(rv) {
	// Once again, a long constructor because of the verbosity of
	// Qt's layout and positionning system when done directly in
	// code. Goddammit, it's verbose.

	// Pickers for tetmesh positionning :
	this->xPicker = new QDoubleSpinBox();
	this->yPicker = new QDoubleSpinBox();
	this->zPicker = new QDoubleSpinBox();

	// Texture color scale bounds :
	this->minValueTexture = new QSpinBox();
	this->maxValueTexture = new QSpinBox();

	// Tex coordinates :
	this->xTex = new QSlider(Qt::Horizontal);
	this->yTex = new QSlider(Qt::Horizontal);
	this->zTex = new QSlider(Qt::Horizontal);

	// Cutting planes :
	this->xPlane_Min = new QSlider(Qt::Horizontal);
	this->yPlane_Min = new QSlider(Qt::Horizontal);
	this->zPlane_Min = new QSlider(Qt::Horizontal);
	this->xPlane_Max = new QSlider(Qt::Horizontal);
	this->yPlane_Max = new QSlider(Qt::Horizontal);
	this->zPlane_Max = new QSlider(Qt::Horizontal);

	// Create checkbox :
	this->toggleTexCubeCheckbox = new QCheckBox("Show texture cube");
	this->toggleTexCubeCheckbox->setCheckState(Qt::Unchecked);

	// Create the container widget :
	this->controlContainer = new QWidget();

	// Set the range to min/max of uchar for texture bounds :
	this->minValueTexture->setRange(0, 255);
	this->minValueTexture->setValue(0);
	this->maxValueTexture->setRange(0, 255);
	this->maxValueTexture->setValue(255);

	// Create labels for sliders :
	QLabel* xSliderLabel = new QLabel("X coordinate");
	QLabel* ySliderLabel = new QLabel("Y coordinate");
	QLabel* zSliderLabel = new QLabel("Z coordinate");
	QLabel* xTexLabel = new QLabel("X texture");
	QLabel* yTexLabel = new QLabel("Y texture");
	QLabel* zTexLabel = new QLabel("Z texture");

	QLabel* minTexLabel = new QLabel("Min texture value");
	QLabel* maxTexLabel = new QLabel("Max texture value");

	QLabel* cutMinLabel = new QLabel("Minimum cutting plane coordinates");
	QLabel* cutMaxLabel = new QLabel("Maximum cutting plane coordinates");

	// Create containers layouts :
	QVBoxLayout* xContainer = new QVBoxLayout();
	QVBoxLayout* yContainer = new QVBoxLayout();
	QVBoxLayout* zContainer = new QVBoxLayout();
	QHBoxLayout* topContainer = new QHBoxLayout();
	QVBoxLayout* texContainer = new QVBoxLayout();
	QVBoxLayout* cutMinContainer = new QVBoxLayout();
	QVBoxLayout* cutMaxContainer = new QVBoxLayout();
	QHBoxLayout* allContainer = new QHBoxLayout(this->controlContainer);

	// Add widgets to layouts :
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

	texContainer->addWidget(minTexLabel);
	texContainer->addWidget(this->minValueTexture);
	texContainer->addWidget(maxTexLabel);
	texContainer->addWidget(this->maxValueTexture);

	cutMinContainer->addWidget(cutMinLabel);
	cutMinContainer->addWidget(this->xPlane_Min);
	cutMinContainer->addWidget(this->yPlane_Min);
	cutMinContainer->addWidget(this->zPlane_Min);
	cutMaxContainer->addWidget(cutMaxLabel);
	cutMaxContainer->addWidget(this->xPlane_Max);
	cutMaxContainer->addWidget(this->yPlane_Max);
	cutMaxContainer->addWidget(this->zPlane_Max);

	allContainer->addLayout(topContainer);
	allContainer->addLayout(texContainer);
	allContainer->addLayout(cutMinContainer);
	allContainer->addLayout(cutMaxContainer);

	// Disable by default the top level container :
	this->controlContainer->setEnabled(false);

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

	this->xPlane_Min->setRange(0, 100);
	this->xPlane_Min->setValue(0);
	this->yPlane_Min->setRange(0, 100);
	this->yPlane_Min->setValue(0);
	this->zPlane_Min->setRange(0, 100);
	this->zPlane_Min->setValue(0);

	this->xPlane_Max->setRange(0, 100);
	this->xPlane_Max->setValue(100);
	this->yPlane_Max->setRange(0, 100);
	this->yPlane_Max->setValue(100);
	this->zPlane_Max->setRange(0, 100);
	this->zPlane_Max->setValue(100);

	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(this->controlContainer);

	this->setLayout(mainLayout);

	this->initSignals();

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

	// Modifies the values of the cutting planes :
	connect(this->xPlane_Min, &QSlider::valueChanged, this, &ControlPanel::setCutPlaneX_Min);
	connect(this->yPlane_Min, &QSlider::valueChanged, this, &ControlPanel::setCutPlaneY_Min);
	connect(this->zPlane_Min, &QSlider::valueChanged, this, &ControlPanel::setCutPlaneZ_Min);
	connect(this->xPlane_Max, &QSlider::valueChanged, this, &ControlPanel::setCutPlaneX_Max);
	connect(this->yPlane_Max, &QSlider::valueChanged, this, &ControlPanel::setCutPlaneY_Max);
	connect(this->zPlane_Max, &QSlider::valueChanged, this, &ControlPanel::setCutPlaneZ_Max);

	// Modifies the min/max values of the texture to be considered valuable data :
	connect(this->minValueTexture, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMinTexVal);
	connect(this->maxValueTexture, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMaxTexVal);
}

void ControlPanel::updateViewers() {
	if (this->leftViewer != nullptr) { this->leftViewer->update(); }
	if (this->rightViewer != nullptr) { this->rightViewer->update(); }
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
	this->updateViewers();
}

void ControlPanel::setYCoord(double coordY) {
	int co = static_cast<float>(coordY);
	this->sceneToControl->slotSetNeighborYCoord(co);
	this->updateViewers();
}

void ControlPanel::setZCoord(double coordZ) {
	int co = static_cast<float>(coordZ);
	this->sceneToControl->slotSetNeighborZCoord(co);
	this->updateViewers();
}

void ControlPanel::setXTexCoord(int coordX) {
	uint co = static_cast<uint>(coordX);
	this->sceneToControl->slotSetTextureXCoord(co);
	this->updateViewers();
}

void ControlPanel::setYTexCoord(int coordY) {
	uint co = static_cast<uint>(coordY);
	this->sceneToControl->slotSetTextureYCoord(co);
	this->updateViewers();
}

void ControlPanel::setZTexCoord(int coordZ) {
	uint co = static_cast<uint>(coordZ);
	this->sceneToControl->slotSetTextureZCoord(co);
	this->updateViewers();
}

void ControlPanel::setTexCube(bool show) {
	this->sceneToControl->slotToggleShowTextureCube(show);
	this->updateViewers();
}

void ControlPanel::setMinTexVal(int val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValue(static_cast<uchar>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMaxTexVal(int val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValue(static_cast<uchar>(val));
	}
	this->updateViewers();
}

void ControlPanel::setCutPlaneX_Min(int val) {
	int max = this->xPlane_Min->maximum();
	float ratio = static_cast<float>(val)/static_cast<float>(max);
	this->sceneToControl->slotSetCutPlaneX_Min(ratio);
	this->updateViewers();
}

void ControlPanel::setCutPlaneY_Min(int val) {
	int max = this->yPlane_Min->maximum();
	float ratio = static_cast<float>(val)/static_cast<float>(max);
	this->sceneToControl->slotSetCutPlaneY_Min(ratio);
	this->updateViewers();
}

void ControlPanel::setCutPlaneZ_Min(int val) {
	int max = this->zPlane_Min->maximum();
	float ratio = static_cast<float>(val)/static_cast<float>(max);
	this->sceneToControl->slotSetCutPlaneZ_Min(ratio);
	this->updateViewers();
}

void ControlPanel::setCutPlaneX_Max(int val) {
	int max = this->xPlane_Max->maximum();
	float ratio = static_cast<float>(val)/static_cast<float>(max);
	this->sceneToControl->slotSetCutPlaneX_Max(ratio);
	this->updateViewers();
}

void ControlPanel::setCutPlaneY_Max(int val) {
	int max = this->yPlane_Max->maximum();
	float ratio = static_cast<float>(val)/static_cast<float>(max);
	this->sceneToControl->slotSetCutPlaneY_Max(ratio);
	this->updateViewers();
}

void ControlPanel::setCutPlaneZ_Max(int val) {
	int max = this->zPlane_Max->maximum();
	float ratio = static_cast<float>(val)/static_cast<float>(max);
	this->sceneToControl->slotSetCutPlaneZ_Max(ratio);
	this->updateViewers();
}
