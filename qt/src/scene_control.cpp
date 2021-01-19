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

	// Texture color scale bounds :
	this->minValueTexture = new QSpinBox();
	this->maxValueTexture = new QSpinBox();
	this->minValueColor = new QSpinBox();
	this->maxValueColor = new QSpinBox();

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

	QLabel* minTexLabel = new QLabel("Min texture value");
	QLabel* maxTexLabel = new QLabel("Max texture value");
	QLabel* minColorLabel = new QLabel("Min color value");
	QLabel* maxColorLabel = new QLabel("Min color value");

	QLabel* label_Color = new QLabel("Color intensities : ");
	QLabel* label_Texture = new QLabel("Texture intensities : ");
	QLabel* label_Min = new QLabel("Minimum");
	QLabel* label_Max = new QLabel("Maximum");

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

	grid->addWidget(label_Min, 0, 1, Qt::AlignCenter);
	grid->addWidget(label_Max, 0, 2, Qt::AlignCenter);
	grid->addWidget(label_Color, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	grid->addWidget(this->minValueColor, 1, 1, Qt::AlignCenter);
	grid->addWidget(this->maxValueColor, 1, 2, Qt::AlignCenter);
	grid->addWidget(label_Texture, 2, 0, Qt::AlignRight | Qt::AlignVCenter);
	grid->addWidget(this->minValueTexture, 2, 1, Qt::AlignCenter);
	grid->addWidget(this->maxValueTexture, 2, 2, Qt::AlignCenter);
	grid->addWidget(this->clipDistance, 3, 0, 1, 3, Qt::AlignCenter);
	this->controlContainer->setLayout(grid);

	grid->setColumnStretch(1, 0);
	grid->setColumnStretch(2, 0);

	// Disable by default the top level container :
	this->controlContainer->setEnabled(false);

	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(this->controlContainer);

	this->setLayout(mainLayout);

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
	connect(this->minValueTexture, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMinTexVal);
	connect(this->maxValueTexture, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMaxTexVal);
	// Same for colors :
	connect(this->minValueColor, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMinColVal);
	connect(this->maxValueColor, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::setMaxColVal);
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

void ControlPanel::setTexCube(bool show) {
	this->sceneToControl->slotToggleShowTextureCube(show);
	this->updateViewers();
}

void ControlPanel::setMinTexVal(int val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinTexValue(static_cast<uchar>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMaxTexVal(int val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMaxTexValue(static_cast<uchar>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMinColVal(int val) {
	if (this->sceneToControl) {
		this->sceneToControl->slotSetMinColorValue(static_cast<uchar>(val));
	}
	this->updateViewers();
}

void ControlPanel::setMaxColVal(int val) {
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

void ControlPanel::setCutPlaneXPos(double val) {
	float ratio = static_cast<float>(val);
	this->sceneToControl->slotSetPlanePositionX(ratio);
	this->updateViewers();
}

void ControlPanel::setCutPlaneYPos(double val) {
	float ratio = static_cast<float>(val);
	this->sceneToControl->slotSetPlanePositionY(ratio);
	this->updateViewers();
}

void ControlPanel::setCutPlaneZPos(double val) {
	float ratio = static_cast<float>(val);
	this->sceneToControl->slotSetPlanePositionZ(ratio);
	this->updateViewers();
}
