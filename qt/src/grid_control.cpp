#include "../include/grid_control.hpp"
#include "../../grid/include/discrete_grid_writer.hpp"
#include "../../viewer/include/scene.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>

GridControl::GridControl(std::shared_ptr<DiscreteGrid> _vg, InterpolationMesh::Ptr _tm, Scene* _scene, QWidget* parent) : QWidget(parent) {
	this->voxelGrid = _vg;
	this->mesh = _tm;
	this->scene = _scene;
	this->baseDir.setPath(QDir::homePath());
	this->setupWidgets();

	if (this->voxelGrid != nullptr) {
		this->updateValues();
		if (this->voxelGrid->isModifiable()) {
			this->setupSignals();
		} else {
			this->disableWidgets();
		}
	}

	// Different output grids available to the user :
	this->output_R = nullptr;
	this->output_B = nullptr;
	this->output_RGB = nullptr;
	// Different interpolation structures available to the user :
	this->interpolator_R = nullptr;
	this->interpolator_G = nullptr;
	this->interpolator_RGB = nullptr;

	this->setAttribute(Qt::WA_DeleteOnClose);
}

GridControl::~GridControl() {
	std::cerr << "Deleting the GridControl panel ...\n";
	if (this->scene != nullptr) {
		this->scene->deleteGrid(this->voxelGrid);
		this->scene->removeController();
	}
	delete this->input_GridSizeX;
	delete this->input_GridSizeY;
	delete this->input_GridSizeZ;
	delete this->methodPicker;
	delete this->info_GridSizeTotal;
	delete this->info_VoxelSize;
	delete this->input_GridBBMinX;
	delete this->input_GridBBMinY;
	delete this->input_GridBBMinZ;
	delete this->input_GridBBMaxX;
	delete this->input_GridBBMaxY;
	delete this->input_GridBBMaxZ;
	delete this->info_VoxelRate;
	delete this->info_TotalTime;
	delete this->info_MemorySize;
	#ifdef ENABLE_SINGLE_DIALOGBOX
	delete this->dialogBox;
	#endif

	for (std::size_t i = 0; i < this->strayObj.size(); ++i) {
		if (strayObj[i] != nullptr) {
			delete this->strayObj[i];
		}
	}
	this->strayObj.clear();
}

void GridControl::enableWidgets() {
	this->input_GridSizeX->setDisabled(false);
	this->input_GridSizeY->setDisabled(false);
	this->input_GridSizeZ->setDisabled(false);
	this->methodPicker->setDisabled(false);
	this->info_GridSizeTotal->setDisabled(false);
	this->info_VoxelSize->setDisabled(false);
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

void GridControl::setupWidgets() {
	// Warning : long constructor-like function ! Ugly, but functionnal.

	// Setting up a layout in code in Qt is quite easy, but
	// goddammit, is it ever verbose. Mainly because all the
	// setXXX() functions return void, disallowing chained
	// calls like "obj->setX()->setY()->setZ();"
	// Also, there are a metric butt-ton of widgets here
	// in order to make the grid controller UI.

	#ifdef ENABLE_SINGLE_DIALOGBOX
	this->dialogBox = new QMessageBox;
	#endif

	// Setup all inputs :
	this->methodPicker = new QComboBox();
	this->input_GridSizeX = new QSpinBox();
	this->input_GridSizeY = new QSpinBox();
	this->input_GridSizeZ = new QSpinBox();
	this->input_VoxelSizeX = new QDoubleSpinBox();
	this->input_VoxelSizeY = new QDoubleSpinBox();
	this->input_VoxelSizeZ = new QDoubleSpinBox();
	this->input_GridBBMinX = new QDoubleSpinBox();
	this->input_GridBBMinY = new QDoubleSpinBox();
	this->input_GridBBMinZ = new QDoubleSpinBox();
	this->input_GridBBMaxX = new QDoubleSpinBox();
	this->input_GridBBMaxY = new QDoubleSpinBox();
	this->input_GridBBMaxZ = new QDoubleSpinBox();
	this->info_GridSizeTotal = new QLabel("");
	this->info_VoxelSize = new QLabel("0x0x0");
	this->button_SaveButton = new QPushButton("Generate and save grid");

	this->groupBox_outputType = new QGroupBox("Output type");
	this->radioButton_rgb = new QRadioButton("RGB");
	this->radioButton_2stacks = new QRadioButton("2 stacks");
	this->radioButton_2stacks->setChecked(true);
	this->radioButton_rgb->setToolTip("Output the grid to generate as an RGB stack of images");
	this->radioButton_2stacks->setToolTip("Output the grid to generate as two stacks of greyscale images.");

	this->button_modifyBaseDir = new QPushButton("Change dir...");
	this->label_baseDir = new QLabel(this->baseDir.path());

	this->comboBox_filetype = new QComboBox;
	this->comboBox_filetype->addItem(".dim, .ima");
	this->comboBox_filetype->addItem(".tif (Multiple files)");
	this->comboBox_filetype->setCurrentIndex(1);

	if (this->voxelGrid != nullptr) {
		this->lineEdit_baseName = new QLineEdit(this->voxelGrid->getGridName().c_str());
	} else {
		this->lineEdit_baseName = new QLineEdit("grid");
	}

	// Setup bounds for the selectors :
	this->setupSpinBoxBounds(this->input_GridSizeX);
	this->setupSpinBoxBounds(this->input_GridSizeY);
	this->setupSpinBoxBounds(this->input_GridSizeZ);
	this->setupDoubleSpinBoxBounds(this->input_VoxelSizeX, true);
	this->setupDoubleSpinBoxBounds(this->input_VoxelSizeY, true);
	this->setupDoubleSpinBoxBounds(this->input_VoxelSizeZ, true);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMinX, false);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMinY, false);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMinZ, false);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMaxX, false);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMaxY, false);
	this->setupDoubleSpinBoxBounds(this->input_GridBBMaxZ, false);

	// Setup the picker for the reconstruction method :
	this->methodPicker->addItem("Nearest Neighbor");
	this->methodPicker->addItem("Trilinear");
	this->methodPicker->setMaxCount(2);
	this->method = InterpolationMethods::NearestNeighbor;

	int vRow = 0; // row counter for 'voxel' layout
	int bRow = 0; // row counter for 'bounding box' layout
	int mRow = 0; // row counter for 'main' layout

	// Labels needed for the layouts :
	QLabel* label_gridControl = new QLabel("Grid Control"); this->strayObj.push_back(label_gridControl);
	QLabel* label_gridSizeHeader = new QLabel("# of voxels"); this->strayObj.push_back(label_gridSizeHeader);
	QLabel* label_voxelSizeHeader = new QLabel("Voxel sizes"); this->strayObj.push_back(label_voxelSizeHeader);
	QLabel* label_Axis_X = new QLabel("X : "); this->strayObj.push_back(label_Axis_X);
	QLabel* label_Axis_Y = new QLabel("Y : "); this->strayObj.push_back(label_Axis_Y);
	QLabel* label_Axis_Z = new QLabel("Z : "); this->strayObj.push_back(label_Axis_Z);
	QLabel* label_Header_X = new QLabel(" X "); this->strayObj.push_back(label_Header_X);
	QLabel* label_Header_Y = new QLabel(" Y "); this->strayObj.push_back(label_Header_Y);
	QLabel* label_Header_Z = new QLabel(" Z "); this->strayObj.push_back(label_Header_Z);
	QLabel* label_InterpolationMethod = new QLabel("Interpolation method :"); this->strayObj.push_back(label_InterpolationMethod);

	// create a few layouts :
	QGridLayout* layout_VoxelSize = new QGridLayout;
	QGridLayout* layout_BoundingBox = new QGridLayout;
	QGridLayout* mainLayout = new QGridLayout;
	QFrame* frame_VoxelSizes = new QFrame;
	QFrame* frame_BoundingBox = new QFrame;
	QGridLayout* layout_saveFile = new QGridLayout;
	QHBoxLayout* groupBoxLayout = new QHBoxLayout;

	//==========================//
	// Add grid and voxel sizes //
	//==========================//
	// Headers :
	layout_VoxelSize->addWidget(label_gridSizeHeader, vRow, 2, Qt::AlignHCenter);
	layout_VoxelSize->addWidget(label_voxelSizeHeader, vRow, 3, Qt::AlignHCenter);
	vRow++;
	// Axis X :
	layout_VoxelSize->addWidget(label_Axis_X, vRow, 0, Qt::AlignHCenter);
	layout_VoxelSize->addWidget(this->input_GridSizeX, vRow, 2, Qt::AlignHCenter);
	layout_VoxelSize->addWidget(this->input_VoxelSizeX, vRow, 3, Qt::AlignHCenter);
	vRow++;
	// Axis Y :
	layout_VoxelSize->addWidget(label_Axis_Y, vRow, 0, Qt::AlignHCenter);
	layout_VoxelSize->addWidget(this->input_GridSizeY, vRow, 2, Qt::AlignHCenter);
	layout_VoxelSize->addWidget(this->input_VoxelSizeY, vRow, 3, Qt::AlignHCenter);
	vRow++;
	// Axis Z :
	layout_VoxelSize->addWidget(label_Axis_Z, vRow, 0, Qt::AlignHCenter);
	layout_VoxelSize->addWidget(this->input_GridSizeZ, vRow, 2, Qt::AlignHCenter);
	layout_VoxelSize->addWidget(this->input_VoxelSizeZ, vRow, 3, Qt::AlignHCenter);
	vRow++;
	frame_VoxelSizes->setLayout(layout_VoxelSize);
	frame_VoxelSizes->setStyleSheet(".QFrame{border: 2px solid grey;border-radius: 4px;}");

	//===========================//
	// Add bounding box controls //
	//===========================//
	layout_BoundingBox->addWidget(label_Header_X, bRow, 2, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(label_Header_Y, bRow, 4, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(label_Header_Z, bRow, 6, Qt::AlignHCenter);
	bRow++;
	// BB min :
	layout_BoundingBox->addWidget(this->input_GridBBMinX, bRow, 2, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_GridBBMinY, bRow, 4, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_GridBBMinZ, bRow, 6, Qt::AlignHCenter);
	bRow++;
	// BB max :
	layout_BoundingBox->addWidget(this->input_GridBBMaxX, bRow, 2, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_GridBBMaxY, bRow, 4, Qt::AlignHCenter);
	layout_BoundingBox->addWidget(this->input_GridBBMaxZ, bRow, 6, Qt::AlignHCenter);
	bRow++;
	frame_BoundingBox->setLayout(layout_BoundingBox);
	frame_BoundingBox->setStyleSheet(".QFrame{border: 2px solid grey;border-radius: 4px;}");

	layout_saveFile->addWidget(this->label_baseDir, 0, 0, 1, 3);
	layout_saveFile->addWidget(this->button_modifyBaseDir, 0, 4, 1, 1);
	layout_saveFile->addWidget(this->lineEdit_baseName, 1, 0, 1, 3);
	layout_saveFile->addWidget(this->comboBox_filetype, 1, 4, 1, 1);

	//===============================================//
	// Add layout for the GroupBox for radio buttons //
	//===============================================//
	groupBoxLayout->addWidget(this->radioButton_rgb);
	groupBoxLayout->addWidget(this->radioButton_2stacks);
	this->groupBox_outputType->setLayout(groupBoxLayout);

	//========================================//
	// Merge grid/voxel and BB layouts in one //
	//========================================//
	// First, add header and grid name :
	mainLayout->addWidget(label_gridControl, mRow, 0, 1, -1, Qt::AlignHCenter); mRow += 2; // bit of spacing
	// Add voxel sizes :
	mainLayout->addWidget(frame_VoxelSizes, mRow, 0, 1, -1); mRow+=2; // space to next widget
	// Add bb controls :
	mainLayout->addWidget(frame_BoundingBox, mRow, 0, 1, -1); mRow+=2; // space to next widget
	// group box for the output selector
	mainLayout->addWidget(this->groupBox_outputType, mRow, 0, 1, -1); mRow+=2;
	// Add save options :
	mainLayout->addLayout(layout_saveFile, mRow, 0, 1, -1); mRow+=2; // space to next widget
	// Add buttons :
	mainLayout->addWidget(label_InterpolationMethod, mRow, 0, Qt::AlignRight);
	mainLayout->addWidget(this->methodPicker, mRow, 1, Qt::AlignJustify);
	mainLayout->addWidget(this->button_SaveButton, mRow, 2, Qt::AlignHCenter);
	mRow++;

	// Default values for debug labels (time to reconstruct and rate in GV/h) :
	this->info_TotalTime = new QLabel("NaN");
	this->info_VoxelRate = new QLabel("NaN");
	this->info_MemorySize = new QLabel("NaN");
	QHBoxLayout* infoTime = new QHBoxLayout();
	QHBoxLayout* infoRate = new QHBoxLayout();
	QHBoxLayout* infoMem = new QHBoxLayout();
	QVBoxLayout* infoAll = new QVBoxLayout();
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

	infoAll->addLayout(infoTime);
	infoAll->addLayout(infoRate);
	infoAll->addLayout(infoMem);

	if (this->voxelGrid != nullptr) {
		if (this->mesh->getInputGrids().size() > 1) {
			this->groupBox_outputType->setEnabled(true);
		} else {
			this->groupBox_outputType->setEnabled(false);
		}
	}

	mainLayout->addLayout(infoAll, mRow++, 0, 1, -1, Qt::AlignLeft);

	// Set the layout :
	this->setLayout(mainLayout);

	//this->setFixedSize(this->size());
}

void GridControl::setupSignals() {
	if (this->voxelGrid == nullptr) {
		std::cerr << "Error : no voxel grid associated with the grid controller !" << '\n';
		return;
	}

	// connect grid sizes :
	QObject::connect(this->input_GridSizeX, QOverload<int>::of(&QSpinBox::valueChanged), this, &GridControl::setGridResolution);
	QObject::connect(this->input_GridSizeY, QOverload<int>::of(&QSpinBox::valueChanged), this, &GridControl::setGridResolution);
	QObject::connect(this->input_GridSizeZ, QOverload<int>::of(&QSpinBox::valueChanged), this, &GridControl::setGridResolution);

	// connect voxel spinboxes :
	QObject::connect(this->input_VoxelSizeX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridVoxelSize);
	QObject::connect(this->input_VoxelSizeY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridVoxelSize);
	QObject::connect(this->input_VoxelSizeZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridVoxelSize);

	QObject::connect(this->input_GridBBMinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBoundingBox);
	QObject::connect(this->input_GridBBMinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBoundingBox);
	QObject::connect(this->input_GridBBMinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBoundingBox);

	QObject::connect(this->input_GridBBMaxX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBoundingBox);
	QObject::connect(this->input_GridBBMaxY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBoundingBox);
	QObject::connect(this->input_GridBBMaxZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GridControl::setGridBoundingBox);

	connect(this->methodPicker, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GridControl::pickMethod);

	connect(this->button_SaveButton, &QPushButton::clicked, this, &GridControl::saveToFile);

	connect(this->button_modifyBaseDir, &QPushButton::clicked, [this]() {
		QDir newDir = QFileDialog::getExistingDirectory(this, "Pick a save dialog", this->baseDir.path(), QFileDialog::DontUseNativeDialog);
		if (newDir.isReadable()) {
			this->baseDir = newDir;
		} else {
			this->dialogBox->critical(this, "Directory Error", "Directory chosen was not readable !");
		}
		this->label_baseDir->setText(this->baseDir.path());
	});
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
		default:
			std::cerr << "Interpolation not recognized : " << m << '\n';
		break;
	}
}

void GridControl::showEvent(QShowEvent *_e) {
	// At this point, the size will be computed according to the widets
	// put inside this class. Beforehand the size was not computed :/
	// So this will show the widget at its most compact :
	this->setFixedSize(this->size());
	QWidget::showEvent(_e);
	return;
}

void GridControl::setupSpinBoxBounds(QSpinBox *sb) {
	sb->setMinimum(std::numeric_limits<int>::min()/2);
	sb->setMaximum(std::numeric_limits<int>::max()/2);
}

void GridControl::setupDoubleSpinBoxBounds(QDoubleSpinBox *dsb, bool lowOrReal) {
	dsb->setSingleStep(.5);
	// if true, defined in [.0, +inf/2], otherwise [-inf/2, +inf/2]
	if (lowOrReal) {
		dsb->setMinimum(std::numeric_limits<double>::min());
	} else {
		dsb->setMinimum(std::numeric_limits<double>::lowest()/2);
	}
	dsb->setMaximum(std::numeric_limits<double>::max()/2);
	dsb->setValue(dsb->maximum()); // allows to resize the widget to max size :)
}

void GridControl::blockSignals(bool b) {
	this->input_GridSizeX->blockSignals(b);
	this->input_GridSizeY->blockSignals(b);
	this->input_GridSizeZ->blockSignals(b);

	this->input_VoxelSizeX->blockSignals(b);
	this->input_VoxelSizeY->blockSignals(b);
	this->input_VoxelSizeZ->blockSignals(b);

	this->input_GridBBMinX->blockSignals(b);
	this->input_GridBBMinY->blockSignals(b);
	this->input_GridBBMinZ->blockSignals(b);
	this->input_GridBBMaxX->blockSignals(b);
	this->input_GridBBMaxY->blockSignals(b);
	this->input_GridBBMaxZ->blockSignals(b);

	return;
}

void GridControl::updateValues() {
	if (this->voxelGrid == nullptr) {
		std::cerr << "Error : no voxel grid associated with the grid controller !" << '\n';
		return;
	}

	glm::ivec3 dims = glm::convert_to<int>(this->voxelGrid->getResolution());
	glm::vec3 vx = this->voxelGrid->getVoxelDimensions();
	DiscreteGrid::bbox_t bb = this->voxelGrid->getBoundingBoxWorldSpace();
	glm::dvec3 min = glm::convert_to<double>(bb.getMin());
	glm::dvec3 max = glm::convert_to<double>(bb.getMax());

	// Block signals first :
	this->blockSignals(true);

	// Grid size :
	this->input_GridSizeX->setValue(dims.x);
	this->input_GridSizeY->setValue(dims.y);
	this->input_GridSizeZ->setValue(dims.z);

	// Voxel sizes :
	this->input_VoxelSizeX->setValue(vx.x);
	this->input_VoxelSizeY->setValue(vx.y);
	this->input_VoxelSizeZ->setValue(vx.z);

	// Min bounding box coordinates :
	this->input_GridBBMinX->setValue(min.x);
	this->input_GridBBMinY->setValue(min.y);
	this->input_GridBBMinZ->setValue(min.z);

	// Max bounding box coordinates
	this->input_GridBBMaxX->setValue(max.x);
	this->input_GridBBMaxY->setValue(max.y);
	this->input_GridBBMaxZ->setValue(max.z);

	// all good, we can release signals now :
	this->blockSignals(false);

	std::size_t size = dims.x * dims.y * dims.z;
	this->info_GridSizeTotal->setText(QString::number(size));

	glm::vec3 vxDims = this->voxelGrid->getVoxelDimensions();
	QString text = QString::number(vxDims.x) + 'x' + QString::number(vxDims.y) + 'x' + QString::number(vxDims.z);
	this->info_VoxelSize->setText(text);

	double memSize = static_cast<double>(size) / 1.e9;
	this->info_MemorySize->setText(QString::number(memSize));
}

void GridControl::launchGridFill() {
	if (voxelGrid == nullptr || this->mesh == nullptr) {
		return;
	}

	this->mesh->populateOutputGrid(this->method);
}

void GridControl::saveToFile() {
	// Check the voxel grid and
	if (this->voxelGrid == nullptr) {
		#ifndef ENABLE_SINGLE_DIALOGBOX
		QMessageBox* messageBox = new QMessageBox;
		messageBox->setAttribute(Qt::WA_DeleteOnClose);
		messageBox->critical(nullptr, "Error", "No grid was attached to this controller.\nPlease close and re-open the save dialog.");
		messageBox->show();
		#else
		this->dialogBox->critical(this, "Error", "No grid was attached to this controller.\nPlease close and re-open the save dialog.");
		#endif
		return;
	}
	if (this->mesh == nullptr) {
		#ifndef ENABLE_SINGLE_DIALOGBOX
		QMessageBox* messageBox = new QMessageBox;
		messageBox->setAttribute(Qt::WA_DeleteOnClose);
		messageBox->critical(nullptr, "Error", "No mesh was attached to this controller.\nPlease close and re-open the save dialog.");
		messageBox->show();
		#else
		this->dialogBox->critical(this, "Error", "No mesh was attached to this controller.\nPlease close and re-open the save dialog.");
		#endif
		return;
	}

	// If nothing was selected !
	if (this->lineEdit_baseName->text().isEmpty()) {
		#ifndef ENABLE_SINGLE_DIALOGBOX
		QMessageBox* messageBox = new QMessageBox;
		messageBox->setAttribute(Qt::WA_DeleteOnClose);
		messageBox->critical(nullptr, "Error", "No filename was given !\nNo grid will be generated.");
		messageBox->show();
		#else
		this->dialogBox->critical(this, "Error", "No filename was given !\nNo grid will be generated.");
		#endif
		return;
	}

	/*================================================*/
	/* It's a this moment we'll populate the right    */
	/* writers, grids, and interpolators to generate  */
	/* the grid the user requested.                   */
	/*================================================*/

	std::string basePath = this->baseDir.path().toStdString();
	QString fileName = this->lineEdit_baseName->text();
	QString filenameR = fileName + "_0";
	QString filenameB = fileName + "_1";

	std::shared_ptr<IO::GenericGridWriter> writerR = nullptr;
	std::shared_ptr<IO::GenericGridWriter> writerB = nullptr;
	std::shared_ptr<IO::GenericGridWriter> writerRGB = nullptr;

	bool doubleoutput = (this->mesh->getInputGrids().size() > 1);
	bool rgboutput = this->radioButton_rgb->isChecked();

	std::function<std::shared_ptr<IO::GenericGridWriter>(const std::string, const std::string)> generator;

	switch (this->comboBox_filetype->currentIndex()) {
		case 0:
			generator = std::bind(&std::make_shared<IO::Writer::DIM, const std::string, const std::string>,
					std::placeholders::_1, std::placeholders::_2);
		break;
		case 1:
			generator = std::bind(&std::make_shared<IO::Writer::MultiTIFF, const std::string, const std::string>,
					std::placeholders::_1, std::placeholders::_2);
		break;
		default:
			std::cerr << "[ERROR] No value was recognized for the writer picker." << '\n';
			std::cerr << "[ERROR] No grid will be written." << '\n';
			return;
		break;
	}

	if (doubleoutput) {
		if (rgboutput) {
			writerRGB = generator(fileName.toStdString(), basePath);
			this->voxelGrid->setOffline().setGridWriter(writerRGB);

			this->setGridBoundingBox();
			this->setGridVoxelSize();
			this->setGridResolution();

			this->mesh->printInfo();

			this->mesh->populateOutputGrid_RGB(this->method);

			std::cerr << "RGB image was generated at a rate of " << this->mesh->getGenerationRate() << "Gv/h.\n";
		} else {
			writerR = generator(filenameR.toStdString(), basePath);
			writerB = generator(filenameB.toStdString(), basePath);
			this->output_R = std::make_shared<OutputGrid>(std::dynamic_pointer_cast<OutputGrid>(this->voxelGrid));
			this->output_B = std::make_shared<OutputGrid>(std::dynamic_pointer_cast<OutputGrid>(this->voxelGrid));
			this->output_R->setOffline().setGridWriter(writerR);
			this->output_B->setOffline().setGridWriter(writerB);

			const std::vector<std::shared_ptr<InputGrid>>& grids = this->mesh->getInputGrids();
			this->interpolator_R = std::make_shared<InterpolationMesh>();
			this->interpolator_G = std::make_shared<InterpolationMesh>();
			this->interpolator_R->addInputGrid(grids[0]);
			this->interpolator_R->setOutputGrid(this->output_R);
			this->interpolator_G->addInputGrid(grids[1]);
			this->interpolator_G->setOutputGrid(this->output_B);

			this->setGridBoundingBox();
			this->setGridVoxelSize();
			this->setGridResolution();

			std::cerr << "Testing both output streams.\n";
			std::cerr << "Writing red grid to the path " << filenameR.toStdString() << " ...\n";
			this->interpolator_R->populateOutputGrid(this->method);
			std::cerr << "Wrote grid to file \"" << filenameR.toStdString() << "\"\n";

			std::cerr << "Writing blue grid to the path " << filenameB.toStdString() << " ...\n";
			this->interpolator_G->populateOutputGrid(this->method);
			std::cerr << "Wrote grid to file \"" << filenameB.toStdString() << "\"\n";

			std::cerr << "Red channel was generated at a rate of " << this->interpolator_R->getGenerationRate() << "Gv/h.\n";
			std::cerr << "Blue channel was generated at a rate of " << this->interpolator_G->getGenerationRate() << "Gv/h.\n";
		}
	} else {
		writerR = generator(fileName.toStdString(), basePath);
		this->voxelGrid->setOffline().setGridWriter(writerR);
		std::cerr << "Writing to file with basename : \"" << fileName.toStdString() << '\"' << '\n';
		this->launchGridFill();
		std::cerr << "Wrote grid to the file \"" << fileName.toStdString() << "\"\n";
	}

	// Once done, close the widget in order to remove the grid from the scene !
	this->close();
}

void GridControl::setGridResolution() {
	if (this->voxelGrid == nullptr) {
		return; // nothing done if no grid 'connected'
	}

	DiscreteGrid::sizevec3 userRes = DiscreteGrid::sizevec3();

	userRes.x = static_cast<DiscreteGrid::sizevec3::value_type>(this->input_GridSizeX->value());
	userRes.y = static_cast<DiscreteGrid::sizevec3::value_type>(this->input_GridSizeY->value());
	userRes.z = static_cast<DiscreteGrid::sizevec3::value_type>(this->input_GridSizeZ->value());

	this->voxelGrid->setResolution(userRes);
	if (this->output_R) { this->output_R->setResolution(userRes); }
	if (this->output_B) { this->output_B->setResolution(userRes); }
	if (this->output_RGB) { this->output_RGB->setResolution(userRes); }

	// update fields and labels :
	this->updateValues();
}

void GridControl::setGridBoundingBox() {
	if (this->voxelGrid == nullptr) {
		return; // nothing done if no grid 'connected'
	}

	DiscreteGrid::bbox_t::vec userBBoxMin;
	DiscreteGrid::bbox_t::vec userBBoxMax;

	userBBoxMin.x = static_cast<DiscreteGrid::bbox_t::vec::value_type>(this->input_GridBBMinX->value());
	userBBoxMin.y = static_cast<DiscreteGrid::bbox_t::vec::value_type>(this->input_GridBBMinY->value());
	userBBoxMin.z = static_cast<DiscreteGrid::bbox_t::vec::value_type>(this->input_GridBBMinZ->value());

	userBBoxMax.x = static_cast<DiscreteGrid::bbox_t::vec::value_type>(this->input_GridBBMaxX->value());
	userBBoxMax.y = static_cast<DiscreteGrid::bbox_t::vec::value_type>(this->input_GridBBMaxY->value());
	userBBoxMax.z = static_cast<DiscreteGrid::bbox_t::vec::value_type>(this->input_GridBBMaxZ->value());

	DiscreteGrid::bbox_t userBBox = DiscreteGrid::bbox_t(userBBoxMin, userBBoxMax);

	this->voxelGrid->setBoundingBox(userBBox);
	if (this->output_R) { this->output_R->setBoundingBox(userBBox); }
	if (this->output_B) { this->output_B->setBoundingBox(userBBox); }
	if (this->output_RGB) { this->output_RGB->setBoundingBox(userBBox); }

	// update fields and labels :
	this->updateValues();
}

void GridControl::setGridVoxelSize() {
	if (this->voxelGrid == nullptr) {
		return; // nothing done if no grid 'connected'
	}

	glm::vec3 userVxDims;

	userVxDims.x = static_cast<glm::vec3::value_type>(this->input_VoxelSizeX->value());
	userVxDims.y = static_cast<glm::vec3::value_type>(this->input_VoxelSizeY->value());
	userVxDims.z = static_cast<glm::vec3::value_type>(this->input_VoxelSizeZ->value());

	this->voxelGrid->setVoxelDimensions(userVxDims);
	if (this->output_R) { this->output_R->setVoxelDimensions(userVxDims); }
	if (this->output_B) { this->output_B->setVoxelDimensions(userVxDims); }
	if (this->output_RGB) { this->output_RGB->setVoxelDimensions(userVxDims); }

	// update fields and labels :
	this->updateValues();
}
