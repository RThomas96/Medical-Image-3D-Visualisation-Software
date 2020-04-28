#include "../include/viewer.hpp"

#include <QKeyEvent>

Viewer::Viewer(Scene* scene, bool isLeftOrRight, QWidget* parent) : QGLViewer(parent), isRealSpace(!isLeftOrRight), scene(scene) {
	this->setAxisIsDrawn();
	this->setGridIsDrawn();
}

void Viewer::init() {
	this->scene->initGl();
}

void Viewer::draw() {
	GLfloat mvMat[16];
	GLfloat pMat[16];

	glEnable(GL_DEPTH);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	this->camera()->getModelViewMatrix(mvMat);
	this->camera()->getProjectionMatrix(pMat);
	glm::vec3 max = this->scene->getSceneBoundaries();
	this->setSceneRadius(std::sqrt(glm::dot(max, max)));
	this->setSceneCenter(qglviewer::Vec());
	//this->camera()->showEntireScene();
	if (this->isRealSpace) {
		this->scene->drawRealSpace(mvMat, pMat, false);
	} else {
		this->scene->drawInitialSpace(mvMat, pMat, false);
	}
}

void Viewer::keyPressEvent(QKeyEvent *e) {
	switch (e->key()) {
		case Qt::Key::Key_R:
			this->scene->reloadShaders();
		break;
		case Qt::Key::Key_X:
			this->scene->toggleTransposeMatrices();
		break;
		default:
			QGLViewer::keyPressEvent(e);
		break;
	}
}
