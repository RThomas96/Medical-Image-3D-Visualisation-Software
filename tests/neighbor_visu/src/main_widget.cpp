#include "../include/main_widget.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

MainWidget::MainWidget() {
	this->setupWidgets();
}

void MainWidget::setupWidgets() {
	this->scene = new Scene();
	this->leftViewer = new Viewer(this->scene, false);
	this->rightViewer = new Viewer(this->scene, true);
	this->controlPanel = new ControlPanel();

	QHBoxLayout* viewerLayout = new QHBoxLayout();
	viewerLayout->addWidget(this->leftViewer);
	viewerLayout->addWidget(this->rightViewer);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(viewerLayout);
	mainLayout->addWidget(this->controlPanel);

	this->setLayout(mainLayout);
}
