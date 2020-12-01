#include "../include/planar_viewer.hpp"

#include <QMouseEvent>

PlanarViewer::PlanarViewer(Scene* const _scene, planes _p, QWidget* parent) :
		sceneToShow(_scene), planeToShow(_p), QGLViewer(parent) {
	this->setGridIsDrawn(false);
	this->setAxisIsDrawn(false);
	this->setCameraIsEdited(false);
}

PlanarViewer::~PlanarViewer(void) {
	// Nothing here yet.
}

void PlanarViewer::init(void) {
	if (this->sceneToShow == nullptr) {
		throw std::runtime_error("[ERROR] Scene was nullptr when initialized.");
	}

	this->makeCurrent();
	this->sceneToShow->initGl(this->context());
}

void PlanarViewer::draw(void) {
	//
}

void PlanarViewer::keyPressEvent(QKeyEvent* _e) {
	//
}

void PlanarViewer::mousePressEvent(QMouseEvent* _e) {
	//
}
