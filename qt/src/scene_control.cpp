#include "../include/scene_control.hpp"
#include "../../viewer/include/scene.hpp"
#include "../../viewer/include/neighbor_visu_viewer.hpp"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

ControlPanel::ControlPanel(Scene* const scene, Viewer* lv, QWidget* parent) : QWidget(parent), sceneToControl(scene), leftViewer(lv) {
	// Once again, a long constructor because of the verbosity of
	// Qt's layout and positionning system when done directly in
	// code. Goddammit, it's verbose.

	// Texture and color scale bounds :
	this->minValueTexture = new QSlider(Qt::Horizontal);
	this->maxValueTexture = new QSlider(Qt::Horizontal);

	this->label_minTexLeft = new QLabel("0");
	this->label_maxTexLeft = new QLabel("255");
	this->label_minTexRight = new QLabel("0");
	this->label_maxTexRight = new QLabel("255");

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

	QLabel* label_Texture = new QLabel("Image intensities");
	QLabel* label_Min_Tex = new QLabel("Min");
	QLabel* label_Max_Tex = new QLabel("Max");

	label_Texture->setToolTip("Controls the minimum/maximum intensity values visible in the grid.");

	/**
	 * The widgets in this widget will be laid out on a grid layout, with the following arragement :
	 * Column 0 : Header label
	 * Column 1 : MIN/MAX labels
	 * Column 2 : Label for the minimum value
	 * Column 3-23 : Slider
	 * Column 24 : Max value
	 */

	// Grid layout for this widget.
	QGridLayout* grid = new QGridLayout();

	// Add top labels :
	grid->addWidget(label_Texture, 0, 0, 2, 1, Qt::AlignCenter);
	// Add lower labels (min/max) :
	grid->addWidget(label_Min_Tex, 0, 1, 1, 1, Qt::AlignCenter);
	grid->addWidget(label_Max_Tex, 1, 1, 1, 1, Qt::AlignCenter);

	// Add texture sliders :
	grid->addWidget(this->label_minTexLeft, 0, 2, 1, 1, Qt::AlignCenter);
	grid->addWidget(this->label_minTexRight, 1, 2, 1, 1, Qt::AlignCenter);

	grid->addWidget(this->minValueTexture, 0, 3, 1, 20, Qt::AlignVCenter);
	grid->addWidget(this->maxValueTexture, 1, 3, 1, 20, Qt::AlignVCenter);

	grid->addWidget(this->label_maxTexLeft, 0, 24, 1, 1, Qt::AlignVCenter);
	grid->addWidget(this->label_maxTexRight, 1, 24, 1, 1, Qt::AlignVCenter);

	this->controlContainer->setLayout(grid);

	// Disable by default the top level container :
	this->controlContainer->setEnabled(false);

	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(this->controlContainer);

	this->setLayout(mainLayout);

	this->initSignals();

	if (this->sceneToControl != nullptr) {
		this->updateValues();
	}

	//this->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
}

ControlPanel::~ControlPanel() = default;

void ControlPanel::initSignals() {
	connect(this->clipDistance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControlPanel::setClipDistance);

	// Modifies the min/max values of the texture to be considered valuable data :
	QObject::connect(this->minValueTexture, &QSlider::valueChanged, this, &ControlPanel::setMinTexVal);
	QObject::connect(this->maxValueTexture, &QSlider::valueChanged, this, &ControlPanel::setMaxTexVal);

}

void ControlPanel::updateViewers() {
	if (this->leftViewer != nullptr) { this->leftViewer->update(); }
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
	this->blockSignals(true);
	this->minValueTexture->blockSignals(true);
	this->maxValueTexture->blockSignals(true);
	this->minValueTexture->setValue(this->sceneToControl->getMinTexValue());
	this->maxValueTexture->setValue(this->sceneToControl->getMaxTexValue());
	this->maxValueTexture->blockSignals(false);
	this->minValueTexture->blockSignals(false);
	this->blockSignals(false);
}

void ControlPanel::setMinTexVal(int val) {
	int otherval = this->maxValueTexture->value();
	if (val >= otherval) {
		// if max already at max, return and do nothing :
		if (otherval == this->maxValueTexture->maximum()) { return; }
		// otherwise, we can do something
		else {
			this->maxValueTexture->setValue(val+1);
		}
	}
	// update scene data :
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValue(static_cast<uchar>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMaxTexVal(int val) {
	int otherval = this->minValueTexture->value();
	if (val <= otherval) {
		// if max already at max, return and do nothing :
		if (otherval == this->minValueTexture->minimum()) { return; }
		// otherwise, we can do something
		else {
			this->minValueTexture->setValue(val-1);
		}
	}
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValue(static_cast<uchar>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMinColVal(int val) {
	/*int otherval = this->maxValueColor->value();
	if (val >= otherval) {
		// if max already at max, return and do nothing :
		if (otherval == this->maxValueColor->maximum()) { return; }
		// otherwise, we can do something
		else {
			this->maxValueColor->setValue(val+1);
		}
	}
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinColorValue(static_cast<uchar>(val));
	}
	this->updateViewers();*/
}

void ControlPanel::setMaxColVal(int val) {
	/*int otherval = this->minValueColor->value();
	if (val <= otherval) {
		// if max already at max, return and do nothing :
		if (otherval == this->minValueColor->minimum()) { return; }
		// otherwise, we can do something
		else {
			this->minValueColor->setValue(val-1);
		}
	}
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxColorValue(static_cast<uchar>(val));
	}
	this->updateViewers();*/
}

void ControlPanel::setClipDistance(double val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetClipDistance(val);
	}
	this->updateViewers();
}
