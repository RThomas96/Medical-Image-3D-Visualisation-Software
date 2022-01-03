#include "../include/viewer_header.hpp"

#include "../../viewer/include/arap_viewer.hpp"
#include "../../viewer/include/planar_viewer.hpp"

ViewerHeader::ViewerHeader(QWidget* parent) :
	QWidget(parent) {
	this->layout = nullptr;

	this->viewerToControl		 = nullptr;
	this->label_PlaneName		 = nullptr;
	this->button_invertPlaneCut	 = nullptr;
	this->button_rotateClockwise = nullptr;
	this->button_togglePlane	 = nullptr;
	this->slider_planeDepth		 = nullptr;
	this->color					 = Qt::GlobalColor::white;
}

ViewerHeader::ViewerHeader(std::string name, QWidget* parent) :
	ViewerHeader(parent) {
	this->layout		  = new QHBoxLayout();
	this->viewerToControl = nullptr;
	// Create the label
	this->label_PlaneName = new QLabel(name.c_str());
	// Create buttons
	this->button_invertPlaneCut	 = new QPushButton();
	this->button_rotateClockwise = new QPushButton();
	this->button_togglePlane	 = new QPushButton();

	this->icon_togglePlane_On  = new QIcon("../resources/eye_open.png");
	this->icon_togglePlane_Off = new QIcon("../resources/eye_close.png");
	this->icon_rotatePlane	   = new QIcon("../resources/rotate.png");
	this->icon_invertPlane	   = new QIcon("../resources/invert.png");

	this->button_togglePlane->setProperty("toggled", true);

	this->button_togglePlane->setIcon(*this->icon_togglePlane_On);
	this->button_invertPlaneCut->setIcon(*this->icon_invertPlane);
	this->button_rotateClockwise->setIcon(*this->icon_rotatePlane);

	this->button_invertPlaneCut->setToolTip("Invert plane direction in the 3D viewer");
	this->button_rotateClockwise->setToolTip("Rotate current plane view clockwise");
	this->button_togglePlane->setToolTip("Show/Hide the plane");

	// Remove padding :
	//this->button_invertPlaneCut->setStyleSheet("padding-left:padding-top; padding-right:padding-top;");
	//this->button_rotateClockwise->setStyleSheet("padding-left:padding-top; padding-right:padding-top;");
	//this->button_togglePlane->setStyleSheet("padding-left:padding-top; padding-right:padding-top;");

	this->slider_planeDepth = new QSlider(Qt::Horizontal);

	// Make it go a bit further than the bounding box of the scene
	this->slider_planeDepth->setRange(0, 1000);

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
	delete this->icon_togglePlane_Off;
	delete this->icon_togglePlane_On;
	delete this->icon_rotatePlane;
	delete this->icon_invertPlane;
	delete this->layout;
}

void ViewerHeader::setName(const std::string _name) {
	this->label_PlaneName->setText(QString(_name.c_str()));
}

void ViewerHeader::connectToViewer(PlanarViewer* _viewer) {
	if (_viewer == nullptr) {
		return;
	}

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
	if (this->viewerToControl == nullptr) {
		return;
	}

	// Choose the background color in the widget :
	if (this->viewerToControl->planeToShow == planes::x) {
		this->color = QColor::fromRgbF(.8, .1, .1);
	} else if (this->viewerToControl->planeToShow == planes::y) {
		this->color = QColor::fromRgbF(.1, .8, .1);
	} else if (this->viewerToControl->planeToShow == planes::z) {
		this->color = QColor::fromRgbF(.1, .1, .8);
	}

	// Set the background color :
	QPalette colorPalette;
	colorPalette.setColor(QPalette::Window, this->color);
	this->setAutoFillBackground(true);
	this->setPalette(colorPalette);

	// Connect plane signals :
	connect(this->slider_planeDepth, &QSlider::valueChanged, this->viewerToControl, &PlanarViewer::updatePlaneDepth);
	connect(this->button_invertPlaneCut, &QPushButton::clicked, [this]() {
		if (this->viewerToControl != nullptr) {
			this->viewerToControl->flipPlaneDirection();
		}
	});
	connect(this->button_togglePlane, &QPushButton::clicked, [this]() {
		if (this->viewerToControl != nullptr) {
			this->viewerToControl->togglePlaneVisibility();
		}
		// get button state :
		QVariant toggled = this->button_togglePlane->property("toggled");
		// switch icons :
		if (toggled.toBool() == true) {
			this->button_togglePlane->setIcon(*this->icon_togglePlane_Off);
		} else {
			this->button_togglePlane->setIcon(*this->icon_togglePlane_On);
		}
		// set new prop value :
		this->button_togglePlane->setProperty("toggled", not toggled.toBool());
	});
	connect(this->button_rotateClockwise, &QPushButton::clicked, this->viewerToControl, &PlanarViewer::rotatePlaneClockwise);

	this->activateWidgets(true);
}

ViewerHeader3D::ViewerHeader3D(QWidget* parent) :
	QWidget(parent) {
	this->layout				= nullptr;
	this->sceneToControl		= nullptr;
	this->viewerToUpdate		= nullptr;
	this->button_invertPlaneCut = nullptr;
	this->button_togglePlane	= nullptr;
	this->button_centerCamera	= nullptr;
	this->color					= Qt::GlobalColor::darkGray;
}

ViewerHeader3D::ViewerHeader3D(Viewer* _viewer, Scene* _scene, QWidget* parent) :
	ViewerHeader3D(parent) {
	this->sceneToControl = _scene;
	this->viewerToUpdate = _viewer;

	this->setupWidgets();
	this->setupSignals();
}

ViewerHeader3D::~ViewerHeader3D() {
	if (this->button_invertPlaneCut) {
		this->button_invertPlaneCut->disconnect();
	}
	if (this->button_togglePlane) {
		this->button_togglePlane->disconnect();
	}

	delete this->icon_hide;
	delete this->icon_show;
	delete this->icon_invert;
	delete this->icon_volumetric_boxed;
	delete this->icon_volumetric;
	delete this->icon_solid;
}

void ViewerHeader3D::setupWidgets() {
	if (this->sceneToControl == nullptr || this->viewerToUpdate == nullptr) {
		return;
	}

	this->button_togglePlane		= new QPushButton();
	this->button_invertPlaneCut		= new QPushButton();
	this->button_centerCamera		= new QPushButton("Center camera");
	this->button_setSolid			= new QPushButton();
	this->button_setVolumetric		= new QPushButton();
	this->button_setVolumetricBoxed = new QPushButton();
	this->label_allPlanes			= new QLabel("All planes : ");
	this->layout					= new QHBoxLayout;
	this->separator					= new QFrame;
	this->separator->setFrameShape(QFrame::VLine);
	this->separator->setFrameShadow(QFrame::Sunken);

	this->icon_solid			= new QIcon("../resources/label_2D.png");
	this->icon_volumetric		= new QIcon("../resources/label_3D.png");
	this->icon_volumetric_boxed = new QIcon("../resources/label_3D_box.png");

	this->icon_invert = new QIcon("../resources/invert.png");
	this->icon_show	  = new QIcon("../resources/eye_open.png");
	this->icon_hide	  = new QIcon("../resources/eye_close.png");

	this->button_togglePlane->setIcon(*this->icon_show);
	this->button_togglePlane->setToolTip("Show/hide all planes");
	this->button_togglePlane->setProperty("toggled", true);
	this->button_invertPlaneCut->setIcon(*this->icon_invert);
	this->button_invertPlaneCut->setToolTip("Invert all planes' directions");

	this->button_setSolid->setIcon(*this->icon_solid);
	this->button_setSolid->setToolTip("Set draw mode to Solid for the 3D viewer.");
	this->button_setVolumetric->setIcon(*this->icon_volumetric);
	this->button_setVolumetric->setToolTip("Set draw mode to Volumetric for the 3D viewer.");
	this->button_setVolumetricBoxed->setIcon(*this->icon_volumetric_boxed);
	this->button_setVolumetricBoxed->setToolTip("Set draw mode to Volumetric (boxed) for the 3D viewer.");

	this->layout->addWidget(this->button_setSolid);
	this->layout->addWidget(this->button_setVolumetric);
	this->layout->addWidget(this->button_setVolumetricBoxed);
	this->layout->addWidget(this->separator);
	this->layout->addWidget(this->label_allPlanes);
	this->layout->addWidget(this->button_togglePlane);
	this->layout->addWidget(this->button_invertPlaneCut);
	this->layout->addWidget(this->button_centerCamera);

	QPalette colorPalette;
	colorPalette.setColor(QPalette::Window, this->color);
	this->setAutoFillBackground(true);
	this->setPalette(colorPalette);

	this->setLayout(this->layout);
}

void ViewerHeader3D::setupSignals() {
	if (this->sceneToControl == nullptr || this->viewerToUpdate == nullptr) {
		return;
	}

	QObject::connect(this->button_setSolid, &QPushButton::clicked, [this]() -> void {
		if (this->sceneToControl == nullptr) {
			return;
		}
		this->sceneToControl->setDrawMode(DrawMode::Solid);
		if (this->viewerToUpdate == nullptr) {
			return;
		}
		this->viewerToUpdate->update();
	});
	QObject::connect(this->button_setVolumetric, &QPushButton::clicked, [this]() -> void {
		if (this->sceneToControl == nullptr) {
			return;
		}
		this->sceneToControl->setDrawMode(DrawMode::Volumetric);
		if (this->viewerToUpdate == nullptr) {
			return;
		}
		this->viewerToUpdate->update();
	});
	QObject::connect(this->button_setVolumetricBoxed, &QPushButton::clicked, [this]() -> void {
		if (this->sceneToControl == nullptr) {
			return;
		}
		this->sceneToControl->setDrawMode(DrawMode::VolumetricBoxed);
		if (this->viewerToUpdate == nullptr) {
			return;
		}
		this->viewerToUpdate->update();
	});
	// connect plane visibility button :
	QObject::connect(this->button_togglePlane, &QPushButton::clicked, [this]() -> void {
		if (this->sceneToControl == nullptr) {
			return;
		}
		this->sceneToControl->toggleAllPlaneVisibilities();
		if (this->viewerToUpdate == nullptr) {
			return;
		}
		this->viewerToUpdate->update();
		bool toggled = this->button_togglePlane->property("toggled").toBool();
		if (toggled) {
			this->button_togglePlane->setIcon(*this->icon_hide);
		} else {
			this->button_togglePlane->setIcon(*this->icon_show);
		}
		this->button_togglePlane->setProperty("toggled", not toggled);
	});
	// connect plane directions button :
	QObject::connect(this->button_invertPlaneCut, &QPushButton::clicked, [this]() -> void {
		if (this->sceneToControl == nullptr) {
			return;
		}
		this->sceneToControl->toggleAllPlaneDirections();
		if (this->viewerToUpdate == nullptr) {
			return;
		}
		this->viewerToUpdate->update();
	});
	// connect reset visu box button :
	QObject::connect(this->button_centerCamera, &QPushButton::clicked, [this]() -> void {
		if (this->viewerToUpdate == nullptr) {
			return;
		}
		this->viewerToUpdate->centerScene();
	});

	return;
}
