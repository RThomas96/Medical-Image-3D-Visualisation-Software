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
	this->minAlternate = std::numeric_limits<DiscreteGrid::data_t>::lowest();
	this->maxAlternate = std::numeric_limits<DiscreteGrid::data_t>::max();

	// Texture and color scale bounds :
	this->minValueTexture_top = new QSlider(Qt::Horizontal);
	this->maxValueTexture_top = new QSlider(Qt::Horizontal);
	this->minValueTexture_bottom = new QSlider(Qt::Horizontal);
	this->maxValueTexture_bottom = new QSlider(Qt::Horizontal);

	// Create the container widget :
	this->controlContainer = new QWidget();

	this->minValueTexture_top->setRange(0, this->max-1);
	this->minValueTexture_top->setValue(0);
	this->maxValueTexture_top->setRange(0, this->max-1);
	this->maxValueTexture_top->setValue(this->max-2);
	this->minValueTexture_bottom->setRange(0, this->maxAlternate-1);
	this->minValueTexture_bottom->setValue(0);
	this->maxValueTexture_bottom->setRange(0, this->maxAlternate-1);
	this->maxValueTexture_bottom->setValue(this->maxAlternate-2);

	QLabel* label_Texture = new QLabel("Image intensities");
	QLabel* label_Min_Tex = new QLabel("Min");
	QLabel* label_Max_Tex = new QLabel("Max");
	QLabel* label_Min_Tex_bottom = new QLabel("Min");
	QLabel* label_Max_Tex_bottom = new QLabel("Max");

	this->label_top_tex_min_min = new QLabel(QString::number(this->min));
	this->label_top_tex_max_min = new QLabel(QString::number(this->min));
	this->label_bottom_tex_min_min = new QLabel(QString::number(this->minAlternate));
	this->label_bottom_tex_max_min = new QLabel(QString::number(this->minAlternate));

	this->label_top_tex_min_max = new QLabel(QString::number(this->max));
	this->label_top_tex_max_max = new QLabel(QString::number(this->max));
	this->label_bottom_tex_min_max = new QLabel(QString::number(this->maxAlternate));
	this->label_bottom_tex_max_max = new QLabel(QString::number(this->maxAlternate));

	this->label_top_tex_min_header = new QLabel("Current value : ");
	this->label_top_tex_max_header = new QLabel("Current value : ");
	this->label_top_tex_min_value = new QLabel(QString::number(this->minValueTexture_top->value()));
	this->label_top_tex_max_value = new QLabel(QString::number(this->maxValueTexture_top->value()));
	this->label_bottom_tex_min_header = new QLabel("Current value : ");
	this->label_bottom_tex_max_header = new QLabel("Current value : ");
	this->label_bottom_tex_min_value = new QLabel(QString::number(this->minValueTexture_top->value()));
	this->label_bottom_tex_max_value = new QLabel(QString::number(this->maxValueTexture_top->value()));

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
	grid->addWidget(label_Texture, 0, 0, 4, 1, Qt::AlignCenter);
	// Add lower labels (min/max) :
	grid->addWidget(label_Min_Tex, 0, 1, 1, 1, Qt::AlignCenter);
	grid->addWidget(label_Max_Tex, 1, 1, 1, 1, Qt::AlignCenter);

	// Add texture sliders :
	grid->addWidget(this->label_top_tex_min_min, 0, 2, 1, 1, Qt::AlignCenter);
	grid->addWidget(this->label_top_tex_max_min, 1, 2, 1, 1, Qt::AlignCenter);

	grid->addWidget(this->minValueTexture_top, 0, 3, 1, 20, Qt::AlignVCenter);
	grid->addWidget(this->maxValueTexture_top, 1, 3, 1, 20, Qt::AlignVCenter);

	grid->addWidget(this->label_top_tex_min_max, 0, 24, 1, 1, Qt::AlignVCenter);
	grid->addWidget(this->label_top_tex_max_max, 1, 24, 1, 1, Qt::AlignVCenter);

	grid->addWidget(this->label_top_tex_min_header, 0, 25, 1, 1, Qt::AlignVCenter);
	grid->addWidget(this->label_top_tex_max_header, 1, 25, 1, 1, Qt::AlignVCenter);

	grid->addWidget(this->label_top_tex_min_value, 0, 26, 1, 1, Qt::AlignVCenter);
	grid->addWidget(this->label_top_tex_max_value, 1, 26, 1, 1, Qt::AlignVCenter);

	grid->addWidget(label_Min_Tex_bottom, 2, 1, 1, 1, Qt::AlignCenter);
	grid->addWidget(label_Max_Tex_bottom, 3, 1, 1, 1, Qt::AlignCenter);

	// Add texture sliders :
	grid->addWidget(this->label_bottom_tex_min_min, 2, 2, 1, 1, Qt::AlignCenter);
	grid->addWidget(this->label_bottom_tex_max_min,3, 2, 1, 1, Qt::AlignCenter);

	grid->addWidget(this->minValueTexture_bottom, 2, 3, 1, 20, Qt::AlignVCenter);
	grid->addWidget(this->maxValueTexture_bottom, 3, 3, 1, 20, Qt::AlignVCenter);

	grid->addWidget(this->label_bottom_tex_min_max, 2, 24, 1, 1, Qt::AlignVCenter);
	grid->addWidget(this->label_bottom_tex_max_max, 3, 24, 1, 1, Qt::AlignVCenter);

	grid->addWidget(this->label_bottom_tex_min_header, 2, 25, 1, 1, Qt::AlignVCenter);
	grid->addWidget(this->label_bottom_tex_max_header, 3, 25, 1, 1, Qt::AlignVCenter);

	grid->addWidget(this->label_bottom_tex_min_value, 2, 26, 1, 1, Qt::AlignVCenter);
	grid->addWidget(this->label_bottom_tex_max_value, 3, 26, 1, 1, Qt::AlignVCenter);

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
	QObject::connect(this->minValueTexture_top, &QSlider::valueChanged, this, &ControlPanel::setMinTexVal);
	QObject::connect(this->maxValueTexture_top, &QSlider::valueChanged, this, &ControlPanel::setMaxTexVal);

	QObject::connect(this->minValueTexture_bottom, &QSlider::valueChanged, this, &ControlPanel::setMinTexValBottom);
	QObject::connect(this->maxValueTexture_bottom, &QSlider::valueChanged, this, &ControlPanel::setMaxTexValBottom);
}

void ControlPanel::updateViewers() {
	if (this->viewer != nullptr) { this->viewer->update(); }
}

void ControlPanel::updateMinValue(int val) {
	this->min = static_cast<DiscreteGrid::data_t>(val);
	this->label_top_tex_min_min->setText(QString::number(this->min));
	this->label_top_tex_max_min->setText(QString::number(this->min));

	this->minValueTexture_top->setMinimum(this->min);
	// if value was previous lower bound, keep it there !
	if (this->minValueTexture_top->value() == this->maxValueTexture_top->minimum()) {
		this->minValueTexture_top->setValue(this->min);
	}
	this->maxValueTexture_top->setMinimum(this->min);

	return;
}

void ControlPanel::updateMaxValue(int val) {
	this->max = static_cast<DiscreteGrid::data_t>(val);
	this->label_top_tex_min_max->setText(QString::number(this->max));
	this->label_top_tex_max_max->setText(QString::number(this->max));

	this->maxValueTexture_top->setMaximum(this->max);
	// if value was previous upper bound, keep it there !
	if (this->maxValueTexture_top->value() == this->minValueTexture_top->maximum()) {
		this->maxValueTexture_top->setValue(this->max);
	}
	this->minValueTexture_top->setMaximum(this->max);

	return;
}

void ControlPanel::updateMinValueAlternate(int val) {
	this->minAlternate = static_cast<DiscreteGrid::data_t>(val);

	this->minValueTexture_bottom->setMinimum(this->minAlternate);
	// if value was previous lower bound, keep it there !
	if (this->minValueTexture_bottom->value() == this->maxValueTexture_bottom->minimum()) {
		this->minValueTexture_bottom->setValue(this->minAlternate);
	}
	this->maxValueTexture_bottom->setMinimum(this->minAlternate);

	this->label_bottom_tex_min_min->setText(QString::number(this->minValueTexture_bottom->minimum()));
	this->label_bottom_tex_max_min->setText(QString::number(this->maxValueTexture_bottom->minimum()));

	return;
}

void ControlPanel::updateMaxValueAlternate(int val) {
	this->maxAlternate = static_cast<DiscreteGrid::data_t>(val);

	this->maxValueTexture_bottom->setMaximum(this->maxAlternate);
	// if value was previous upper bound, keep it there !
	if (this->maxValueTexture_bottom->value() == this->minValueTexture_bottom->maximum()) {
		this->maxValueTexture_bottom->setValue(this->maxAlternate);
	}
	this->minValueTexture_bottom->setMaximum(this->maxAlternate);

	this->label_bottom_tex_min_max->setText(QString::number(this->minValueTexture_bottom->maximum()));
	this->label_bottom_tex_max_max->setText(QString::number(this->maxValueTexture_bottom->maximum()));

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
	this->minValueTexture_top->blockSignals(true);
	this->maxValueTexture_top->blockSignals(true);
	this->minValueTexture_top->setValue(this->sceneToControl->getMinTexValue());
	this->maxValueTexture_top->setValue(this->sceneToControl->getMaxTexValue());
	this->maxValueTexture_top->blockSignals(false);
	this->minValueTexture_top->blockSignals(false);
	this->minValueTexture_bottom->blockSignals(true);
	this->maxValueTexture_bottom->blockSignals(true);
	this->minValueTexture_bottom->setValue(this->sceneToControl->getMinTexValueAlternate());
	this->maxValueTexture_bottom->setValue(this->sceneToControl->getMaxTexValueAlternate());
	this->maxValueTexture_bottom->blockSignals(false);
	this->minValueTexture_bottom->blockSignals(false);
	this->blockSignals(false);
}

void ControlPanel::updateLabels(void) {
	//
}

void ControlPanel::setMinTexVal(int val) {
	int otherval = this->maxValueTexture_top->value();
	if (val >= otherval) {
		// if max already at max, return and do nothing :
		if (otherval >= this->maxValueTexture_top->maximum()-1) {
			this->minValueTexture_top->setValue(otherval-1);
			return;
		}
		// otherwise, we can do something
		this->maxValueTexture_top->setValue(val+1);
	}
	// update scene data :
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValue(static_cast<DiscreteGrid::data_t>(val));
	}
	this->label_top_tex_min_value->setText(QString::number(val));
	this->updateViewers();
}

void ControlPanel::setMaxTexVal(int val) {
	int otherval = this->minValueTexture_top->value();
	if (val <= otherval) {
		// if max already at max, return and do nothing :
		if (otherval <= this->minValueTexture_top->minimum()+1) {
			this->maxValueTexture_top->setValue(otherval+1);
			return;
		}
		// otherwise, we can do something
		this->minValueTexture_top->setValue(val-1);
	}
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValue(static_cast<DiscreteGrid::data_t>(val));
	}
	this->label_top_tex_max_value->setText(QString::number(val));
	this->updateViewers();
}

void ControlPanel::setMinTexValBottom(int val) {
	int otherval = this->maxValueTexture_bottom->value();
	if (val >= otherval) {
		// if max already at max, return and do nothing :
		if (otherval >= this->maxValueTexture_bottom->maximum()-1) {
			this->minValueTexture_bottom->setValue(otherval-1);
			return;
		}
		// otherwise, we can do something
		this->maxValueTexture_bottom->setValue(val+1);
	}
	// update scene data :
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValueAlternate(static_cast<DiscreteGrid::data_t>(val));
	}
	this->label_bottom_tex_min_value->setText(QString::number(val));
	this->updateViewers();
}

void ControlPanel::setMaxTexValBottom(int val) {
	int otherval = this->minValueTexture_bottom->value();
	if (val <= otherval) {
		// if max already at max, return and do nothing :
		if (otherval <= this->minValueTexture_bottom->minimum()+1) {
			this->maxValueTexture_bottom->setValue(otherval+1);
			return;
		}
		// otherwise, we can do something
		this->minValueTexture_bottom->setValue(val-1);
	}
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValueAlternate(static_cast<DiscreteGrid::data_t>(val));
	}
	this->label_bottom_tex_max_value->setText(QString::number(val));
	this->updateViewers();
}

void ControlPanel::setClipDistance(double val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetClipDistance(val);
	}
	this->updateViewers();
}
