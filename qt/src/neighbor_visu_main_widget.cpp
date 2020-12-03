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
	delete this->gridController;
	delete this->controlPanel;
	#ifndef ENABLE_QUAD_VIEW
	delete this->viewer;
	#endif
	/*
	delete this->viewer;
	delete this->viewer_planeX;
	delete this->viewer_planeY;
	delete this->viewer_planeZ;
	*/
	this->strayObj.clear();
}

void MainWidget::setupWidgets() {
	this->gridController = new GridControl(nullptr);
	this->scene = new Scene(this->gridController);

#ifdef ENABLE_QUAD_VIEW
	QSplitter* mainSplit = new QSplitter(Qt::Vertical);
	QSplitter* splitAbove = new QSplitter(mainSplit);
	QSplitter* splitBelow = new QSplitter(mainSplit);
#endif

#ifdef ENABLE_QUAD_VIEW
	this->viewer = new Viewer(this->scene, true, planes::x, splitAbove);
	this->viewer_planeX = new Viewer(this->scene, false, planes::x, splitAbove);
	this->viewer_planeY = new Viewer(this->scene, false, planes::y, splitBelow);
	this->viewer_planeZ = new Viewer(this->scene, false, planes::z, splitBelow);
#else
	this->viewer = new Viewer(this->scene, false, planes::x, nullptr);
#endif
	this->controlPanel = new ControlPanel(this->scene, this->viewer, nullptr, nullptr);
	this->scene->setControlPanel(this->controlPanel);

	this->gridController->show(); // Enable grid controller as a floating window

	QHBoxLayout* viewerLayout = new QHBoxLayout();
	#ifdef ENABLE_QUAD_VIEW
	viewerLayout->addWidget(mainSplit);
	#else
	viewerLayout->addWidget(this->viewer);
	#endif

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(viewerLayout);
	mainLayout->addWidget(this->controlPanel);
	mainLayout->setAlignment(this->controlPanel, Qt::AlignHCenter);

	this->strayObj.push_back(viewerLayout);
	this->strayObj.push_back(mainLayout);
	#ifdef ENABLE_QUAD_VIEW
	this->strayObj.push_back(mainSplit);
	#endif

	this->setLayout(mainLayout);
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
