#include "../include/main_widget.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QSizePolicy>

MainWidget::MainWidget() {
	this->setupWidgets();
	this->widgetSizeSet = false;
}

void MainWidget::setupWidgets() {
	this->scene = new Scene();
	this->leftViewer = new Viewer(this->scene, false);
	this->rightViewer = new Viewer(this->scene, false);
	this->controlPanel = new ControlPanel();

	QHBoxLayout* viewerLayout = new QHBoxLayout();
	viewerLayout->addWidget(this->leftViewer);
	viewerLayout->addWidget(this->rightViewer);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	//mainLayout->addWidget(this->leftViewer);
	mainLayout->addLayout(viewerLayout);
	mainLayout->addWidget(this->controlPanel);
	mainLayout->setAlignment(this->controlPanel, Qt::AlignHCenter);

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
		this->leftViewer->setMinimumWidth(this->controlPanel->minimumWidth());
		this->leftViewer->setMinimumHeight(this->controlPanel->minimumWidth());
	}
	// Return false, to handle the rest of the event normally
	return false;
}
