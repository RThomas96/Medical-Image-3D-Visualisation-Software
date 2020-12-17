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

	// Cutting planes :
	this->xPlanePos = new QDoubleSpinBox();
	this->yPlanePos = new QDoubleSpinBox();
	this->zPlanePos = new QDoubleSpinBox();

	// Create the container widget :
	this->controlContainer = new QWidget();

	// Set the range to min/max of uchar for texture bounds :
	this->minValueTexture->setRange(0, 255);
	this->minValueTexture->setValue(0);
	this->maxValueTexture->setRange(0, 255);
	this->maxValueTexture->setValue(255);

	QLabel* minTexLabel = new QLabel("Min texture value");
	QLabel* maxTexLabel = new QLabel("Max texture value");

	QLabel* cutMinLabel = new QLabel("Cutting plane coordinates");

	// Create containers layouts :
	QHBoxLayout* topContainer = new QHBoxLayout();
	QVBoxLayout* texContainer = new QVBoxLayout();
	QVBoxLayout* cutMinContainer = new QVBoxLayout();
	QHBoxLayout* allContainer = new QHBoxLayout(this->controlContainer);

	texContainer->addWidget(minTexLabel);
	texContainer->addWidget(this->minValueTexture);
	texContainer->addWidget(maxTexLabel);
	texContainer->addWidget(this->maxValueTexture);

	cutMinContainer->addWidget(cutMinLabel);
	cutMinContainer->addWidget(this->xPlanePos);
	cutMinContainer->addWidget(this->yPlanePos);
	cutMinContainer->addWidget(this->zPlanePos);

	allContainer->addLayout(topContainer);
	allContainer->addLayout(texContainer);
	allContainer->addLayout(cutMinContainer);

	// Disable by default the top level container :
	this->controlContainer->setEnabled(false);

	double min = std::numeric_limits<double>::lowest();
	double max = std::numeric_limits<double>::max();
	this->xPlanePos->setRange(min, max);
	this->xPlanePos->setValue(0);
	this->yPlanePos->setRange(min, max);
	this->yPlanePos->setValue(0);
	this->zPlanePos->setRange(min, max);
	this->zPlanePos->setValue(0);

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

	this->xPlanePos->disconnect();
	this->yPlanePos->disconnect();
	this->zPlanePos->disconnect();

	deletePtr(this->minValueTexture);
	deletePtr(this->maxValueTexture);
	deletePtr(this->xPlanePos);
	deletePtr(this->yPlanePos);
	deletePtr(this->zPlanePos);
	deletePtr(this->controlContainer);

#ifndef NDEBUG
	std::cerr << "[TRACE][" << __PRETTY_FUNCTION__ << "] : Deleted control panel.\n";
#endif
}

void ControlPanel::initSignals() {
	// Modifies the values of the cutting planes :
	connect(this->xPlanePos, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControlPanel::setCutPlaneXPos);
	connect(this->yPlanePos, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControlPanel::setCutPlaneYPos);
	connect(this->zPlanePos, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControlPanel::setCutPlaneZPos);

	// Modifies the min/max values of the texture to be considered valuable data :
	connect(this->minValueTexture, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMinTexVal);
	connect(this->maxValueTexture, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMaxTexVal);
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
	this->xPlanePos->blockSignals(true);
	this->yPlanePos->blockSignals(true);
	this->zPlanePos->blockSignals(true);
	this->minValueTexture->blockSignals(true);
	this->maxValueTexture->blockSignals(true);
	this->xPlanePos->setValue(pos.x);
	this->yPlanePos->setValue(pos.y);
	this->zPlanePos->setValue(pos.z);
	this->minValueTexture->setValue(this->sceneToControl->getMinTexValue());
	this->maxValueTexture->setValue(this->sceneToControl->getMaxTexValue());
	this->maxValueTexture->blockSignals(false);
	this->minValueTexture->blockSignals(false);
	this->zPlanePos->blockSignals(false);
	this->yPlanePos->blockSignals(false);
	this->xPlanePos->blockSignals(false);
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
