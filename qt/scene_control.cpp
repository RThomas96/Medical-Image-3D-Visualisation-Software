#include "scene_control.hpp"
#include "3D_viewer.hpp"
#include "scene.hpp"
#include "qtabwidget.h"

#include <QColorDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

ColorButton::ColorButton(QColor _color, QWidget* parent) :
	QWidget(parent) {
	this->color	 = _color;
	this->button = new QPushButton;
	this->pixmap = new QPixmap(30, 30);
	this->icon	 = nullptr;
	this->setColor(this->color);
	this->button->setFixedSize(this->button->sizeHint());

	this->layout = new QVBoxLayout();
	this->layout->addWidget(this->button);
	this->setLayout(this->layout);

	QObject::connect(this->button, &QPushButton::clicked, this, [this](void) -> void {
		QColor c = QColorDialog::getColor(this->color, this, "Pick a color", QColorDialog::ColorDialogOption::DontUseNativeDialog);
		if (c.isValid() == false) {
			return;
		}
		this->setColor(c);
		return;
	});
}

ColorButton::~ColorButton() {
	delete this->icon;
	delete this->pixmap;
	delete this->button;
}

void ColorButton::setColor(QColor _color) {
	this->color = _color;
	this->pixmap->fill(this->color);
	if (this->icon != nullptr) {
		delete this->icon;
	}
	this->icon = new QIcon(*this->pixmap);
	this->button->setIcon(*this->icon);
	/*
	QPalette button_palette;
	button_palette.setColor(QPalette::Button, this->color);
	this->button->setPalette(button_palette);
	*/
	emit this->colorChanged(this->color);
}

QColor ColorButton::getColor() const {
	return this->color;
}

ColorBoundsControl::ColorBoundsControl(Scene* _scene, bool _prim, QWidget* parent) :
	QWidget(parent) {
	this->_primary = _prim;
	this->scene	   = _scene;
	this->sb_min   = new QSpinBox;
	this->sb_max   = new QSpinBox;
	this->sb_min->setRange(0, 65535);
	this->sb_max->setRange(0, 65535);
	this->layout	  = new QGridLayout;
	QLabel* label_min = new QLabel("Minimum");
	QLabel* label_max = new QLabel("Maximum");
	this->getCurrentValues();
	this->layout->addWidget(label_min, 0, 0);
	this->layout->addWidget(label_max, 0, 1);
	this->layout->addWidget(this->sb_min, 1, 0);
	this->layout->addWidget(this->sb_max, 1, 1);
	this->setLayout(this->layout);
	QObject::connect(this->sb_min, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorBoundsControl::minChanged);
	QObject::connect(this->sb_max, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorBoundsControl::maxChanged);
	this->setAttribute(Qt::WA_DeleteOnClose);
	if (this->_primary) {
		this->setWindowTitle("Channel 1 bounds");
	} else {
		this->setWindowTitle("Channel 2 bounds");
	}
}

ColorBoundsControl::~ColorBoundsControl() = default;

void ColorBoundsControl::getCurrentValues() {
	int mi, ma;
	if (this->_primary) {
		mi = static_cast<int>(this->scene->getMinColorValue());
		ma = static_cast<int>(this->scene->getMaxColorValue());
	} else {
		mi = static_cast<int>(this->scene->getMinColorValueAlternate());
		ma = static_cast<int>(this->scene->getMaxColorValueAlternate());
	}
	this->sb_min->setValue(mi);
	this->sb_max->setValue(ma);
}

ControlPanel::ControlPanel(Scene* const scene, Viewer* lv, QWidget* parent) :
	QWidget(parent), sceneToControl(scene), viewer(lv) {
	this->min		   = 0;
	this->max		   = 1;
	this->minAlternate = 0;
	this->maxAlternate = 1;

	this->cb_red_bounds	  = nullptr;
	this->cb_green_bounds = nullptr;

	this->button_red_colorbounds   = new QPushButton("Change min/max");
	this->button_green_colorbounds = new QPushButton("Change min/max");

	// Create the groupboxes and their layouts :
	this->groupbox_red		   = new QGroupBox;
	this->groupbox_green	   = new QGroupBox;
	this->layout_widgets_red   = new QHBoxLayout;
	this->layout_widgets_green = new QHBoxLayout;
	this->groupbox_red->setCheckable(true);
	this->groupbox_green->setCheckable(true);
    this->groupbox_red->setTitle("Grid 1");
    this->groupbox_green->setTitle("Grid 2");

	// Texture bounds for red and green channels :
	this->rangeslider_red	= new DoubleSlider;
	this->rangeslider_green = new DoubleSlider;

	this->red_coloration = new QComboBox;
	this->red_coloration->addItem("Greyscale", ColorFunction::SingleChannel);
	this->red_coloration->addItem("HSV to RGB", ColorFunction::HSV2RGB);
	this->red_coloration->addItem("User colors", ColorFunction::ColorMagnitude);
    this->red_coloration->setCurrentIndex(2);

	this->green_coloration = new QComboBox;
	this->green_coloration->addItem("Greyscale", ColorFunction::SingleChannel);
	this->green_coloration->addItem("HSV to RGB", ColorFunction::HSV2RGB);
	this->green_coloration->addItem("User colors", ColorFunction::ColorMagnitude);
    this->green_coloration->setCurrentIndex(2);

	QColor r					= Qt::GlobalColor::red;
	QColor b					= Qt::GlobalColor::blue;
	QColor d					= Qt::GlobalColor::darkCyan;
	QColor y					= Qt::GlobalColor::yellow;
	this->colorbutton_red_min	= new ColorButton(r);
	this->colorbutton_red_max	= new ColorButton(b);
	this->colorbutton_green_min = new ColorButton(d);
	this->colorbutton_green_max = new ColorButton(y);
	this->sceneToControl->setColor0(r.redF(), r.greenF(), r.blueF());
	this->sceneToControl->setColor1(b.redF(), b.greenF(), b.blueF());
	this->sceneToControl->setColor0Alternate(d.redF(), d.greenF(), d.blueF());
	this->sceneToControl->setColor1Alternate(y.redF(), y.greenF(), y.blueF());

	this->rangeslider_red->setRange(0, this->max - 1);
	this->rangeslider_red->setMinValue(0);
	this->rangeslider_red->setMaxValue(this->max - 2);
	this->rangeslider_green->setRange(0, this->max - 1);
	this->rangeslider_green->setMinValue(0);
	this->rangeslider_green->setMaxValue(this->max - 2);

	this->layout_widgets_red->addWidget(this->colorbutton_red_min);
	this->layout_widgets_red->addWidget(this->rangeslider_red);
	//	this->layout_widgets_red->setColumnStretch(1, 10);
	this->layout_widgets_red->addWidget(this->colorbutton_red_max);
	this->layout_widgets_red->addWidget(this->red_coloration);
	this->layout_widgets_red->addWidget(this->button_red_colorbounds);
	this->groupbox_red->setLayout(this->layout_widgets_red);

	this->layout_widgets_green->addWidget(this->colorbutton_green_min);
	this->layout_widgets_green->addWidget(this->rangeslider_green);
	//	this->layout_widgets_green->setColumnStretch(1, 10);
	this->layout_widgets_green->addWidget(this->colorbutton_green_max);
	this->layout_widgets_green->addWidget(this->green_coloration);
	this->layout_widgets_green->addWidget(this->button_green_colorbounds);
	this->groupbox_green->setLayout(this->layout_widgets_green);

	QLabel* label_Texture = new QLabel("Image intensities");
	label_Texture->setToolTip("Controls the minimum/maximum intensity values visible in the grid.");

	// Grid layout for this widget.
	QGridLayout* grid = new QGridLayout();

	// Add top labels :
	//grid->addWidget(label_Texture, 0, 0, 2, 1, Qt::AlignCenter);
	grid->addWidget(this->groupbox_red, 0, 1, 1, 20);
	grid->addWidget(this->groupbox_green, 1, 1, 1, 20);
	grid->setRowStretch(0, 0);
	grid->setRowStretch(1, 0);

    this->slideColorControl = new QWidget();
    this->slideColorControl->setLayout(grid);
    this->segmentedColorControl = new QWidget();

    this->tab = new QTabWidget();
    this->tab->addTab(this->slideColorControl, QString("Slider"));
    //this->tab->addTab(this->segmentedColorControl, QString("Classes"));

    QGridLayout* finalGrid = new QGridLayout();
    finalGrid->addWidget(this->tab);
    this->setLayout(finalGrid);

	this->initSignals();

	//	this->layout_widgets_red->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
	//	this->layout_widgets_green->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
	//	this->layout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

	if (this->sceneToControl != nullptr) {
		this->updateValues();
	}
}

ControlPanel::~ControlPanel() {
	delete this->button_green_colorbounds;
	delete this->button_red_colorbounds;
	delete this->green_coloration;
	delete this->red_coloration;
	delete this->colorbutton_green_max;
	delete this->colorbutton_green_min;
	delete this->colorbutton_red_max;
	delete this->colorbutton_red_min;
	delete this->rangeslider_green;
	delete this->rangeslider_red;
	delete this->groupbox_green;
	delete this->groupbox_red;
}

void ControlPanel::initSignals() {
	// Modifies the min/max values of the texture to be considered valuable data :
	QObject::connect(this->rangeslider_red, &DoubleSlider::minChanged, this, &ControlPanel::setMinTexVal);
	QObject::connect(this->rangeslider_red, &DoubleSlider::maxChanged, this, &ControlPanel::setMaxTexVal);

	QObject::connect(this->rangeslider_green, &DoubleSlider::minChanged, this, &ControlPanel::setMinTexValAlternate);
	QObject::connect(this->rangeslider_green, &DoubleSlider::maxChanged, this, &ControlPanel::setMaxTexValAlternate);

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

	QObject::connect(this->button_red_colorbounds, &QPushButton::clicked, this, &ControlPanel::launchRedColorBounds);
	QObject::connect(this->button_green_colorbounds, &QPushButton::clicked, this, &ControlPanel::launchGreenColorBounds);
}

void ControlPanel::updateViewers() {
	if (this->viewer != nullptr) {
		this->viewer->update();
	}
}

void ControlPanel::updateMinValue(double val) {
	this->min = val;
	this->rangeslider_red->setMin(val);
    this->rangeslider_red->setMinValue(val);
    return;
}

void ControlPanel::updateMaxValue(double val) {
	this->max = val;
	this->rangeslider_red->setMax(val);
    this->rangeslider_red->setMaxValue(val);
    return;
}

void ControlPanel::updateMinValueAlternate(double val) {
	this->minAlternate = val;
	this->rangeslider_green->setMin(val);
    this->rangeslider_green->setMinValue(val);
    return;
}

void ControlPanel::updateMaxValueAlternate(double val) {
	this->maxAlternate = val;
	this->rangeslider_green->setMax(val);
    this->rangeslider_green->setMaxValue(val);
    return;
}

void ControlPanel::updateValues(void) {
	if (this->sceneToControl == nullptr) {
		return;
	}
	this->blockSignals(true);
	this->rangeslider_red->blockSignals(true);
	this->rangeslider_green->blockSignals(true);

	this->min		   = this->sceneToControl->getMinTexValue();
	this->max		   = this->sceneToControl->getMaxTexValue();
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

void ControlPanel::launchRedColorBounds() {
	if (this->cb_red_bounds == nullptr) {
		this->cb_red_bounds = new ColorBoundsControl(this->sceneToControl, true, nullptr);
		QObject::connect(this->cb_red_bounds, &QWidget::destroyed, this, [this]() -> void {
			this->cb_red_bounds = nullptr;
		});
		QObject::connect(this->cb_red_bounds, &ColorBoundsControl::minChanged, this, [this](int val) {
			this->updateMinValue(val);
			this->sceneToControl->slotSetMinColorValue(static_cast<double>(val));
		});
		QObject::connect(this->cb_red_bounds, &ColorBoundsControl::maxChanged, this, [this](int val) {
			this->updateMaxValue(val);
			this->sceneToControl->slotSetMaxColorValue(static_cast<double>(val));
		});
	}
	this->cb_red_bounds->raise();
	this->cb_red_bounds->show();
}

void ControlPanel::launchGreenColorBounds() {
	if (this->cb_green_bounds == nullptr) {
		this->cb_green_bounds = new ColorBoundsControl(this->sceneToControl, false, nullptr);
		QObject::connect(this->cb_green_bounds, &QWidget::destroyed, this, [this]() -> void {
			this->cb_green_bounds = nullptr;
		});
		QObject::connect(this->cb_green_bounds, &ColorBoundsControl::minChanged, this, [this](int val) {
			this->updateMinValueAlternate(val);
			this->sceneToControl->slotSetMinColorValueAlternate(static_cast<double>(val));
		});
		QObject::connect(this->cb_green_bounds, &ColorBoundsControl::maxChanged, this, [this](int val) {
			this->updateMaxValueAlternate(val);
			this->sceneToControl->slotSetMaxColorValueAlternate(static_cast<double>(val));
		});
	}
	this->cb_green_bounds->raise();
	this->cb_green_bounds->show();
}

void ControlPanel::updateRGBMode() {
	bool r = this->groupbox_red->isChecked();
	bool g = this->groupbox_green->isChecked();

	if (! r && ! g) {
		this->sceneToControl->setColorChannel(ColorChannel::None);
        this->sceneToControl->setGridsToDraw({});
        return;
	}
	if (r && ! g) {
		this->sceneToControl->setColorChannel(ColorChannel::RedOnly);
        this->sceneToControl->setGridsToDraw({0});
        return;
	}
	if (! r && g) {
		this->sceneToControl->setColorChannel(ColorChannel::GreenOnly);
        this->sceneToControl->setGridsToDraw({1});
        return;
	}
	if (r && g) {
		this->sceneToControl->setColorChannel(ColorChannel::RedAndGreen);
        this->sceneToControl->setGridsToDraw({0, 1});
        return;
	}
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

void ControlPanel::setMinTexVal(double val) {
	this->min = val;
	// update scene data :
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValue(val);
	}
	this->updateViewers();
}

void ControlPanel::setMaxTexVal(double val) {
	this->max = val;
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValue(val);
	}
	this->updateViewers();
}

void ControlPanel::setMinTexValAlternate(double val) {
	this->minAlternate = val;
	// update scene data :
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValueAlternate(val);
	}
	this->updateViewers();
}

void ControlPanel::setMaxTexValAlternate(double val) {
	this->maxAlternate = val;
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValueAlternate(val);
	}
	this->updateViewers();
}

void ControlPanel::setSlidersToNumericalLimits(int gridIdx) {
	// TODO: make the slider a ratio

	double minValue = sceneToControl->getMinNumericLimit(0);
	double maxValue = sceneToControl->getMaxNumericLimit(0);

	if (minValue < static_cast<double>(std::numeric_limits<int>::lowest())) {
		minValue = static_cast<double>(std::numeric_limits<int>::lowest());
		std::cerr << "Error: sliders cannot handle datatypes bigger than integer. Values are crop." << std::endl;
	}

	if (maxValue > static_cast<double>(std::numeric_limits<int>::max())) {
		maxValue = static_cast<double>(std::numeric_limits<int>::max());
		std::cerr << "Error: sliders cannot handle datatypes bigger than integer. Values are crop." << std::endl;
	}

    if(gridIdx == 0) {
        setMinTexVal(minValue);
        setMaxTexVal(maxValue);

        updateMinValue(minValue);
        updateMaxValue(maxValue);
    } else {
        setMinTexValAlternate(minValue);
        setMaxTexValAlternate(maxValue);

        updateMinValueAlternate(minValue);
        updateMaxValueAlternate(maxValue);
    }

}
