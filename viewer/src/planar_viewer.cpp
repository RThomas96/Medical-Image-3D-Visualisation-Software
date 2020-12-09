#include "../include/planar_viewer.hpp"

#include <QMouseEvent>

PlanarViewer::PlanarViewer(Scene* const _scene, planes _p, QWidget* parent) :
		QGLViewer(parent), sceneToShow(_scene), planeToShow(_p) {
	this->setGridIsDrawn(false);
	this->setAxisIsDrawn(false);
	this->setCameraIsEdited(false);
	this->refreshTimer = new QTimer();
	this->refreshTimer->setInterval(std::chrono::milliseconds(16)); // ~7 ms for 144fps, ~16ms for 60fps and ~33ms for 30 FPS
	this->refreshTimer->setSingleShot(false);
	connect(this->refreshTimer, &QTimer::timeout, this, &PlanarViewer::updateView);
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
	this->refreshTimer->start();
}

void PlanarViewer::draw(void) {
	float coef = .7f;
	/*
	if (this->planeToShow == planes::x) { glClearColor(coef,  .0f, .0f, 1.f); }
	if (this->planeToShow == planes::y) { glClearColor(.0f,  coef, .0f, 1.f); }
	if (this->planeToShow == planes::z) { glClearColor(.0f,  .0f, coef, 1.f); }
	*/
	glClearColor(.8, .8, .8, 1.);
	QSize viewerSize = this->size();
	glm::vec2 fbDims = glm::vec2(static_cast<float>(viewerSize.width()), static_cast<float>(viewerSize.height()));

	this->sceneToShow->drawPlaneView(fbDims, this->planeToShow);
}

void PlanarViewer::keyPressEvent(QKeyEvent* _e) {
	switch (_e->key()) {
		/*
		SHADER PROGRAMS
		*/
		case Qt::Key::Key_R:
			this->sceneToShow->recompileShaders();
			this->update();
		break;
		case Qt::Key::Key_P:
			this->sceneToShow->printVAOStateNext();
			this->update();
		break;
		default:
			QGLViewer::keyPressEvent(_e);
			this->update();
		break;
	}
}

void PlanarViewer::mousePressEvent(QMouseEvent* _e) {
	QGLViewer::mousePressEvent(_e);
	this->update();
}

void PlanarViewer::updateView() {
	this->update();
}
