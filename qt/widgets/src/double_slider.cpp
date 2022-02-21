#include "../include/double_slider.hpp"

#include <iostream>

DoubleSlider::DoubleSlider(QWidget* parent) :
	QWidget(parent) {
	this->value_slider = new RangeSlider(Qt::Horizontal);
	this->value_slider->setHandleToolTip("%1");
	this->label_min_header_current = new QLabel("");
	this->label_header_min		   = new QLabel("Min : ");
	this->label_header_max		   = new QLabel("Max : ");
	this->label_min_value_current  = new QLabel(QString::number(65535));
	this->label_value_min		   = new QLabel(QString::number(65535));
	this->label_value_max		   = new QLabel(QString::number(65535));

	// Columns : [ Warning, outdated ]
	// 0 : header min
	// 1 : value min
	// 2 : slider			(stretchable to infinity)
	// 3 : header current
	// 4 : value current
	// 5 : header max
	// 6 : value max
	this->layout_grid = new QGridLayout;
	this->layout_grid->addWidget(this->label_header_min, 0, 0);
	this->layout_grid->addWidget(this->label_value_min, 0, 1);
	this->layout_grid->addWidget(this->value_slider, 0, 2);
	this->layout_grid->addWidget(this->label_min_header_current, 0, 3);
	this->layout_grid->addWidget(this->label_min_value_current, 0, 4);
	this->layout_grid->addWidget(this->label_header_max, 0, 5);
	this->layout_grid->addWidget(this->label_value_max, 0, 6);

	this->layout_grid->setColumnStretch(0, 0);
	this->layout_grid->setColumnStretch(1, 0);
	this->layout_grid->setColumnStretch(2, 10);
	this->layout_grid->setColumnStretch(3, 0);
	this->layout_grid->setColumnStretch(4, 0);
	this->layout_grid->setColumnStretch(5, 0);
	this->layout_grid->setColumnStretch(6, 0);

	this->setLayout(this->layout_grid);

	// Connect to internal functions :
	QObject::connect(this->value_slider, &RangeSlider::minimumValueChanged, this, &DoubleSlider::changeMin);
	QObject::connect(this->value_slider, &RangeSlider::maximumValueChanged, this, &DoubleSlider::changeMax);
	QObject::connect(this->value_slider, &RangeSlider::minimumValueChanged, this, &DoubleSlider::updateLabels);
	QObject::connect(this->value_slider, &RangeSlider::maximumValueChanged, this, &DoubleSlider::updateLabels);

	this->setMin(INT_MAX);
	this->setMax(INT_MAX);
	this->setMinValue(0);
	this->setMaxValue(65535);

	this->layout()->setSizeConstraint(QLayout::SizeConstraint::SetMaximumSize);
}

void DoubleSlider::disable(bool _dis) {
	this->value_slider->setDisabled(_dis);
	this->label_min_header_current->setDisabled(_dis);
	this->label_header_min->setDisabled(_dis);
	this->label_header_max->setDisabled(_dis);
	this->label_min_value_current->setDisabled(_dis);
	this->label_value_max->setDisabled(_dis);
	this->label_value_min->setDisabled(_dis);
}

void DoubleSlider::setRange(int min, int max) {
	this->setMin(min);
	this->setMax(max);
}

void DoubleSlider::setMin(int min) {
	if (this->value_slider->minimumValue() < min) {
		this->value_slider->setMinimumValue(min);
		this->value_slider->setMinimumPosition(min);
	}
	if (this->value_slider->maximumValue() <= min) {
		if (min != this->value_slider->minimumValue()) {
			this->value_slider->setMaximumValue(min);
			this->value_slider->setMaximumPosition(min);
		} else {
			this->value_slider->setMaximumValue(min + 1);
			this->value_slider->setMaximumPosition(min + 1);
		}
	}
	this->value_slider->setMinimum(min);
	this->updateLabels();
}

void DoubleSlider::setMax(int max) {
	if (this->value_slider->maximumValue() > max) {
		this->value_slider->setMaximumValue(max);
		this->value_slider->setMaximumPosition(max);
	}
	if (this->value_slider->minimumValue() >= max) {
		if (max != this->value_slider->maximumValue()) {
			this->value_slider->setMinimumValue(max);
			this->value_slider->setMinimumPosition(max);
		} else {
			this->value_slider->setMinimumValue(max - 1);
			this->value_slider->setMinimumPosition(max - 1);
		}
	}
	this->value_slider->setMaximum(max);
	this->updateLabels();
}

void DoubleSlider::setMinValue(int min) {
	int other = this->value_slider->maximumValue();
	if (min >= other) {
		if (other >= this->value_slider->maximum()) {
			this->value_slider->setMinimumValue(min - 1);
			this->value_slider->setMinimumPosition(min - 1);
			return;
		}
		this->value_slider->setMaximumValue(min + 1);
		this->value_slider->setMaximumPosition(min + 1);
	}
	this->value_slider->setMinimumValue(min);
	this->value_slider->setMinimumPosition(min);
	this->updateLabels();
}

void DoubleSlider::setMaxValue(int max) {
	int other = this->value_slider->minimumValue();
	if (max <= other) {
		if (other <= this->value_slider->minimum()) {
			this->value_slider->setMaximumValue(max + 1);
			this->value_slider->setMaximumPosition(max + 1);
			return;
		}
		this->value_slider->setMinimumValue(max - 1);
		this->value_slider->setMinimumPosition(max - 1);
	}
	this->value_slider->setMaximumValue(max);
	this->value_slider->setMaximumPosition(max);
	this->updateLabels();
}

void DoubleSlider::changeMin(int min) {
	int other = this->value_slider->maximumValue();
	if (min >= other) {
		if (other >= this->value_slider->maximum()) {
			this->value_slider->setMinimumValue(min - 1);
			this->value_slider->setMinimumPosition(min - 1);
			return;
		}
		this->value_slider->setMaximumValue(min + 1);
		this->value_slider->setMaximumPosition(min + 1);
	}
	emit this->minChanged(min);
}

void DoubleSlider::changeMax(int max) {
	int other = this->value_slider->minimumValue();
	if (max <= other) {
		if (other <= this->value_slider->minimum()) {
			this->value_slider->setMaximumValue(max + 1);
			this->value_slider->setMaximumPosition(max + 1);
			return;
		}
		this->value_slider->setMinimumValue(max - 1);
		this->value_slider->setMinimumPosition(max - 1);
	}
	emit this->maxChanged(max);
}

void DoubleSlider::dumpSliderState() {
	std::cerr << "Range values : [" << this->value_slider->minimum() << " < " << this->value_slider->minimumValue() << " < " << this->value_slider->maximumValue() << this->value_slider->maximum() << "]\n";
}

void DoubleSlider::updateLabels() {
	this->label_value_min->setText(QString::number(this->value_slider->minimum()));
	this->label_value_min->setText(QString::number(this->value_slider->minimum()));

	this->label_value_max->setText(QString::number(this->value_slider->maximum()));
	this->label_value_max->setText(QString::number(this->value_slider->maximum()));

	this->label_min_value_current->setText(QString("(") + QString::number(this->value_slider->minimumValue()) +
										   QString(" - ") + QString::number(this->value_slider->maximumValue()) +
										   QString(")"));
}
