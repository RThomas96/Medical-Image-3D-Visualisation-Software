#include "../include/color_control.hpp"

#include <QColorDialog>
#include <QIcon>

ColorBoundControl::ColorBoundControl(Scene* _sc, ControlPanel* _cp, MainWidget* _mw, bool _p, QWidget* parent) : QWidget(parent) {
	this->_main = _mw;
	this->_scene = _sc;
	this->_controlpanel = _cp;
	this->_primary = _p;

	if (this->_primary) {
		this->_min = this->_scene->getMinColorValue();
		this->_max = this->_scene->getMaxColorValue();
	} else {
		this->_min = this->_scene->getMinColorValue();
		this->_max = this->_scene->getMaxColorValue();
	}

	this->setAttribute(Qt::WA_DeleteOnClose);

	this->spinbox_min = new QSpinBox;
	this->spinbox_min->setMinimum(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::lowest()));
	this->spinbox_min->setMaximum(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::max()));
	this->spinbox_min->setValue(this->_min);
	this->spinbox_max = new QSpinBox;
	this->spinbox_max->setMinimum(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::lowest()));
	this->spinbox_max->setMaximum(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::max()));
	this->spinbox_max->setValue(this->_max);

	this->label_min = new QLabel("Minimum color intensity :");
	this->label_max = new QLabel("Maximum color intensity :");

	this->label_color0 = new QLabel("Color segment beginning :");
	this->label_color1 = new QLabel("Color segment ending :");

	this->button_baseColor0 = new QPushButton;
	this->button_baseColor1 = new QPushButton;

	this->color_0 = Qt::GlobalColor::red;
	this->color_1 = Qt::GlobalColor::blue;
	if (this->_primary) {
		this->_scene->setColor0(this->color_0.redF(), this->color_0.greenF(), this->color_0.blueF());
		this->_scene->setColor1(this->color_1.redF(), this->color_1.greenF(), this->color_1.blueF());
	}

	this->pixmap_baseColor0 = new QPixmap(30, 30);
	this->pixmap_baseColor0->fill(this->color_0);
	QIcon c0(*this->pixmap_baseColor0);

	this->pixmap_baseColor1 = new QPixmap(30, 30);
	this->pixmap_baseColor1->fill(this->color_1);
	QIcon c1(*this->pixmap_baseColor1);

	this->button_baseColor0->setIcon(c0);
	this->button_baseColor1->setIcon(c1);

	this->grid = new QGridLayout;
	this->grid->addWidget(this->label_min, 0, 0);
	this->grid->addWidget(this->spinbox_min, 0, 1);
	this->grid->addWidget(this->label_max, 1, 0);
	this->grid->addWidget(this->spinbox_max, 1, 1);
	this->grid->addWidget(this->label_color0, 2, 0);
	this->grid->addWidget(this->button_baseColor0, 2, 1);
	this->grid->addWidget(this->label_color1, 3, 0);
	this->grid->addWidget(this->button_baseColor1, 3, 1);
	this->setLayout(this->grid);

	QObject::connect(this->spinbox_min, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorBoundControl::setMinColorBound);
	QObject::connect(this->spinbox_max, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorBoundControl::setMaxColorBound);
	QObject::connect(this->button_baseColor0, &QPushButton::clicked, this, &ColorBoundControl::setColor0);
	QObject::connect(this->button_baseColor1, &QPushButton::clicked, this, &ColorBoundControl::setColor1);

	this->_scene->setColor0(this->color_0.redF(), this->color_0.greenF(), this->color_0.blueF());
	this->_scene->setColor1(this->color_1.redF(), this->color_1.greenF(), this->color_1.blueF());
}

ColorBoundControl::~ColorBoundControl() {
	this->spinbox_min->disconnect();
	this->spinbox_max->disconnect();
	this->button_baseColor0->disconnect();
	this->button_baseColor1->disconnect();

	delete this->spinbox_max;
	delete this->spinbox_min;
	delete this->label_max;
	delete this->label_min;
	delete this->label_color0;
	delete this->label_color1;
	delete this->button_baseColor0;
	delete this->button_baseColor1;

	delete this->grid;
}

void ColorBoundControl::setMinColorBound(int val) {
		int other = this->spinbox_max->value();
		if (val >= other) {
			if (other >= this->spinbox_max->maximum()-1) {
				this->spinbox_min->setValue(other-1);
				return;
			}
			this->spinbox_max->setValue(val+1);
		}
		if (this->_primary) {
			this->_scene->slotSetMinColorValue(val);
			this->_scene->slotSetMaxColorValue(other);
			this->_controlpanel->updateMinValue(val);
			this->_controlpanel->updateMaxValue(other);
		} else {
			this->_scene->slotSetMinColorValueAlternate(val);
			this->_scene->slotSetMaxColorValueAlternate(other);
			this->_controlpanel->updateMinValueAlternate(val);
			this->_controlpanel->updateMaxValueAlternate(other);
		}
}

void ColorBoundControl::setMaxColorBound(int val) {
		int other = this->spinbox_min->value();
		if (val <= other) {
			if (other <= this->spinbox_min->minimum()+1) {
				this->spinbox_max->setValue(other+1);
				return;
			}
			this->spinbox_min->setValue(val-1);
		}

		if (this->_primary) {
			this->_scene->slotSetMinColorValue(other);
			this->_scene->slotSetMaxColorValue(val);
			this->_controlpanel->updateMinValue(other);
			this->_controlpanel->updateMaxValue(val);
		} else {
			this->_scene->slotSetMinColorValueAlternate(other);
			this->_scene->slotSetMaxColorValueAlternate(val);
			this->_controlpanel->updateMinValueAlternate(other);
			this->_controlpanel->updateMaxValueAlternate(val);
		}
}

void ColorBoundControl::setColor0() {
	QColor c = QColorDialog::getColor(this->color_0, this);
	if (c.isValid() == false) { return; }
	this->color_0 = c;
	this->pixmap_baseColor0->fill(this->color_0);
	QIcon c0(*this->pixmap_baseColor0);
	this->button_baseColor0->setIcon(c0);

	if (this->_primary) {
		this->_scene->setColor0(this->color_0.redF(), this->color_0.greenF(), this->color_0.blueF());
	} else {
		this->_scene->setColor0Alternate(this->color_0.redF(), this->color_0.greenF(), this->color_0.blueF());
	}

	return;
}

void ColorBoundControl::setColor1() {
	QColor c = QColorDialog::getColor(this->color_1, this);
	if (c.isValid() == false) { return; }
	this->color_1 = c;
	this->pixmap_baseColor1->fill(this->color_1);
	QIcon c1(*this->pixmap_baseColor1);
	this->button_baseColor1->setIcon(c1);

	if (this->_primary) {
		this->_scene->setColor1(this->color_1.redF(), this->color_1.greenF(), this->color_1.blueF());
	} else {
		this->_scene->setColor1Alternate(this->color_1.redF(), this->color_1.greenF(), this->color_1.blueF());
	}

	return;
}

ColorBoundWidget::ColorBoundWidget(Scene* _sc, ControlPanel* _cp, MainWidget* main, QWidget* parent) : QWidget(parent) {
	this->scene = _sc;
	this->_controlpanel = _cp;
	this->_mainwidget = main;

	this->_control_primary = new ColorBoundControl(_sc, _cp, main, true);
	this->_control_secondary = new ColorBoundControl(_sc, _cp, main, false);

	this->main_layout = new QVBoxLayout(nullptr);

	this->main_layout->addWidget(this->_control_primary);
	this->main_layout->addWidget(this->_control_secondary);

	this->setLayout(this->main_layout);
}

ColorBoundWidget::~ColorBoundWidget() {
	// nothing here for now ...
}
