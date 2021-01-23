#include "../include/neighbor_visu_main_widget.hpp"

#include <QLabel>
#include <QEvent>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QToolBar>
#include <QMenuBar>

MainWidget::MainWidget() {
	this->strayObj.clear();
	this->setupWidgets();
	this->widgetSizeSet = false;
}

MainWidget::~MainWidget() {
	this->removeEventFilter(this);
	this->headerZ->unregisterPlaneViewer();
	this->headerY->unregisterPlaneViewer();
	this->headerX->unregisterPlaneViewer();

	this->action_add1Grid->disconnect();
	this->action_saveGrid->disconnect();
	this->action_exitProgram->disconnect();

	delete this->action_add1Grid;
	delete this->action_saveGrid;
	delete this->viewer_planeZ;
	delete this->viewer_planeY;
	delete this->viewer_planeX;
	delete this->scene;
	delete this->controlPanel;
	delete this->headerZ;
	delete this->headerY;
	delete this->headerX;
	for (std::size_t i = 0; i < this->strayObj.size(); ++i) {
		if (this->strayObj[i] != nullptr) {
			delete this->strayObj[i];
			this->strayObj[i] = nullptr;
		}
	}
	this->strayObj.clear();
}

void MainWidget::setupWidgets() {
	this->scene = new Scene();

	this->action_add1Grid = new QAction("Open acquisition");
	this->action_saveGrid = new QAction("Save acquisition");
	this->action_showVisuBox = new QAction("Show visu box controller");
	this->action_exitProgram = new QAction("Exit program");

	this->fileMenu = this->menuBar()->addMenu("File");

	this->fileMenu->addAction(this->action_add1Grid);
	this->fileMenu->addAction(this->action_saveGrid);
	this->fileMenu->addAction(this->action_showVisuBox);
	this->fileMenu->addAction(this->action_exitProgram);

	// Connect add and save button to the slots/functions in the program :
	QObject::connect(this->action_add1Grid, &QAction::triggered, [this](){this->viewer->addGrid();});
	QObject::connect(this->action_saveGrid, &QAction::triggered, [this](){this->scene->launchSaveDialog();});
	QObject::connect(this->action_showVisuBox, &QAction::triggered, [this](){this->scene->showVisuBoxController();});
	QObject::connect(this->action_exitProgram, &QAction::triggered, this, &QMainWindow::close);

	this->viewer = new Viewer(this->scene, nullptr);
	this->header3d = new ViewerHeader3D(this->viewer, this->scene, nullptr);
	this->controlPanel = new ControlPanel(this->scene, this->viewer, nullptr);
	this->scene->setControlPanel(this->controlPanel);

	this->viewer_planeX = new PlanarViewer(this->scene, planes::x, planeHeading::North, nullptr);
	this->viewer_planeY = new PlanarViewer(this->scene, planes::y, planeHeading::North, nullptr);
	this->viewer_planeZ = new PlanarViewer(this->scene, planes::z, planeHeading::North, nullptr);

	// Sliders for each plane (also sets range and values) :
	this->headerX = new ViewerHeader("X Plane"); this->headerX->connectToViewer(this->viewer_planeX);
	this->headerY = new ViewerHeader("Y Plane"); this->headerY->connectToViewer(this->viewer_planeY);
	this->headerZ = new ViewerHeader("Z Plane"); this->headerZ->connectToViewer(this->viewer_planeZ);

	// Splitters : one main (hor.) and two secondaries (vert.) :
	QSplitter* mainSplit = new QSplitter(Qt::Horizontal);
	QSplitter* splitAbove = new QSplitter(Qt::Vertical);
	QSplitter* splitAbove1 = new QSplitter(Qt::Vertical);

	// Layouts to place a viewer and a header in the same place :
	QVBoxLayout* vP3 = new QVBoxLayout(); vP3->setSpacing(0);
	QVBoxLayout* vPX = new QVBoxLayout(); vPX->setSpacing(0);
	QVBoxLayout* vPY = new QVBoxLayout(); vPY->setSpacing(0);
	QVBoxLayout* vPZ = new QVBoxLayout(); vPZ->setSpacing(0);

	// Sets the background color of widgets :
	// QPalette paletteX, paletteY, paletteZ;
	// paletteX.setColor(QPalette::Window, Qt::red);
	// paletteY.setColor(QPalette::Window, Qt::green);
	// paletteZ.setColor(QPalette::Window, Qt::blue);

	// Those will encapsulate the layouts above :
	QWidget* _ViewerCapsule = new QWidget(); // _ViewerCapsule->setAutoFillBackground(true); xViewerCapsule->setPalette(paletteX);
	QWidget* xViewerCapsule = new QWidget(); // xViewerCapsule->setAutoFillBackground(true); xViewerCapsule->setPalette(paletteX);
	QWidget* yViewerCapsule = new QWidget(); // yViewerCapsule->setAutoFillBackground(true); yViewerCapsule->setPalette(paletteY);
	QWidget* zViewerCapsule = new QWidget(); // zViewerCapsule->setAutoFillBackground(true); zViewerCapsule->setPalette(paletteZ);

	this->header3d->setFixedHeight(this->header3d->sizeHint().height());
	this->headerX->setFixedHeight(this->headerX->sizeHint().height());
	this->headerY->setFixedHeight(this->headerY->sizeHint().height());
	this->headerZ->setFixedHeight(this->headerZ->sizeHint().height());

	// Set the layouts for the plane viewers :
	vP3->addWidget(this->header3d);	vP3->addWidget(this->viewer);
	vPX->addWidget(this->headerX);	vPX->addWidget(this->viewer_planeX);
	vPY->addWidget(this->headerY);	vPY->addWidget(this->viewer_planeY);
	vPZ->addWidget(this->headerZ);	vPZ->addWidget(this->viewer_planeZ);

	// Get content margins by default :
	int left = 0, right = 0, top = 0, bottom = 0;
	// This is the same arrangement for the setCM() function :
	vPX->getContentsMargins(&left, &top, &right, &bottom);
	// Set the content margins, no side margins :
	vP3->setContentsMargins(0, top*2, 0, bottom);
	vPX->setContentsMargins(0, top*2, 0, bottom);
	vPY->setContentsMargins(0, top*2, 0, bottom);
	vPZ->setContentsMargins(0, top*2, 0, bottom);

	// Encapsulate the layouts above :
	_ViewerCapsule->setLayout(vP3);
	xViewerCapsule->setLayout(vPX);
	yViewerCapsule->setLayout(vPY);
	zViewerCapsule->setLayout(vPZ);

	// Add to splits in order to show them all :
	splitAbove->addWidget(_ViewerCapsule);
	splitAbove->addWidget(xViewerCapsule);
	splitAbove1->addWidget(yViewerCapsule);
	splitAbove1->addWidget(zViewerCapsule);
	// Add the sub-splits to the main one :
	mainSplit->addWidget(splitAbove);
	mainSplit->addWidget(splitAbove1);

	QHBoxLayout* viewerLayout = new QHBoxLayout();
	viewerLayout->addWidget(mainSplit);

	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(this->controlPanel);
	mainLayout->addLayout(viewerLayout);
	mainLayout->setAlignment(this->controlPanel, Qt::AlignVCenter);

	this->strayObj.push_back(zViewerCapsule);
	this->strayObj.push_back(yViewerCapsule);
	this->strayObj.push_back(xViewerCapsule);
	this->strayObj.push_back(splitAbove1);
	this->strayObj.push_back(splitAbove);
	this->strayObj.push_back(mainSplit);
	this->strayObj.push_back(viewerLayout);
	this->strayObj.push_back(mainLayout);

	QWidget* mainWidget = new QWidget();
	mainWidget->setLayout(mainLayout);
	this->setCentralWidget(mainWidget);

	QSizePolicy policy;
	QSize vSize = viewerLayout->sizeHint();
	this->controlPanel->setFixedHeight(static_cast<int>(static_cast<float>(vSize.height()) * .75f));

	this->installEventFilter(this);
}

bool MainWidget::eventFilter(QObject* obj, QEvent* e) {
	// Set our code to run after the original event handling :
	if (this->widgetSizeSet == false && obj == this && e->type() == QEvent::Show) {
		this->widgetSizeSet = true;
		// lock control panel size to the current size it has :
		this->controlPanel->setMinimumSize(this->controlPanel->size());
		this->controlPanel->setMaximumSize(this->controlPanel->size());
		// set the viewer to have a minimum size of controlPanelSize on width/height :
		this->viewer->setMinimumWidth(this->controlPanel->minimumWidth());
		this->viewer->setMinimumHeight(this->controlPanel->minimumWidth()/2);
	}
	// Return false, to handle the rest of the event normally
	return false;
}
