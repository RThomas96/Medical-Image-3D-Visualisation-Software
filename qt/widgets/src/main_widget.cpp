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

	this->action_addGrid->disconnect();
	this->action_saveGrid->disconnect();
	this->action_exitProgram->disconnect();


	delete this->action_addGrid;
	delete this->action_saveGrid;
	delete this->action_exitProgram;
	delete this->scene;
	delete this->controlPanel;
}

void MainWidget::setupWidgets() {
	this->scene	  = new Scene();

    this->setupActions();
    this->setupForms();
	this->statusBar = new QStatusBar;

	this->setStatusBar(this->statusBar);
	this->scene->addStatusBar(this->statusBar);

    this->openMeshWidget = new OpenMeshWidget(this->scene, this);
    this->saveMeshWidget = new SaveMeshWidget(this->scene, this);
    this->applyCageWidget = new ApplyCageWidget(this->scene, this);

    this->display_pannel = new DisplayPannel("Display", *this->actionManager);
    this->cutPlane_pannel = new CutPlaneGroupBox("Cutting planes");
    this->cutPlane_pannel->setCheckable(false);

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

    this->toolbar->addAction(this->actionManager->getAction("Open"));
    this->toolbar->addAction(this->actionManager->getAction("QuickSaveCage"));
    this->toolbar->addAction(this->actionManager->getAction("QuickSaveAsCage"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("ToggleNoneTool"));
    this->toolbar->addAction(this->actionManager->getAction("ToggleMoveTool"));
    this->toolbar->addAction(this->actionManager->getAction("ToggleDirectTool"));
    this->toolbar->addAction(this->actionManager->getAction("ToggleARAPTool"));
    this->toolbar->addAction(this->actionManager->getAction("ToggleRegisterTool"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("Reset"));
    this->toolbar->addAction(this->actionManager->getAction("Undo"));
    this->toolbar->addAction(this->actionManager->getAction("Redo"));

    this->toolbar->addSeparator();

    //this->toolbar->addAction(this->actionManager->getAction("SaveImage"));
    this->toolbar->addAction(this->actionManager->getAction("Layout1View"));
    this->toolbar->addAction(this->actionManager->getAction("Layout2View"));
    this->toolbar->addAction(this->actionManager->getAction("Layout4View"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("Transform"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("Clear"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("OpenAtlas"));
    this->toolbar->addAction(this->actionManager->getAction("OpenIRM"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("Sorting"));

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
	//QObject::connect(this->tool_none, &QAction::triggered, [this]() {
    //        this->scene->updateSceneCenter();
    //        this->scene->changeCurrentTool(UITool::MeshManipulatorType::NONE);
    //        this->tool_pannel->changeCurrentTool(UITool::MeshManipulatorType::NONE);
	//});
    QObject::connect(this->combo_mesh, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
            this->scene->changeActiveMesh(std::string((this->combo_mesh->itemText(this->combo_mesh->currentIndex())).toStdString()));
            this->changeActiveMesh();
    });

    /***/
    // Plane control
    /***/
    //QObject::connect(this->cutPlaneDisplay, &CutPlaneGroupBox::clicked, this->scene, &Scene::slotToggleDisplayGrid);

    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::xSliderValueChanged, this->scene, &Scene::slotSetPlaneDisplacementX);
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::ySliderValueChanged, this->scene, &Scene::slotSetPlaneDisplacementY);
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::zSliderValueChanged, this->scene, &Scene::slotSetPlaneDisplacementZ);

    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::clickedInvertXPushButton, this->scene, &Scene::slotTogglePlaneDirectionX);
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::clickedInvertYPushButton, this->scene, &Scene::slotTogglePlaneDirectionY);
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::clickedInvertZPushButton, this->scene, &Scene::slotTogglePlaneDirectionZ);

    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::clickedDisplayXCut, this->scene, &Scene::slotTogglePlaneX);
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::clickedDisplayYCut, this->scene, &Scene::slotTogglePlaneY);
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::clickedDisplayZCut, this->scene, &Scene::slotTogglePlaneZ);

    QObject::connect(this->scene, &Scene::planeControlWidgetNeedUpdate, [=](const glm::vec3& values) {
        this->cutPlane_pannel->setValues(values.x, values.y, values.z);
    });

    /***/

	// Viewer(s) creation along with control panel :
	this->viewer		= new Viewer(this->scene, this->statusBar, nullptr);
	this->controlPanel	= new ControlPanel(this->scene, this->viewer, nullptr);
	this->scene->setControlPanel(this->controlPanel);

    this->info_pannel = new InfoPannel("Infos", this->scene);
    this->tool_pannel = new ToolPannel("Tool", *this->actionManager);
    this->quickSaveCage = new QuickSaveCage(this->scene);

	this->viewer->addStatusBar(this->statusBar);

    /***/

    hSplit = new QSplitter(Qt::Horizontal);
    vSplit1 = new QSplitter(Qt::Vertical);
    vSplit2 = new QSplitter(Qt::Vertical);

    hSplit->addWidget(vSplit1);
    hSplit->addWidget(vSplit2);

    vSplit1->addWidget(this->viewer);
    hSplit->setContentsMargins(0, 0, 0, 0);
    vSplit1->setContentsMargins(0, 0, 0, 0);
    vSplit2->setContentsMargins(0, 0, 0, 0);

    /***/

    this->viewerFrame = new QFrame();
    this->viewerFrame->setFrameRect(QRect(QRect(0, 0, 0, 0)));
    this->viewerFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    this->viewerFrame->setLineWidth(2);

    /***/

	QHBoxLayout* viewerLayout = new QHBoxLayout(this->viewerFrame);

	QVBoxLayout* sidePannelLayout = new QVBoxLayout();
    sidePannelLayout->setAlignment(Qt::AlignBottom);

    sidePannelLayout->addWidget(this->info_pannel);
    //this->tool_pannel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sidePannelLayout->addWidget(this->tool_pannel);
    sidePannelLayout->addWidget(this->display_pannel);
	sidePannelLayout->addWidget(this->cutPlane_pannel);

    viewerLayout->addLayout(sidePannelLayout);
    viewerLayout->addWidget(hSplit, 4);
	viewerLayout->addWidget(this->deformationWidget);

    /***/

    QFrame * lowFrame = new QFrame();
    lowFrame->setFrameRect(QRect(QRect(0, 0, 0, 0)));
    lowFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    lowFrame->setLineWidth(2);

    QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(this->combo_mesh, 2);
	mainLayout->addWidget(this->viewerFrame, 3);
	//mainLayout->addWidget(this->viewerFrame, 3);

    QHBoxLayout* lowLayout = new QHBoxLayout(lowFrame);
    lowLayout->addWidget(this->planarViewer, 1);
    lowLayout->addWidget(this->controlPanel, 3);
    this->controlPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addWidget(lowFrame);

    //QSize v = viewerLayout->sizeHint();
    //this->controlPanel->setMinimumWidth(static_cast<int>(static_cast<float>(v.width()) * .7f));

	QWidget* mainWidget = new QWidget();
	mainWidget->setLayout(mainLayout);
	this->setCentralWidget(mainWidget);

    this->updateForms();
    this->planarViewer->initialize(this->scene);

    this->vSplit2->addWidget(this->planarViewer->viewers["View_1"]->viewer2D);
    this->vSplit2->addWidget(this->planarViewer->viewers["View_2"]->viewer2D);
    this->vSplit1->addWidget(this->planarViewer->viewers["View_3"]->viewer2D);

    hSplit->setSizes(QList<int>({INT_MAX, INT_MAX}));
    vSplit1->setSizes(QList<int>({INT_MAX, INT_MAX}));
    vSplit2->setSizes(QList<int>({INT_MAX, INT_MAX}));

    this->planarViewer->viewers["View_1"]->viewer2D->hide();
    this->planarViewer->viewers["View_2"]->viewer2D->hide();
    this->planarViewer->viewers["View_3"]->viewer2D->hide();

    this->planarViewer->show();

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
        //this->controlPanel->setMinimumWidth(static_cast<int>(static_cast<float>(centerSize.width()) * .66f));
        //this->controlPanel->setMaximumHeight(static_cast<int>(this->controlPanel->height()));
	}
	// Return false, to handle the rest of the event normally
	return false;
}

void MainWidget::setupActions() {
    this->actionManager = new QActionManager();

    // Display
    this->actionManager->createQActionToggleButton("ToggleDisplayMesh", "Mesh", "M", "Display/Show mesh", "visible", "hidden");
    QObject::connect(this->actionManager->getAction("ToggleDisplayMesh"), &QAction::triggered, [this](){this->scene->toggleDisplayMesh();});

    this->actionManager->createQActionToggleButton("ToggleDisplayGrid", "Grids", "G", "Display/Show grid", "visible", "hidden");
    QObject::connect(this->actionManager->getAction("ToggleDisplayGrid"), &QAction::triggered, [this](){this->scene->slotToggleDisplayGrid();});

    this->actionManager->createQActionToggledButton("ToggleDisplayMultiView", "MView", "M", "Display/Show multi grid display", "visible", "hidden");
    QObject::connect(this->actionManager->getAction("ToggleDisplayMultiView"), &QAction::triggered, [this](){this->scene->setMultiGridRendering(!this->actionManager->getAction("ToggleDisplayMultiView")->isChecked()); this->controlPanel->updateRGBMode();});

    this->actionManager->createQActionToggledButton("ToggleDisplayWireframe", "TetM", "W", "Display/Show the tethraedral mesh wireframe", "visible", "hidden");
    QObject::connect(this->actionManager->getAction("ToggleDisplayWireframe"), &QAction::triggered, [this](){this->scene->toggleWireframe();});

    // Tools
    this->actionManager->createQActionToggleButton("ToggleNoneTool", "None", "Ctrl+N", "Deactivate all tools", "none");
    QObject::connect(this->actionManager->getAction("ToggleNoneTool"), &QAction::triggered, [this](){this->scene->updateTools(UITool::MeshManipulatorType::NONE);});
    QObject::connect(this->actionManager->getAction("ToggleNoneTool"), &QAction::triggered, [this](){this->changeCurrentTool(UITool::MeshManipulatorType::NONE);});

    this->actionManager->createQActionToggleButton("ToggleMoveTool", "Move", "Ctrl+M", "Activate move tool", "move");
    QObject::connect(this->actionManager->getAction("ToggleMoveTool"), &QAction::triggered, [this](){this->scene->updateTools(UITool::MeshManipulatorType::POSITION);});
    QObject::connect(this->actionManager->getAction("ToggleMoveTool"), &QAction::triggered, [this](){this->changeCurrentTool(UITool::MeshManipulatorType::POSITION);});
    QObject::connect(this, &MainWidget::gridSelected, [this](){this->actionManager->getAction("ToggleMoveTool")->setDisabled(true);});
    QObject::connect(this, &MainWidget::meshSelected, [this](){this->actionManager->getAction("ToggleMoveTool")->setDisabled(false);});

    this->actionManager->createQActionToggleButton("ToggleDirectTool", "Direct", "Ctrl+D", "Activate direct tool", "direct");
    QObject::connect(this->actionManager->getAction("ToggleDirectTool"), &QAction::triggered, [this](){this->scene->updateTools(UITool::MeshManipulatorType::DIRECT);});
    QObject::connect(this->actionManager->getAction("ToggleDirectTool"), &QAction::triggered, [this](){this->changeCurrentTool(UITool::MeshManipulatorType::DIRECT);});
    QObject::connect(this, &MainWidget::gridSelected, [this](){this->actionManager->getAction("ToggleDirectTool")->setDisabled(true);});
    QObject::connect(this, &MainWidget::meshSelected, [this](){this->actionManager->getAction("ToggleDirectTool")->setDisabled(false);});

    this->actionManager->createQActionToggleButton("ToggleARAPTool", "ARAP", "Ctrl+A", "Activate ARAP tool", "araps");
    QObject::connect(this->actionManager->getAction("ToggleARAPTool"), &QAction::triggered, [this](){this->scene->updateTools(UITool::MeshManipulatorType::ARAP);});
    QObject::connect(this->actionManager->getAction("ToggleARAPTool"), &QAction::triggered, [this](){this->changeCurrentTool(UITool::MeshManipulatorType::ARAP);});
    QObject::connect(this, &MainWidget::gridSelected, [this](){this->actionManager->getAction("ToggleARAPTool")->setDisabled(true);});
    QObject::connect(this, &MainWidget::meshSelected, [this](){this->actionManager->getAction("ToggleARAPTool")->setDisabled(false);});

    this->actionManager->createQActionToggleButton("ToggleRegisterTool", "Register", "Ctrl+R", "Activate Register tool", "register");
    QObject::connect(this->actionManager->getAction("ToggleRegisterTool"), &QAction::triggered, [this](){this->scene->updateTools(UITool::MeshManipulatorType::FIXED_REGISTRATION);});
    QObject::connect(this->actionManager->getAction("ToggleRegisterTool"), &QAction::triggered, [this](){this->changeCurrentTool(UITool::MeshManipulatorType::FIXED_REGISTRATION);});
    QObject::connect(this, &MainWidget::gridSelected, [this](){this->actionManager->getAction("ToggleRegisterTool")->setDisabled(true);});
    QObject::connect(this, &MainWidget::meshSelected, [this](){this->actionManager->getAction("ToggleRegisterTool")->setDisabled(false);});

    this->actionManager->createQExclusiveActionGroup("ToogleTools", {"ToggleNoneTool", "ToggleMoveTool", "ToggleDirectTool", "ToggleARAPTool", "ToggleRegisterTool"});

    // Move
    this->actionManager->createQActionToggleButton("MoveTool_toggleEvenMode", "Even", "E", "Toggle the even mode to scale evenly in 3 dimensions", "even");
    QObject::connect(this->actionManager->getAction("MoveTool_toggleEvenMode"), &QAction::triggered, [this](){this->scene->moveTool_toggleEvenMode();});

    this->actionManager->createQActionToggledButton("MoveTool_toggleMoveCage", "Link", "", "If this mode is active, and the manipulated mesh is a cage, the cage and the associated grid movements are linked", "link");
    QObject::connect(this->actionManager->getAction("MoveTool_toggleMoveCage"), &QAction::triggered, [this](){this->scene->toggleBindMeshToCageMove();});

    this->actionManager->createQActionButton("MoveTool_reset", "Reset", "R", "Reset the manipulator size", "reset");
    QObject::connect(this->actionManager->getAction("MoveTool_reset"), &QAction::triggered, [this](){
        QAction * evenAction = this->actionManager->getAction("MoveTool_toggleEvenMode");
        QAction * linkAction = this->actionManager->getAction("MoveTool_toggleMoveCage");
        bool evenModeWasActive = evenAction->isChecked();
        bool linkModeWasActive = linkAction->isChecked();
        this->scene->changeActiveMesh(std::string((this->combo_mesh->itemText(this->combo_mesh->currentIndex())).toStdString()));
        evenAction->blockSignals(true);
        evenAction->setChecked(false);
        evenAction->blockSignals(false);
        linkAction->blockSignals(true);
        linkAction->setChecked(true);
        linkAction->blockSignals(false);
        if(evenModeWasActive)
            evenAction->trigger();
        if(!linkModeWasActive)
            linkAction->trigger();
    });

    this->actionManager->createQActionGroup("MoveTool", {"MoveTool_toggleEvenMode", "MoveTool_toggleMoveCage", "MoveTool_reset"});
            
    // ARAP
    this->actionManager->createQActionToggledButton("ARAPTool_moveMode", "Move", "O", "Toggle move mode of ARAP tool, you can now move points with the selection", "move");
    QObject::connect(this->actionManager->getAction("ARAPTool_moveMode"), &QAction::triggered, [this](){this->scene->toggleARAPManipulatorMode();});

    this->actionManager->createQActionToggleButton("ARAPTool_handleMode", "Handle", "H", "Toggle handle mode of ARAP tool, you can now set points as fixed", "handle");
    QObject::connect(this->actionManager->getAction("ARAPTool_handleMode"), &QAction::triggered, [this](){this->scene->toggleARAPManipulatorMode();});

    this->actionManager->createQActionToggleButton("ARAPTool_toggleEvenMode", "Even", "E", "Toggle the even mode to scale evenly in 3 dimensions", "even");
    QObject::connect(this->actionManager->getAction("ARAPTool_toggleEvenMode"), &QAction::triggered, [this](){this->scene->ARAPTool_toggleEvenMode();});

    this->actionManager->createQExclusiveActionGroup("ARAPTool_toggleMode", {"ARAPTool_moveMode", "ARAPTool_handleMode"});

    this->actionManager->createQActionGroup("ARAPTool", {"ARAPTool_moveMode", "ARAPTool_handleMode", "ARAPTool_toggleEvenMode"});

    // Registration
    this->actionManager->createQActionButton("FixedTool_apply", "Register", "A", "Apply the registration", "register");
    QObject::connect(this->actionManager->getAction("FixedTool_apply"), &QAction::triggered, [this](){this->scene->applyFixedRegistrationTool();});

    this->actionManager->createQActionButton("FixedTool_clear", "Clear", "C", "Clear the associated points in the registration tool", "clear");
    QObject::connect(this->actionManager->getAction("FixedTool_clear"), &QAction::triggered, [this](){this->scene->clearFixedRegistrationTool();});

    this->actionManager->createQActionGroup("FixedTool", {"FixedTool_apply", "FixedTool_clear"});

    // Undo
    this->actionManager->createQActionButton("Undo", "Undo", "ctrl+z", "Undo", "undo");
    QObject::connect(this->actionManager->getAction("Undo"), &QAction::triggered, [this](){this->scene->undo();});

    this->actionManager->createQActionButton("Redo", "Redo", "ctrl+alt+z", "Redo", "redo");
    QObject::connect(this->actionManager->getAction("Redo"), &QAction::triggered, [this](){this->scene->redo();});

    this->actionManager->createQActionButton("Reset", "Reset", "", "Put the mesh vertices at there original positions", "reset");
    QObject::connect(this->actionManager->getAction("Reset"), &QAction::triggered, [this](){this->scene->reset();});

    this->actionManager->createQActionButton("Clear", "Clear", "", "Clear the entire scene", "clearScene");
    QObject::connect(this->actionManager->getAction("Clear"), &QAction::triggered, [this](){
            QMessageBox msgBox;
            msgBox.setText("The scene will be clear.");
            msgBox.setInformativeText("Are you sure?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            int ret = msgBox.exec();
            if(ret == QMessageBox::Ok)
                this->scene->clear();
    });

    // Pipeline
    this->actionManager->createQActionButton("Transform", "Transform", "", "Get the point in the associated image", "deform");
    QObject::connect(this->actionManager->getAction("Transform"), &QAction::triggered, [this](){
            this->updateForms();
            this->deformationForm->show();
    });

    this->actionManager->createQActionButton("SaveImage", "SaveImage", "", "Save the deformed image", "saveDeformedImage");
    QObject::connect(this->actionManager->getAction("SaveImage"), &QAction::triggered, [this](){
            this->updateForms();
            this->saveImageForm->show();
    });

    this->actionManager->createQActionButton("QuickSaveCage", "CageSave", "Ctrl+S", "Save the current cage", "saveDeformedImage");
    QObject::connect(this->actionManager->getAction("QuickSaveCage"), &QAction::triggered, [this](){
            this->quickSaveCage->save();
    });

    this->actionManager->createQActionButton("QuickSaveAsCage", "CageSaveAs", "", "Save as the current cage", "saveDeformedImage");
    QObject::connect(this->actionManager->getAction("QuickSaveAsCage"), &QAction::triggered, [this](){
            this->quickSaveCage->saveAs();
    });

    this->actionManager->createQActionButton("Layout1View", "Solo view", "", "Display a 2D view", "soloScreen");
    QObject::connect(this->actionManager->getAction("Layout1View"), &QAction::triggered, [this](){
            this->planarViewer->viewers["View_1"]->viewer2D->hide();
            this->planarViewer->viewers["View_2"]->viewer2D->hide();
            this->planarViewer->viewers["View_3"]->viewer2D->hide();
            this->planarViewer->setDisabled(true);
    });

    this->actionManager->createQActionButton("Layout2View", "Dual view", "", "Display a 2D view", "dualScreen");
    QObject::connect(this->actionManager->getAction("Layout2View"), &QAction::triggered, [this](){
            this->planarViewer->viewers["View_1"]->viewer2D->show();
            this->planarViewer->initViewer("View_1");
            this->planarViewer->viewers["View_2"]->viewer2D->hide();
            this->planarViewer->viewers["View_3"]->viewer2D->hide();
    });

    this->actionManager->createQActionButton("Layout4View", "Quad view", "", "Display a 2D view", "quadScreen");
    QObject::connect(this->actionManager->getAction("Layout4View"), &QAction::triggered, [this](){
            this->planarViewer->viewers["View_1"]->viewer2D->show();
            this->planarViewer->initViewer("View_1");
            this->planarViewer->viewers["View_2"]->viewer2D->show();
            this->planarViewer->initViewer("View_2");
            this->planarViewer->viewers["View_3"]->viewer2D->show();
            this->planarViewer->initViewer("View_3");
    });

    this->actionManager->createQActionButton("Open", "Open", "", "Open the deformed image", "open");
    QObject::connect(this->actionManager->getAction("Open"), &QAction::triggered, [this](){
            this->updateForms();
            this->openImageForm->show();
    });

    // Debug

    this->actionManager->createQActionButton("OpenAtlas", "OpenAtlas", "", "Open the atlas", "open");
    QObject::connect(this->actionManager->getAction("OpenAtlas"), &QAction::triggered, [this](){
            this->scene->openAtlas();
            this->updateForms();
    });

    this->actionManager->createQActionButton("OpenIRM", "OpenIRM", "", "Open the IRM", "open");
    QObject::connect(this->actionManager->getAction("OpenIRM"), &QAction::triggered, [this](){
            this->scene->openIRM();
            this->updateForms();
    });

    this->actionManager->createQActionToggleButton("Sorting", "Sorting", "", "Enable the sorting rendering feature", "arap");
    QObject::connect(this->actionManager->getAction("Sorting"), &QAction::triggered, [this](){
            this->scene->setSortingRendering(this->actionManager->getAction("Sorting")->isChecked());
    });
}

void MainWidget::setupForms() {
    this->deformationForm = new DeformationForm(this->scene);
    this->saveImageForm = new SaveImageForm(this->scene);
    this->planarViewer = new PlanarViewer2D(this->scene);
    this->openImageForm = new OpenImageForm(this->scene);
    QObject::connect(this->openImageForm, &OpenImageForm::loaded, [this](){this->updateForms();});
}

void MainWidget::updateForms() {
    this->deformationForm->update(this->scene);
    this->saveImageForm->update(this->scene);
    this->planarViewer->update(this->scene);
    this->openImageForm->update(this->scene);
}
