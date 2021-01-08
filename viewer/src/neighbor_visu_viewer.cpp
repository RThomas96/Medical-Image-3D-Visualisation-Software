#include "../include/neighbor_visu_viewer.hpp"
#include "../../image/include/writer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QCoreApplication>
#include <QProgressDialog>
#include <QKeyEvent>
#include <fstream>
#include <dlfcn.h>

float Viewer::sceneRadiusMultiplier{.5f};

Viewer::Viewer(Scene* const scene, QWidget* parent) :
	QGLViewer(parent), scene(scene) {

	this->refreshTimer = new QTimer();
	this->refreshTimer->setInterval(std::chrono::milliseconds(7)); // ~7 ms for 144fps, ~16ms for 60fps and ~33ms for 30 FPS
	this->refreshTimer->setSingleShot(false);
	connect(this->refreshTimer, &QTimer::timeout, this, &Viewer::updateView);

	this->drawVolumetric = false;
	this->shouldCapture = false;
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

void Viewer::preDraw() {
	QGLViewer::preDraw();
/*
	if (this->shouldCapture) {
		std::cerr << "Will capture ... ";
		if (rdocAPI != nullptr) {
			// Save in current folder, under 'capture_frameXXX.rdoc'
			rdocAPI->SetCaptureFilePathTemplate("./rdoc");
			// Start capture :
			rdocAPI->StartFrameCapture(NULL, NULL);
			std::cerr << "started\n";
		} else {
			std::cerr << " NOPE !\n";
		}
	}
*/
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

void Viewer::postDraw() {
/*	if (this->shouldCapture) {
		std::cerr << "Trying to end ... ";
		if (rdocAPI != nullptr) {
			if (rdocAPI->EndFrameCapture(NULL, NULL) == 0) {
				std::cerr << "an error occured while capturing the frame !\n";
			} else {
				std::cerr << "done.\n";
			}
			this->shouldCapture = false;
		} else {
			std::cerr << " NOPE !\n";
		}
	}*/

	QGLViewer::postDraw();
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
		case Qt::Key::Key_P:
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
		case Qt::Key::Key_W:
			this->scene->writeGridDIM("outputGrid");
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
		RENDERDOC
		*/
		case Qt::Key::Key_C:
			this->shouldCapture = true;
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
