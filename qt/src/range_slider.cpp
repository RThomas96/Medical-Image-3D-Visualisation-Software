#include "../include/range_slider.hpp"

RangeSlider::RangeSlider(QWidget* parent) : QWidget(parent) {
	this->slider_min = new QSlider(Qt::Horizontal);
	this->slider_max = new QSlider(Qt::Horizontal);
	this->label_min_header_current = new QLabel("Current : ");
	this->label_max_header_current = new QLabel("Current : ");
	this->label_min_header_min = new QLabel("Min : ");
	this->label_max_header_min = new QLabel("Min : ");
	this->label_min_header_max = new QLabel("Max : ");
	this->label_max_header_max = new QLabel("Max : ");
	this->label_min_value_current = new QLabel(QString::number(this->slider_min->value()));
	this->label_max_value_current = new QLabel(QString::number(this->slider_max->value()));
	this->label_min_value_min = new QLabel(QString::number(this->slider_min->minimum()));
	this->label_max_value_min = new QLabel(QString::number(this->slider_max->minimum()));
	this->label_min_value_max = new QLabel(QString::number(this->slider_min->maximum()));
	this->label_max_value_max = new QLabel(QString::number(this->slider_max->maximum()));

	this->layout_grid = new QGridLayout;
	this->layout_grid->addWidget(this->label_min_header_min,	0, 0);
	this->layout_grid->addWidget(this->label_min_value_min,		0, 1);
	this->layout_grid->addWidget(this->slider_min,			0, 2, 1, 20);
	this->layout_grid->addWidget(this->label_min_header_current,	0, 23);
	this->layout_grid->addWidget(this->label_min_value_current,	0, 24);
	this->layout_grid->addWidget(this->label_min_header_max,	0, 25);
	this->layout_grid->addWidget(this->label_min_value_max,		0, 26);
	this->layout_grid->addWidget(this->label_max_header_min,	1, 0);
	this->layout_grid->addWidget(this->label_max_value_min,		1, 1);
	this->layout_grid->addWidget(this->slider_max,			1, 2, 1, 20);
	this->layout_grid->addWidget(this->label_max_header_current,	1, 23);
	this->layout_grid->addWidget(this->label_max_value_current,	1, 24);
	this->layout_grid->addWidget(this->label_max_header_max,	1, 25);
	this->layout_grid->addWidget(this->label_max_value_max,		1, 26);

	this->setLayout(this->layout_grid);

	// Connect to external signals :
	QObject::connect(this->slider_min, &QSlider::valueChanged, this, &RangeSlider::minChanged);
	QObject::connect(this->slider_max, &QSlider::valueChanged, this, &RangeSlider::maxChanged);

	this->setMin(0);
	this->setMax(INT_MAX);
	this->setValue(0);
}

void RangeSlider::setDisabled(bool _dis) {
	this->slider_min->setDisabled(_dis);
	this->slider_max->setDisabled(_dis);
	this->label_min_header_current->setDisabled(_dis);
	this->label_min_header_min->setDisabled(_dis);
	this->label_min_header_max->setDisabled(_dis);
	this->label_min_value_current->setDisabled(_dis);
	this->label_min_value_max->setDisabled(_dis);
	this->label_min_value_min->setDisabled(_dis);
	this->label_max_header_current->setDisabled(_dis);
	this->label_max_header_min->setDisabled(_dis);
	this->label_max_header_max->setDisabled(_dis);
	this->label_max_value_current->setDisabled(_dis);
	this->label_max_value_max->setDisabled(_dis);
	this->label_max_value_min->setDisabled(_dis);
}

void RangeSlider::setRange(int min, int max) {
	this->setMin(min);
	this->setMax(max);
}

void RangeSlider::setMin(int min) {
	if (this->slider_min->value() < min) {
		this->slider_min->setValue(min);
	}
	if (this->slider_max->value() <= min) {
		if (min != this->slider_min->value()) {
			this->slider_max->setValue(min);
		} else {
			this->slider_max->setValue(min+1);
		}
	}
	this->slider_min->setMinimum(min);
	this->slider_max->setMinimum(min);
}

void RangeSlider::setMax(int max) {
	if (this->slider_max->value() > max) {
		this->slider_max->setValue(max);
	}
	if (this->slider_min->value() >= max) {
		if (max != this->slider_max->value()) {
			this->slider_min->setValue(max);
		} else {
			this->slider_min->setValue(max-1);
		}
	}
	this->slider_min->setMinimum(min);
	this->slider_max->setMinimum(min);
}

void RangeSlider::setMinValue(int min) {
	this->slider_min->setValue(min);
}

void RangeSlider::setMaxValue(int max) {
	this->slider_max->setValue(max);
}

void RangeSlider::updateLabels(int unused_val) {
	this->label_min_value_min->setValue(QString::number(this->slider_min->minimum()));
	this->label_max_value_min->setValue(QString::number(this->slider_min->minimum()));

	this->label_min_value_max->setValue(QString::number(this->slider_max->maximum()));
	this->label_max_value_max->setValue(QString::number(this->slider_max->maximum()));

	this->label_min_value_current->setValue(QString::number(this->slider_min->value()));
	this->label_max_value_current->setValue(QString::number(this->slider_max->value()));
}
