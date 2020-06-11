#include "../include/grid_control.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

GridControl::GridControl(VoxelGrid* const vg, QWidget* parent) : QWidget(parent) {
	this->voxelGrid = vg;
	this->setupWidgets();
	if (this->voxelGrid != nullptr) {
		this->setupSignals();
		this->updateGridDimensions();
	}
}

GridControl::~GridControl() {
	delete this->input_GridSizeX;
	delete this->input_GridSizeY;
	delete this->input_GridSizeZ;
	delete this->input_GridBBMinX;
	delete this->input_GridBBMinY;
	delete this->input_GridBBMinZ;
	delete this->input_GridBBMaxX;
	delete this->input_GridBBMaxY;
	delete this->input_GridBBMaxZ;
	delete this->methodPicker;
	delete this->button_FillButton;
	delete this->info_GridSizeTotal;
	delete this->info_VoxelSize;
	delete this->info_VoxelRate;
	delete this->info_TotalTime;
}

void GridControl::setVoxelGrid(VoxelGrid *vg) {
	this->voxelGrid = vg;
	this->setupSignals();
	this->updateGridDimensions();
}

void GridControl::setupWidgets() {
	// Warning : long constructor ! Ugly, but functionnal.
	// Setting up a layout in code in Qt is quite easy, but
	// goddammit, is it ever verbose.

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

	this->methodPicker->addItem("Nearest Neighbor");
	this->methodPicker->addItem("Trilinear");
	this->methodPicker->addItem("Tricubic");
	this->methodPicker->addItem("Barycentric");
	this->methodPicker->setMaxCount(4);
	this->method = InterpolationMethods::NearestNeighbor;

	int currentRow = 0;

	QGridLayout* mainLayout = new QGridLayout();
	mainLayout->addWidget(new QLabel("Grid Control"), currentRow, 0, 1, -1, Qt::AlignCenter);
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

	mainLayout->addWidget(new QLabel("Method to interpolate :"), currentRow, 0, Qt::AlignLeft);
	mainLayout->addWidget(this->methodPicker, currentRow, 2, Qt::AlignRight);
	currentRow += 2;

	mainLayout->addWidget(this->button_FillButton, currentRow, 0, 1, -1, Qt::AlignCenter);

	currentRow += 2;
	this->info_TotalTime = new QLabel("NaN");
	this->info_VoxelRate = new QLabel("NaN");
	QHBoxLayout* infoTime =new QHBoxLayout();
	QHBoxLayout* infoRate =new QHBoxLayout();
	QLabel* timePrefix = new QLabel("Time to fill grid : ");
	QLabel* timeSuffix = new QLabel(" seconds");
	QLabel* ratePrefix = new QLabel("Generating voxels at ");
	QLabel* rateSuffix = new QLabel(" GV/h");
	infoTime->addWidget(timePrefix);
	infoRate->addWidget(ratePrefix);
	infoTime->addWidget(this->info_TotalTime);
	infoRate->addWidget(this->info_VoxelRate);
	infoTime->addWidget(timeSuffix);
	infoRate->addWidget(rateSuffix);
	mainLayout->addLayout(infoTime, currentRow++, 0, 1, -1, Qt::AlignLeft);
	mainLayout->addLayout(infoRate, currentRow++, 0, 1, -1, Qt::AlignLeft);

	std::cerr << "Grid control : generated layout.\nGrid layout has " << currentRow << " rows.\n";
	this->setLayout(mainLayout);
}

void GridControl::setupSignals() {
	if (this->voxelGrid == nullptr) {
		std::cerr << "Error : no voxel grid associated with the grid controller !" << '\n';
		return;
	}

	connect(this->input_GridSizeX, QOverload<int>::of(&QSpinBox::valueChanged), this->voxelGrid, &VoxelGrid::slotSetGridDimensionX);
	connect(this->input_GridSizeY, QOverload<int>::of(&QSpinBox::valueChanged), this->voxelGrid, &VoxelGrid::slotSetGridDimensionY);
	connect(this->input_GridSizeZ, QOverload<int>::of(&QSpinBox::valueChanged), this->voxelGrid, &VoxelGrid::slotSetGridDimensionZ);

	connect(this->input_GridBBMinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this->voxelGrid, &VoxelGrid::slotSetGridBBMinX);
	connect(this->input_GridBBMinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this->voxelGrid, &VoxelGrid::slotSetGridBBMinY);
	connect(this->input_GridBBMinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this->voxelGrid, &VoxelGrid::slotSetGridBBMinZ);

	connect(this->input_GridBBMaxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this->voxelGrid, &VoxelGrid::slotSetGridBBMaxX);
	connect(this->input_GridBBMaxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this->voxelGrid, &VoxelGrid::slotSetGridBBMaxY);
	connect(this->input_GridBBMaxZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this->voxelGrid, &VoxelGrid::slotSetGridBBMaxZ);

	connect(this->methodPicker, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GridControl::pickMethod);

	connect(this->button_FillButton, &QPushButton::clicked, this, &GridControl::launchGridFill);
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

	BoundingBox_General<float> bb = this->voxelGrid->getRenderBB();
	float vxSize = (bb.getMax().x - bb.getMin().x) / static_cast<float>(dims.x);
	float vySize = (bb.getMax().y - bb.getMin().y) / static_cast<float>(dims.y);
	float vzSize = (bb.getMax().z - bb.getMin().z) / static_cast<float>(dims.z);
	QString text = QString::number(vxSize) + 'x' + QString::number(vySize) + 'x' + QString::number(vzSize);
	this->info_VoxelSize->setText(text);
}

void GridControl::updateGridDimensions() {
	if (this->voxelGrid == nullptr) {
		return;
	}

	svec3 dims = this->voxelGrid->getGridDimensions();
	BoundingBox_General<float> bb = this->voxelGrid->getRenderBB();

	this->input_GridSizeX->setValue(static_cast<int>(dims.x));
	this->input_GridSizeY->setValue(static_cast<int>(dims.y));
	this->input_GridSizeZ->setValue(static_cast<int>(dims.z));
	this->input_GridBBMinX->setValue(static_cast<double>(bb.getMin().x));
	this->input_GridBBMinY->setValue(static_cast<double>(bb.getMin().y));
	this->input_GridBBMinZ->setValue(static_cast<double>(bb.getMin().z));
	this->input_GridBBMaxX->setValue(static_cast<double>(bb.getMax().x));
	this->input_GridBBMaxY->setValue(static_cast<double>(bb.getMax().y));
	this->input_GridBBMaxZ->setValue(static_cast<double>(bb.getMax().z));
	this->updateGridLabels();
}

void GridControl::launchGridFill() {
	if (voxelGrid == nullptr) {
		return;
	}
	std::cerr << "Launching grid fill ... will freeze the application.\n";
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<double, std::ratio<1,1>>> start_point = std::chrono::high_resolution_clock::now();
	this->voxelGrid->populateGrid(this->method);
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<double, std::ratio<1,1>>> end_point = std::chrono::high_resolution_clock::now();
	double time = (end_point - start_point).count();
	this->info_TotalTime->setText(QString::number(time));
	svec3 dims = this->voxelGrid->getGridDimensions();
	std::size_t size = dims.x * dims.y * dims.z;
	this->info_VoxelRate->setText(QString::number(((static_cast<double>(size)/time)*3600.)/1.e9));
}
