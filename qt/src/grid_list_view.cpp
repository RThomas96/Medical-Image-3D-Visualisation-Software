#include "../include/grid_list_view.hpp"

#include "../include/grid_detailed_view.hpp"
#include "../../grid/include/input_discrete_grid.hpp"
#include "../../grid/include/output_discrete_grid.hpp"

#include <cassert>
#include <type_traits>

GridView::GridView(GridDetailedView* const _details, const std::shared_ptr<DiscreteGrid>& _grid) : grid(_grid) {
	if (_details == nullptr) { throw std::runtime_error("[ERROR] Details was null.\n"); }

	this->detailedView = _details;
	if (_grid.get() == nullptr) { throw std::runtime_error("[ERROR] Grid was null.\n"); }

	this->label_gridType = new QLabel("<no grid type defined>");
	this->label_gridType->setTextFormat(Qt::TextFormat::RichText);
	this->label_gridName = new QLabel("<no grid>");
	this->label_gridImageCount = new QLabel("<no grid>");
	this->label_gridResolution = new QLabel("<no grid>");
	this->label_gridDiskSize = new QLabel("<no grid>");

	this->label_headerName = new QLabel("Name : ");
	this->label_headerImageCount = new QLabel("Image count : ");
	this->label_headerResolution = new QLabel("Resolution : ");
	this->label_headerDiskSize = new QLabel("Size on disk : ");

	this->button_modifyGrid = new QPushButton("View/Modify");

	this->gridLayout = new QGridLayout(this);

	// Try to read values :
	try {
		this->readValuesFromGrid();
	}  catch (const std::runtime_error& e) {
		std::cerr << "[ERROR] Caught a signal while reading values from grid.\nMessage : " << e.what()<< '\n';
		throw std::runtime_error("[ERROR] Could not read values from grid.");
	}

	// Try to place widgets on grid :
	try {
		this->placeWidgetsOnGrid(); // Place the widgets on the grid.
	} catch (const std::runtime_error& e) {
		std::cerr << "[ERROR] Caught a signal while putting widgets on the grid.\nMessage" << e.what() << '\n';
		throw std::runtime_error("[ERROR] Could not place widgets on the grid.");
	}

	try {
		this->setupWidgetConnections();
	} catch (std::runtime_error& e) {
		std::cerr << "[ERROR] Caught a signal while connecting widgets\nMessage" << e.what() << '\n';
		throw std::runtime_error("[ERROR] Could not connect widgets.");
	}
}

GridView::~GridView(void) {
	this->deleteQObjects();
}

void GridView::readValuesFromGrid() {
	// Should already be caught by the constructor, but we never know :
	if (this->grid == nullptr) {
		throw std::runtime_error("[ERROR] Trying to read values from no grid (shared_ptr equal to nullptr).\n");
	}

	/// Grid name
	std::string gridName = this->grid->getGridName();
	/// Resolution to display to the user
	std::string resolutionString = this->resolutionToString(this->grid->getResolution());
	/// Size on disk and number of images : [TODO]
	// std::size_t diskSize = this->grid->getDiskSize();
	// std::size_t imagesLoaded = this->grid->getImagesLoaded().size();

	this->label_gridImageCount->setText("<N/A TODO>");
	this->label_gridDiskSize->setText("<N/A TODO>");

	QString formattedText = "<b>" + QString(gridName.c_str()) + "</b>";
#ifdef GRID_LIST_ITEM_ENABLE_ELLIPSIS_TEXT
	QFontMetrics fontMetrics(this->label_gridName->font());
	QString ellipsisText = fontMetrics.elidedText(formattedText, Qt::ElideRight, this->label_gridName->width());

	this->label_gridName->setText(ellipsisText);
#else
	this->label_gridName->setText(formattedText);
#endif
	this->label_gridResolution->setText(QString(resolutionString.c_str()));

	auto ptr_t = dynamic_cast<InputGrid*>(this->grid.get());
	if (ptr_t == nullptr) {
		// The grid is NOT an input (is of OutputGrid type) :
		this->label_gridType->setText("Output Grid");
	} else {
		this->label_gridType->setText("Input Grid");
	}

	return;
}

void GridView::setupWidgetConnections(void) {
	if (this->detailedView == nullptr) {
		throw std::runtime_error("[ERROR] : Detailed view widget was not satisfied.\n");
	}

	// Connect 'View' button to showing the grid :
	connect(this->button_modifyGrid, &QPushButton::pressed, this, &GridView::proxy_viewGrid);
}

void GridView::blockEverySignal(bool blocked) {
	this->button_modifyGrid->blockSignals(blocked);
	this->blockSignals(blocked);
}

std::string GridView::resolutionToString(const resolution_t& res) const {
	using size_t = resolution_t::value_type;

	size_t sizeX = res.x;
	size_t sizeY = res.y;
	size_t sizeZ = res.z;
	size_t totalVoxelCount = sizeX * sizeY * sizeZ;

	// Separate into exponent and floating point value :
	double doubleVx = static_cast<double>(totalVoxelCount);
	std::string vxUnit = "";
	// exponent will always be positive :
	unsigned int exp = (doubleVx==0)? 0 : 1 + static_cast<unsigned int>(std::floor(std::log10(doubleVx)));
	switch (exp) {
		case  0: [[fallthrough]];
		case  1: [[fallthrough]];
		case  2: vxUnit = ""; break;
		case  3: [[fallthrough]];
		case  4: [[fallthrough]];
		case  5: vxUnit = "kilo"; break;
		case  6: [[fallthrough]];
		case  7: [[fallthrough]];
		case  8: vxUnit = "mega"; break;
		case  9: [[fallthrough]];
		case 10: [[fallthrough]];
		case 11: vxUnit = "giga"; break;
		case 12: [[fallthrough]];
		case 13: [[fallthrough]];
		case 14: vxUnit = "tera"; break;
		case 15: [[fallthrough]];
		case 16: [[fallthrough]];
		case 17: vxUnit = "peta"; break;
		case 18: [[fallthrough]];
		case 19: [[fallthrough]];
		case 20: vxUnit = "exa"; break;
		case 21: [[fallthrough]];
		case 22: [[fallthrough]];
		case 23: vxUnit = "zetta"; break;
		case 24: [[fallthrough]];
		case 25: [[fallthrough]];
		case 26: vxUnit = "yotta"; break;
		default: vxUnit = "[too much]"; break;
	}
	unsigned int realExp = static_cast<unsigned int>(std::floor(static_cast<double>(exp - exp % 3)));
	double vxFloat = (realExp > 0) ? (doubleVx / std::pow(10, realExp)) : doubleVx;
	vxFloat = std::floor(vxFloat);
	std::string formattedResolution = std::to_string(sizeX) + "x" + std::to_string(sizeY) + "x" +
					std::to_string(sizeZ)+" ≈ "+std::to_string(vxFloat) +" "+vxUnit+"Voxels";
	return formattedResolution;
}

void GridView::placeWidgetsOnGrid(void) {
	if (this->gridLayout == nullptr) {
		throw std::runtime_error("[ERROR] GridLayout for grid was null.\n");
	}

	int currentRow = 0;
	// Place the grid type in the middle of the first row :
	this->gridLayout->addWidget(this->label_gridType, currentRow, 0, 1, -1, Qt::AlignmentFlag::AlignCenter);
	currentRow+=2; // a bit of space

	// Add the grid name :
	this->gridLayout->addWidget(this->label_headerName, currentRow, 0, Qt::AlignmentFlag::AlignLeft);
	this->gridLayout->addWidget(this->label_gridName, currentRow, 1, Qt::AlignmentFlag::AlignLeft);
	currentRow += 1;

	// Add the grid resolution :
	this->gridLayout->addWidget(this->label_headerResolution, currentRow, 0, Qt::AlignmentFlag::AlignLeft);
	this->gridLayout->addWidget(this->label_gridResolution, currentRow, 1, Qt::AlignmentFlag::AlignLeft);
	currentRow += 1;

	// Add the disk size :
	this->gridLayout->addWidget(this->label_headerDiskSize, currentRow, 0, Qt::AlignmentFlag::AlignLeft);
	this->gridLayout->addWidget(this->label_gridDiskSize, currentRow, 1, Qt::AlignmentFlag::AlignLeft);
	currentRow += 1;

	// Add the image count :
	this->gridLayout->addWidget(this->label_headerImageCount, currentRow, 0, Qt::AlignmentFlag::AlignLeft);
	this->gridLayout->addWidget(this->label_gridImageCount, currentRow, 1, Qt::AlignmentFlag::AlignLeft);
	currentRow += 1;

	// Add the button to mofify the grid :
	this->gridLayout->addWidget(this->button_modifyGrid, currentRow/2, 3, 3, 1, Qt::AlignmentFlag::AlignCenter);

	return;
}

void GridView::updateValues() {
	this->readValuesFromGrid();
	return;
}

void GridView::proxy_viewGrid() {
	this->detailedView->showGrid(this, this->grid);
	return;
}

void GridView::deleteQObjects(void) {
	auto deletePtr = [](auto* obj) {
		if (obj != nullptr) {
			delete obj;
		}
		obj = nullptr;
	};

	disconnect(this->button_modifyGrid, nullptr, nullptr, nullptr);
	this->button_modifyGrid->disconnect(this->detailedView);

	deletePtr(this->label_gridType);
	deletePtr(this->label_gridName);
	deletePtr(this->label_gridResolution);
	deletePtr(this->label_gridImageCount);
	deletePtr(this->label_gridDiskSize);
	deletePtr(this->label_headerName);
	deletePtr(this->label_headerResolution);
	deletePtr(this->label_headerImageCount);
	deletePtr(this->label_headerDiskSize);
	deletePtr(this->button_modifyGrid);
	deletePtr(this->gridLayout);
	this->detailedView = nullptr; // might still be used for other things, better not to delete it here.
}
