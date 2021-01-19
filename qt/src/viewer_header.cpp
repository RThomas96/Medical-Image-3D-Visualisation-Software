#include "../include/viewer_header.hpp"

#include "../../viewer/include/planar_viewer.hpp"

ViewerHeader::ViewerHeader(QWidget* parent) : QWidget(parent) {
	this->layout = nullptr;

	this->viewerToControl = nullptr;
	this->label_PlaneName = nullptr;
	this->button_invertPlaneCut = nullptr;
	this->button_rotateClockwise = nullptr;
	this->button_togglePlane = nullptr;
	this->slider_planeDepth = nullptr;
	this->color = Qt::GlobalColor::white;
}

ViewerHeader::ViewerHeader(std::string name, QWidget* parent) : ViewerHeader(parent) {
	this->layout = new QHBoxLayout();
	this->viewerToControl = nullptr;
	// Create the label
	this->label_PlaneName = new QLabel(name.c_str());
	// Create buttons
	this->button_invertPlaneCut = new QPushButton("Invert");
	this->button_rotateClockwise = new QPushButton("RCW");
	this->button_togglePlane = new QPushButton("Show ?");

	// Remove padding :
	this->button_invertPlaneCut->setStyleSheet("padding-left:padding-top; padding-right:padding-top;");
	this->button_rotateClockwise->setStyleSheet("padding-left:padding-top; padding-right:padding-top;");
	this->button_togglePlane->setStyleSheet("padding-left:padding-top; padding-right:padding-top;");

	this->slider_planeDepth = new QSlider(Qt::Horizontal);

	// Make it go a bit further than the bounding box of the scene
	this->slider_planeDepth->setRange(1, 99);

	this->layout->addWidget(this->label_PlaneName);
	this->layout->addWidget(this->slider_planeDepth);
	this->layout->addWidget(this->button_invertPlaneCut);
	this->layout->addWidget(this->button_rotateClockwise);
	this->layout->addWidget(this->button_togglePlane);

	this->setLayout(this->layout);
	// By default, nothing is activated since it's not connected to the plane viewer.
	this->activateWidgets(false);
}

ViewerHeader::~ViewerHeader() {
	this->unregisterPlaneViewer();
	delete this->label_PlaneName;
	delete this->button_invertPlaneCut;
	delete this->button_rotateClockwise;
	delete this->button_togglePlane;
	delete this->slider_planeDepth;
	delete this->layout;
}

void ViewerHeader::setName(const std::string _name) {
	this->label_PlaneName->setText(QString(_name.c_str()));
}

void ViewerHeader::connectToViewer(PlanarViewer *_viewer) {
	if (_viewer == nullptr) { return; }

	this->viewerToControl = _viewer;
	this->registerWithViewer();
}

void ViewerHeader::unregisterPlaneViewer() {
	// Disconnect signals :
	this->button_invertPlaneCut->disconnect();
	this->button_rotateClockwise->disconnect();
	this->button_togglePlane->disconnect();
	this->slider_planeDepth->disconnect();
	// Remove text :
	this->label_PlaneName->setText("- Nothing connected -");

	// Reset sliders :
	this->slider_planeDepth->setValue(1);

	// De-activate widgets :
	this->activateWidgets(false);

	// Remove pointer to viewer.
	this->viewerToControl = nullptr;
}

void ViewerHeader::activateWidgets(bool activated) {
	this->label_PlaneName->setEnabled(activated);
	this->button_invertPlaneCut->setEnabled(activated);
	this->button_rotateClockwise->setEnabled(activated);
	this->button_togglePlane->setEnabled(activated);
	this->slider_planeDepth->setEnabled(activated);
	return;
}

void ViewerHeader::registerWithViewer(void) {
	if (this->viewerToControl == nullptr) { return; }

	// Choose the background color in the widget :
	if (this->viewerToControl->planeToShow == planes::x) {
		this->color = Qt::GlobalColor::red;
	} else if (this->viewerToControl->planeToShow == planes::y) {
		this->color = Qt::GlobalColor::green;
	} else if (this->viewerToControl->planeToShow == planes::z) {
		this->color = Qt::GlobalColor::blue;
	}

	// Set the background color :
	QPalette colorPalette;
	colorPalette.setColor(QPalette::Window, this->color);
	this->setAutoFillBackground(true);
	this->setPalette(colorPalette);

	// Connect plane signals :
	connect(this->slider_planeDepth, &QSlider::valueChanged, this->viewerToControl, &PlanarViewer::updatePlaneDepth);
	connect(this->button_invertPlaneCut, &QPushButton::clicked, this->viewerToControl, &PlanarViewer::flipPlaneDirection);
	connect(this->button_togglePlane, &QPushButton::clicked, this->viewerToControl, &PlanarViewer::togglePlaneVisibility);
	connect(this->button_rotateClockwise, &QPushButton::clicked, this->viewerToControl, &PlanarViewer::rotatePlaneClockwise);

	this->activateWidgets(true);
}
