#include "../include/scene_control.hpp"
#include "../../viewer/include/scene.hpp"
#include "../../viewer/include/neighbor_visu_viewer.hpp"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QColorDialog>

ColorButton::ColorButton(QColor _color, QWidget* parent) : QWidget(parent) {
	this->color = _color;
	this->button = new QPushButton;
	this->pixmap = new QPixmap(30,30);
	this->icon = nullptr;
	this->setColor(this->color);
	this->button->setFixedSize(this->button->sizeHint());

	this->layout = new QVBoxLayout();
	this->layout->addWidget(this->button);
	this->setLayout(this->layout);

	QObject::connect(this->button, &QPushButton::clicked, this, [this](void) -> void {
		QColor c = QColorDialog::getColor(this->color, this, "Pick a color", QColorDialog::ColorDialogOption::DontUseNativeDialog);
		if (c.isValid() == false) { return ; }
		this->setColor(c);
		return ;
	});
}

void ColorButton::setColor(QColor _color) {
	this->color = _color;
	this->pixmap->fill(this->color);
	if (this->icon != nullptr) { delete this->icon; }
	this->icon = new QIcon(*this->pixmap);
	this->button->setIcon(*this->icon);
	/*
	QPalette button_palette;
	button_palette.setColor(QPalette::Button, this->color);
	this->button->setPalette(button_palette);
	*/
	emit this->colorChanged(this->color);
}

QColor ColorButton::getColor() const { return this->color; }

ControlPanel::ControlPanel(Scene* const scene, Viewer* lv, QWidget* parent) : QWidget(parent), sceneToControl(scene), viewer(lv) {
	this->min = std::numeric_limits<DiscreteGrid::data_t>::lowest();
	this->max = std::numeric_limits<DiscreteGrid::data_t>::max();
	this->minAlternate = std::numeric_limits<DiscreteGrid::data_t>::lowest();
	this->maxAlternate = std::numeric_limits<DiscreteGrid::data_t>::max();

	// Create the groupboxes and their layouts :
	this->groupbox_red = new QGroupBox;
	this->groupbox_green = new QGroupBox;
	this->layout_widgets_red = new QHBoxLayout;
	this->layout_widgets_green = new QHBoxLayout;
	this->groupbox_red->setCheckable(true);
	this->groupbox_green->setCheckable(true);
	this->groupbox_red->setTitle("Red");
	this->groupbox_green->setTitle("Green");

	// Texture bounds for red and green channels :
	this->rangeslider_red = new RangeSlider;
	this->rangeslider_green = new RangeSlider;

	this->red_coloration = new QComboBox;this->green_coloration = new QComboBox;
	this->red_coloration->addItem("Greyscale", ColorFunction::SingleChannel);
	this->red_coloration->addItem("HSV to RGB", ColorFunction::HSV2RGB);
	this->red_coloration->addItem("User colors", ColorFunction::ColorMagnitude);

	this->green_coloration = new QComboBox;
	this->green_coloration->addItem("Greyscale", ColorFunction::SingleChannel);
	this->green_coloration->addItem("HSV to RGB", ColorFunction::HSV2RGB);
	this->green_coloration->addItem("User colors", ColorFunction::ColorMagnitude);

	QColor r = Qt::GlobalColor::red;
	QColor b = Qt::GlobalColor::blue;
	QColor d = Qt::GlobalColor::darkCyan;
	QColor y = Qt::GlobalColor::yellow;
	this->colorbutton_red_min = new ColorButton(r);
	this->colorbutton_red_max = new ColorButton(b);
	this->colorbutton_green_min = new ColorButton(d);
	this->colorbutton_green_max = new ColorButton(y);
	this->sceneToControl->setColor0(r.redF(), r.greenF(), r.blueF());
	this->sceneToControl->setColor1(b.redF(), b.greenF(), b.blueF());
	this->sceneToControl->setColor0Alternate(d.redF(), d.greenF(), d.blueF());
	this->sceneToControl->setColor1Alternate(y.redF(), y.greenF(), y.blueF());

	this->rangeslider_red->setRange(0, this->max-1);
	this->rangeslider_red->setMinValue(0);
	this->rangeslider_red->setMaxValue(this->max-2);
	this->rangeslider_green->setRange(0, this->max-1);
	this->rangeslider_green->setMinValue(0);
	this->rangeslider_green->setMaxValue(this->max-2);

	this->layout_widgets_red->addWidget(this->colorbutton_red_min);
	this->layout_widgets_red->addWidget(this->rangeslider_red); this->layout_widgets_red->setStretch(1, 10);
	this->layout_widgets_red->addWidget(this->colorbutton_red_max);
	this->layout_widgets_red->addWidget(this->red_coloration);
	this->groupbox_red->setLayout(this->layout_widgets_red);

	this->layout_widgets_green->addWidget(this->colorbutton_green_min);
	this->layout_widgets_green->addWidget(this->rangeslider_green); this->layout_widgets_green->setStretch(1, 10);
	this->layout_widgets_green->addWidget(this->colorbutton_green_max);
	this->layout_widgets_green->addWidget(this->green_coloration);
	this->groupbox_green->setLayout(this->layout_widgets_green);

	QLabel* label_Texture = new QLabel("Image intensities");
	label_Texture->setToolTip("Controls the minimum/maximum intensity values visible in the grid.");

	// Grid layout for this widget.
	QGridLayout* grid = new QGridLayout();

	// Add top labels :
	grid->addWidget(label_Texture, 0, 0, 2, 1, Qt::AlignCenter);
	grid->addWidget(this->groupbox_red, 0, 1, 1, 20);
	grid->addWidget(this->groupbox_green, 1, 1, 1, 20);
	this->setLayout(grid);

	this->initSignals();

	if (this->sceneToControl != nullptr) {
		this->updateValues();
	}
}

ControlPanel::~ControlPanel() = default;

void ControlPanel::initSignals() {
	// Modifies the min/max values of the texture to be considered valuable data :
	QObject::connect(this->rangeslider_red, &RangeSlider::minChanged, this, &ControlPanel::setMinTexVal);
	QObject::connect(this->rangeslider_red, &RangeSlider::maxChanged, this, &ControlPanel::setMaxTexVal);

	QObject::connect(this->rangeslider_green, &RangeSlider::minChanged, this, &ControlPanel::setMinTexValBottom);
	QObject::connect(this->rangeslider_green, &RangeSlider::maxChanged, this, &ControlPanel::setMaxTexValBottom);

	// Connect color changes to their respective slots :
	QObject::connect(this->colorbutton_red_min, &ColorButton::colorChanged, this, [this](QColor c) -> void {
		this->sceneToControl->setColor0(c.redF(), c.greenF(), c.blueF());
	});
	QObject::connect(this->colorbutton_red_max, &ColorButton::colorChanged, this, [this](QColor c) -> void {
		this->sceneToControl->setColor1(c.redF(), c.greenF(), c.blueF());
	});
	QObject::connect(this->colorbutton_green_min, &ColorButton::colorChanged, this, [this](QColor c) -> void {
		this->sceneToControl->setColor0Alternate(c.redF(), c.greenF(), c.blueF());
	});
	QObject::connect(this->colorbutton_green_max, &ColorButton::colorChanged, this, [this](QColor c) -> void {
		this->sceneToControl->setColor1Alternate(c.redF(), c.greenF(), c.blueF());
	});

	// Connect the fact of selecting a groupbox to change the scene's information on how to display images
	QObject::connect(this->groupbox_red, &QGroupBox::toggled, this, &ControlPanel::updateRGBMode);
	// Connect the fact of selecting a groupbox to change the scene's information on how to display images
	QObject::connect(this->groupbox_green, &QGroupBox::toggled, this, &ControlPanel::updateRGBMode);

	// connect the comboboxes to their methods :
	QObject::connect(this->red_coloration, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ControlPanel::updateChannelRed);
	QObject::connect(this->green_coloration, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ControlPanel::updateChannelGreen);
}

void ControlPanel::updateViewers() {
	if (this->viewer != nullptr) { this->viewer->update(); }
}

void ControlPanel::updateMinValue(int val) {
	this->min = static_cast<DiscreteGrid::data_t>(val);
	this->rangeslider_red->setMin(val);
	return;
}

void ControlPanel::updateMaxValue(int val) {
	this->max = static_cast<DiscreteGrid::data_t>(val);
	this->rangeslider_red->setMax(val);
	return;
}

void ControlPanel::updateMinValueAlternate(int val) {
	this->minAlternate = static_cast<DiscreteGrid::data_t>(val);
	this->rangeslider_green->setMin(val);
	return;
}

void ControlPanel::updateMaxValueAlternate(int val) {
	this->maxAlternate = static_cast<DiscreteGrid::data_t>(val);
	this->rangeslider_green->setMax(val);
	return;
}

void ControlPanel::updateValues(void) {
	if (this->sceneToControl == nullptr) { return; }
	this->blockSignals(true);
	this->rangeslider_red->blockSignals(true);
	this->rangeslider_green->blockSignals(true);

	this->min = this->sceneToControl->getMinTexValue();
	this->max = this->sceneToControl->getMaxTexValue();
	this->minAlternate = this->sceneToControl->getMinTexValueAlternate();
	this->maxAlternate = this->sceneToControl->getMaxTexValueAlternate();

	this->rangeslider_red->setRange(this->min, this->max);
	this->rangeslider_green->setRange(this->minAlternate, this->maxAlternate);

	this->rangeslider_green->blockSignals(false);
	this->rangeslider_red->blockSignals(false);
	this->blockSignals(false);
}

void ControlPanel::updateLabels(void) {
	//
}

void ControlPanel::updateRGBMode() {
	bool r = this->groupbox_red->isChecked();
	bool g = this->groupbox_green->isChecked();

	if (!r && !g) { this->sceneToControl->setRGBMode(RGBMode::None); return; }
	if ( r && !g) { this->sceneToControl->setRGBMode(RGBMode::RedOnly); return; }
	if (!r &&  g) { this->sceneToControl->setRGBMode(RGBMode::GreenOnly); return; }
	if ( r &&  g) { this->sceneToControl->setRGBMode(RGBMode::RedAndGreen); return; }
}

void ControlPanel::updateChannelRed(int value) {
	switch (value) {
		case 0:
			// Should switch to greyscale
			this->sceneToControl->setColorFunction_r(ColorFunction::SingleChannel);
			this->updateViewers();
		break;
		case 1:
			// Should switch to hsv
			this->sceneToControl->setColorFunction_r(ColorFunction::HSV2RGB);
			this->updateViewers();
		break;
		case 2:
			// Should switch to hsv
			this->sceneToControl->setColorFunction_r(ColorFunction::ColorMagnitude);
			this->updateViewers();
		break;
		default:
			std::cerr << "Cannot switch to function " << value << "\n";
		break;
	}
}

void ControlPanel::updateChannelGreen(int value) {
	switch (value) {
		case 0:
			// Should switch to greyscale
			this->sceneToControl->setColorFunction_g(ColorFunction::SingleChannel);
			this->updateViewers();
		break;
		case 1:
			// Should switch to hsv
			this->sceneToControl->setColorFunction_g(ColorFunction::HSV2RGB);
			this->updateViewers();
		break;
		case 2:
			// Should switch to hsv
			this->sceneToControl->setColorFunction_g(ColorFunction::ColorMagnitude);
			this->updateViewers();
		break;
		default:
			std::cerr << "Cannot switch to function " << value << "\n";
		break;
	}
}

void ControlPanel::setMinTexVal(int val) {
	this->min = static_cast<DiscreteGrid::data_t>(val);
	// update scene data :
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValue(static_cast<DiscreteGrid::data_t>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMaxTexVal(int val) {
	this->max = static_cast<DiscreteGrid::data_t>(val);
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValue(static_cast<DiscreteGrid::data_t>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMinTexValBottom(int val) {
	this->minAlternate = static_cast<DiscreteGrid::data_t>(val);
	// update scene data :
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValueAlternate(static_cast<DiscreteGrid::data_t>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMaxTexValBottom(int val) {
	this->maxAlternate = static_cast<DiscreteGrid::data_t>(val);
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValueAlternate(static_cast<DiscreteGrid::data_t>(val));
	}
	this->updateViewers();
}

void ControlPanel::setClipDistance(double val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetClipDistance(val);
	}
	this->updateViewers();
}
