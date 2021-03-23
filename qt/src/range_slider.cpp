#include "../include/range_slider.hpp"

RangeSlider::RangeSlider(QWidget* parent) : QWidget(parent) {
	this->slider_min = new QSlider(Qt::Horizontal);
	this->slider_max = new QSlider(Qt::Horizontal);
	this->label_min_header_current = new QLabel("");
	this->label_max_header_current = new QLabel("");
	this->label_header_min = new QLabel("Min : ");
	this->label_header_max = new QLabel("Max : ");
	this->label_min_value_current = new QLabel(QString::number(this->slider_min->value()));
	this->label_max_value_current = new QLabel(QString::number(this->slider_max->value()));
	this->label_value_min = new QLabel(QString::number(this->slider_min->minimum()));
	this->label_value_max = new QLabel(QString::number(this->slider_min->maximum()));

	this->layout_grid = new QGridLayout;
	this->layout_grid->addWidget(this->label_header_min,			0, 0, 2, 1);
	this->layout_grid->addWidget(this->label_value_min,				0, 1, 2, 1);
	this->layout_grid->addWidget(this->slider_min,					0, 2, 1, 15);
	this->layout_grid->addWidget(this->label_min_header_current,	0, 18);
	this->layout_grid->addWidget(this->label_min_value_current,		0, 19);
	this->layout_grid->addWidget(this->label_header_max,			0, 20, 2, 1);
	this->layout_grid->addWidget(this->label_value_max,				0, 21, 2, 1);
	this->layout_grid->addWidget(this->slider_max,					1, 2, 1, 15);
	this->layout_grid->addWidget(this->label_max_header_current,	1, 18);
	this->layout_grid->addWidget(this->label_max_value_current,		1, 19);

	this->setLayout(this->layout_grid);

	// Connect to internal functions :
	QObject::connect(this->slider_min, &QSlider::valueChanged, this, &RangeSlider::changeMin);
	QObject::connect(this->slider_max, &QSlider::valueChanged, this, &RangeSlider::changeMax);
	QObject::connect(this->slider_min, &QSlider::valueChanged, this, &RangeSlider::updateLabels);
	QObject::connect(this->slider_max, &QSlider::valueChanged, this, &RangeSlider::updateLabels);

	this->setMin(0);
	this->setMax(INT_MAX);
	this->setMinValue(0);
	this->setMaxValue(65535);
}

void RangeSlider::disable(bool _dis) {
	this->slider_min->setDisabled(_dis);
	this->slider_max->setDisabled(_dis);
	this->label_min_header_current->setDisabled(_dis);
	this->label_header_min->setDisabled(_dis);
	this->label_header_max->setDisabled(_dis);
	this->label_min_value_current->setDisabled(_dis);
	this->label_value_max->setDisabled(_dis);
	this->label_value_min->setDisabled(_dis);
	this->label_max_header_current->setDisabled(_dis);
	this->label_max_value_current->setDisabled(_dis);
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
	this->updateLabels();
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
	this->slider_min->setMaximum(max);
	this->slider_max->setMaximum(max);
	this->updateLabels();
}

void RangeSlider::setMinValue(int min) {
	int other = this->slider_max->value();
	if (min >= other) {
		if (other >= this->slider_max->maximum()-1) {
			this->slider_min->setValue(min-1);
			return;
		}
		this->slider_max->setValue(min+1);
	}
	this->slider_min->setValue(min);
	this->updateLabels();
}

void RangeSlider::setMaxValue(int max) {
	int other = this->slider_min->value();
	if (max <= other) {
		if (other <= this->slider_min->minimum()+1) {
			this->slider_max->setValue(max+1);
			return;
		}
		this->slider_min->setValue(max-1);
	}
	this->slider_max->setValue(max);
	this->updateLabels();
}

void RangeSlider::changeMin(int min) {
	int other = this->slider_max->value();
	if (min >= other) {
		if (other >= this->slider_max->maximum()-1) {
			this->slider_min->setValue(min-1);
			return;
		}
		this->slider_max->setValue(min+1);
	}
	emit this->minChanged(min);
}

void RangeSlider::changeMax(int max) {
	int other = this->slider_min->value();
	if (max <= other) {
		if (other <= this->slider_min->minimum()+1) {
			this->slider_max->setValue(max+1);
			return;
		}
		this->slider_min->setValue(max-1);
	}
	emit this->maxChanged(max);
}

void RangeSlider::updateLabels() {
	this->label_value_min->setText(QString::number(this->slider_min->minimum()));
	this->label_value_min->setText(QString::number(this->slider_min->minimum()));

	this->label_value_max->setText(QString::number(this->slider_max->maximum()));
	this->label_value_max->setText(QString::number(this->slider_max->maximum()));

	this->label_min_value_current->setText(QString("(") + QString::number(this->slider_min->value()) + QString(")"));
	this->label_max_value_current->setText(QString("(") + QString::number(this->slider_max->value()) + QString(")"));
}
