#include "../include/neighbor_visu_viewer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QKeyEvent>
#include <fstream>

float Viewer::sceneRadiusMultiplier{1.5f};

Viewer::Viewer(Scene* const scene, bool isLeftOrRight, QWidget* parent) : QGLViewer(parent), scene(scene), isRealSpace(isLeftOrRight) {
	this->setGridIsDrawn();
	this->focusType = FocusStates::DefaultFocus;
}

void Viewer::init() {
	this->makeCurrent();
	this->scene->initGl(this->context(), 3, 3, 3);
	this->scene->setDrawModeSolid();

	if (this->focusType == FocusStates::NeighborFocus) {
		this->updateNeighborFocus();
	} else {
		this->updateTextureFocus();
	}
}

void Viewer::draw() {
	GLfloat mvMat[16];
	GLfloat pMat[16];

	glClearColor(.8, .8, .8, 1.);

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
		case Qt::Key::Key_R:
			this->scene->recompileShaders();
			this->update();
		break;
		case Qt::Key::Key_F:
			this->scene->toggleTexCubeVisibility();
			this->update();
		break;
		// Focus of the viewer :
		case Qt::Key::Key_N :
			this->focusType = FocusStates::NeighborFocus;
			this->updateNeighborFocus();
			this->update();
		break;
		case Qt::Key::Key_T:
			this->focusType = FocusStates::DefaultFocus;
			this->updateTextureFocus();
			this->update();
		break;
		case Qt::Key::Key_H:
			this->scene->populateGrid();
			this->update();
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
		case Qt::Key::Key_F5:
			this->scene->updateNeighborTetMesh();
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

void Viewer::updateNeighborFocus() {
	nbCoord c = this->scene->getNeighborBoundaries(this->isRealSpace);

	glm::vec3 span = c.p - c.o;
	glm::vec3 center = ((c.p - c.o)/2.f) + c.o;

	this->setSceneRadius(sceneRadiusMultiplier * glm::length(span));
	this->setSceneCenter(qglviewer::Vec(center.x, center.y, center.z));
	this->showEntireScene();
}

void Viewer::updateTextureFocus() {
	glm::vec3 bbDiag = this->scene->getTexCubeBoundaries(this->isRealSpace);
	float sceneSize = glm::length(bbDiag);

	this->setSceneRadius(sceneSize*sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
}
