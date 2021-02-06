#include "../include/visu_box_controller.hpp"

#include "../../viewer/include/scene.hpp"

#include <QLabel>
#include <QGridLayout>

VisuBoxController::VisuBoxController(Scene* _scene) : QWidget(nullptr) {
	this->strayObj.clear();
	this->input_BBMinX = nullptr;
	this->input_BBMinY = nullptr;
	this->input_BBMinZ = nullptr;
	this->input_BBMaxX = nullptr;
	this->input_BBMaxY = nullptr;
	this->input_BBMaxZ = nullptr;
	this->scene = _scene;
	this->setupWidgets();
	this->setAttribute(Qt::WA_DeleteOnClose);

	if (this->scene != nullptr) {
		this->updateValues();
		this->setupSignals();
	}
}

VisuBoxController::~VisuBoxController() {
	if (this->scene != nullptr) {
		this->scene->removeVisuBoxController();
	}
	this->scene = nullptr;
	delete this->input_BBMinX;
	delete this->input_BBMinY;
	delete this->input_BBMinZ;
	delete this->input_BBMaxX;
	delete this->input_BBMaxY;
	delete this->input_BBMaxZ;
	for (std::size_t i = 0; i < this->strayObj.size(); ++i) {
		if (this->strayObj[i]) {
			delete this->strayObj[i];
			this->strayObj[i] = nullptr;
		}
	}
	this->strayObj.clear();
}

void VisuBoxController::setupWidgets() {
	this->input_BBMinX = new QDoubleSpinBox;
	this->input_BBMinY = new QDoubleSpinBox;
	this->input_BBMinZ = new QDoubleSpinBox;
	this->input_BBMaxX = new QDoubleSpinBox;
	this->input_BBMaxY = new QDoubleSpinBox;
	this->input_BBMaxZ = new QDoubleSpinBox;
	this->button_resetBox = new QPushButton("Reset coordinates");

	auto dsbLimits = [](QDoubleSpinBox* d) -> void {
		d->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
		d->setValue(.0);
		d->setSingleStep(.1);
	};

	dsbLimits(this->input_BBMinX);
	dsbLimits(this->input_BBMinY);
	dsbLimits(this->input_BBMinZ);
	dsbLimits(this->input_BBMaxX);
	dsbLimits(this->input_BBMaxY);
	dsbLimits(this->input_BBMaxZ);

	int bRow = 0; // row counter for 'bounding box' layout
	int mRow = 0; // row counter for 'main' layout

	// Labels needed for the layouts :
	QLabel* label_gridControl = new QLabel("Visu Box Control"); this->strayObj.push_back(label_gridControl);
	QLabel* label_Header_X = new QLabel(" X "); this->strayObj.push_back(label_Header_X);
	QLabel* label_Header_Y = new QLabel(" Y "); this->strayObj.push_back(label_Header_Y);
	QLabel* label_Header_Z = new QLabel(" Z "); this->strayObj.push_back(label_Header_Z);
	QLabel* label_position = new QLabel("Position : "); this->strayObj.push_back(label_position);
	QLabel* label_diagonal = new QLabel("Diagonal : "); this->strayObj.push_back(label_diagonal);

	// create a few layouts :
	QGridLayout* layout_BoundingBox = new QGridLayout;
	QGridLayout* mainLayout = new QGridLayout;
	QFrame* frame_BoundingBox = new QFrame;

	//===========================//
	// Add bounding box controls //
	//===========================//
	layout_BoundingBox->addWidget(label_Header_X, bRow, 2, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(label_Header_Y, bRow, 4, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(label_Header_Z, bRow, 6, Qt::AlignHCenter);
	bRow++;
	// BB min :
	layout_BoundingBox->addWidget(label_position, bRow, 0, Qt::AlignLeft);
	layout_BoundingBox->addWidget(this->input_BBMinX, bRow, 2, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_BBMinY, bRow, 4, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_BBMinZ, bRow, 6, Qt::AlignHCenter);
	bRow++;
	// BB max :
	layout_BoundingBox->addWidget(label_diagonal, bRow, 0, Qt::AlignLeft);
	layout_BoundingBox->addWidget(this->input_BBMaxX, bRow, 2, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_BBMaxY, bRow, 4, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_BBMaxZ, bRow, 6, Qt::AlignHCenter);
	bRow++;
	// button to reset :
	layout_BoundingBox->addWidget(this->button_resetBox, bRow, 1, 1, 4, Qt::AlignCenter);
	frame_BoundingBox->setLayout(layout_BoundingBox);
	frame_BoundingBox->setStyleSheet(".QFrame{border: 2px solid grey;border-radius: 4px;}");

	//========================================//
	// Merge grid/voxel and BB layouts in one //
	//========================================//
	// First, add header and grid name :
	mainLayout->addWidget(label_gridControl, mRow, 0, 1, -1, Qt::AlignHCenter); mRow += 2; // bit of spacing
	// Add bb controls :
	mainLayout->addWidget(frame_BoundingBox, mRow, 0, 1, -1); mRow+=2; // space to next widget
	mRow++;

	this->setLayout(mainLayout);

	return;
}

void VisuBoxController::setupSignals() {
	QObject::connect(this->input_BBMinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_BBMinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_BBMinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_BBMaxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_BBMaxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_BBMaxZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->button_resetBox, &QPushButton::clicked, [this]() {
		if (this->scene != nullptr) { this->scene->resetVisuBox(); }
	});
}

void VisuBoxController::updateValues() {
	if (this->scene == nullptr) {
		return;
	}
	this->blockSignals(true);
	auto box = this->scene->getVisuBox();
	this->input_BBMinX->setValue(box.getMin().x);
	this->input_BBMinY->setValue(box.getMin().y);
	this->input_BBMinZ->setValue(box.getMin().z);
	auto diag = box.getDiagonal();
	this->input_BBMaxX->setValue(diag.x);
	this->input_BBMaxY->setValue(diag.y);
	this->input_BBMaxZ->setValue(diag.z);
	this->blockSignals(false);
	return;
}

void VisuBoxController::blockSignals(bool b) {
	this->input_BBMinX->blockSignals(b);
	this->input_BBMinY->blockSignals(b);
	this->input_BBMinZ->blockSignals(b);
	this->input_BBMaxX->blockSignals(b);
	this->input_BBMaxY->blockSignals(b);
	this->input_BBMaxZ->blockSignals(b);
}

void VisuBoxController::updateBox() {
	DiscreteGrid::bbox_t::vec::value_type minX = this->input_BBMinX->value();
	DiscreteGrid::bbox_t::vec::value_type minY = this->input_BBMinY->value();
	DiscreteGrid::bbox_t::vec::value_type minZ = this->input_BBMinZ->value();
	DiscreteGrid::bbox_t::vec min{minX, minY, minZ};
	DiscreteGrid::bbox_t::vec::value_type maxX = this->input_BBMaxX->value();
	DiscreteGrid::bbox_t::vec::value_type maxY = this->input_BBMaxY->value();
	DiscreteGrid::bbox_t::vec::value_type maxZ = this->input_BBMaxZ->value();
	DiscreteGrid::bbox_t::vec max{maxX, maxY, maxZ};
	this->scene->setVisuBox(DiscreteGrid::bbox_t(min, min + max));
	return;
}
