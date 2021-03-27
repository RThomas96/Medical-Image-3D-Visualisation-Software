#include "../include/visu_box_controller.hpp"

#include "../include/neighbor_visu_main_widget.hpp"
#include "../../viewer/include/scene.hpp"
#include "../../viewer/include/neighbor_visu_viewer.hpp"

#include <QLabel>
#include <QGridLayout>

VisuBoxController::VisuBoxController(Scene* _scene, MainWidget* _main) : QWidget(nullptr) {
	this->strayObj.clear();
	this->input_coordMinX = nullptr;
	this->input_coordMinY = nullptr;
	this->input_coordMinZ = nullptr;
	this->input_coordMaxX = nullptr;
	this->input_coordMaxY = nullptr;
	this->input_coordMaxZ = nullptr;
	this->scene = _scene;
	this->main = _main;
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
	delete this->input_coordMinX;
	delete this->input_coordMinY;
	delete this->input_coordMinZ;
	delete this->input_coordMaxX;
	delete this->input_coordMaxY;
	delete this->input_coordMaxZ;
	for (std::size_t i = 0; i < this->strayObj.size(); ++i) {
		if (this->strayObj[i]) {
			delete this->strayObj[i];
			this->strayObj[i] = nullptr;
		}
	}
	this->strayObj.clear();
}

void VisuBoxController::setupWidgets() {
	this->input_coordMinX = new QSpinBox;
	this->input_coordMinY = new QSpinBox;
	this->input_coordMinZ = new QSpinBox;
	this->input_coordMaxX = new QSpinBox;
	this->input_coordMaxY = new QSpinBox;
	this->input_coordMaxZ = new QSpinBox;
	this->button_resetBox = new QPushButton("Reset coordinates");
	this->button_loadROI = new QPushButton("Load high-res");

	auto dsbLimits = [](QSpinBox* d) -> void {
		d->setRange(0, std::numeric_limits<int>::max());
		d->setValue(0);
		d->setSingleStep(1);
	};

	dsbLimits(this->input_coordMinX);
	dsbLimits(this->input_coordMinY);
	dsbLimits(this->input_coordMinZ);
	dsbLimits(this->input_coordMaxX);
	dsbLimits(this->input_coordMaxY);
	dsbLimits(this->input_coordMaxZ);

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
	layout_BoundingBox->addWidget(this->input_coordMinX, bRow, 2, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_coordMinY, bRow, 4, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_coordMinZ, bRow, 6, Qt::AlignHCenter);
	bRow++;
	// BB max :
	layout_BoundingBox->addWidget(label_diagonal, bRow, 0, Qt::AlignLeft);
	layout_BoundingBox->addWidget(this->input_coordMaxX, bRow, 2, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_coordMaxY, bRow, 4, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_coordMaxZ, bRow, 6, Qt::AlignHCenter);
	bRow++;
	// button to reset :
	layout_BoundingBox->addWidget(this->button_resetBox, bRow, 0, 1, 3, Qt::AlignCenter);
	layout_BoundingBox->addWidget(this->button_resetBox, bRow, 3, 1, 3, Qt::AlignCenter);
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
	QObject::connect(this->input_coordMinX, QOverload<int>::of(&QSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_coordMinY, QOverload<int>::of(&QSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_coordMinZ, QOverload<int>::of(&QSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_coordMaxX, QOverload<int>::of(&QSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_coordMaxY, QOverload<int>::of(&QSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->input_coordMaxZ, QOverload<int>::of(&QSpinBox::valueChanged), this, &VisuBoxController::updateBox);
	QObject::connect(this->button_resetBox, &QPushButton::clicked, [this]() {
		if (this->scene != nullptr) { this->scene->resetVisuBox(); }
		this->updateValues();
	});
	QObject::connect(this->button_loadROI, &QPushButton::clicked, this, [this](void) -> void {
		if (this->scene == nullptr) { return; }
		Viewer* viewer = this->main->getViewer3D();
		viewer->makeCurrent();
		this->scene->loadGridROI();
		viewer->doneCurrent();
		viewer->update();
	});
}

void VisuBoxController::updateValues() {
	if (this->scene == nullptr) {
		return;
	}
	this->blockSignals(true);
	auto box = this->scene->getVisuBoxCoordinates();
	this->input_coordMinX->setValue(box.first.x);
	this->input_coordMinY->setValue(box.first.y);
	this->input_coordMinZ->setValue(box.first.z);
	this->input_coordMaxX->setValue(box.second.x);
	this->input_coordMaxY->setValue(box.second.y);
	this->input_coordMaxZ->setValue(box.second.z);
	this->blockSignals(false);
	return;
}

void VisuBoxController::blockSignals(bool b) {
	this->input_coordMinX->blockSignals(b);
	this->input_coordMinY->blockSignals(b);
	this->input_coordMinZ->blockSignals(b);
	this->input_coordMaxX->blockSignals(b);
	this->input_coordMaxY->blockSignals(b);
	this->input_coordMaxZ->blockSignals(b);
}

void VisuBoxController::updateBox() {
	unsigned int minX = static_cast<unsigned int>((this->input_coordMinX->value() > 0) ? this->input_coordMinX->value() : 0);
	unsigned int minY = static_cast<unsigned int>((this->input_coordMinY->value() > 0) ? this->input_coordMinY->value() : 0);
	unsigned int minZ = static_cast<unsigned int>((this->input_coordMinZ->value() > 0) ? this->input_coordMinZ->value() : 0);
	unsigned int maxX = static_cast<unsigned int>((this->input_coordMaxX->value() > 0) ? this->input_coordMaxX->value() : 0);
	unsigned int maxY = static_cast<unsigned int>((this->input_coordMaxY->value() > 0) ? this->input_coordMaxY->value() : 0);
	unsigned int maxZ = static_cast<unsigned int>((this->input_coordMaxZ->value() > 0) ? this->input_coordMaxZ->value() : 0);
	glm::uvec3 max{maxX, maxY, maxZ};
	glm::uvec3 min{minX, minY, minZ};
	this->scene->setVisuBoxMinCoord(min);
	this->scene->setVisuBoxMaxCoord(max);

	return;
}
