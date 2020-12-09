#include "../include/neighbor_visu_main_widget.hpp"

#include <QLabel>
#include <QEvent>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>

MainWidget::MainWidget() {
	this->strayObj.clear();
	this->setupWidgets();
	this->widgetSizeSet = false;
}

MainWidget::~MainWidget() {
	this->removeEventFilter(this);
	for (std::size_t i = 0; i < this->strayObj.size(); ++i) {
		if (this->strayObj[i] != nullptr) {
			delete this->strayObj[i];
			this->strayObj[i] = nullptr;
		}
	}
	delete this->scene;
	#ifdef ENABLE_QUAD_VIEW
	delete this->gridController;
	delete this->controlPanel;
	#endif
	this->strayObj.clear();
}

void MainWidget::setupWidgets() {
	QSplitter* mainSplit = new QSplitter(Qt::Horizontal);
#ifdef ENABLE_QUAD_VIEW
	this->gridController = new GridControl(nullptr);
	this->scene = new Scene(this->gridController);

	QSplitter* splitAbove = new QSplitter(Qt::Vertical);

	this->viewer = new Viewer(this->scene, mainSplit);
	this->controlPanel = new ControlPanel(this->scene, this->viewer, nullptr, nullptr);
	this->scene->setControlPanel(this->controlPanel);
	mainSplit->addWidget(splitAbove);
	this->viewer_planeX = new PlanarViewer(this->scene, planes::x, splitAbove);
	this->viewer_planeY = new PlanarViewer(this->scene, planes::y, splitAbove);
	this->viewer_planeZ = new PlanarViewer(this->scene, planes::z, splitAbove);
#endif

#if not defined ENABLE_QUAD_VIEW && defined ENABLE_VIEW_X
	this->scene = new Scene(nullptr);
	this->viewer_planeX = new PlanarViewer(this->scene, planes::x, mainSplit);
#endif
#if not defined ENABLE_QUAD_VIEW && defined ENABLE_VIEW_Y
	this->scene = new Scene(nullptr);
	this->viewer_planeY = new PlanarViewer(this->scene, planes::y, mainSplit);
#endif
#if not defined ENABLE_QUAD_VIEW && defined ENABLE_VIEW_Z
	this->scene = new Scene(nullptr);
	this->viewer_planeZ = new PlanarViewer(this->scene, planes::z, mainSplit);
#endif


	QHBoxLayout* viewerLayout = new QHBoxLayout();
	viewerLayout->addWidget(mainSplit);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(viewerLayout);
#ifdef ENABLE_QUAD_VIEW
	mainLayout->addWidget(this->controlPanel);
	mainLayout->setAlignment(this->controlPanel, Qt::AlignHCenter);
	this->gridController->show(); // Enable grid controller as a floating window
#endif
	this->strayObj.push_back(viewerLayout);
	this->strayObj.push_back(mainLayout);
	this->strayObj.push_back(mainSplit);

	this->setLayout(mainLayout);
	this->installEventFilter(this);
}

bool MainWidget::eventFilter(QObject* obj, QEvent* e) {
	// Set our code to run after the original event handling :
	#ifdef ENABLE_QUAD_VIEW
	if (this->widgetSizeSet == false && obj == this && e->type() == QEvent::Show) {
		this->widgetSizeSet = true;
		// lock control panel size to the current size it has :
		this->controlPanel->setMinimumSize(this->controlPanel->size());
		this->controlPanel->setMaximumSize(this->controlPanel->size());
		// set the viewer to have a minimum size of controlPanelSize on width/height :
		this->viewer->setMinimumWidth(this->controlPanel->minimumWidth());
		this->viewer->setMinimumHeight(this->controlPanel->minimumWidth()/2);
	}
	#endif
	// Return false, to handle the rest of the event normally
	return false;
}
