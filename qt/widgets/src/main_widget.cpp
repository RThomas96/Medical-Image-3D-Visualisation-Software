#include "../include/main_widget.hpp"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QMenuBar>
#include <QSizePolicy>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>

MainWidget::MainWidget() {
	this->setupWidgets();
	this->widgetSizeSet = false;
	this->loaderWidget	= nullptr;
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
	delete this->action_exitProgram;
	delete this->viewer_planeZ;
	delete this->viewer_planeY;
	delete this->viewer_planeX;
	delete this->scene;
	delete this->controlPanel;
	delete this->headerZ;
	delete this->headerY;
	delete this->headerX;

	this->showGLLog->disconnect();
}

void MainWidget::setupWidgets() {
	this->glDebug = new OpenGLDebugLog;
	this->scene	  = new Scene();
	this->scene->addOpenGLOutput(this->glDebug);

	this->statusBar = new QStatusBar;
	this->showGLLog = new QPushButton("Show GL log");
	this->statusBar->addPermanentWidget(this->showGLLog);

	this->setStatusBar(this->statusBar);
	this->scene->addStatusBar(this->statusBar);

    this->openMeshWidget = new OpenMeshWidget(this->scene, this);
    this->saveMeshWidget = new SaveMeshWidget(this->scene, this);
    this->applyCageWidget = new ApplyCageWidget(this->scene, this);

    this->cutPlane = new CutPlaneGroupBox("Display grid");

	QObject::connect(this->showGLLog, &QPushButton::clicked, this->glDebug, &QWidget::show);

	this->deformationWidget = new GridDeformationWidget(this->scene);
    this->deformationWidget->hide();
	// Actions creation :
	this->action_addGrid	   = new QAction("Open images");
	this->action_saveGrid	   = new QAction("Save acquisition");
	this->action_showPlanarViewers   = new QAction("Show planar viewer");
	this->action_exitProgram   = new QAction("Exit program");
	this->action_loadMesh	   = new QAction("Load mesh (OFF)");
	this->action_saveMesh	   = new QAction("Save mesh (OFF)");
	this->action_applyCage	   = new QAction("Apply cage on another cage");
	this->action_openDevPannel = new QAction("Open dev pannel");

	this->action_addGrid->setShortcut(QKeySequence::Open);

	this->fileMenu = this->menuBar()->addMenu("&File");
	this->fileMenu->addAction(this->action_addGrid);
	this->fileMenu->addAction(this->action_loadMesh);
	this->fileMenu->addAction(this->action_saveGrid);
	this->fileMenu->addAction(this->action_saveMesh);
	this->fileMenu->addAction(this->action_applyCage);
	this->fileMenu->addAction(this->action_exitProgram);

    /***/
    this->combo_mesh = new QComboBox();
	QObject::connect(scene, &Scene::meshAdded, this, &MainWidget::addNewMesh);

    QTabBar * tabBar = new QTabBar();
    tabBar->addTab("Tools");
    tabBar->addTab("Files");

    this->toolbar = this->addToolBar("Tools");
    this->toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    //this->toolbar->addWidget(tabBar);

	this->tool_open = new QAction("Open");
    this->toolbar->addAction(this->tool_open);
    this->tool_open->setIcon(QIcon("../resources/arap.svg"));
    this->tool_open->setIconVisibleInMenu(true);

	this->tool_save = new QAction("Save");
    this->toolbar->addAction(this->tool_save);
    this->tool_save->setIcon(QIcon("../resources/arap.svg"));
    this->tool_save->setIconVisibleInMenu(true);

    this->toolbar->addSeparator();

	this->tool_position = new QAction("Move");
    this->toolbar->addAction(this->tool_position);
    this->tool_position->setIcon(QIcon("../resources/arap.svg"));
    this->tool_position->setIconVisibleInMenu(true);

	this->tool_direct = new QAction("Direct");
    this->toolbar->addAction(this->tool_direct);
    this->tool_direct->setIcon(QIcon("../resources/arap.svg"));
    this->tool_direct->setIconVisibleInMenu(true);

	this->tool_ARAP = new QAction("ARAP");
    this->toolbar->addAction(this->tool_ARAP);
    this->tool_ARAP->setIcon(QIcon("../resources/arap.svg"));
    this->tool_ARAP->setIconVisibleInMenu(true);

	this->tool_registration = new QAction("Register");
    this->toolbar->addAction(this->tool_registration);
    this->tool_registration->setIcon(QIcon("../resources/arap.svg"));
    this->tool_registration->setIconVisibleInMenu(true);

    /***/

	this->viewMenu = this->menuBar()->addMenu("&View");
	this->viewMenu->addAction(this->action_showPlanarViewers);

	this->otherMenu = this->menuBar()->addMenu("&Other");
	this->otherMenu->addAction(this->action_openDevPannel);

	QObject::connect(this->action_addGrid, &QAction::triggered, [this]() {
		if (this->loaderWidget == nullptr) {
			this->loaderWidget = new GridLoaderWidget(this->scene, this->viewer, this->controlPanel);
			QObject::connect(this->loaderWidget, &QWidget::destroyed, [this]() {
				this->loaderWidget = nullptr;
			});
		}
		this->loaderWidget->show();
		this->loaderWidget->raise();
	});
	QObject::connect(this->action_saveGrid, &QAction::triggered, [this]() {
		this->scene->launchSaveDialog();
	});
	QObject::connect(this->action_showPlanarViewers, &QAction::triggered, [this]() {
            if(this->xViewerCapsule->isHidden()) {
                this->xViewerCapsule->show();
                this->yViewerCapsule->show();
                this->zViewerCapsule->show();
            } else {
                this->xViewerCapsule->hide();
                this->yViewerCapsule->hide();
                this->zViewerCapsule->hide();
            }
	});
	QObject::connect(this->action_exitProgram, &QAction::triggered, this, &QMainWindow::close);
	QObject::connect(this->action_loadMesh, &QAction::triggered, [this]() {
		//this->scene->loadMesh();
        QStringList potentialCages;
        std::vector<std::string> allNonTetrahedralMeshes = this->scene->getAllBaseMeshesName();
        for(int i = 0; i < allNonTetrahedralMeshes.size(); ++i)
            potentialCages += QString(allNonTetrahedralMeshes[i].c_str());
        this->openMeshWidget->setPotentialCages(potentialCages);
        this->openMeshWidget->show();
	});
	QObject::connect(this->action_saveMesh, &QAction::triggered, [this]() {
        QStringList potentialMeshes;
        std::vector<std::string> allNonTetrahedralMeshes = this->scene->getAllNonTetrahedralMeshesName();
        for(int i = 0; i < allNonTetrahedralMeshes.size(); ++i)
            potentialMeshes += QString(allNonTetrahedralMeshes[i].c_str());
        this->saveMeshWidget->setPotentialMeshToSave(potentialMeshes);
        this->saveMeshWidget->show();
	});
	QObject::connect(this->action_applyCage, &QAction::triggered, [this]() {
        QStringList potentialCages;
        std::vector<std::string> allCages = this->scene->getAllCagesName();
        for(int i = 0; i < allCages.size(); ++i)
            potentialCages += QString(allCages[i].c_str());
        this->applyCageWidget->setPotentialCageToApply(potentialCages);
        this->applyCageWidget->show();
	});
	QObject::connect(this->action_addGrid, &QAction::triggered, [this]() {
		if (this->loaderWidget == nullptr) {
			this->loaderWidget = new GridLoaderWidget(this->scene, this->viewer, this->controlPanel);
			QObject::connect(this->loaderWidget, &QWidget::destroyed, [this]() {
				this->loaderWidget = nullptr;
			});
		}
		this->loaderWidget->show();
		this->loaderWidget->raise();
	});
	QObject::connect(this->action_openDevPannel, &QAction::triggered, [this]() {
            if(!this->deformationWidget->isVisible()) {
                this->deformationWidget->show();
            } else {
                this->deformationWidget->hide();
            }
	});
	QObject::connect(this->tool_ARAP, &QAction::triggered, [this]() {
            this->scene->updateSceneCenter();
            this->scene->changeCurrentTool(UITool::MeshManipulatorType::ARAP);
            this->tool_pannel->changeCurrentTool(UITool::MeshManipulatorType::ARAP);
	});
	QObject::connect(this->tool_direct, &QAction::triggered, [this]() {
            this->scene->updateSceneCenter();
            this->scene->changeCurrentTool(UITool::MeshManipulatorType::DIRECT);
            this->tool_pannel->changeCurrentTool(UITool::MeshManipulatorType::DIRECT);
	});
	QObject::connect(this->tool_position, &QAction::triggered, [this]() {
            this->scene->updateSceneCenter();
            this->scene->changeCurrentTool(UITool::MeshManipulatorType::POSITION);
            this->tool_pannel->changeCurrentTool(UITool::MeshManipulatorType::POSITION);
	});
	QObject::connect(this->tool_registration, &QAction::triggered, [this]() {
            this->scene->updateSceneCenter();
            this->scene->changeCurrentTool(UITool::MeshManipulatorType::FIXED_REGISTRATION);
            this->tool_pannel->changeCurrentTool(UITool::MeshManipulatorType::FIXED_REGISTRATION);
	});
    QObject::connect(this->combo_mesh, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
            this->scene->changeActiveMesh(std::string((this->combo_mesh->itemText(this->combo_mesh->currentIndex())).toStdString()));
            this->tool_pannel->changeCurrentTool(UITool::MeshManipulatorType::DIRECT);
    });

    /***/
    // Plane control
    /***/
    QObject::connect(this->cutPlane, &CutPlaneGroupBox::clicked, this->scene, &Scene::slotToggleDisplayGrid);

    QObject::connect(this->cutPlane, &CutPlaneGroupBox::xSliderValueChanged, this->scene, &Scene::slotSetPlaneDisplacementX);
    QObject::connect(this->cutPlane, &CutPlaneGroupBox::ySliderValueChanged, this->scene, &Scene::slotSetPlaneDisplacementY);
    QObject::connect(this->cutPlane, &CutPlaneGroupBox::zSliderValueChanged, this->scene, &Scene::slotSetPlaneDisplacementZ);

    QObject::connect(this->cutPlane, &CutPlaneGroupBox::clickedInvertXPushButton, this->scene, &Scene::slotTogglePlaneDirectionX);
    QObject::connect(this->cutPlane, &CutPlaneGroupBox::clickedInvertYPushButton, this->scene, &Scene::slotTogglePlaneDirectionY);
    QObject::connect(this->cutPlane, &CutPlaneGroupBox::clickedInvertZPushButton, this->scene, &Scene::slotTogglePlaneDirectionZ);

    QObject::connect(this->cutPlane, &CutPlaneGroupBox::clickedDisplayXCut, this->scene, &Scene::slotTogglePlaneX);
    QObject::connect(this->cutPlane, &CutPlaneGroupBox::clickedDisplayYCut, this->scene, &Scene::slotTogglePlaneY);
    QObject::connect(this->cutPlane, &CutPlaneGroupBox::clickedDisplayZCut, this->scene, &Scene::slotTogglePlaneZ);

    /***/

	// Viewer(s) creation along with control panel :
	this->viewer		= new Viewer(this->scene, this->statusBar, nullptr);
	this->viewer_planeX = new PlanarViewer(this->scene, planes::x, this->statusBar, planeHeading::North, nullptr);
	this->viewer_planeY = new PlanarViewer(this->scene, planes::y, this->statusBar, planeHeading::North, nullptr);
	this->viewer_planeZ = new PlanarViewer(this->scene, planes::z, this->statusBar, planeHeading::North, nullptr);
	this->controlPanel	= new ControlPanel(this->scene, this->viewer, nullptr);
	this->scene->setControlPanel(this->controlPanel);

    this->info_pannel = new InfoPannel("Infos");
    this->tool_pannel = new ToolPannel("Tool pannel", this->scene);

	this->viewer->addStatusBar(this->statusBar);
	this->viewer_planeX->addParentStatusBar(this->statusBar);
	this->viewer_planeY->addParentStatusBar(this->statusBar);
	this->viewer_planeZ->addParentStatusBar(this->statusBar);

	// Sliders for each plane (also sets range and values) :
	this->headerX  = new ViewerHeader("X Plane");
	this->headerX->connectToViewer(this->viewer_planeX);
	this->headerY = new ViewerHeader("Y Plane");
	this->headerY->connectToViewer(this->viewer_planeY);
	this->headerZ = new ViewerHeader("Z Plane");
	this->headerZ->connectToViewer(this->viewer_planeZ);

	// Splitters : one main (hor.) and two secondaries (vert.) :
	QSplitter* mainSplit   = new QSplitter(Qt::Horizontal);
	QSplitter* splitAbove  = new QSplitter(Qt::Vertical);
	QSplitter* splitAbove1 = new QSplitter(Qt::Vertical);

	// Layouts to place a viewer and a header in the same place :
	QVBoxLayout* vP3 = new QVBoxLayout();
	vP3->setSpacing(0);	   // header above 3D view
	QVBoxLayout* vPX = new QVBoxLayout();
	vPX->setSpacing(0);	   // header above plane X
	QVBoxLayout* vPY = new QVBoxLayout();
	vPY->setSpacing(0);	   // header above plane Y
	QVBoxLayout* vPZ = new QVBoxLayout();
	vPZ->setSpacing(0);	   // header above plane Z

	// Those will encapsulate the layouts above :
	this->_ViewerCapsule = new QWidget();
	this->xViewerCapsule = new QWidget();
	this->yViewerCapsule = new QWidget();
	this->zViewerCapsule = new QWidget();

	this->headerY->setFixedHeight(this->headerY->sizeHint().height());
	this->headerZ->setFixedHeight(this->headerZ->sizeHint().height());

	// Add widgets in layouts to compose the plane viewers :
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
    mainSplit->setContentsMargins(0, 0, 0, 0);

    this->viewerFrame = new QFrame();
    this->viewerFrame->setFrameRect(QRect(QRect(0, 0, 0, 0)));
    this->viewerFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    this->viewerFrame->setLineWidth(2);

	QHBoxLayout* viewerLayout = new QHBoxLayout(this->viewerFrame);

	QVBoxLayout* sidePannelLayout = new QVBoxLayout();
    sidePannelLayout->setAlignment(Qt::AlignBottom);
	QVBoxLayout* toolPannelLayout = new QVBoxLayout();
    toolPannelLayout->setAlignment(Qt::AlignTop);
    this->info_pannel->setFixedSize(200, 85);
    this->tool_pannel->setFixedWidth(200);
    toolPannelLayout->addWidget(this->info_pannel, Qt::AlignTop);
    toolPannelLayout->addWidget(this->tool_pannel, Qt::AlignTop);

    this->cutPlane->setFixedSize(200, 200);
    sidePannelLayout->addLayout(toolPannelLayout);
	sidePannelLayout->addWidget(this->cutPlane);

    viewerLayout->addLayout(sidePannelLayout);
	viewerLayout->addWidget(mainSplit, 4);
	viewerLayout->addWidget(this->deformationWidget);

    //this->viewerGroup->setLayout(viewerLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(this->combo_mesh, 2);
	mainLayout->addWidget(this->viewerFrame, 3);
	//mainLayout->addWidget(this->viewerFrame, 3);
    mainLayout->addWidget(this->controlPanel, 0, Qt::AlignHCenter);

	xViewerCapsule->hide();
	yViewerCapsule->hide();
	zViewerCapsule->hide();

	QSize v = viewerLayout->sizeHint();
	this->controlPanel->setMinimumWidth(static_cast<int>(static_cast<float>(v.width()) * .7f));

	QWidget* mainWidget = new QWidget();
	mainWidget->setLayout(mainLayout);
	this->setCentralWidget(mainWidget);

	this->installEventFilter(this);
}

bool MainWidget::eventFilter(QObject* obj, QEvent* e) {
	// Set our code to run after the original "Show" event :
	if (this->widgetSizeSet == false && obj == this && e->type() == QEvent::Show) {
		this->widgetSizeSet = true;
		this->resize(1600, 900);
		this->setMinimumSize(1280, 720);
		// lock control panel size to the current size it has :
		QSize centerSize = this->size();
		this->controlPanel->setMinimumWidth(static_cast<int>(static_cast<float>(centerSize.width()) * .99f));
		this->controlPanel->setMaximumHeight(static_cast<int>(this->controlPanel->height()));
	}
	// Return false, to handle the rest of the event normally
	return false;
}
