#include "../include/grid_detailed_view.hpp"

#include "../../grid/include/input_discrete_grid.hpp"
#include "../../grid/include/output_discrete_grid.hpp"

GridDetailedView::GridDetailedView() {
	this->grid = nullptr;
	this->proxy_boundingBox = nullptr;
	this->gridListItem = nullptr;

	// For the moment, no grid has been set. We can only create the widget but not populate them :
	this->createWidgets();
	this->setNonEditable();
}

GridDetailedView::~GridDetailedView() {
	this->disconnectSignals();
	this->deleteWidgets();
	this->grid.reset();
}

void GridDetailedView::setEditable() {
	// enable the bounding box controls :
	for (auto& b : this->editable_BBControls) {
		b->blockSignals(false);
		b->setReadOnly(false);
		b->setEnabled(true);
	}
	for (auto& b : this->editable_GridResolution) {
		b->blockSignals(false);
		b->setReadOnly(false);
		b->setEnabled(true);
	}
	this->blockEverySignal(false);
	std::cerr << "[INFO]" << __PRETTY_FUNCTION__ << " : Check if we need to enable more items here.";
}

void GridDetailedView::setNonEditable() {
	// enable the bounding box controls :
	for (auto& b : this->editable_BBControls) {
		b->blockSignals(true);
		b->setReadOnly(true);
		b->setEnabled(false);
	}
	for (auto& b : this->editable_GridResolution) {
		b->blockSignals(true);
		b->setReadOnly(true);
		b->setEnabled(false);
	}
	std::cerr << "[INFO][" << __PRETTY_FUNCTION__ << "] : Check if we need to disable more items here.\n";
}

void GridDetailedView::updateSignals(void) {
	// Early sanity check :
	if (this->grid == nullptr) { return; }
	// Disconnect already connected signals :
	this->disconnectSignals();
	this->updateValues();

	// Connect the text field to the grid name :
	connect(this->editable_gridName, &QLineEdit::textEdited, this, &GridDetailedView::proxy_ChangeGridName);

	// Reset proxies for the bounding box :
	if (this->proxy_boundingBox != nullptr) {
		this->proxy_boundingBox.reset();
		this->proxy_boundingBox = nullptr;
	}
	if (this->proxy_gridResolution != nullptr) {
		this->proxy_gridResolution.reset();
		this->proxy_gridResolution = nullptr;
	}

	if (this->grid->isModifiable()) {
		// Create a new proxy for the bounding box and resolution of this grid :
		this->proxy_boundingBox = std::make_shared<Proxies::BoundingBox>(this->grid, &this->grid.get()->boundingBox);
		this->proxy_gridResolution = std::make_shared<Proxies::Resolution>(this->grid, &this->grid->gridDimensions);

		// Connect the bounding box to the proxy :
		if (this->proxy_boundingBox != nullptr) {
			// Setting the min point of the bounding box :
			connect(this->editable_BBControls[0], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
				this->proxy_boundingBox.get(), &Proxies::BoundingBox::setMinX);
			connect(this->editable_BBControls[1], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
				this->proxy_boundingBox.get(), &Proxies::BoundingBox::setMinY);
			connect(this->editable_BBControls[2], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
				this->proxy_boundingBox.get(), &Proxies::BoundingBox::setMinZ);

			// Setting the max point of the bounding box :
			connect(this->editable_BBControls[3], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
				this->proxy_boundingBox.get(), &Proxies::BoundingBox::setMaxX);
			connect(this->editable_BBControls[4], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
				this->proxy_boundingBox.get(), &Proxies::BoundingBox::setMaxY);
			connect(this->editable_BBControls[5], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
				this->proxy_boundingBox.get(), &Proxies::BoundingBox::setMaxZ);
			//connect(this->editable_GridResolution[0], QOverload<int>::of(&QSpinBox::valueChanged),);
			connect(this->editable_GridResolution[0], QOverload<int>::of(&QSpinBox::valueChanged),
				this->proxy_gridResolution.get(), &Proxies::Resolution::setResolutionX);
			connect(this->editable_GridResolution[1], QOverload<int>::of(&QSpinBox::valueChanged),
				this->proxy_gridResolution.get(), &Proxies::Resolution::setResolutionY);
			connect(this->editable_GridResolution[2], QOverload<int>::of(&QSpinBox::valueChanged),
				this->proxy_gridResolution.get(), &Proxies::Resolution::setResolutionZ);
		}
	} else {
		// Do nothing.
	}
}

void GridDetailedView::showGrid(GridView* _caller, const std::shared_ptr<DiscreteGrid>& _grid) {
	this->grid.reset();
	this->grid = _grid;
	this->gridListItem = _caller;

	this->updateValues();
	this->updateSignals();

	OutputGrid* gridType = dynamic_cast<OutputGrid*>(_grid.get());
	if (gridType == nullptr) {
		// If the dynamic_cast didn't go through, then it is NOT an output grid :
		this->setNonEditable();
	} else {
		this->setEditable();
	}
}

void GridDetailedView::proxy_ChangeGridName(const QString& newText) {
	if (this->grid != nullptr) {
		this->grid->setGridName(newText.toStdString());
		if (this->gridListItem != nullptr) {
			this->gridListItem->updateValues();
		}
	}
	return;
}

void GridDetailedView::createWidgets(void) {
	// Create layouts :
	this->layout_leftPane = new QVBoxLayout();
	this->layout_rightPane = new QVBoxLayout();
	this->layout_widget = new QVBoxLayout();
	this->layout_separator = new QFrame();
	this->layout_sideBySide = new QHBoxLayout();

	// Create group boxes :
	this->groupBox_viewer = new QGroupBox();
	this->groupBox_transformation = new QGroupBox();
	this->groupBox_transformationDetails = new QGroupBox("Transformation Details");
	this->groupBox_gridDetails = new QGroupBox("Grid details");
	this->groupBox_boundingBox = new QGroupBox("Bounding box controls :");

	// Create labels :
	this->label_headerGridName = new QLabel("Name :");
	this->label_headerGridResolution = new QLabel("Size :");
	this->label_headerDetailedView = new QLabel("<h1>Grid details</h1>");
	this->label_headerTransformation = new QLabel("Transformation details :");

	// Create resolution controls :
	this->editable_GridResolution=  {
		new QSpinBox(), new QSpinBox(), new QSpinBox()
	};
	// Create the bounding box controls ...
	this->editable_BBControls = {
		new QDoubleSpinBox(), new QDoubleSpinBox(), new QDoubleSpinBox(),
		new QDoubleSpinBox(), new QDoubleSpinBox(), new QDoubleSpinBox()
	};
	// ... and their labels :
	this->label_BBControls = {
		new QLabel("Min X :"), new QLabel("Min Y :"), new QLabel("Min Z :"),
		new QLabel("Max X :"), new QLabel("Max Y :"), new QLabel("Max Z :")
	};

	// Get the bounding box's data type :
	using data_t = typename std::decay_t<decltype(*this->grid.get())>::bbox_t::data_t;
	for (std::size_t i = 0; i < 6; ++i) {
		this->editable_BBControls[i]->setMinimum(std::numeric_limits<data_t>::lowest());
		this->editable_BBControls[i]->setMaximum(std::numeric_limits<data_t>::max());
	}
	for (std::size_t i = 0; i < 3; ++i) {
		this->editable_GridResolution[i]->setMinimum(0);
		this->editable_GridResolution[i]->setMaximum(std::numeric_limits<int>::max());
	}

	this->label_gridResolution = new QLabel("<grid resolution>");

	// Create the editable text field :
	this->editable_gridName = new QLineEdit("<grid name>");

	/**********************/
	/* Set up the widgets */
	/**********************/
	this->layout_separator->setFrameShape(QFrame::VLine);
	this->layout_separator->setFrameShadow(QFrame::Plain);

	QGridLayout* detailsView = new QGridLayout(); ///< Grid layout for the grid details
	QGridLayout* bbView = new QGridLayout(); ///< Grid layout for the BB controls
	QGridLayout* trView = new QGridLayout(); ///< Grid layout for the transformation picker
	QHBoxLayout* tempLayout = new QHBoxLayout(); ///< HBox layout to fill the transformation groupbox
	QHBoxLayout* rightL = new QHBoxLayout(); ///< HBox layout to fill the transformation groupbox
	QHBoxLayout* resLayout = new QHBoxLayout(); ///< HBoxlayout to control the grid resolution

#ifdef GRID_DETAILS_ENABLE_TRANSFORMATION_VIEWER
	std::cerr << "[WARNING][" << __PRETTY_FUNCTION__ << "] : Did not implement the transformation viewer !\n";
#endif

	this->otherBoxLayouts.push_back(detailsView);
	this->otherBoxLayouts.push_back(bbView);
	this->otherBoxLayouts.push_back(trView);
	this->otherBoxLayouts.push_back(tempLayout);
	this->otherBoxLayouts.push_back(rightL);
	this->otherBoxLayouts.push_back(resLayout);

	detailsView->addWidget(this->label_headerGridName, 0, 0, Qt::AlignLeft);
	detailsView->addWidget(this->editable_gridName, 0, 1, Qt::AlignCenter);

	resLayout->addWidget(this->editable_GridResolution[0]);
	resLayout->addWidget(this->editable_GridResolution[1]);
	resLayout->addWidget(this->editable_GridResolution[2]);

	detailsView->addWidget(this->label_headerGridResolution, 1, 0, Qt::AlignLeft);
	detailsView->addLayout(resLayout, 1, 1, Qt::AlignLeft);

	this->groupBox_gridDetails->setLayout(detailsView);
	this->layout_leftPane->addWidget(this->groupBox_gridDetails);

	// Row 0 :
	bbView->addWidget(this->label_BBControls[0], 0, 0, Qt::AlignCenter);
	bbView->addWidget(this->label_BBControls[1], 0, 1, Qt::AlignCenter);
	bbView->addWidget(this->label_BBControls[2], 0, 2, Qt::AlignCenter);
	// Row 1 :
	bbView->addWidget(this->editable_BBControls[0], 1, 0, Qt::AlignCenter);
	bbView->addWidget(this->editable_BBControls[1], 1, 1, Qt::AlignCenter);
	bbView->addWidget(this->editable_BBControls[2], 1, 2, Qt::AlignCenter);
	// Row 2 :
	bbView->addWidget(this->label_BBControls[3], 2, 0, Qt::AlignCenter);
	bbView->addWidget(this->label_BBControls[4], 2, 1, Qt::AlignCenter);
	bbView->addWidget(this->label_BBControls[5], 2, 2, Qt::AlignCenter);
	// Row 3 :
	bbView->addWidget(this->editable_BBControls[3], 3, 0, Qt::AlignCenter);
	bbView->addWidget(this->editable_BBControls[4], 3, 1, Qt::AlignCenter);
	bbView->addWidget(this->editable_BBControls[5], 3, 2, Qt::AlignCenter);

	this->groupBox_boundingBox->setLayout(bbView);
	this->layout_leftPane->addWidget(this->groupBox_boundingBox);

	trView->addWidget(this->label_headerTransformation, 0, 0, Qt::AlignLeft);
	#ifdef GRID_DETAILS_ENABLE_TRANSFORMATION_PICKER
	trView->addWidget(this->picker_transformationStyle)
	#endif
	this->groupBox_transformationDetails->setLayout(tempLayout);
	trView->addWidget(this->groupBox_transformationDetails, 1, 0, Qt::AlignCenter);
	this->layout_leftPane->addLayout(trView);

	this->groupBox_viewer->setLayout(rightL);
	this->layout_rightPane->addWidget(this->groupBox_viewer);
	this->layout_sideBySide->addLayout(this->layout_leftPane);
	this->layout_sideBySide->addWidget(this->layout_separator);
	this->layout_sideBySide->addLayout(this->layout_rightPane);
	this->layout_widget->addWidget(this->label_headerDetailedView);
	this->layout_widget->addLayout(this->layout_sideBySide);

	this->setLayout(this->layout_widget);
}

void GridDetailedView::deleteWidgets(void) {
	// Lambda to check if the object should be deleted before doing it :
	auto deletePtr = [](auto* obj) {
		if (obj != nullptr) {
			delete obj;
		}
		obj = nullptr;
	};
	deletePtr(this->label_gridResolution);
	deletePtr(this->label_headerDetailedView);
	deletePtr(this->label_headerGridName);
	deletePtr(this->label_headerGridResolution);
	deletePtr(this->label_headerTransformation);

	deletePtr(this->editable_gridName);
	for (std::size_t i = 0; i < 6; ++i) {
		deletePtr(this->editable_BBControls[i]);
		deletePtr(this->label_BBControls[i]);
	}
	for (std::size_t i = 0; i < this->otherBoxLayouts.size(); ++i) {
		deletePtr(this->otherBoxLayouts[i]);
	}
	this->otherBoxLayouts.clear();
}

void GridDetailedView::disconnectSignals(void) {
	for (auto& b : this->editable_GridResolution) { b->disconnect(); }
	for (auto& b : this->editable_BBControls) { b->disconnect(); }
	this->editable_gridName->disconnect();
}

void GridDetailedView::blockEverySignal(bool _block) {
	for (auto& b : this->editable_BBControls) { b->blockSignals(_block); }
	for (auto& b : this->editable_GridResolution) { b->blockSignals(_block); }
	this->editable_gridName->blockSignals(_block);
}

void GridDetailedView::updateValues(void) {
	// First of all : block signals
	this->blockEverySignal(true);

	if (this->grid == nullptr) {
		throw std::runtime_error("[ERROR][Fatal] Grid shared_ptr was 'nullptr' when showing the grid.");
	}
	this->editable_gridName->setText(QString(this->grid->getGridName().c_str()));
	DiscreteGrid::sizevec3 dims = this->grid->getGridDimensions();
	DiscreteGrid::bbox_t::vec minBB = this->grid->getBoundingBox().getMin();
	DiscreteGrid::bbox_t::vec maxBB = this->grid->getBoundingBox().getMax();

	this->editable_GridResolution[0]->setValue(static_cast<int>(dims.x));
	this->editable_GridResolution[1]->setValue(static_cast<int>(dims.y));
	this->editable_GridResolution[2]->setValue(static_cast<int>(dims.z));
	this->editable_BBControls[0]->setValue(minBB.x);
	this->editable_BBControls[1]->setValue(minBB.y);
	this->editable_BBControls[2]->setValue(minBB.z);
	this->editable_BBControls[3]->setValue(maxBB.x);
	this->editable_BBControls[4]->setValue(maxBB.y);
	this->editable_BBControls[5]->setValue(maxBB.z);

	// We can finally unblock signals :
	this->blockEverySignal(false);
}
