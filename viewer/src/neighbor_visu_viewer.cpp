#include "../include/neighbor_visu_viewer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QCoreApplication>
#include <QProgressDialog>
#include <QKeyEvent>
#include <fstream>

float Viewer::sceneRadiusMultiplier{1.5f};

Viewer::Viewer(Scene* const scene, QWidget* parent) :
	QGLViewer(parent), scene(scene) {
	this->setGridIsDrawn();

	this->refreshTimer = new QTimer();
	this->refreshTimer->setInterval(std::chrono::milliseconds(7)); // ~7 ms for 144fps, ~16ms for 60fps and ~33ms for 30 FPS
	this->refreshTimer->setSingleShot(false);
	connect(this->refreshTimer, &QTimer::timeout, this, &Viewer::updateView);

	this->drawVolumetric = false;
}

Viewer::~Viewer() {
	disconnect(this->refreshTimer, &QTimer::timeout, this, &Viewer::updateView);
	delete this->refreshTimer;
}

void Viewer::init() {
	this->makeCurrent();
	if (not this->scene->isInitialized) {
		QProgressDialog* progress = new QProgressDialog("Initializing scene ...", QString(), 0, 10);
		progress->show();
		QCoreApplication::processEvents();
		this->scene->initGl(this->context());
		this->scene->setDrawModeSolid();
		progress->setValue(10);
	}

	glm::vec3 bbDiag = this->scene->getSceneBoundaries(true);
	float sceneSize = glm::length(bbDiag);

	this->setSceneRadius(sceneSize*sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();

	this->refreshTimer->start(); // Update every 'n' milliseconds from here on out
}

void Viewer::draw() {
	GLfloat mvMat[16];
	GLfloat pMat[16];

	glClearColor(.8, .8, .8, 1.);

	this->camera()->getModelViewMatrix(mvMat);
	this->camera()->getProjectionMatrix(pMat);

	qglviewer::Vec cam = this->camera()->worldCoordinatesOf(qglviewer::Vec(0., 0., 0.));
	glm::vec3 camPos = glm::vec3(static_cast<float>(cam.x), static_cast<float>(cam.y), static_cast<float>(cam.z));

	if (this->drawVolumetric) {
		this->scene->drawVolumetric(mvMat, pMat, camPos);
	} else {
		this->scene->drawWithPlanes(mvMat, pMat);
	}
}

void Viewer::keyPressEvent(QKeyEvent *e) {
	switch (e->key()) {
		/*
		SHADER PROGRAMS
		*/
		case Qt::Key::Key_R:
			this->scene->recompileShaders();
			this->update();
		break;
		case Qt::Key::Key_C:
			this->scene->printVAOStateNext();
		break;
		case Qt::Key::Key_V:
			this->drawVolumetric = !this->drawVolumetric;
			this->update();
		break;
		/*
		GRID VISIBILITY
		*/
		case Qt::Key::Key_I:
			this->scene->toggleInputGridVisible();
			this->update();
		break;
		case Qt::Key::Key_O:
			this->scene->toggleOutputGridVisible();
			this->update();
		break;
		/*
		INTERPOLATION
		*/
		case Qt::Key::Key_T:
			this->scene->fillTrilinear();
			this->update();
		break;
		case Qt::Key::Key_N:
			this->scene->fillNearestNeighbor();
			this->update();
		break;
		/*
		DRAW MODES
		*/
		case Qt::Key::Key_F1:
			this->scene->setDrawModeSolid();
			this->update();
		break;
		case Qt::Key::Key_F2:
			this->scene->setDrawModeSolidAndWireframe();
			this->update();
		break;
		case Qt::Key::Key_F3:
			this->scene->setDrawModeWireframe();
			this->update();
		break;
		case Qt::Key::Key_F4:
			this->scene->toggleColorOrTexture();
			this->update();
		break;
		/*
		Default handler.
		*/
		default:
			QGLViewer::keyPressEvent(e);
		break;
	}
}
