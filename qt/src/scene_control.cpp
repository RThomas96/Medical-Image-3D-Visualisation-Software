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

	// Texture color scale bounds :
	this->minValueTexture = new QSpinBox();
	this->maxValueTexture = new QSpinBox();
	this->minValueColor = new QSpinBox();
	this->maxValueColor = new QSpinBox();

	this->clipDistance = new QDoubleSpinBox();
	this->clipDistance->setRange(.0, 1000.);
	this->clipDistance->setValue(5.);

	// Create the container widget :
	this->controlContainer = new QWidget();

	// Set the range to min/max of uchar for texture bounds :
	this->minValueTexture->setRange(0, 255);
	this->minValueTexture->setValue(0);
	this->maxValueTexture->setRange(0, 255);
	this->maxValueTexture->setValue(255);

	this->minValueColor->setRange(0, 255);
	this->minValueColor->setValue(0);
	this->maxValueColor->setRange(0, 255);
	this->maxValueColor->setValue(255);

	QLabel* minTexLabel = new QLabel("Min texture value");
	QLabel* maxTexLabel = new QLabel("Max texture value");
	QLabel* minColorLabel = new QLabel("Min color value");
	QLabel* maxColorLabel = new QLabel("Min color value");

	// Create containers layouts :
	QVBoxLayout* texContainer = new QVBoxLayout();
	QVBoxLayout* colContainer = new QVBoxLayout();
	QVBoxLayout* cutMinContainer = new QVBoxLayout();
	QHBoxLayout* allContainer = new QHBoxLayout(this->controlContainer);

	texContainer->addWidget(minTexLabel);
	texContainer->addWidget(this->minValueTexture);
	texContainer->addWidget(maxTexLabel);
	texContainer->addWidget(this->maxValueTexture);

	colContainer->addWidget(minColorLabel);
	colContainer->addWidget(this->minValueColor);
	colContainer->addWidget(maxColorLabel);
	colContainer->addWidget(this->maxValueColor);

	allContainer->addLayout(colContainer);
	allContainer->addLayout(texContainer);
	allContainer->addLayout(cutMinContainer);
	allContainer->addWidget(this->clipDistance);

	// Disable by default the top level container :
	this->controlContainer->setEnabled(false);

	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(this->controlContainer);

	this->setLayout(mainLayout);

	this->initSignals();

	this->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
}

ControlPanel::~ControlPanel() {
#ifndef NDEBUG
	std::cerr << "[TRACE][" << __PRETTY_FUNCTION__ << "] : deleting control panel tied to scene " << this->sceneToControl << "...\n";
#endif
	auto deletePtr = [](auto* obj) {
		if (obj != nullptr) {
			delete obj;
		}
		obj = nullptr;
	};

	this->clipDistance->disconnect();

	deletePtr(this->minValueTexture);
	deletePtr(this->maxValueTexture);
	deletePtr(this->clipDistance);
	deletePtr(this->controlContainer);
}

void ControlPanel::initSignals() {
	connect(this->clipDistance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControlPanel::setClipDistance);

	// Modifies the min/max values of the texture to be considered valuable data :
	connect(this->minValueTexture, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMinTexVal);
	connect(this->maxValueTexture, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMaxTexVal);
	// Same for colors :
	connect(this->minValueColor, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMinColVal);
	connect(this->maxValueColor, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMaxColVal);
}

void ControlPanel::updateViewers() {
	if (this->leftViewer != nullptr) { this->leftViewer->update(); }
	if (this->rightViewer != nullptr) { this->rightViewer->update(); }
}

void ControlPanel::activatePanels(bool activeStatus) {
	this->updateValues();
	this->controlContainer->setEnabled(activeStatus);
	if (activeStatus) {
		this->controlContainer->show();
	} else {
		this->controlContainer->hide();
	}
	this->update();
}

void ControlPanel::updateValues(void) {
	if (this->sceneToControl == nullptr) { return; }
	glm::vec3 pos = this->sceneToControl->getPlanePositions();
	this->blockSignals(true);
	this->minValueTexture->blockSignals(true);
	this->maxValueTexture->blockSignals(true);
	this->minValueTexture->setValue(this->sceneToControl->getMinTexValue());
	this->maxValueTexture->setValue(this->sceneToControl->getMaxTexValue());
	this->maxValueTexture->blockSignals(false);
	this->minValueTexture->blockSignals(false);
	this->blockSignals(false);
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

void ControlPanel::setMinColVal(int val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinColorValue(static_cast<uchar>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMaxColVal(int val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxColorValue(static_cast<uchar>(val));
	}
	this->updateViewers();
}

void ControlPanel::setClipDistance(double val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetClipDistance(val);
	}
	this->updateViewers();
}

void ControlPanel::setCutPlaneXPos(double val) {
	float ratio = static_cast<float>(val);
	this->sceneToControl->slotSetPlanePositionX(ratio);
	this->updateViewers();
}

void ControlPanel::setCutPlaneYPos(double val) {
	float ratio = static_cast<float>(val);
	this->sceneToControl->slotSetPlanePositionY(ratio);
	this->updateViewers();
}

void ControlPanel::setCutPlaneZPos(double val) {
	float ratio = static_cast<float>(val);
	this->sceneToControl->slotSetPlanePositionZ(ratio);
	this->updateViewers();
}
