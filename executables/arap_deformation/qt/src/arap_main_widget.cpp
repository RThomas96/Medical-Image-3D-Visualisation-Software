#include "qt/include/arap_main_widget.hpp"

#include <QEvent>
#include <QHBoxLayout>
#include <QList>
#include <QMenuBar>
#include <QSizePolicy>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>

ARAPMainWidget::ARAPMainWidget() {
	this->strayObj.clear();
	this->widgetSizeSet = false;
	this->usettings		= nullptr;
	this->loaderWidget	= nullptr;
	this->boxController = nullptr;
	this->arap_controller=nullptr;
	this->setupWidgets();
	// Query a user settings instance to initialize it :
	UserSettings set = UserSettings::getInstance();
}

ARAPMainWidget::~ARAPMainWidget() {
	this->removeEventFilter(this);
	this->headerZ->unregisterPlaneViewer();
	this->headerY->unregisterPlaneViewer();
	this->headerX->unregisterPlaneViewer();
	if (this->boxController) {
		this->boxController->close();
	}
	// TODO : this sometimes segfaults on close.
	this->boxController = nullptr;

	this->action_exitProgram->disconnect();
	this->action_drawModeS->disconnect();
	this->action_drawModeV->disconnect();
	this->action_drawModeVB->disconnect();
	this->action_showVisuBox->disconnect();
	this->action_showSettings->disconnect();
	this->action_showHelp3D->disconnect();
	this->action_showHelpPlane->disconnect();

	delete this->action_showHelp3D;
	delete this->action_showHelpPlane;
	delete this->action_showSettings;

	delete this->action_exitProgram;
	delete this->action_drawModeVB;
	delete this->action_drawModeV;
	delete this->action_drawModeS;
	delete this->action_showVisuBox;
	delete this->viewer_planeZ;
	delete this->viewer_planeY;
	delete this->viewer_planeX;
	delete this->scene;
	delete this->controlPanel;
	delete this->headerZ;
	delete this->headerY;
	delete this->headerX;
	delete this->header3d;

	this->showGLLog->disconnect();
	this->deform->disconnect();

	for (auto strayObject : this->strayObj) {
		if (strayObject != nullptr) {
			delete strayObject;
		}
	}
	this->strayObj.clear();
}

void ARAPMainWidget::setupWidgets() {
	this->glDebug = new OpenGLDebugLog;
	this->scene	  = new Scene();
	this->scene->addOpenGLOutput(this->glDebug);

	this->statusBar = new QStatusBar;
	this->showGLLog = new QPushButton("Show GL log");
	this->deform	= new QPushButton("Deform the grid");
	this->statusBar->addPermanentWidget(this->showGLLog);
	this->statusBar->addPermanentWidget(this->deform);

	this->setStatusBar(this->statusBar);
	this->scene->addStatusBar(this->statusBar);

	// Viewer(s) creation along with control panel :
	this->viewer		= new Viewer(this->scene, this->statusBar, nullptr);
	this->viewer_planeX = new PlanarViewer(this->scene, planes::x, this->statusBar, planeHeading::North, nullptr);
	this->viewer_planeY = new PlanarViewer(this->scene, planes::y, this->statusBar, planeHeading::North, nullptr);
	this->viewer_planeZ = new PlanarViewer(this->scene, planes::z, this->statusBar, planeHeading::North, nullptr);
	this->controlPanel	= new ControlPanel(this->scene, this->viewer, nullptr);

	this->arap_controller = new ARAPController(this->viewer, this->scene);

	this->viewer->addStatusBar(this->statusBar);
	this->viewer_planeX->addParentStatusBar(this->statusBar);
	this->viewer_planeY->addParentStatusBar(this->statusBar);
	this->viewer_planeZ->addParentStatusBar(this->statusBar);
	// Sliders for each plane (also sets range and values) :
	this->header3d = new ViewerHeader3D(this->viewer, this->scene, nullptr);
	this->headerX  = new ViewerHeader("X Plane");
	this->headerX->connectToViewer(this->viewer_planeX);
	this->headerY = new ViewerHeader("Y Plane");
	this->headerY->connectToViewer(this->viewer_planeY);
	this->headerZ = new ViewerHeader("Z Plane");
	this->headerZ->connectToViewer(this->viewer_planeZ);

	// Actions creation :
	this->action_addGrid       = new QAction("Load images");
	this->action_showVisuBox   = new QAction("Show visu box controller");
	this->action_exitProgram   = new QAction("Exit program");
	this->action_drawModeS	   = new QAction("Set draw mode to Solid");
	this->action_drawModeV	   = new QAction("Set draw mode to Volumetric");
	this->action_drawModeVB	   = new QAction("Set draw mode to Volumetric(Boxed)");
	this->action_showHelp3D	   = new QAction("3D Viewer Help Page");
	this->action_showHelpPlane = new QAction("Planar Viewer Help Page");
	this->action_showSettings  = new QAction("Settings");

	// Menu creation :
	// File menu :
	this->fileMenu = this->menuBar()->addMenu("&File");
	this->fileMenu->addAction(this->action_showSettings);
	this->fileMenu->addAction(this->action_exitProgram);
	// view menu :
	this->viewMenu = this->menuBar()->addMenu("&View");
	this->viewMenu->addAction(this->action_drawModeS);
	this->viewMenu->addAction(this->action_drawModeV);
	this->viewMenu->addAction(this->action_drawModeVB);
	this->viewMenu->addAction(this->action_showVisuBox);
	// help menu :
	this->helpMenu = this->menuBar()->addMenu("&Help");
	this->helpMenu->addAction(this->action_showHelp3D);
	this->helpMenu->addAction(this->action_showHelpPlane);

	// Splitters : one main (hor.) and two secondaries (vert.) :
	auto* mainSplit   = new QSplitter(Qt::Horizontal);
	auto* splitAbove  = new QSplitter(Qt::Vertical);
	auto* splitAbove1 = new QSplitter(Qt::Vertical);

	// Layouts to place a viewer and a header in the same place :
	auto* vP3 = new QVBoxLayout();
	vP3->setSpacing(0);	   // header above 3D view
	auto* vPX = new QVBoxLayout();
	vPX->setSpacing(0);	   // header above plane X
	auto* vPY = new QVBoxLayout();
	vPY->setSpacing(0);	   // header above plane Y
	auto* vPZ = new QVBoxLayout();
	vPZ->setSpacing(0);	   // header above plane Z

	// Those will encapsulate the layouts above :
	auto* _ViewerCapsule = new QWidget();
	auto* xViewerCapsule = new QWidget();
	auto* yViewerCapsule = new QWidget();
	auto* zViewerCapsule = new QWidget();

	this->header3d->setFixedHeight(this->header3d->sizeHint().height());
	this->headerX->setFixedHeight(this->headerX->sizeHint().height());
	this->headerY->setFixedHeight(this->headerY->sizeHint().height());
	this->headerZ->setFixedHeight(this->headerZ->sizeHint().height());

	// Add widgets in layouts to compose the plane viewers :
	vP3->addWidget(this->header3d);
	vP3->addWidget(this->viewer);
	vPX->addWidget(this->headerX);
	vPX->addWidget(this->viewer_planeX);
	vPY->addWidget(this->headerY);
	vPY->addWidget(this->viewer_planeY);
	vPZ->addWidget(this->headerZ);
	vPZ->addWidget(this->viewer_planeZ);

	// Get content margins by default :
	int left = 0, right = 0, top = 0, bottom = 0;
	// This is the same arrangement for the setCM() function :
	vPX->getContentsMargins(&left, &top, &right, &bottom);
	// Set the content margins, no side margins :
	vP3->setContentsMargins(0, top * 2, 0, bottom);
	vPX->setContentsMargins(0, top * 2, 0, bottom);
	vPY->setContentsMargins(0, top * 2, 0, bottom);
	vPZ->setContentsMargins(0, top * 2, 0, bottom);

	// Encapsulate the layouts above :
	_ViewerCapsule->setLayout(vP3);
	xViewerCapsule->setLayout(vPX);
	yViewerCapsule->setLayout(vPY);
	zViewerCapsule->setLayout(vPZ);

	int max = std::numeric_limits<int>::max();
	// Add to splits in order to show them all :
	splitAbove->addWidget(_ViewerCapsule);
	splitAbove->addWidget(xViewerCapsule);
	splitAbove->setSizes(QList<int>({max, max}));
	splitAbove1->addWidget(yViewerCapsule);
	splitAbove1->addWidget(zViewerCapsule);
	splitAbove1->setSizes(QList<int>({max, max}));
	// Add the sub-splits to the main one :
	mainSplit->addWidget(splitAbove);
	mainSplit->addWidget(splitAbove1);

	auto* viewerLayout = new QHBoxLayout();
	viewerLayout->addWidget(mainSplit);

	auto* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(viewerLayout);
	mainLayout->addWidget(this->controlPanel, 1);

	// add pointers to Qobjects needed for this widget
	// that we need to detroy at cleanup time :
	this->strayObj.push_back(zViewerCapsule);
	this->strayObj.push_back(yViewerCapsule);
	this->strayObj.push_back(xViewerCapsule);
	this->strayObj.push_back(splitAbove1);
	this->strayObj.push_back(splitAbove);
	this->strayObj.push_back(mainSplit);
	this->strayObj.push_back(viewerLayout);
	this->strayObj.push_back(mainLayout);

	QSize v = viewerLayout->sizeHint();
	this->controlPanel->setMinimumWidth(static_cast<int>(static_cast<float>(v.width()) * .7f));

	auto* container_dock = new QDockWidget;
	container_dock->setWidget(this->arap_controller);
	container_dock->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
	this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, container_dock);

	auto* mainWidget = new QWidget();
	mainWidget->setLayout(mainLayout);
	this->setCentralWidget(mainWidget);

	this->setupSignals();

	this->installEventFilter(this);
}

void ARAPMainWidget::setupSignals() {
	QObject::connect(this->showGLLog, &QPushButton::clicked, this->glDebug, &QWidget::show);
	QObject::connect(this->deform, &QPushButton::clicked, [this]() {
	  this->scene->toggleManipulatorDisplay();
	  this->viewer->toggleManipulators();
	});

	// Connect actions to the slots/functions in the program :
	QObject::connect(this->action_showVisuBox, &QAction::triggered, [this]() {
	  if (this->boxController == nullptr) {
		  this->boxController = new VisuBoxController(this->scene, this->viewer);
		  // Connect the destruction of this widget with the closing of the box controller, if opened :
		  QObject::connect(this, &QWidget::destroyed, this->boxController, &VisuBoxController::close);
		  // Connect the destruction of the box controller with its removal from the scene and from this widget :
		  QObject::connect(this->boxController, &QWidget::destroyed, this, [this]() -> void {
			this->scene->removeVisuBoxController();
			this->boxController = nullptr;
		  });
	  }
	  this->scene->showVisuBoxController(this->boxController);
	});
	QObject::connect(this->action_exitProgram, &QAction::triggered, this, &QMainWindow::close);
	QObject::connect(this->action_drawModeS, &QAction::triggered, [this]() {
	  this->scene->setDrawMode(DrawMode::Solid);
	});
	QObject::connect(this->action_drawModeV, &QAction::triggered, [this]() {
	  this->scene->setDrawMode(DrawMode::Volumetric);
	});
	QObject::connect(this->action_drawModeVB, &QAction::triggered, [this]() {
	  this->scene->setDrawMode(DrawMode::VolumetricBoxed);
	});
	QObject::connect(this->action_showHelp3D, &QAction::triggered, [this]() {
	  this->viewer->help();
	});
	QObject::connect(this->action_showHelpPlane, &QAction::triggered, [this]() {
	  this->viewer_planeX->help();
	});
	QObject::connect(this->action_showSettings, &QAction::triggered, [this]() {
	  if (this->usettings != nullptr) {
		  this->usettings->show();
		  this->usettings->raise();
	  } else {
		  this->usettings = new UserSettingsWidget;
		  this->usettings->show();
		  this->usettings->raise();
		  QObject::connect(this->usettings, &QWidget::destroyed, [this]() {
			this->usettings = nullptr;
		  });
	  }
	});

	QObject::connect(this->arap_controller, &ARAPController::requestImageLoad, this->action_addGrid, &QAction::trigger);
	QObject::connect(this->action_addGrid, &QAction::triggered, this, &ARAPMainWidget::showLoader);

	QObject::connect(this->viewer, &Viewer::hasInitializedScene, this->viewer_planeX, &PlanarViewer::canInitializeScene);
	QObject::connect(this->viewer, &Viewer::hasInitializedScene, this->viewer_planeY, &PlanarViewer::canInitializeScene);
	QObject::connect(this->viewer, &Viewer::hasInitializedScene, this->viewer_planeZ, &PlanarViewer::canInitializeScene);

	// Update min/max bounds of the control panel whenever an image is loaded !
	QObject::connect(this->arap_controller, &ARAPController::imageIsLoaded, this->viewer, &Viewer::updateSceneTextureLimits);
}

void ARAPMainWidget::showHelper() {
	return;
}

void ARAPMainWidget::showLoader() {
	if (this->loaderWidget == nullptr) {
		this->loaderWidget = new GridLoaderWidget(this->scene, this->viewer, this->controlPanel);
		QObject::connect(this->loaderWidget, &QWidget::destroyed, [this]() {
			this->loaderWidget = nullptr;
		});
		QObject::connect(this->loaderWidget, &GridLoaderWidget::newAPIGridLoaded, this->arap_controller, &ARAPController::setImagePointer);
	}
	this->loaderWidget->show();
	this->loaderWidget->raise();
}

bool ARAPMainWidget::eventFilter(QObject* obj, QEvent* e) {
	// Set our code to run after the original "Show" event :
	if (not this->widgetSizeSet && obj == this && e->type() == QEvent::Show) {
		this->widgetSizeSet = true;
		this->resize(1600, 900);
		this->setMinimumSize(1280, 720);
		// lock control panel size to the current height it has :
		this->controlPanel->setMaximumHeight(static_cast<int>(this->controlPanel->height()));
		this->arap_controller->setMaximumWidth(this->arap_controller->width());
	}
	// Return false, to handle the rest of the event normally
	return false;
}
