#include "../include/viewer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QKeyEvent>
#include <fstream>

Viewer::Viewer(Scene* const scene, bool isLeftOrRight, QWidget* parent) : QGLViewer(parent), scene(scene), isRealSpace(isLeftOrRight) {
	this->setAxisIsDrawn();
	this->setGridIsDrawn();
}

void Viewer::init() {
	this->scene->initGl(this->context(), 2, 2, 2);
	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	std::cerr << "Boundaries : " << bbDiag.x << ',' << bbDiag.y << ',' << bbDiag.z << '\n';
	float sceneSize = glm::length(bbDiag);
	std::cerr << "Scene size " << sceneSize << '\n';
	this->setSceneRadius(sceneSize);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
}

void Viewer::draw() {
	GLfloat mvMat[16];
	GLfloat pMat[16];

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
		case Qt::Key::Key_F:
			this->scene->toggleTexCubeVisibility();
			this->update();
		break;
		default:
			QGLViewer::keyPressEvent(e);
		break;
	}
}
