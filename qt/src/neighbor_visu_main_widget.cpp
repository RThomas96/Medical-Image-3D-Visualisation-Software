#include "../include/neighbor_visu_main_widget.hpp"

#include "../include/color_control.hpp"

#include <QLabel>
#include <QEvent>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QToolBar>
#include <QMenuBar>
#include <QList>
#include <QStatusBar>

MainWidget::MainWidget() {
	this->strayObj.clear();
	this->setupWidgets();
	this->widgetSizeSet = false;
	this->usettings = nullptr;
	this->loaderWidget = nullptr;
	this->colorControl = nullptr;
	// Query a user settings instance to initialize it :
	UserSettings set = UserSettings::getInstance();
}

MainWidget::~MainWidget() {
	this->removeEventFilter(this);
	this->headerZ->unregisterPlaneViewer();
	this->headerY->unregisterPlaneViewer();
	this->headerX->unregisterPlaneViewer();

	this->action_addGrid->disconnect();
	this->action_saveGrid->disconnect();
	this->action_exitProgram->disconnect();

	delete this->action_addGrid;
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

void MainWidget::addColorControl() {
	if (this->colorControl != nullptr) {
		this->colorControl->show();
		return;
	}

	this->colorControl = new ColorBoundControl(this->scene, this->controlPanel, this, nullptr);
	this->colorControl->show();
}

void MainWidget::removeColorControl() {
	this->colorControl = nullptr;
	return;
}

void MainWidget::setupWidgets() {
	this->glDebug = new OpenGLDebugLog;
	this->scene = new Scene();
	this->scene->addOpenGLOutput(this->glDebug);

	this->statusBar = new QStatusBar;
	this->showGLLog = new QPushButton("Show GL log");
	this->statusBar->addPermanentWidget(this->showGLLog);
	this->setStatusBar(this->statusBar);
	this->scene->addStatusBar(this->statusBar);
	QObject::connect(this->showGLLog, &QPushButton::clicked, this->glDebug, &QWidget::show);

	// Actions creation :
	this->action_addGrid = new QAction("Open images");
	this->action_saveGrid = new QAction("Save acquisition");
	this->action_showVisuBox = new QAction("Show visu box controller");
	this->action_showColorControl = new QAction("Show color controller");
	this->action_exitProgram = new QAction("Exit program");
	this->action_drawModeS = new QAction("Set draw mode to Solid");
	this->action_drawModeV = new QAction("Set draw mode to Volumetric");
	this->action_drawModeVB= new QAction("Set draw mode to Volumetric(Boxed)");
	this->action_showHelp3D = new QAction("3D Viewer Help Page");
	this->action_showHelpPlane = new QAction("Planar Viewer Help Page");
	this->action_showSettings = new QAction("Settings");

	this->action_addGrid->setShortcut(QKeySequence::Open);

	// Menu creation :
	// File menu :
	this->fileMenu = this->menuBar()->addMenu("&File");
	this->fileMenu->addAction(this->action_addGrid);
	this->fileMenu->addAction(this->action_saveGrid);
	this->fileMenu->addAction(this->action_showSettings);
	this->fileMenu->addAction(this->action_exitProgram);
	// view menu :
	this->viewMenu = this->menuBar()->addMenu("&View");
	this->viewMenu->addAction(this->action_drawModeS);
	this->viewMenu->addAction(this->action_drawModeV);
	this->viewMenu->addAction(this->action_drawModeVB);
	this->viewMenu->addAction(this->action_showVisuBox);
	this->viewMenu->addAction(this->action_showColorControl);
	// help menu :
	this->helpMenu = this->menuBar()->addMenu("&Help");
	this->helpMenu->addAction(this->action_showHelp3D);
	this->helpMenu->addAction(this->action_showHelpPlane);

	// Connect actions to the slots/functions in the program :
	QObject::connect(this->action_addGrid, &QAction::triggered, [this](){
		if (this->loaderWidget == nullptr) {
			this->loaderWidget = new GridLoaderWidget(this->scene, this->viewer, this->controlPanel);
			QObject::connect(this->loaderWidget, &QWidget::destroyed, [this]() {
				this->loaderWidget = nullptr;
			});
		}
		this->loaderWidget->show();
	});
	QObject::connect(this->action_saveGrid, &QAction::triggered, [this](){this->scene->launchSaveDialog();});
	QObject::connect(this->action_showVisuBox, &QAction::triggered, [this](){this->scene->showVisuBoxController();});
	QObject::connect(this->action_showColorControl, &QAction::triggered, this, &MainWidget::addColorControl);
	QObject::connect(this->action_exitProgram, &QAction::triggered, this, &QMainWindow::close);
	QObject::connect(this->action_drawModeS, &QAction::triggered, [this](){this->scene->setDrawMode(DrawMode::Solid);});
	QObject::connect(this->action_drawModeV, &QAction::triggered, [this](){this->scene->setDrawMode(DrawMode::Volumetric);});
	QObject::connect(this->action_drawModeVB, &QAction::triggered, [this](){this->scene->setDrawMode(DrawMode::VolumetricBoxed);});
	QObject::connect(this->action_showHelp3D, &QAction::triggered, [this](){this->viewer->help();});
	QObject::connect(this->action_showHelpPlane, &QAction::triggered, [this](){this->viewer_planeX->help();});
	QObject::connect(this->action_showSettings, &QAction::triggered, [this](){
		if (this->usettings != nullptr) { this->usettings->show(); }
		else {
			this->usettings = new UserSettingsWidget;
			this->usettings->show();
			QObject::connect(this->usettings, &QWidget::destroyed, [this](){
				this->usettings = nullptr;
			});
		}
	});

	// Viewer(s) creation along with control panel :
	this->viewer = new Viewer(this->scene, nullptr);
	this->viewer_planeX = new PlanarViewer(this->scene, planes::x, planeHeading::North, nullptr);
	this->viewer_planeY = new PlanarViewer(this->scene, planes::y, planeHeading::North, nullptr);
	this->viewer_planeZ = new PlanarViewer(this->scene, planes::z, planeHeading::North, nullptr);
	this->controlPanel = new ControlPanel(this->scene, this->viewer, nullptr);
	this->scene->setControlPanel(this->controlPanel);

	// Sliders for each plane (also sets range and values) :
	this->header3d = new ViewerHeader3D(this->viewer, this->scene, nullptr);
	this->headerX = new ViewerHeader("X Plane"); this->headerX->connectToViewer(this->viewer_planeX);
	this->headerY = new ViewerHeader("Y Plane"); this->headerY->connectToViewer(this->viewer_planeY);
	this->headerZ = new ViewerHeader("Z Plane"); this->headerZ->connectToViewer(this->viewer_planeZ);

	// Splitters : one main (hor.) and two secondaries (vert.) :
	QSplitter* mainSplit = new QSplitter(Qt::Horizontal);
	QSplitter* splitAbove = new QSplitter(Qt::Vertical);
	QSplitter* splitAbove1 = new QSplitter(Qt::Vertical);

	// Layouts to place a viewer and a header in the same place :
	QVBoxLayout* vP3 = new QVBoxLayout(); vP3->setSpacing(0);	// header above 3D view
	QVBoxLayout* vPX = new QVBoxLayout(); vPX->setSpacing(0);	// header above plane X
	QVBoxLayout* vPY = new QVBoxLayout(); vPY->setSpacing(0);	// header above plane Y
	QVBoxLayout* vPZ = new QVBoxLayout(); vPZ->setSpacing(0);	// header above plane Z

	// Those will encapsulate the layouts above :
	QWidget* _ViewerCapsule = new QWidget();
	QWidget* xViewerCapsule = new QWidget();
	QWidget* yViewerCapsule = new QWidget();
	QWidget* zViewerCapsule = new QWidget();

	this->header3d->setFixedHeight(this->header3d->sizeHint().height());
	this->headerX->setFixedHeight(this->headerX->sizeHint().height());
	this->headerY->setFixedHeight(this->headerY->sizeHint().height());
	this->headerZ->setFixedHeight(this->headerZ->sizeHint().height());

	// Add widgets in layouts to compose the plane viewers :
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

	QHBoxLayout* viewerLayout = new QHBoxLayout();
	viewerLayout->addWidget(mainSplit);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(viewerLayout);
	mainLayout->addWidget(this->controlPanel, 0, Qt::AlignHCenter);

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

	QWidget* mainWidget = new QWidget();
	mainWidget->setLayout(mainLayout);
	this->setCentralWidget(mainWidget);

	this->installEventFilter(this);
}

bool MainWidget::eventFilter(QObject* obj, QEvent* e) {
	// Set our code to run after the original event handling :
	if (this->widgetSizeSet == false && obj == this && e->type() == QEvent::Show) {
		this->widgetSizeSet = true;
		this->resize(1024, 576);
		this->setMinimumSize(768, 432);
		// lock control panel size to the current size it has :
		QSize centerSize = this->size();
		this->controlPanel->setMinimumWidth(static_cast<int>(static_cast<float>(centerSize.width()) * .9f));
	}
	// Return false, to handle the rest of the event normally
	return false;
}
