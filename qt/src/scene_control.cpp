#include "../include/scene_control.hpp"
#include "../../viewer/include/scene.hpp"
#include "../../viewer/include/neighbor_visu_viewer.hpp"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

ControlPanel::ControlPanel(Scene* const scene, Viewer* lv, QWidget* parent) : QWidget(parent), sceneToControl(scene), viewer(lv) {
	// Once again, a long constructor because of the verbosity of
	// Qt's layout and positionning system when done directly in
	// code. Goddammit, it's verbose.

	this->min = std::numeric_limits<DiscreteGrid::data_t>::lowest();
	this->max = std::numeric_limits<DiscreteGrid::data_t>::max();

	// Texture and color scale bounds :
	this->minValueTexture = new QSlider(Qt::Horizontal);
	this->maxValueTexture = new QSlider(Qt::Horizontal);

	this->label_minTexLeft = new QLabel(QString::number(this->min));
	this->label_minTexRight = new QLabel(QString::number(this->min));

	// Create the container widget :
	this->controlContainer = new QWidget();

	this->minValueTexture->setRange(0, this->max-1);
	this->minValueTexture->setValue(0);
	this->maxValueTexture->setRange(0, this->max-1);
	this->maxValueTexture->setValue(this->max-2);

	this->label_maxTexLeft = new QLabel(QString::number(this->max));
	this->label_maxTexRight = new QLabel(QString::number(this->max));

	QLabel* label_Texture = new QLabel("Image intensities");
	QLabel* label_Min_Tex = new QLabel("Min");
	QLabel* label_Max_Tex = new QLabel("Max");

	this->label_currentHeader0 = new QLabel("Current value : ");
	this->label_currentHeader1 = new QLabel("Current value : ");
	this->label_currentValue0 = new QLabel(QString::number(this->minValueTexture->value()));
	this->label_currentValue1 = new QLabel(QString::number(this->maxValueTexture->value()));

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

	grid->addWidget(this->label_currentHeader0, 0, 25, 1, 1, Qt::AlignVCenter);
	grid->addWidget(this->label_currentHeader1, 1, 25, 1, 1, Qt::AlignVCenter);

	grid->addWidget(this->label_currentValue0, 0, 26, 1, 1, Qt::AlignVCenter);
	grid->addWidget(this->label_currentValue1, 1, 26, 1, 1, Qt::AlignVCenter);

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
}

ControlPanel::~ControlPanel() = default;

void ControlPanel::initSignals() {
	// Modifies the min/max values of the texture to be considered valuable data :
	QObject::connect(this->minValueTexture, &QSlider::valueChanged, this, &ControlPanel::setMinTexVal);
	QObject::connect(this->maxValueTexture, &QSlider::valueChanged, this, &ControlPanel::setMaxTexVal);
}

void ControlPanel::updateViewers() {
	if (this->viewer != nullptr) { this->viewer->update(); }
}

void ControlPanel::updateMinValue(int val) {
	this->min = static_cast<DiscreteGrid::data_t>(val);
	this->label_minTexLeft->setText(QString::number(this->min));
	this->label_minTexRight->setText(QString::number(this->min));

	this->minValueTexture->setMinimum(this->min);
	this->maxValueTexture->setMinimum(this->min);

	return;
}

void ControlPanel::updateMaxValue(int val) {
	this->max = static_cast<DiscreteGrid::data_t>(val);
	this->label_maxTexLeft->setText(QString::number(this->max));
	this->label_maxTexRight->setText(QString::number(this->max));

	this->minValueTexture->setMaximum(this->max);
	this->maxValueTexture->setMaximum(this->max);

	return;
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
		if (otherval >= this->maxValueTexture->maximum()-1) {
			this->minValueTexture->setValue(otherval-1);
			return;
		}
		// otherwise, we can do something
		this->maxValueTexture->setValue(val+1);
	}
	// update scene data :
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValue(static_cast<DiscreteGrid::data_t>(val));
	}
	this->label_currentValue0->setText(QString::number(val));
	this->updateViewers();
}

void ControlPanel::setMaxTexVal(int val) {
	int otherval = this->minValueTexture->value();
	if (val <= otherval) {
		// if max already at max, return and do nothing :
		if (otherval <= this->minValueTexture->minimum()+1) {
			this->maxValueTexture->setValue(otherval+1);
			return;
		}
		// otherwise, we can do something
		this->minValueTexture->setValue(val-1);
	}
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValue(static_cast<DiscreteGrid::data_t>(val));
	}
	this->label_currentValue1->setText(QString::number(val));
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
