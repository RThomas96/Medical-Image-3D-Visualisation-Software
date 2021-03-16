#include "../include/loader_widget.hpp"

#include "../include/user_settings_widget.hpp"

#include <QMessageBox>
#include <QDialog>
#include <QFileDialog>
#include <QCoreApplication>

#include <iomanip>

GridLoaderWidget::GridLoaderWidget(Scene* _scene, Viewer* _viewer, ControlPanel* cp, QWidget* parent) : QWidget(parent) {
	this->setAttribute(Qt::WA_DeleteOnClose); // delete widget and resources on close.
	this->basePath.setPath(QDir::homePath());
	this->scene = _scene;
	this->viewer = _viewer;
	this->_cp = cp;
	this->dsLevel = IO::DownsamplingLevel::Original;
	this->readerR = nullptr;
	this->readerG = nullptr;
	this->inputGridR = nullptr;
	this->inputGridG = nullptr;
	this->groupbox_userLimits = nullptr;
	this->spinbox_userLimitMin = nullptr;
	this->spinbox_userLimitMax = nullptr;
	this->setupWidgets();
	this->setupLayouts();
	this->setupSignals();
}

GridLoaderWidget::~GridLoaderWidget() {
	this->inputGridR.reset();
	this->inputGridG.reset();
	this->readerR.reset();
	this->readerG.reset();
	this->scene = nullptr;
	this->viewer = nullptr;
	this->interpolator.reset();

	this->button_loadDIM_1channel->disconnect();
	this->button_loadDIM_2channel->disconnect();
	this->button_loadTIF_1channel->disconnect();
	this->button_loadTIF_2channel->disconnect();
	this->button_loadGrids->disconnect();

	delete this->groupbox_userLimits;

	delete this->label_headerLoader;
	delete this->label_load1channel;
	delete this->label_load2channel;
	delete this->label_headerTransformation;
	delete this->label_transformationAngle;
	delete this->label_transformationDimensions;

	delete this->button_loadDIM_1channel;
	delete this->button_loadDIM_2channel;
	delete this->button_loadTIF_1channel;
	delete this->button_loadTIF_2channel;
	delete this->button_loadGrids;

	delete this->dsb_transformationA;
	delete this->dsb_transformationDX;
	delete this->dsb_transformationDY;
	delete this->dsb_transformationDZ;

	delete this->layout_mainLayout;
	delete this->layout_load1channel;
	delete this->layout_load2channel;
	delete this->layout_transfoDetails;
}

void GridLoaderWidget::setupWidgets() {
	this->label_headerLoader = new QLabel("<h2><i>Load a grid</i></h2>");
	this->label_load1channel = new QLabel("Load a grid containing 1 color channel : ");
	this->label_load2channel = new QLabel("Load a grid containing 2 color channels : ");
	this->label_headerTransformation = new QLabel("Transformation details");
	this->label_transformationAngle = new QLabel("Angle of capture (degrees) : ");
	this->label_transformationDimensions = new QLabel("Physical resolution of a pixel (micrometers, on X, Y, and Z) :");
	this->label_gridInfoR = new QLabel("<No grid loaded>");
	this->label_gridInfoG = new QLabel("");

	this->button_loadDIM_1channel = new QPushButton("DIM/IMA");
	this->button_loadDIM_2channel = new QPushButton("DIM/IMA");
	this->button_loadTIF_1channel = new QPushButton("TIFF");
	this->button_loadTIF_2channel = new QPushButton("TIFF");
	this->button_loadGrids = new QPushButton("Load grids into program");

	this->dsb_transformationA  = new QDoubleSpinBox;
	this->dsb_transformationDX = new QDoubleSpinBox;
	this->dsb_transformationDY = new QDoubleSpinBox;
	this->dsb_transformationDZ = new QDoubleSpinBox;

	// Angle from -180° to 180°, set to 0 by default :
	this->dsb_transformationA->setRange(-180., 180.);
	this->dsb_transformationA->setSingleStep(.5);
	this->dsb_transformationA->setValue(.0);
	// Voxel dimensions on X set to [0, 100] with 0 default (not loaded) :
	this->dsb_transformationDX->setRange(.0, 100.);
	this->dsb_transformationDX->setValue(1.);
	this->dsb_transformationDX->setSingleStep(.01);
	// Voxel dimensions on Y set to [0, 100] with 0 default (not loaded) :
	this->dsb_transformationDY->setRange(.0, 100.);
	this->dsb_transformationDY->setValue(1.);
	this->dsb_transformationDY->setSingleStep(.01);
	// Voxel dimensions on Z set to [0, 100] with 0 default (not loaded) :
	this->dsb_transformationDZ->setRange(.0, 100.);
	this->dsb_transformationDZ->setValue(1.);
	this->dsb_transformationDZ->setSingleStep(.01);

	this->frame_load1channel = new QFrame;
	this->frame_load2channel = new QFrame;
	this->frame_transfoDetails = new QFrame;

	this->frame_load1channel->setFrameShape(QFrame::Shape::StyledPanel);
	this->frame_load2channel->setFrameShape(QFrame::Shape::StyledPanel);
	this->frame_transfoDetails->setFrameShape(QFrame::Shape::StyledPanel);
	this->frame_load1channel->setFrameShadow(QFrame::Shadow::Raised);
	this->frame_load2channel->setFrameShadow(QFrame::Shadow::Raised);
	this->frame_transfoDetails->setFrameShadow(QFrame::Shadow::Raised);

	this->groupBox_downsampling = new QGroupBox("Image resolution to load");
	this->groupBox_interpolator = new QGroupBox("Interpolation to use");

	this->radioButton_original = new QRadioButton("Original");
	this->radioButton_low = new QRadioButton("Low");
	this->radioButton_lower = new QRadioButton("Lower");
	this->radioButton_lowest = new QRadioButton("Lowest");
	this->radioButton_nn = new QRadioButton("Nearest neighbor");
	this->radioButton_mean = new QRadioButton("Mean value");
	this->radioButton_mp = new QRadioButton("Most present value");
	this->radioButton_min = new QRadioButton("Min value");
	this->radioButton_max = new QRadioButton("Max value");

	this->groupbox_userLimits = new QGroupBox("Predefined minimum and maximum intensities to evaluate");
	this->groupbox_userLimits->setCheckable(true);
	this->groupbox_userLimits->setChecked(false);

	this->label_roiMin = new QLabel("Minimum intensity : ");
	this->label_roiMax = new QLabel("Minimum intensity : ");

	this->spinbox_userLimitMin = new QSpinBox;
	this->spinbox_userLimitMin->setMinimum(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::lowest()));
	this->spinbox_userLimitMin->setMaximum(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::max()));
	this->spinbox_userLimitMin->setValue(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::lowest()));
	this->spinbox_userLimitMax = new QSpinBox;
	this->spinbox_userLimitMax->setMinimum(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::lowest()));
	this->spinbox_userLimitMax->setMaximum(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::max()));
	this->spinbox_userLimitMax->setValue(static_cast<int>(std::numeric_limits<DiscreteGrid::data_t>::max()));

	this->groupBox_interpolator->setDisabled(true);
	this->radioButton_original->setChecked(true);
	this->radioButton_nn->setChecked(true);
	this->interpolator = std::make_shared<Interpolators::nearestNeighboor<DiscreteGrid::data_t>>();
}

void GridLoaderWidget::setupLayouts() {
	this->layout_mainLayout = new QVBoxLayout;
	this->layout_load1channel = new QHBoxLayout;
	this->layout_load2channel = new QHBoxLayout;
	this->layout_transfoDetails = new QGridLayout;
	this->layout_downsampling = new QHBoxLayout;
	this->layout_interpolator = new QGridLayout;
	this->layout_roiSelection = new QGridLayout;

	// layout and frame for the 1-channel loader :
	this->layout_load1channel->addWidget(this->label_load1channel);
	this->layout_load1channel->addWidget(this->button_loadDIM_1channel);
	this->layout_load1channel->addWidget(this->button_loadTIF_1channel);
	this->frame_load1channel->setLayout(this->layout_load1channel);
	// layout and frame for the 2-channel loader :
	this->layout_load2channel->addWidget(this->label_load2channel);
	this->layout_load2channel->addWidget(this->button_loadDIM_2channel);
	this->layout_load2channel->addWidget(this->button_loadTIF_2channel);
	this->frame_load2channel->setLayout(this->layout_load2channel);
	// layout and frame for the transformation details :
	this->layout_transfoDetails->addWidget(this->label_transformationAngle, 0, 0, 1, 2);
	this->layout_transfoDetails->addWidget(this->dsb_transformationA, 0, 2, 1, 1);
	this->layout_transfoDetails->addWidget(this->label_transformationDimensions, 1, 0, 1, 3);
	this->layout_transfoDetails->addWidget(this->dsb_transformationDX, 2, 0, 1, 1);
	this->layout_transfoDetails->addWidget(this->dsb_transformationDY, 2, 1, 1, 1);
	this->layout_transfoDetails->addWidget(this->dsb_transformationDZ, 2, 2, 1, 1);
	this->frame_transfoDetails->setLayout(this->layout_transfoDetails);

	//downsampling layout :
	this->layout_downsampling->addWidget(this->radioButton_original);
	this->layout_downsampling->addWidget(this->radioButton_low);
	this->layout_downsampling->addWidget(this->radioButton_lower);
	this->layout_downsampling->addWidget(this->radioButton_lowest);
	this->groupBox_downsampling->setLayout(this->layout_downsampling);
	// interpolator layout :
	this->layout_interpolator->addWidget(this->radioButton_nn, 0, 0);
	this->layout_interpolator->addWidget(this->radioButton_mean, 0, 1);
	this->layout_interpolator->addWidget(this->radioButton_mp, 0, 2);
	this->layout_interpolator->addWidget(this->radioButton_min, 1, 0);
	this->layout_interpolator->addWidget(this->radioButton_max, 1, 1);
	this->groupBox_interpolator->setLayout(this->layout_interpolator);

	// ROI selection layout :
	this->layout_roiSelection->addWidget(this->label_roiMin, 0, 0);
	this->layout_roiSelection->addWidget(this->spinbox_userLimitMin, 0, 1);
	this->layout_roiSelection->addWidget(this->label_roiMax, 1, 0);
	this->layout_roiSelection->addWidget(this->spinbox_userLimitMax, 1, 1);
	this->groupbox_userLimits->setLayout(this->layout_roiSelection);

	// main layout :
	this->layout_mainLayout->addWidget(this->label_headerLoader, 0, Qt::AlignCenter);
	this->layout_mainLayout->addWidget(this->frame_load1channel);
	this->layout_mainLayout->addWidget(this->frame_load2channel);
	this->layout_mainLayout->addWidget(this->groupBox_downsampling);
	this->layout_mainLayout->addWidget(this->groupBox_interpolator);
	this->layout_mainLayout->addWidget(this->groupbox_userLimits);
	this->layout_mainLayout->addStretch(1);
	this->layout_mainLayout->addWidget(this->label_gridInfoR);
	this->layout_mainLayout->addWidget(this->label_gridInfoG);
	this->layout_mainLayout->addStretch(1);
	this->layout_mainLayout->addWidget(this->frame_transfoDetails);
	this->layout_mainLayout->addStretch(1);
	this->layout_mainLayout->addWidget(this->button_loadGrids);

	// set main layout :
	this->setLayout(this->layout_mainLayout);
}

void GridLoaderWidget::setupSignals() {
	// check if scene and viewer are valid before setting up the signals :
	if (this->scene == nullptr) {
		QMessageBox* msgBox = new QMessageBox;
		msgBox->setAttribute(Qt::WA_DeleteOnClose);
		msgBox->critical(this, "Error : no scene associated", "An error has occured, and no scene was associated with this widget. Please retry again later.");
		return;
	}
	if (this->viewer == nullptr) {
		QMessageBox* msgBox = new QMessageBox;
		msgBox->setAttribute(Qt::WA_DeleteOnClose);
		msgBox->critical(this, "Error : no viewer associated", "An error has occured, and no viewer was associated with this widget. Please retry again later.");
		return;
	}

	// load from DIM/IMA/TIFF files :
	QObject::connect(this->button_loadDIM_1channel, &QPushButton::clicked, this, &GridLoaderWidget::loadGridDIM1channel);
	QObject::connect(this->button_loadDIM_2channel, &QPushButton::clicked, this, &GridLoaderWidget::loadGridDIM2channel);
	QObject::connect(this->button_loadTIF_1channel, &QPushButton::clicked, this, &GridLoaderWidget::loadGridTIF1channel);
	QObject::connect(this->button_loadTIF_2channel, &QPushButton::clicked, this, &GridLoaderWidget::loadGridTIF2channel);

	// load grid into mem :
	QObject::connect(this->button_loadGrids, &QPushButton::clicked, this, &GridLoaderWidget::loadGrid);

	QObject::connect(this->radioButton_original, &QRadioButton::toggled, [this]() {
		this->resetGridInfoLabel();
		if (this->radioButton_original->isChecked()) {
			this->dsLevel = IO::DownsamplingLevel::Original;
			this->groupBox_interpolator->setEnabled(false);
			if (this->readerR != nullptr) { this->readerR->enableDownsampling(this->dsLevel); }
			if (this->readerG != nullptr) { this->readerG->enableDownsampling(this->dsLevel); }
		}
		this->computeGridInfoLabel();
	});
	QObject::connect(this->radioButton_low, &QRadioButton::toggled, [this]() {
		this->resetGridInfoLabel();
		if (this->radioButton_low->isChecked()) {
			this->dsLevel = IO::DownsamplingLevel::Low;
			this->groupBox_interpolator->setEnabled(true);
			if (this->readerR != nullptr) { this->readerR->enableDownsampling(this->dsLevel); }
			if (this->readerG != nullptr) { this->readerG->enableDownsampling(this->dsLevel); }
		}
		this->computeGridInfoLabel();
	});
	QObject::connect(this->radioButton_lower, &QRadioButton::toggled, [this]() {
		this->resetGridInfoLabel();
		if (this->radioButton_lower->isChecked()) {
			this->dsLevel = IO::DownsamplingLevel::Lower;
			this->groupBox_interpolator->setEnabled(true);
			if (this->readerR != nullptr) { this->readerR->enableDownsampling(this->dsLevel); }
			if (this->readerG != nullptr) { this->readerG->enableDownsampling(this->dsLevel); }
		}
		this->computeGridInfoLabel();
	});
	QObject::connect(this->radioButton_lowest, &QRadioButton::toggled, [this]() {
		this->resetGridInfoLabel();
		if (this->radioButton_lowest->isChecked()) {
			this->dsLevel = IO::DownsamplingLevel::Lowest;
			this->groupBox_interpolator->setEnabled(true);
			if (this->readerR != nullptr) { this->readerR->enableDownsampling(this->dsLevel); }
			if (this->readerG != nullptr) { this->readerG->enableDownsampling(this->dsLevel); }
		}
		this->computeGridInfoLabel();
	});

	QObject::connect(this->radioButton_nn, &QRadioButton::toggled, [this]() {
		if (this->radioButton_nn->isChecked()) {
			this->interpolator.reset();
			this->interpolator = std::make_shared<Interpolators::nearestNeighboor<DiscreteGrid::data_t>>();
		}
	});
	QObject::connect(this->radioButton_mean, &QRadioButton::toggled, [this]() {
		if (this->radioButton_mean->isChecked()) {
			this->interpolator.reset();
			this->interpolator = std::make_shared<Interpolators::meanValue<DiscreteGrid::data_t>>();
		}
	});
	QObject::connect(this->radioButton_mp, &QRadioButton::toggled, [this]() {
		if (this->radioButton_mp->isChecked()) {
			this->interpolator.reset();
			this->interpolator = std::make_shared<Interpolators::mostPresent<DiscreteGrid::data_t>>();
		}
	});
	QObject::connect(this->radioButton_min, &QRadioButton::toggled, [this]() {
		if (this->radioButton_min->isChecked()) {
			this->interpolator.reset();
			this->interpolator = std::make_shared<Interpolators::min<DiscreteGrid::data_t>>();
		}
	});
	QObject::connect(this->radioButton_max, &QRadioButton::toggled, [this]() {
		if (this->radioButton_max->isChecked()) {
			this->interpolator.reset();
			this->interpolator = std::make_shared<Interpolators::max<DiscreteGrid::data_t>>();
		}
	});
}

void GridLoaderWidget::disableWidgets() {
	this->button_loadDIM_1channel->setDisabled(true);
	this->button_loadDIM_2channel->setDisabled(true);
	this->button_loadTIF_1channel->setDisabled(true);
	this->button_loadTIF_2channel->setDisabled(true);
	this->button_loadGrids->setDisabled(true);

	this->dsb_transformationA->setDisabled(true);
	this->dsb_transformationDX->setDisabled(true);
	this->dsb_transformationDY->setDisabled(true);
	this->dsb_transformationDZ->setDisabled(true);

	this->frame_load1channel->setDisabled(true);
	this->frame_load2channel->setDisabled(true);
	this->frame_transfoDetails->setDisabled(true);

	this->groupBox_downsampling->setDisabled(true);
	this->groupBox_interpolator->setDisabled(true);

	this->radioButton_original->setDisabled(true);
	this->radioButton_low->setDisabled(true);
	this->radioButton_lower->setDisabled(true);
	this->radioButton_lowest->setDisabled(true);
	this->radioButton_nn->setDisabled(true);
	this->radioButton_mean->setDisabled(true);
	this->radioButton_mp->setDisabled(true);
	this->radioButton_min->setDisabled(true);
	this->radioButton_max->setDisabled(true);
}

void GridLoaderWidget::resetGridInfoLabel() {
	this->label_gridInfoR->setText("Loading grid information ...");
	this->label_gridInfoG->setText("");
	return;
}

void GridLoaderWidget::computeGridInfoLabel() {
	if (this->readerR == nullptr) {
		this->label_gridInfoR->setText("<No grid loaded>");
		this->label_gridInfoG->setText("");
		return;
	}

	this->readerR->enableDownsampling(this->dsLevel);
	auto dims = this->readerR->getGridDimensions();
	std::size_t fnSize = this->readerR->getFilenames().size();
	QString infoGridR = "Image dimensions : " + QString::number(dims.x) + "x" + QString::number(dims.y) + "x" +
				QString::number(dims.z) + " in " + QString::number(fnSize) + " images.";
	this->label_gridInfoR->setText(infoGridR);

	if (this->readerG != nullptr) {
		this->readerG->enableDownsampling(this->dsLevel);
		auto dimsG = this->readerG->getGridDimensions();
		std::size_t fnSizeG = this->readerG->getFilenames().size();
		QString g = "Image dimensions : " + QString::number(dimsG.x) + "x" + QString::number(dimsG.y) + "x" +
				QString::number(dimsG.z) + " in " + QString::number(fnSizeG) + " images.";
		this->label_gridInfoG->setText(g);
	}
}

void GridLoaderWidget::loadGridDIM1channel() {
	if (this->readerR != nullptr) { this->readerR.reset(); this->readerR = nullptr; }
	if (this->readerG != nullptr) { this->readerG.reset(); this->readerG = nullptr; }
	IO::GenericGridReader::data_t defaultThreshold = 5;
	this->readerR = std::make_shared<IO::DIMReader>(defaultThreshold);
	// if an error occurs :
	QMessageBox* msgBox = new QMessageBox;
	msgBox->setAttribute(Qt::WA_DeleteOnClose);

	QString filenameR = QFileDialog::getOpenFileName(this, "Open DIM/IMA image", this->basePath.path(), "DIM/IMA header files (*.dim)");
	if (filenameR.isEmpty()) {
		msgBox->critical(this, "Error !", "No filenames provided !");
		this->readerR.reset(); this->readerR = nullptr;
		this->computeGridInfoLabel();
		return;
	}
	// update path from last file picker :
	this->basePath.setPath(QFileInfo(filenameR).path());

	std::vector<std::string> fnR;	// filenames, red channel
	fnR.push_back(filenameR.toStdString());

	delete msgBox;

	this->readerR->setFilenames(fnR);
	this->readerR->parseImageInfo();
	this->computeGridInfoLabel();
}

void GridLoaderWidget::loadGridDIM2channel() {
	if (this->readerR != nullptr) { this->readerR.reset(); this->readerR = nullptr; }
	if (this->readerG != nullptr) { this->readerG.reset(); this->readerG = nullptr; }
	IO::GenericGridReader::data_t defaultThreshold = 5;
	this->readerR = std::make_shared<IO::DIMReader>(defaultThreshold);
	this->readerG = std::make_shared<IO::DIMReader>(defaultThreshold);
	// if an error occurs :
	QMessageBox* msgBox = new QMessageBox;
	msgBox->setAttribute(Qt::WA_DeleteOnClose);

	QString filenameR = QFileDialog::getOpenFileName(this, "Open DIM/IMA image (Red channel)", this->basePath.path(), "DIM/IMA header files (*.dim)");
	if (filenameR.isEmpty()) {
		msgBox->critical(this, "Error !", "No filenames provided !");
		this->readerR.reset(); this->readerR = nullptr;
		this->readerG.reset(); this->readerG = nullptr;
		this->computeGridInfoLabel();
		return;
	}
	// update path from last file picker :
	this->basePath.setPath(QFileInfo(filenameR).path());

	QString filenameG = QFileDialog::getOpenFileName(this, "Open DIM/IMA image (Blue channel)", this->basePath.path(), "DIM/IMA header files (*.dim)");
	if (filenameG.isEmpty()) {
		msgBox->critical(this, "Error !", "No filenames provided !");
		this->readerR.reset(); this->readerR = nullptr;
		this->readerG.reset(); this->readerG = nullptr;
		this->computeGridInfoLabel();
		return;
	}
	// update path from last file picker :
	this->basePath.setPath(QFileInfo(filenameG).path());

	std::vector<std::string> fnR;	// filenames, red channel
	fnR.push_back(filenameR.toStdString());
	std::vector<std::string> fnG;	// filenames, green channel
	fnG.push_back(filenameG.toStdString());

	delete msgBox;

	this->readerR->setFilenames(fnR);
	this->readerR->parseImageInfo();
	this->readerG->setFilenames(fnG);
	this->readerG->parseImageInfo();
	this->computeGridInfoLabel();
}

void GridLoaderWidget::loadGridTIF1channel() {
	if (this->readerR != nullptr) { this->readerR.reset(); }
	if (this->readerG != nullptr) { this->readerG.reset(); }
	IO::GenericGridReader::data_t defaultThreshold = 5;
	this->readerR = std::make_shared<IO::libTIFFReader>(defaultThreshold);
	// if an error occurs :
	QMessageBox* msgBox = new QMessageBox;
	msgBox->setAttribute(Qt::WA_DeleteOnClose);

	QStringList filenamesR = QFileDialog::getOpenFileNames(this, "Open TIFF images (Red channel)", this->basePath.path(), "TIFF files (*.tiff *.tif)");
	if (filenamesR.empty()) {
		msgBox->critical(this, "Error !", "No filenames provided !");
		this->readerR.reset(); this->readerR = nullptr;
		this->computeGridInfoLabel();
		return;
	}
	// update path from last file picker :
	this->basePath.setPath(QFileInfo(filenamesR[0]).path());

	std::vector<std::string> fnR;	// filenames, red channel
	for (int i = 0; i < filenamesR.size(); ++i) { fnR.push_back(filenamesR[i].toStdString()); }

	delete msgBox;

	this->readerR->setFilenames(fnR);
	this->readerR->parseImageInfo();
	this->computeGridInfoLabel();
}

void GridLoaderWidget::loadGridTIF2channel() {
	if (this->readerR != nullptr) { this->readerR.reset(); }
	if (this->readerG != nullptr) { this->readerG.reset(); }
	IO::GenericGridReader::data_t defaultThreshold = 5;
	this->readerR = std::make_shared<IO::libTIFFReader>(defaultThreshold);
	this->readerG = std::make_shared<IO::libTIFFReader>(defaultThreshold);
	// if an error occurs :
	QMessageBox* msgBox = new QMessageBox;
	msgBox->setAttribute(Qt::WA_DeleteOnClose);

	QStringList filenamesR = QFileDialog::getOpenFileNames(this, "Open TIFF images (Red channel)", this->basePath.path(), "TIFF files (*.tiff *.tif)");
	if (filenamesR.empty()) {
		msgBox->critical(this, "Error !", "No filenames provided !");
		this->readerR.reset(); this->readerR = nullptr;
		this->readerG.reset(); this->readerG = nullptr;
		this->computeGridInfoLabel();
		return;
	}
	// update path from last file picker :
	this->basePath.setPath(QFileInfo(filenamesR[0]).path());

	QStringList filenamesG = QFileDialog::getOpenFileNames(this, "Open TIFF images (Blue channel)", this->basePath.path(), "TIFF files (*.tiff *.tif)");
	if (filenamesG.empty()) {
		msgBox->critical(this, "Error !", "No filenames provided !");
		this->readerR.reset(); this->readerR = nullptr;
		this->readerG.reset(); this->readerG = nullptr;
		this->computeGridInfoLabel();
		return;
	}
	// update path from last file picker :
	this->basePath.setPath(QFileInfo(filenamesG[0]).path());

	std::vector<std::string> fnR;	// filenames, red channel
	for (int i = 0; i < filenamesR.size(); ++i) { fnR.push_back(filenamesR[i].toStdString()); }
	std::vector<std::string> fnG;	// filenames, green channel
	for (int i = 0; i < filenamesG.size(); ++i) { fnG.push_back(filenamesG[i].toStdString()); }

	delete msgBox;

	this->readerR->setFilenames(fnR);
	this->readerR->parseImageInfo();
	this->readerG->setFilenames(fnG);
	this->readerG->parseImageInfo();
	this->computeGridInfoLabel();
}

void GridLoaderWidget::loadGrid() {
	LOG_ENTER(GridLoaderWidget::loadGrid())
	if (readerR == nullptr) {
		QMessageBox* msgBox = new QMessageBox;
		msgBox->setAttribute(Qt::WA_DeleteOnClose);
		msgBox->critical(this, "Error", "No grid was selected.");
		return;
	}

	bool hasUserBounds = this->groupbox_userLimits->isChecked();
	DiscreteGrid::data_t userMin = static_cast<DiscreteGrid::data_t>(this->spinbox_userLimitMin->value());
	DiscreteGrid::data_t userMax = static_cast<DiscreteGrid::data_t>(this->spinbox_userLimitMax->value());

	float dx = this->dsb_transformationDX->value();
	float dy = this->dsb_transformationDY->value();
	float dz = this->dsb_transformationDZ->value();

	this->readerR->enableDownsampling(this->dsLevel);
	this->readerR->setUserVoxelSize(dx, dy, dz);
	if (hasUserBounds) { this->readerR->setUserIntensityLimits(userMin, userMax); }
	if (this->readerG != nullptr) {
		this->readerG->enableDownsampling(this->dsLevel);
		this->readerG->setUserVoxelSize(dx, dy, dz);
		if (hasUserBounds) { this->readerG->setUserIntensityLimits(userMin, userMax); }
	}

	if (this->dsLevel != IO::DownsamplingLevel::Original) {
		this->readerR->setInterpolationMethod(this->interpolator);
		if (this->readerG != nullptr) {
			this->readerG->setInterpolationMethod(this->interpolator);
		}
	}

	std::size_t completeSizeBits = this->readerR->getGridSizeBytes() * 8;
	if (this->readerG != nullptr) {
		completeSizeBits += this->readerG->getGridSizeBytes() * 8;
	}

	// Check user memory allowed, and ask for confirmation if necessary :
	UserSettings settings = UserSettings::getInstance();
	PRINTVAL(settings.getUserRemainingBitSize())
	PRINTVAL(completeSizeBits);
	if (settings.getUserRemainingBitSize() < completeSizeBits) {
		QMessageBox* confirmBox = new QMessageBox();
		confirmBox->setWindowTitle("Warning : Memory load");
		confirmBox->setText("Loading those images will go over your max memory budget. Would you like to continue regardless ?");
		QPushButton* confirm_Accept = confirmBox->addButton("Yes", QMessageBox::ButtonRole::AcceptRole);
		QPushButton* confirm_Deny = confirmBox->addButton("No, take me back", QMessageBox::ButtonRole::RejectRole);

		confirmBox->exec();
		if (confirmBox->clickedButton() == confirm_Deny) {
			// if the user doesn't want anything like this, return.
			LOG_LEAVE(GridLoaderWidget::loadGrid())
			return;
		}

		// user wants to do this.
		settings.loadImageSize(completeSizeBits);
	}

	this->readerR->loadImage();
	// generate input grids :
	this->inputGridR = std::make_shared<InputGrid>();
	this->inputGridR->setGridReader(this->readerR);
	this->inputGridR->fromGridReader();

	if (this->readerG != nullptr) {
		this->readerG->loadImage();
		this->inputGridG = std::make_shared<InputGrid>();
		this->inputGridG->setGridReader(this->readerG);
		this->inputGridG->fromGridReader();
	}

	// setup transformation :
	glm::vec3 vxdims = glm::vec3(
		static_cast<float>(this->dsb_transformationDX->value()),
		static_cast<float>(this->dsb_transformationDY->value()),
		static_cast<float>(this->dsb_transformationDZ->value())
	);
	svec3 dims = this->inputGridR->getResolution();
	float a = this->dsb_transformationA->value();

	// Add transformation matrix to red, and possibly green matrix :
	auto colB = this->readerR->getTextureLimits();
	this->inputGridR->setTransform_GridToWorld(computeTransfoShear(a, this->inputGridR, vxdims));
	if (this->readerG != nullptr) {
		dims = this->inputGridG->getResolution();
		this->inputGridG->setTransform_GridToWorld(computeTransfoShear(a, this->inputGridG, vxdims));
		auto colG = this->readerG->getTextureLimits();
		colB.x = std::min(colB.x, colG.x);
		colB.y = std::max(colB.y, colG.y);
	}

	// Disable widgets purely so that no data races
	// or events might cause unexpected behaviour :
	this->disableWidgets();

	if (this->readerG == nullptr) {
		this->viewer->loadGrid(this->inputGridR);
	} else {
		this->viewer->loadTwoGrids(this->inputGridR, this->inputGridG);
	}
	this->viewer->centerScene();
	this->scene->slotSetMinColorValue(colB.x);
	this->scene->slotSetMaxColorValue(colB.y);

	this->_cp->updateMinValue(colB.x);
	this->_cp->updateMaxValue(colB.y);

	LOG_LEAVE(GridLoaderWidget::loadGrid())
	this->close();
}
