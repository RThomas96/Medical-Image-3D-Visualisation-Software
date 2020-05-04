#include "../include/viewer.hpp"

#include <QKeyEvent>

Viewer::Viewer(Scene* scene, bool isLeftOrRight, QWidget* parent) : QGLViewer(parent), isRealSpace(!isLeftOrRight), scene(scene) {
	this->setAxisIsDrawn();
	this->setGridIsDrawn();
}

void Viewer::init() {
	if (this->scene->isInitialized == false) {
		this->scene->initGl(this->context());
	}
}

void Viewer::draw() {
	if (this->scene->isInitialized == false) {
		this->scene->initGl(this->context());
	}
	GLfloat mvMat[16];
	GLfloat pMat[16];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH);
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	this->camera()->getModelViewMatrix(mvMat);
	this->camera()->getProjectionMatrix(pMat);
	glm::vec3 max = this->scene->getSceneBoundaries();
	this->setSceneRadius(std::sqrt(2.0));
	this->setSceneCenter(qglviewer::Vec(.0,.0,.0));

	if (this->isRealSpace) {
		this->scene->drawRealSpace(mvMat, pMat, false);
	} else {
		glPointSize(10.0);
		this->scene->drawInitialSpace(mvMat, pMat, false);
	}

	glPointSize(8.0);
	glBegin(GL_POINTS);
	{
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(0.0,0.0,1.0);
		glVertex3f(0.0,1.0,0.0);
		glVertex3f(0.0,1.0,1.0);
		glVertex3f(1.0,0.0,0.0);
		glVertex3f(1.0,0.0,1.0);
		glVertex3f(1.0,1.0,0.0);
		glVertex3f(1.0,1.0,1.0);
	}
	glEnd();

}

void Viewer::keyPressEvent(QKeyEvent *e) {
	switch (e->key()) {
		case Qt::Key::Key_R:
			this->scene->reloadShaders();
		break;
		case Qt::Key::Key_X:
			this->scene->toggleTransposeMatrices();
			std::cerr << "Toggled matrix transpose" << '\n';
			this->update();
		break;
		case Qt::Key::Key_Y:
			this->camera()->showEntireScene();
			this->update();
		break;
		default:
			QGLViewer::keyPressEvent(e);
		break;
	}
}
