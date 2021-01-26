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
	this->minValueTexture = new QSlider(Qt::Vertical);
	this->maxValueTexture = new QSlider(Qt::Vertical);
	this->minValueColor = new QSlider(Qt::Vertical);
	this->maxValueColor = new QSlider(Qt::Vertical);

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

	this->minValueColor->setRange(0, 255);
	this->minValueColor->setValue(1);
	this->maxValueColor->setRange(0, 255);
	this->maxValueColor->setValue(255);

	QSizePolicy policy;
	policy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
	this->minValueColor->setSizePolicy(policy);

	QLabel* label_Color = new QLabel("Color");
	QLabel* label_Texture = new QLabel("Texture");
	QLabel* label_Min_Col = new QLabel("Min");
	QLabel* label_Max_Col = new QLabel("Max");
	QLabel* label_Min_Tex = new QLabel("Min");
	QLabel* label_Max_Tex = new QLabel("Max");

	label_Color->setToolTip("Controls the minimum/maximum values for the color scale computation.");
	label_Texture->setToolTip("Controls the minimum/maximum intensity values visible in the grid.");

	/**
	 * The widgets in this widget will be laid out on a grid layout, with the following arragement :
	 * Row 0 : MIN and MAX labels for the columns below
	 * Row 1 : Color control. This will include a label for color intensity, and the spinboxes to control the color
	 *         bounds given to the shader programs when drawing the grid(s).
	 * Row 2 : Texture control. This will include a label for texture intensity, and the spinboxes to control the
	 *         texure bounds given to the shader programs to compute visibility of different subdomains.
	 * Row 3 : Camera cutting plane control
	 */

	// Grid layout for this widget.
	QGridLayout* grid = new QGridLayout();

	QFrame* frame = new QFrame;
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(this->minValueColor);
	frame->setLayout(layout);

	// Add top labels :
	grid->addWidget(label_Color, 0, 0, 1, 2, Qt::AlignHCenter);
	grid->addWidget(label_Texture, 0, 2, 1, 2, Qt::AlignHCenter);
	// Add lower labels (min/max) :
	grid->addWidget(label_Min_Col, 1, 0, 1, 1, Qt::AlignHCenter);
	grid->addWidget(label_Max_Col, 1, 1, 1, 1, Qt::AlignHCenter);
	grid->addWidget(label_Min_Tex, 1, 2, 1, 1, Qt::AlignHCenter);
	grid->addWidget(label_Max_Tex, 1, 3, 1, 1, Qt::AlignHCenter);
	// Add color sliders :
	grid->addWidget(this->minValueColor, 2, 0, 20, 1, Qt::AlignHCenter);
	grid->addWidget(this->maxValueColor, 2, 1, 20, 1, Qt::AlignHCenter);
	// Add texture sliders :
	grid->addWidget(this->minValueTexture, 2, 2, 20, 1, Qt::AlignHCenter);
	grid->addWidget(this->maxValueTexture, 2, 3, 20, 1, Qt::AlignHCenter);
	grid->addWidget(this->clipDistance, 23, 0, 1, 4, Qt::AlignHCenter);

	// Huge row for the sliders !
	//grid->setRowMinimumHeight(2, 600);

	this->controlContainer->setLayout(grid);

	// Disable by default the top level container :
	this->controlContainer->setEnabled(false);

	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(this->controlContainer);

	this->setLayout(mainLayout);
	//this->setStyleSheet("background-color:red;");

	this->initSignals();

	if (this->sceneToControl != nullptr) {
		this->updateValues();
	}

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

	this->clipDistance->disconnect();

	deletePtr(this->minValueTexture);
	deletePtr(this->maxValueTexture);
	deletePtr(this->clipDistance);
	deletePtr(this->controlContainer);
}

void ControlPanel::initSignals() {
	connect(this->clipDistance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControlPanel::setClipDistance);

	// Modifies the min/max values of the texture to be considered valuable data :
	QObject::connect(this->minValueTexture, &QSlider::valueChanged, this, &ControlPanel::setMinTexVal);
	QObject::connect(this->maxValueTexture, &QSlider::valueChanged, this, &ControlPanel::setMaxTexVal);
	// Same for colors :
	QObject::connect(this->minValueColor, &QSlider::valueChanged, this, &ControlPanel::setMinColVal);
	QObject::connect(this->maxValueColor, &QSlider::valueChanged, this, &ControlPanel::setMaxColVal);

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
	this->minValueColor->blockSignals(true);
	this->maxValueColor->blockSignals(true);
	this->minValueTexture->setValue(this->sceneToControl->getMinTexValue());
	this->maxValueTexture->setValue(this->sceneToControl->getMaxTexValue());
	this->minValueColor->setValue(this->sceneToControl->getMinColorValue());
	this->maxValueColor->setValue(this->sceneToControl->getMaxColorValue());
	this->maxValueColor->blockSignals(false);
	this->minValueColor->blockSignals(false);
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
	int otherval = this->maxValueColor->value();
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
	this->updateViewers();
}

void ControlPanel::setMaxColVal(int val) {
	int otherval = this->minValueColor->value();
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
	this->updateViewers();
}

void ControlPanel::setClipDistance(double val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetClipDistance(val);
	}
	this->updateViewers();
}
