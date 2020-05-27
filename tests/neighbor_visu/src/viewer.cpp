#include "../include/viewer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QKeyEvent>
#include <fstream>

Viewer::Viewer(Scene* const scene, bool isLeftOrRight, QWidget* parent) : QGLViewer(parent), scene(scene), isRealSpace(isLeftOrRight) {
	this->setAxisIsDrawn();
	this->setGridIsDrawn();
	this->focusType = FocusStates::DefaultFocus;
}

void Viewer::init() {
	this->makeCurrent();
	this->scene->initGl(this->context(), 3, 3, 3);

	if (this->focusType == FocusStates::TextureFocus) {
		glm::vec3 bbDiag = this->scene->getTexCubeBoundaries(this->isRealSpace);
		float sceneSize = glm::length(bbDiag);

		this->setSceneRadius(sceneSize);
		// center scene on center of grid
		this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
		this->showEntireScene();
	} else if (this->focusType == FocusStates::NeighborFocus) {
		nbCoord c = this->scene->getNeighborBoundaries(this->isRealSpace);

		glm::vec3 span = c.p - c.o;
		glm::vec3 center = ((c.p - c.o)/2.f)+c.o;

		this->setSceneRadius(2.*glm::length(span));
		this->setSceneCenter(qglviewer::Vec(center.x, center.y, center.z));
		this->showEntireScene();
	}
}

void Viewer::draw() {
	if (this->focusType == FocusStates::NeighborFocus){
		nbCoord n = this->scene->getNeighborBoundaries(this->isRealSpace);
		glm::vec3 center = n.o + ((n.p - n.o)/2.f);
		this->setSceneCenter(qglviewer::Vec(center.x, center.y, center.z));
		this->setSceneRadius(glm::length(n.p - n.o)*2.f);
	}
	GLfloat mvMat[16];
	GLfloat pMat[16];

	glClearColor(.9, .9, .9, 1.);

	this->camera()->getModelViewMatrix(mvMat);
	this->camera()->getProjectionMatrix(pMat);

	if (isRealSpace) {
		this->scene->drawRealSpace(mvMat, pMat);
	} else {
		this->scene->drawInitialSpace(mvMat, pMat);
	}
}

void Viewer::keyPressEvent(QKeyEvent *e) {
	switch (e->key()) {
		case Qt::Key::Key_X:
			this->update();
		break;
		case Qt::Key::Key_R:
			this->scene->compileShaders();
			this->update();
		break;
		case Qt::Key::Key_1:
			this->applyMatrix = !this->applyMatrix;
			this->update();
		break;
		case Qt::Key::Key_F:
			this->scene->toggleTexCubeVisibility();
			this->update();
		break;
		case Qt::Key::Key_N : {
			this->focusType = FocusStates::NeighborFocus;
			nbCoord c = this->scene->getNeighborBoundaries(this->isRealSpace);

			glm::vec3 span = c.p - c.o;
			glm::vec3 center = ((c.p - c.o)/2.f)+c.o;

			this->setSceneRadius(2.*glm::length(span));
			this->setSceneCenter(qglviewer::Vec(center.x, center.y, center.z));
			this->showEntireScene();
		}
		break;
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
		case Qt::Key::Key_T:
			this->focusType = FocusStates::DefaultFocus;
			this->updateTextureFocus();
			this->update();
		break;
		default:
			QGLViewer::keyPressEvent(e);
		break;
	}
}

void Viewer::setFocusState(int state) {
	switch (state) {
		case 0:
			this->focusType = FocusStates::NoFocus;
		break;
		case 1:
			this->focusType = FocusStates::TextureFocus;
			this->updateTextureFocus();
		break;
		case 2:
			this->focusType = FocusStates::NeighborFocus;
		default:
			this->focusType = FocusStates::DefaultFocus;
		break;
	}
	this->update();
}

void Viewer::updateTextureFocus() {
	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	float sceneSize = glm::length(bbDiag);

	this->setSceneRadius(sceneSize);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
}
