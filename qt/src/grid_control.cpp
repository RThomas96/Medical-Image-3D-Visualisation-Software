#include "../include/grid_control.hpp"
#include "../../image/include/writer.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>

GridControl::GridControl(std::shared_ptr<DiscreteGrid> vg, QWidget* parent) : QWidget(parent) {
	this->voxelGrid = vg;
	this->setupWidgets();
	if (this->voxelGrid != nullptr) {
		this->updateGridDimensions();
		if (this->voxelGrid->isModifiable()) {
			this->setupSignals();
		} else {
			this->disableWidgets();
		}
	}
}

GridControl::~GridControl() {
	delete this->input_GridSizeX;
	delete this->input_GridSizeY;
	delete this->input_GridSizeZ;
	delete this->methodPicker;
	delete this->info_GridSizeTotal;
	delete this->info_VoxelSize;
	delete this->button_FillButton;
	delete this->input_GridBBMinX;
	delete this->input_GridBBMinY;
	delete this->input_GridBBMinZ;
	delete this->input_GridBBMaxX;
	delete this->input_GridBBMaxY;
	delete this->input_GridBBMaxZ;
	delete this->info_VoxelRate;
	delete this->info_TotalTime;
	delete this->info_MemorySize;
	delete this->nameLabel;
}

void GridControl::enableWidgets() {
	this->input_GridSizeX->setDisabled(false);
	this->input_GridSizeY->setDisabled(false);
	this->input_GridSizeZ->setDisabled(false);
	this->methodPicker->setDisabled(false);
	this->info_GridSizeTotal->setDisabled(false);
	this->info_VoxelSize->setDisabled(false);
	this->button_FillButton->setDisabled(false);
	this->input_GridBBMinX->setDisabled(false);
	this->input_GridBBMinY->setDisabled(false);
	this->input_GridBBMinZ->setDisabled(false);
	this->input_GridBBMaxX->setDisabled(false);
	this->input_GridBBMaxY->setDisabled(false);
	this->input_GridBBMaxZ->setDisabled(false);
	this->info_VoxelRate->setDisabled(false);
	this->info_TotalTime->setDisabled(false);
	this->info_MemorySize->setDisabled(false);
}

void GridControl::disableWidgets() {
	this->input_GridSizeX->setDisabled(true);
	this->input_GridSizeY->setDisabled(true);
	this->input_GridSizeZ->setDisabled(true);
	this->methodPicker->setDisabled(true);
	this->info_GridSizeTotal->setDisabled(true);
	this->info_VoxelSize->setDisabled(true);
	this->button_FillButton->setDisabled(true);
	this->input_GridBBMinX->setDisabled(true);
	this->input_GridBBMinY->setDisabled(true);
	this->input_GridBBMinZ->setDisabled(true);
	this->input_GridBBMaxX->setDisabled(true);
	this->input_GridBBMaxY->setDisabled(true);
	this->input_GridBBMaxZ->setDisabled(true);
	this->info_VoxelRate->setDisabled(true);
	this->info_TotalTime->setDisabled(true);
	this->info_MemorySize->setDisabled(true);
}

void GridControl::setVoxelGrid(std::shared_ptr<DiscreteGrid> vg) {
	if (vg != nullptr) {
		this->voxelGrid = vg;
		this->nameLabel->setText(QString(this->voxelGrid->getGridName().c_str()));
		this->updateGridDimensions();
		this->setupSignals();
	}
}

void GridControl::setupWidgets() {
	// Warning : long constructor-like function ! Ugly, but functionnal.

	// Setting up a layout in code in Qt is quite easy, but
	// goddammit, is it ever verbose. Mainly because all the
	// setXXX() functions return void, disallowing chained
	// calls like "obj->setX()->setY()->setZ();"
	// Also, there are a metric butt-ton of widgets here
	// in order to make the grid controller UI.

	// Setup all inputs :
	this->methodPicker = new QComboBox();
	this->input_GridSizeX = new QSpinBox();
	this->input_GridSizeY = new QSpinBox();
	this->input_GridSizeZ = new QSpinBox();
	this->input_GridBBMinX = new QDoubleSpinBox();
	this->input_GridBBMinY = new QDoubleSpinBox();
	this->input_GridBBMinZ = new QDoubleSpinBox();
	this->input_GridBBMaxX = new QDoubleSpinBox();
	this->input_GridBBMaxY = new QDoubleSpinBox();
	this->input_GridBBMaxZ = new QDoubleSpinBox();
	this->info_GridSizeTotal = new QLabel("");
	this->info_VoxelSize = new QLabel("0x0x0");
	this->button_FillButton = new QPushButton("Populate Grid");
	this->button_SaveButton = new QPushButton("Save to file");

	// Setup bounds for the selectors :
	this->setupSpinBoxBounds(this->input_GridSizeX);
	this->setupSpinBoxBounds(this->input_GridSizeY);
	this->setupSpinBoxBounds(this->input_GridSizeZ);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMinX);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMinY);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMinZ);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMaxX);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMaxY);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMaxZ);

	// Setup the picker for the reconstruction method :
	this->methodPicker->addItem("Nearest Neighbor");
	this->methodPicker->addItem("Trilinear");
	this->methodPicker->addItem("Tricubic");
	this->methodPicker->addItem("Barycentric");
	this->methodPicker->setMaxCount(4);
	this->method = InterpolationMethods::NearestNeighbor;

	// Builds the grid layout, iterating on the rows :
	int currentRow = 0;

	QLabel* pre_nameLabel = new QLabel("Controlling grid named ");
	if (this->voxelGrid != nullptr) {
		this->nameLabel = new QLabel(QString(this->voxelGrid->getGridName().c_str()));
	} else {
		this->nameLabel = new QLabel("-- undefined --");
	}

	QGridLayout* mainLayout = new QGridLayout();
	mainLayout->addWidget(new QLabel("Grid Control"), currentRow++, 0, 1, -1, Qt::AlignCenter);
	mainLayout->addWidget(pre_nameLabel, currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->nameLabel, currentRow, 1, Qt::AlignLeft);
	currentRow +=2; // Add an extra row for spacing

	// Control of voxel number :
	mainLayout->addWidget(new QLabel("Grid size on X : "), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->input_GridSizeX, currentRow, 2, Qt::AlignRight);
	currentRow++;
	mainLayout->addWidget(new QLabel("Grid size on Y : "), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->input_GridSizeY, currentRow, 2, Qt::AlignRight);
	currentRow++;
	mainLayout->addWidget(new QLabel("Grid size on Z : "), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->input_GridSizeZ, currentRow, 2, Qt::AlignRight);
	currentRow++;

	// Add voxel grid size layout :
	QHBoxLayout* infoGridSizeLayout = new QHBoxLayout();
	QLabel* gridSizePrefix = new QLabel("Voxel grid contains ");
	QLabel* gridSizeSuffix = new QLabel(" elements");
	infoGridSizeLayout->addWidget(gridSizePrefix);
	infoGridSizeLayout->addWidget(this->info_GridSizeTotal);
	infoGridSizeLayout->addWidget(gridSizeSuffix);
	mainLayout->addLayout(infoGridSizeLayout, currentRow, 0, 1, -1, Qt::AlignLeft);
	currentRow += 2; // Add a bit of spacing

	// Control of BB min/max vertex positions :
	mainLayout->addWidget(new QLabel("Bounding box control :"), currentRow, 0, 1, -1, Qt::AlignCenter);
	currentRow++;
	mainLayout->addWidget(new QLabel("Minimum along X : "), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->input_GridBBMinX, currentRow, 2, Qt::AlignRight);
	currentRow++;
	mainLayout->addWidget(new QLabel("Minimum along Y : "), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->input_GridBBMinY, currentRow, 2, Qt::AlignRight);
	currentRow++;
	mainLayout->addWidget(new QLabel("Minimum along Z : "), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->input_GridBBMinZ, currentRow, 2, Qt::AlignRight);
	currentRow++;
	mainLayout->addWidget(new QLabel("Maximum along X : "), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->input_GridBBMaxX, currentRow, 2, Qt::AlignRight);
	currentRow++;
	mainLayout->addWidget(new QLabel("Maximum along Y : "), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->input_GridBBMaxY, currentRow, 2, Qt::AlignRight);
	currentRow++;
	mainLayout->addWidget(new QLabel("Maximum along Z : "), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->input_GridBBMaxZ, currentRow, 2, Qt::AlignRight);
	currentRow++;

	// Add voxel size info layout :
	QHBoxLayout* infoVoxelSizeLayout = new QHBoxLayout();
	QLabel* voxelSizePrefix = new QLabel("Voxels in the grid are ");
	QLabel* voxelSizeSuffix = new QLabel(" units.");
	infoVoxelSizeLayout->addWidget(voxelSizePrefix);
	infoVoxelSizeLayout->addWidget(this->info_VoxelSize);
	infoVoxelSizeLayout->addWidget(voxelSizeSuffix);
	mainLayout->addLayout(infoVoxelSizeLayout, currentRow, 0, 1, -1, Qt::AlignLeft);
	currentRow += 2; // Add a bit of padding inbetween sections

	// Add method picker :
	mainLayout->addWidget(new QLabel("Method to interpolate :"), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->methodPicker, currentRow, 2, Qt::AlignRight);
	currentRow += 2;

	// "Launch" button :
	mainLayout->addWidget(this->button_FillButton, currentRow++, 0, 1, -1, Qt::AlignCenter);
	// Save button :
	mainLayout->addWidget(this->button_SaveButton, currentRow, 0, 1, -1, Qt::AlignCenter);
	currentRow += 2;

	// Default values for debug labels (time to reconstruct and rate in GV/h) :
	this->info_TotalTime = new QLabel("NaN");
	this->info_VoxelRate = new QLabel("NaN");
	this->info_MemorySize = new QLabel("NaN");
	QHBoxLayout* infoTime = new QHBoxLayout();
	QHBoxLayout* infoRate = new QHBoxLayout();
	QHBoxLayout* infoMem = new QHBoxLayout();
	QLabel* timePrefix = new QLabel("Time to fill grid : ");
	QLabel* timeSuffix = new QLabel(" seconds");
	QLabel* ratePrefix = new QLabel("Generating voxels at ");
	QLabel* rateSuffix = new QLabel(" GV/h");
	QLabel* memPrefix = new QLabel("Memory size estimated : ");
	QLabel* memSuffix = new QLabel(" GB");
	infoTime->addWidget(timePrefix);
	infoRate->addWidget(ratePrefix);
	infoMem->addWidget(memPrefix);
	infoTime->addWidget(this->info_TotalTime);
	infoRate->addWidget(this->info_VoxelRate);
	infoMem->addWidget(this->info_MemorySize);
	infoTime->addWidget(timeSuffix);
	infoRate->addWidget(rateSuffix);
	infoMem->addWidget(memSuffix);
	mainLayout->addLayout(infoTime, currentRow++, 0, 1, -1, Qt::AlignLeft);
	mainLayout->addLayout(infoRate, currentRow++, 0, 1, -1, Qt::AlignLeft);
	mainLayout->addLayout(infoMem, currentRow++, 0, 1, -1, Qt::AlignLeft);

	std::cerr << "Grid control : generated layout.\nGrid layout has " << currentRow << " rows.\n";
	this->setLayout(mainLayout);
}

void GridControl::setupSignals() {
	if (this->voxelGrid == nullptr) {
		std::cerr << "Error : no voxel grid associated with the grid controller !" << '\n';
		return;
	}

	connect(this->input_GridSizeX, QOverload<int>::of(&QSpinBox::valueChanged), this, &GridControl::setGridDimensionX);
	connect(this->input_GridSizeY, QOverload<int>::of(&QSpinBox::valueChanged), this, &GridControl::setGridDimensionY);
	connect(this->input_GridSizeZ, QOverload<int>::of(&QSpinBox::valueChanged), this, &GridControl::setGridDimensionZ);

	connect(this->input_GridBBMinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBBMinX);
	connect(this->input_GridBBMinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBBMinY);
	connect(this->input_GridBBMinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBBMinZ);

	connect(this->input_GridBBMaxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBBMaxX);
	connect(this->input_GridBBMaxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBBMaxY);
	connect(this->input_GridBBMaxZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBBMaxZ);

	connect(this->methodPicker, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GridControl::pickMethod);

	connect(this->button_FillButton, &QPushButton::clicked, this, &GridControl::launchGridFill);
	connect(this->button_SaveButton, &QPushButton::clicked, this, &GridControl::saveToFile);
}

void GridControl::pickMethod(int m) {
	switch(m) {
		case 0:
			this->method = InterpolationMethods::NearestNeighbor;
			std::cerr << "Set interpolation to NN" << '\n';
		break;
		case 1:
			this->method = InterpolationMethods::TriLinear;
			std::cerr << "Set interpolation to TriLinear" << '\n';
		break;
		case 2:
			this->method = InterpolationMethods::TriCubic;
			std::cerr << "Set interpolation to TriCubic" << '\n';
		break;
		case 3:
			this->method = InterpolationMethods::Barycentric;
			std::cerr << "Set interpolation to Barycentric" << '\n';
		break;
		default:
			std::cerr << "Interpolation not recognized : " << m << '\n';
		break;
	}
}

void GridControl::setupSpinBoxBounds(QSpinBox *sb) {
	sb->setMinimum(std::numeric_limits<int>::min()/2);
	sb->setMaximum(std::numeric_limits<int>::max()/2);
}

void GridControl::setupDoubleSpinBoxBounds(QDoubleSpinBox *dsb) {
	dsb->setSingleStep(.5);
	dsb->setMinimum(std::numeric_limits<double>::min()/2);
	dsb->setMaximum(std::numeric_limits<double>::max()/2);
}

void GridControl::updateGridLabels() {
	if (this->voxelGrid == nullptr) {
		std::cerr << "Error : no voxel grid associated with the grid controller !" << '\n';
		return;
	}

	svec3 dims = this->voxelGrid->getGridDimensions();
	std::size_t size = dims.x * dims.y * dims.z;
	this->info_GridSizeTotal->setText(QString::number(size));

	BoundingBox_General<float> bb = this->voxelGrid->getBoundingBox();
	float vxSize = (bb.getMax().x - bb.getMin().x) / static_cast<float>(dims.x);
	float vySize = (bb.getMax().y - bb.getMin().y) / static_cast<float>(dims.y);
	float vzSize = (bb.getMax().z - bb.getMin().z) / static_cast<float>(dims.z);
	QString text = QString::number(vxSize) + 'x' + QString::number(vySize) + 'x' + QString::number(vzSize);
	this->info_VoxelSize->setText(text);

	double memSize = static_cast<double>(size) / 1.e9;
	this->info_MemorySize->setText(QString::number(memSize));
}

void GridControl::updateGridDimensions() {
	if (this->voxelGrid == nullptr) {
		return;
	}

	svec3 dims = this->voxelGrid->getGridDimensions();
	DiscreteGrid::bbox_t bb = this->voxelGrid->getBoundingBox();

	bb.printInfo("Grid bounding box : ");

	// A bit (very much) verbose, but we need to block signals from the spinboxes when updating
	// the values, in order to remove the possibility of a 'feedback loop' inbetween objects :

	// Grid size :
	this->input_GridSizeX->blockSignals(true);
	this->input_GridSizeX->setValue(static_cast<int>(dims.x));
	this->input_GridSizeX->blockSignals(false);
	this->input_GridSizeY->blockSignals(true);
	this->input_GridSizeY->setValue(static_cast<int>(dims.y));
	this->input_GridSizeY->blockSignals(false);
	this->input_GridSizeZ->blockSignals(true);
	this->input_GridSizeZ->setValue(static_cast<int>(dims.z));
	this->input_GridSizeZ->blockSignals(false);

	// Min bounding box coordinates :
	this->input_GridBBMinX->blockSignals(true);
	this->input_GridBBMinX->setValue(static_cast<double>(bb.getMin().x));
	this->input_GridBBMinX->blockSignals(false);
	this->input_GridBBMinY->blockSignals(true);
	this->input_GridBBMinY->setValue(static_cast<double>(bb.getMin().y));
	this->input_GridBBMinY->blockSignals(false);
	this->input_GridBBMinZ->blockSignals(true);
	this->input_GridBBMinZ->setValue(static_cast<double>(bb.getMin().z));
	this->input_GridBBMinZ->blockSignals(false);

	// Max bounding box coordinates
	this->input_GridBBMaxX->blockSignals(true);
	this->input_GridBBMaxX->setValue(static_cast<double>(bb.getMax().x));
	this->input_GridBBMaxX->blockSignals(false);
	this->input_GridBBMaxY->blockSignals(true);
	this->input_GridBBMaxY->setValue(static_cast<double>(bb.getMax().y));
	this->input_GridBBMaxY->blockSignals(false);
	this->input_GridBBMaxZ->blockSignals(true);
	this->input_GridBBMaxZ->setValue(static_cast<double>(bb.getMax().z));
	this->input_GridBBMaxZ->blockSignals(false);

	this->updateGridLabels();
}

void GridControl::launchGridFill() {
	if (voxelGrid == nullptr) {
		return;
	}
	std::cerr << "Launching grid fill ... will freeze the application.\n";
	/*
	this->voxelGrid->populateGrid(this->method);
	double time = this->voxelGrid->getTimeToCompute().count();
	this->info_TotalTime->setText(QString::number(time));
	svec3 dims = this->voxelGrid->getGridDimensions();
	std::size_t size = dims.x * dims.y * dims.z;
	this->info_VoxelRate->setText(QString::number(((static_cast<double>(size)/time)*3600.)/1.e9));
	*/
	std::cerr << "[ERROR] Not yet implemented the filling from the controller\n";
}

void GridControl::saveToFile() {
	if (this->voxelGrid == nullptr) {
		std::cerr << "Cannot save, no voxel grid attached." << '\n';
		return;
	}

	//QString fileName = QFileDialog::getSaveFileName(nullptr, "Save to DIM/IMA files", "../", "BrainVisa DIM/IMA (*.dim, *.ima)");
	//IO::Writer::DIM* dimWriter = new IO::Writer::DIM(fileName.toStdString());
	//std::cerr << "Writing to file with basename : \"" << fileName.toStdString() << '\"' << '\n';
	//dimWriter->write(this->voxelGrid.get());

	std::cerr << "[ERROR] Not yet implemented the discretegrid writer\n";
}

void GridControl::setGridDimensionX(int newDim) {
	this->voxelGrid->gridDimensions.x = static_cast<std::size_t>(newDim);
	this->voxelGrid->updateVoxelDimensions();
}
void GridControl::setGridDimensionY(int newDim) {
	this->voxelGrid->gridDimensions.y = static_cast<std::size_t>(newDim);
	this->voxelGrid->updateVoxelDimensions();
}
void GridControl::setGridDimensionZ(int newDim) {
	this->voxelGrid->gridDimensions.z = static_cast<std::size_t>(newDim);
	this->voxelGrid->updateVoxelDimensions();
}
void GridControl::setGridBBMinX(double newDim) {
	DiscreteGrid::bbox_t::vec v = this->voxelGrid->boundingBox.getMin();
	v.x = static_cast<DiscreteGrid::bbox_t::vec::value_type>(newDim);
	this->voxelGrid->boundingBox.setMin(v);
	this->voxelGrid->updateVoxelDimensions();
}
void GridControl::setGridBBMinY(double newDim) {
	DiscreteGrid::bbox_t::vec v = this->voxelGrid->boundingBox.getMin();
	v.y = static_cast<DiscreteGrid::bbox_t::vec::value_type>(newDim);
	this->voxelGrid->boundingBox.setMin(v);
	this->voxelGrid->updateVoxelDimensions();
}
void GridControl::setGridBBMinZ(double newDim) {
	DiscreteGrid::bbox_t::vec v = this->voxelGrid->boundingBox.getMin();
	v.z = static_cast<DiscreteGrid::bbox_t::vec::value_type>(newDim);
	this->voxelGrid->boundingBox.setMin(v);
	this->voxelGrid->updateVoxelDimensions();
}
void GridControl::setGridBBMaxX(double newDim) {
	DiscreteGrid::bbox_t::vec v = this->voxelGrid->boundingBox.getMax();
	v.x = static_cast<DiscreteGrid::bbox_t::vec::value_type>(newDim);
	this->voxelGrid->boundingBox.setMax(v);
	this->voxelGrid->updateVoxelDimensions();
}
void GridControl::setGridBBMaxY(double newDim) {
	DiscreteGrid::bbox_t::vec v = this->voxelGrid->boundingBox.getMax();
	v.y = static_cast<DiscreteGrid::bbox_t::vec::value_type>(newDim);
	this->voxelGrid->boundingBox.setMax(v);
	this->voxelGrid->updateVoxelDimensions();
}
void GridControl::setGridBBMaxZ(double newDim) {
	DiscreteGrid::bbox_t::vec v = this->voxelGrid->boundingBox.getMax();
	v.z = static_cast<DiscreteGrid::bbox_t::vec::value_type>(newDim);
	this->voxelGrid->boundingBox.setMax(v);
	this->voxelGrid->updateVoxelDimensions();
}
