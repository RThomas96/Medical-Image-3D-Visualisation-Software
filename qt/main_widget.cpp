#include "main_widget.hpp"
#include "scene.hpp"

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
	// Query a user settings instance to initialize it :
}

MainWidget::~MainWidget() {
	this->removeEventFilter(this);
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
    QObject::connect(this->openMeshWidget, &OpenMeshWidget::loaded, [this](){
        this->actionManager->getAction("ToggleNoneTool")->trigger();
        this->combo_mesh->setCurrentIndex(this->combo_mesh->count()-1);
    });
    this->applyCageWidget = new ApplyCageWidget(this->scene, this);

    this->display_pannel = new DisplayPannel("Display", *this->actionManager);
    this->cutPlane_pannel = new CutPlaneGroupBox("Cutting planes");
    this->cutPlane_pannel->setCheckable(false);
    this->cutPlane_pannel->setDisabledAlpha(true);

	// Actions creation :
    this->fileMenu = this->menuBar()->addMenu("&File");
    this->fileMenu->addAction(this->actionManager->getAction("OpenImage"));
    this->fileMenu->addAction(this->actionManager->getAction("OpenMesh"));
    this->fileMenu->addAction(this->actionManager->getAction("OpenGraph"));
    this->fileMenu->addSeparator();
    this->fileMenu->addAction(this->actionManager->getAction("SaveCage"));
    this->fileMenu->addAction(this->actionManager->getAction("SaveAsCage"));
    this->fileMenu->addAction(this->actionManager->getAction("SaveImage"));
    //this->fileMenu->addAction(this->actionManager->getAction("SaveImageColormap"));
    this->actionManager->getAction("SaveImage")->setDisabled(true);
    this->actionManager->getAction("SaveImageColormap")->setDisabled(true);

    this->editMenu = this->menuBar()->addMenu("&Edit");
    this->editMenu->addAction(this->actionManager->getAction("ApplyCage"));

    this->windowsMenu = this->menuBar()->addMenu("&Windows");
    this->windowsMenu->addAction(this->actionManager->getAction("DisplayRangeControl"));

    /***/
    this->combo_mesh = new QComboBox();
	QObject::connect(scene, &Scene::meshAdded, this, &MainWidget::addNewMesh);

    QTabBar * tabBar = new QTabBar();
    tabBar->addTab("Tools");
    tabBar->addTab("Files");

    this->toolbar = this->addToolBar("Tools");
    this->toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    //this->toolbar->addWidget(tabBar);

    this->toolbar->addAction(this->actionManager->getAction("OpenImage"));
    this->toolbar->addAction(this->actionManager->getAction("SaveImage"));

    this->toolbar->addAction(this->actionManager->getAction("ToggleNoneTool"));
    this->toolbar->addAction(this->actionManager->getAction("ToggleMoveTool"));
    this->toolbar->addAction(this->actionManager->getAction("ToggleDirectTool"));
    this->toolbar->addAction(this->actionManager->getAction("ToggleARAPTool"));
    this->toolbar->addAction(this->actionManager->getAction("ToggleSliceTool"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("Reset"));
    this->toolbar->addAction(this->actionManager->getAction("Undo"));
    this->toolbar->addAction(this->actionManager->getAction("Redo"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("Layout1View"));
    this->toolbar->addAction(this->actionManager->getAction("Layout2View"));
    this->toolbar->addAction(this->actionManager->getAction("Layout4View"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("Transform"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("Clear"));

    this->toolbar->addSeparator();

    this->toolbar->addAction(this->actionManager->getAction("OpenAtlas"));
    this->toolbar->addAction(this->actionManager->getAction("Boundaries"));

    /***/

    QObject::connect(this->combo_mesh, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
            this->scene->changeActiveMesh(std::string((this->combo_mesh->itemText(this->combo_mesh->currentIndex())).toStdString()));
            this->changeActiveMesh();
    });

    /***/
    // Plane control
    /***/
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::xSliderValueChanged, this->scene, &Scene::slotSetPlaneDisplacementX);
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::ySliderValueChanged, this->scene, &Scene::slotSetPlaneDisplacementY);
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::zSliderValueChanged, this->scene, &Scene::slotSetPlaneDisplacementZ);
    QObject::connect(this->cutPlane_pannel, &CutPlaneGroupBox::aSliderValueChanged, this->scene, &Scene::setBlendFirstPass);

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
    this->range = new RangeControl(this->scene);
    QWidget * tabWidget = new QWidget();
    QHBoxLayout * hlayout = new QHBoxLayout();
    hlayout->addWidget(this->range->rangeOptionUnit);
    hlayout->addWidget(this->range);
    tabWidget->setLayout(hlayout);
    //this->controlPanel->tab->addTab(this->range, QString("Classes"));
    this->controlPanel->tab->addTab(tabWidget, QString("Classes"));
    this->scene->setControlPanel(this->controlPanel);

    this->info_pannel = new InfoPannel("Infos", this->scene);
    this->tool_pannel = new ToolPannel("Tool", *this->actionManager);
    this->quickSaveCage = new QuickSaveMesh(this->scene);

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
    sidePannelLayout->addWidget(this->tool_pannel);
    sidePannelLayout->addWidget(this->display_pannel);
	sidePannelLayout->addWidget(this->cutPlane_pannel);

    viewerLayout->addLayout(sidePannelLayout);
    viewerLayout->addWidget(hSplit, 4);

    /***/

    QFrame * lowFrame = new QFrame();
    lowFrame->setFrameRect(QRect(QRect(0, 0, 0, 0)));
    lowFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    lowFrame->setLineWidth(2);

    QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(this->combo_mesh, 2);
	mainLayout->addWidget(this->viewerFrame, 3);

    QHBoxLayout* lowLayout = new QHBoxLayout(lowFrame);
    lowLayout->addWidget(this->planarViewer, 1);
    lowLayout->addWidget(this->controlPanel, 3);

    this->planarViewer->hide();
    this->controlPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addWidget(lowFrame);

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

    this->planarViewer->hide();

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
    this->actionManager->createQActionToggleButton("ToggleDisplayMesh", "Cage", "C", "Display/Show cage", "visible", "hidden");
    QObject::connect(this->actionManager->getAction("ToggleDisplayMesh"), &QAction::triggered, [this](){this->scene->toggleDisplayMesh();});

    this->actionManager->createQActionToggleButton("ToggleDisplayGrid", "Grids", "G", "Display/Show grid", "visible", "hidden");
    QObject::connect(this->actionManager->getAction("ToggleDisplayGrid"), &QAction::triggered, [this](){this->scene->slotToggleDisplayGrid();});

    this->actionManager->createQActionToggledButton("ToggleDisplayMultiView", "MView", "M", "Display/Show multi grid display", "visible", "hidden");
    QObject::connect(this->actionManager->getAction("ToggleDisplayMultiView"), &QAction::triggered, [this](){this->scene->setMultiGridRendering(!this->actionManager->getAction("ToggleDisplayMultiView")->isChecked()); this->controlPanel->updateRGBMode();});
    this->actionManager->getAction("ToggleDisplayMultiView")->setDisabled(true);

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
    QObject::connect(this, &MainWidget::gridSelected, [this](){this->actionManager->getAction("ToggleDirectTool")->setDisabled(false);});
    QObject::connect(this, &MainWidget::meshSelected, [this](){this->actionManager->getAction("ToggleDirectTool")->setDisabled(false);});

    this->actionManager->createQActionToggleButton("ToggleARAPTool", "ARAP", "Ctrl+A", "Activate ARAP tool", "araps");
    QObject::connect(this->actionManager->getAction("ToggleARAPTool"), &QAction::triggered, [this](){this->scene->updateTools(UITool::MeshManipulatorType::ARAP);});
    QObject::connect(this->actionManager->getAction("ToggleARAPTool"), &QAction::triggered, [this](){this->changeCurrentTool(UITool::MeshManipulatorType::ARAP);});
    QObject::connect(this, &MainWidget::gridSelected, [this](){this->actionManager->getAction("ToggleARAPTool")->setDisabled(true);});
    QObject::connect(this, &MainWidget::meshSelected, [this](){this->actionManager->getAction("ToggleARAPTool")->setDisabled(false);});

    this->actionManager->createQActionToggleButton("ToggleSliceTool", "Slice", "Ctrl+A", "Activate Slice tool", "slice");
    QObject::connect(this->actionManager->getAction("ToggleSliceTool"), &QAction::triggered, [this](){this->scene->updateTools(UITool::MeshManipulatorType::SLICE);});
    QObject::connect(this->actionManager->getAction("ToggleSliceTool"), &QAction::triggered, [this](){this->changeCurrentTool(UITool::MeshManipulatorType::SLICE);});
    QObject::connect(this, &MainWidget::gridSelected, [this](){this->actionManager->getAction("ToggleSliceTool")->setDisabled(true);});
    QObject::connect(this, &MainWidget::meshSelected, [this](){this->actionManager->getAction("ToggleSliceTool")->setDisabled(false);});

    this->actionManager->createQActionToggleButton("ToggleRegisterTool", "Register", "Ctrl+R", "Activate Register tool", "register");
    QObject::connect(this->actionManager->getAction("ToggleRegisterTool"), &QAction::triggered, [this](){this->scene->updateTools(UITool::MeshManipulatorType::FIXED_REGISTRATION);});
    QObject::connect(this->actionManager->getAction("ToggleRegisterTool"), &QAction::triggered, [this](){this->changeCurrentTool(UITool::MeshManipulatorType::FIXED_REGISTRATION);});
    QObject::connect(this, &MainWidget::gridSelected, [this](){this->actionManager->getAction("ToggleRegisterTool")->setDisabled(true);});
    QObject::connect(this, &MainWidget::meshSelected, [this](){this->actionManager->getAction("ToggleRegisterTool")->setDisabled(false);});

    this->actionManager->createQExclusiveActionGroup("ToogleTools", {"ToggleNoneTool", "ToggleMoveTool", "ToggleDirectTool", "ToggleARAPTool", "ToggleRegisterTool", "ToggleSliceTool"});

    // Move
    this->actionManager->createQActionToggleButton("MoveTool_toggleEvenMode", "Even", "E", "Toggle the even mode to scale evenly in 3 dimensions", "even");
    this->actionManager->getAction("MoveTool_toggleEvenMode")->setVisible(false);
    QObject::connect(this->actionManager->getAction("MoveTool_toggleEvenMode"), &QAction::triggered, [this](){this->scene->moveTool_toggleEvenMode();});

    this->actionManager->createQActionToggledButton("MoveTool_toggleMoveCage", "Link", "", "If this mode is active, and the manipulated mesh is a cage, the cage and the associated grid movements are linked", "link");
    this->actionManager->getAction("MoveTool_toggleMoveCage")->setVisible(false);
    QObject::connect(this->actionManager->getAction("MoveTool_toggleMoveCage"), &QAction::triggered, [this](){this->scene->toggleBindMeshToCageMove();});

    this->actionManager->createQActionButton("MoveTool_reset", "Reset", "R", "Reset the manipulator size", "reset");
    this->actionManager->getAction("MoveTool_reset")->setVisible(false);
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
    //this->actionManager->createQActionToggledButton("ARAPTool_moveMode", "Move", "O", "Toggle move mode of ARAP tool, you can now move points with the selection", "move");
    //this->actionManager->getAction("ARAPTool_moveMode")->setVisible(false);
    //QObject::connect(this->actionManager->getAction("ARAPTool_moveMode"), &QAction::triggered, [this](){this->scene->toggleARAPManipulatorMode();});

    //this->actionManager->createQActionToggleButton("ARAPTool_handleMode", "Handle", "H", "Toggle handle mode of ARAP tool, you can now set points as fixed", "handle");
    //this->actionManager->getAction("ARAPTool_handleMode")->setVisible(false);
    //QObject::connect(this->actionManager->getAction("ARAPTool_handleMode"), &QAction::triggered, [this](){this->scene->toggleARAPManipulatorMode();});

    this->actionManager->createQActionToggleButton("ARAPTool_toggleEvenMode", "Even", "E", "Toggle the even mode to scale evenly in 3 dimensions", "even");
    this->actionManager->getAction("ARAPTool_toggleEvenMode")->setVisible(false);
    QObject::connect(this->actionManager->getAction("ARAPTool_toggleEvenMode"), &QAction::triggered, [this](){this->scene->ARAPTool_toggleEvenMode(this->actionManager->getAction("ARAPTool_toggleEvenMode")->isChecked());});

    this->actionManager->createQActionGroup("ARAPTool", {"ARAPTool_toggleEvenMode"});

    // Registration
    this->actionManager->createQActionButton("FixedTool_apply", "Register", "A", "Apply the registration", "register");
    this->actionManager->getAction("FixedTool_apply")->setVisible(false);
    QObject::connect(this->actionManager->getAction("FixedTool_apply"), &QAction::triggered, [this](){this->scene->applyFixedRegistrationTool();});

    this->actionManager->createQActionButton("FixedTool_clear", "Clear", "C", "Clear the associated points in the registration tool", "clear");
    this->actionManager->getAction("FixedTool_clear")->setVisible(false);
    QObject::connect(this->actionManager->getAction("FixedTool_clear"), &QAction::triggered, [this](){this->scene->clearFixedRegistrationTool();});

    this->actionManager->createQActionGroup("FixedTool", {"FixedTool_apply", "FixedTool_clear"});

    this->actionManager->createQActionButton("CenterCamera", "Center", "", "Center the camera on the selected object", "camera");
    QObject::connect(this->actionManager->getAction("CenterCamera"), &QAction::triggered, [this](){this->scene->updateSceneCenter();});

    // Slice
    this->actionManager->createQActionToggledButton("SliceTool_switchX", "X", "", "Toggle the even mode to scale evenly in 3 dimensions", "slice");
    this->actionManager->getAction("SliceTool_switchX")->setVisible(false);
    QObject::connect(this->actionManager->getAction("SliceTool_switchX"), &QAction::triggered, [this](){this->scene->changeSliceToSelect(UITool::SliceOrientation::X);});

    this->actionManager->createQActionToggleButton("SliceTool_switchY", "Y", "", "Toggle the even mode to scale evenly in 3 dimensions", "slice");
    this->actionManager->getAction("SliceTool_switchY")->setVisible(false);
    QObject::connect(this->actionManager->getAction("SliceTool_switchY"), &QAction::triggered, [this](){this->scene->changeSliceToSelect(UITool::SliceOrientation::Y);});

    this->actionManager->createQActionToggleButton("SliceTool_switchZ", "Z", "", "Toggle the even mode to scale evenly in 3 dimensions", "slice");
    this->actionManager->getAction("SliceTool_switchZ")->setVisible(false);
    QObject::connect(this->actionManager->getAction("SliceTool_switchZ"), &QAction::triggered, [this](){this->scene->changeSliceToSelect(UITool::SliceOrientation::Z);});

    this->actionManager->createQExclusiveActionGroup("SliceTool", {"SliceTool_switchX", "SliceTool_switchY", "SliceTool_switchZ"});
    this->actionManager->createQActionGroup("SliceTool", {"SliceTool_switchX", "SliceTool_switchY", "SliceTool_switchZ"});

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
            this->updateForms();
            this->combo_mesh->clear();

            this->actionManager->getAction("OpenImage")->setDisabled(false);
            this->actionManager->getAction("SaveImage")->setDisabled(true);
            this->actionManager->getAction("SaveImageColormap")->setDisabled(true);
            this->actionManager->getAction("ToggleDisplayMultiView")->setDisabled(true);
            this->actionManager->getAction("Transform")->setDisabled(true);
            this->actionManager->getAction("Boundaries")->setVisible(false);
            this->cutPlane_pannel->setDisabledAlpha(true);
    });

    // Pipeline
    this->actionManager->createQActionButton("Transform", "Transform points", "", "Get the point in the associated image", "deform");
    QObject::connect(this->actionManager->getAction("Transform"), &QAction::triggered, [this](){
            this->updateForms();
            this->deformationForm->show();
    });
    this->actionManager->getAction("Transform")->setDisabled(true);

    this->actionManager->createQActionButton("SaveCage", "Save...", "Ctrl+S", "Save the current cage", "");
    QObject::connect(this->actionManager->getAction("SaveCage"), &QAction::triggered, [this](){
            this->quickSaveCage->save();
    });

    this->actionManager->createQActionButton("SaveAsCage", "Save as...", "Maj+Ctrl+S", "Save as the current cage", "");
    QObject::connect(this->actionManager->getAction("SaveAsCage"), &QAction::triggered, [this](){
            this->quickSaveCage->saveAs();
    });

    this->actionManager->createQActionButton("SaveImage", "Export...", "", "Save the deformed image", "save");
    QObject::connect(this->actionManager->getAction("SaveImage"), &QAction::triggered, [this](){
        this->saveImageForm->show();
    });

    this->actionManager->createQActionButton("SaveImageColormap", "Export image with colormap...", "", "Save the deformed image with the current colormap applied", "save");
    QObject::connect(this->actionManager->getAction("SaveImageColormap"), &QAction::triggered, [this](){
            FileChooser * fileChooser = new FileChooser("File", FileChooserType::SAVE, FileChooserFormat::TIFF);
            fileChooser->click();

            std::string gridName = this->combo_mesh->itemText(this->combo_mesh->currentIndex()).toStdString();
            if(!fileChooser->filename.isEmpty()) {
                if(scene->isCage(gridName)) {
                    gridName = scene->grids_name[scene->getGridIdxLinkToCage(gridName)];
                }
                this->scene->writeDeformedImage(fileChooser->filename.toStdString(), gridName, true, ResolutionMode::SAMPLER_RESOLUTION);
            }
            delete fileChooser;
    });

    this->actionManager->createMenuButton("SaveMenu", "Save", "Save the current cage", "saveDeformedImage", {"SaveImage", "SaveAsImage", "-", "SaveCage", "SaveAsCage"});

    this->actionManager->createQActionButton("ApplyCage", "Apply cage...", "", "Apply a cage on a previously loaded cage", "");
    QObject::connect(this->actionManager->getAction("ApplyCage"), &QAction::triggered, [this](){
        QStringList potentialCages;
        std::vector<std::string> allCages = this->scene->getAllCagesName();
        for(int i = 0; i < allCages.size(); ++i)
            potentialCages += QString(allCages[i].c_str());
        this->applyCageWidget->setPotentialCageToApply(potentialCages);
        this->applyCageWidget->show();
    });

    this->actionManager->createQActionButton("Layout1View", "Solo view", "", "Display a 2D view", "soloScreen");
    QObject::connect(this->actionManager->getAction("Layout1View"), &QAction::triggered, [this](){
            this->planarViewer->viewers["View_1"]->viewer2D->hide();
            this->planarViewer->viewers["View_2"]->viewer2D->hide();
            this->planarViewer->viewers["View_3"]->viewer2D->hide();
            //this->planarViewer->setDisabled(true);
            this->planarViewer->hide();
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

    this->actionManager->createQActionButton("OpenImage", "Open...", "Ctrl+O", "Open the image to deform", "open");
    QObject::connect(this->actionManager->getAction("OpenImage"), &QAction::triggered, [this](){
            this->updateForms();
            this->openImageForm->show();
            this->actionManager->getAction("ToggleNoneTool")->trigger();
    });

    this->actionManager->createQActionButton("OpenMesh", "Open mesh...", "", "Open mesh", "");
    QObject::connect(this->actionManager->getAction("OpenMesh"), &QAction::triggered, [this]() {
        //this->scene->loadMesh();
        QStringList potentialCages;
        std::vector<std::string> allNonTetrahedralMeshes = this->scene->getAllBaseMeshesName();
        for(int i = 0; i < allNonTetrahedralMeshes.size(); ++i)
            potentialCages += QString(allNonTetrahedralMeshes[i].c_str());
        this->openMeshWidget->setPotentialCages(potentialCages);
        this->openMeshWidget->show();
    });

    this->actionManager->createQActionButton("OpenGraph", "Open graph...", "", "Open graph", "");
    QObject::connect(this->actionManager->getAction("OpenGraph"), &QAction::triggered, [this]() {
        FileChooser fileChooser(QString("dummy"), FileChooserType::SELECT, FileChooserFormat::OFF);
        fileChooser.click();
        this->scene->openGraph(std::string("graph"), fileChooser.filename.toStdString());
    });

    this->actionManager->createMenuButton("OpenMenu", "Open", "Open", "open", {"OpenImage", "OpenMesh"});

    // Windows

    this->actionManager->createQActionToggledButton("DisplayRangeControl", "Display/show color control window", "", "Display/show color control window", "");
    QObject::connect(this->actionManager->getAction("DisplayRangeControl"), &QAction::triggered, [this](){
        this->controlPanel->setVisible(this->actionManager->getAction("DisplayRangeControl")->isChecked());
        this->planarViewer->setVisible(this->actionManager->getAction("DisplayRangeControl")->isChecked());
    });

    // Debug

    this->actionManager->createQActionButton("OpenAtlas", "Open atlas", "", "Open the atlas", "open");
    QObject::connect(this->actionManager->getAction("OpenAtlas"), &QAction::triggered, [this](){
            this->scene->openAtlas();
            this->actionManager->getAction("ToggleNoneTool")->trigger();
            this->updateForms();
    });

    this->actionManager->createQActionButton("OpenIRM", "Open IRM", "", "Open the IRM", "open");
    QObject::connect(this->actionManager->getAction("OpenIRM"), &QAction::triggered, [this](){
            this->scene->openIRM();
            this->actionManager->getAction("ToggleNoneTool")->trigger();
            this->updateForms();
    });

    this->actionManager->createQActionToggleButton("Sorting", "Sorting", "", "Enable the sorting rendering feature", "arap");
    QObject::connect(this->actionManager->getAction("Sorting"), &QAction::triggered, [this](){
            this->scene->setSortingRendering(this->actionManager->getAction("Sorting")->isChecked());
    });

    this->actionManager->createQActionToggleButton("Shader", "Shader", "", "Reload shaders", "arap");
    QObject::connect(this->actionManager->getAction("Shader"), &QAction::triggered, [this](){
            this->scene->recompileShaders(true);
    });

    this->actionManager->createQActionToggleButton("Boundaries", "Boundaries", "", "", "arap");
    QObject::connect(this->actionManager->getAction("Boundaries"), &QAction::triggered, [this](){
        this->scene->setDrawOnlyBoundaries(this->actionManager->getAction("Boundaries")->isChecked());
    });
    this->actionManager->getAction("Boundaries")->setVisible(false);
}

void MainWidget::setupForms() {
    this->deformationForm = new DeformationForm(this->scene);
    this->saveImageForm = new SaveImageForm(this->scene);
    this->planarViewer = new PlanarViewer2D(this->scene);
    this->planarViewer->hide();
    this->openImageForm = new OpenImageForm(this->scene);
    QObject::connect(this->openImageForm, &OpenImageForm::loaded, [this](){
        this->updateForms();
        this->combo_mesh->setCurrentIndex(this->combo_mesh->count()-1);
        this->actionManager->getAction("ToggleNoneTool")->trigger();
        if(this->openImageForm->checkBoxes["Segmented"]->isChecked()) {
            this->range->addUnitsAuto();
            this->controlPanel->tab->setCurrentIndex(1);
        }
        //this->actionManager->getAction("OpenImage")->setDisabled(true);
        //this->actionManager->getAction("OpenMesh")->setDisabled(true);
    });
}

void MainWidget::updateForms() {
    this->deformationForm->update(this->scene);
    this->saveImageForm->update(this->scene);
    this->planarViewer->update(this->scene);
    this->openImageForm->update(this->scene);

    this->range->updateRanges();
}
