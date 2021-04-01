#include "../include/neighbor_visu_viewer.hpp"
#include "../../image/include/writer.hpp"
#include "../../features.hpp"
#include "../../qt/include/user_settings_widget.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QGLViewer/manipulatedFrame.h>

#include <QCoreApplication>
#include <QProgressDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QGuiApplication>

#include <fstream>

float Viewer::sceneRadiusMultiplier{.5f};

Viewer::Viewer(Scene* const scene, QWidget* parent) :
	QGLViewer(parent), scene(scene) {

	this->refreshTimer = new QTimer();
	this->refreshTimer->setInterval(std::chrono::milliseconds(7)); // ~7 ms for 144fps, ~16ms for 60fps and ~33ms for 30 FPS
	this->refreshTimer->setSingleShot(false);
	connect(this->refreshTimer, &QTimer::timeout, this, &Viewer::updateView);

	this->drawVolumetric = true;
	this->shouldCapture = false;
}

Viewer::~Viewer() {
	disconnect(this->refreshTimer, &QTimer::timeout, this, &Viewer::updateView);
	delete this->refreshTimer;
}

void Viewer::init() {
	this->makeCurrent();
	this->setMouseTracking(true);

	this->scene->initGl(this->context());

	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
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

	this->scene->draw3DView(mvMat, pMat, camPos);
}

void Viewer::keyPressEvent(QKeyEvent *e) {
	switch (e->key()) {
		/*
		SHADER PROGRAMS
		*/
		case Qt::Key::Key_F5:
			this->scene->recompileShaders();
			this->update();
		break;
		case Qt::Key::Key_P:
			this->scene->printVAOStateNext();
		break;
		case Qt::Key::Key_V:
			if ((e->modifiers() & Qt::KeyboardModifier::ShiftModifier) != 0) {
				this->scene->setDrawMode(DrawMode::VolumetricBoxed);
			} else {
				this->scene->setDrawMode(DrawMode::Volumetric);
			}
			this->update();
		break;
		case Qt::Key::Key_S:
			if ((e->modifiers() & Qt::KeyboardModifier::ControlModifier) != 0) {
				this->scene->launchSaveDialog();
			} else {
				this->scene->setDrawMode(DrawMode::Solid);
				this->update();
			}
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

void Viewer::centerScene(void) {
	// Don't update the scene radius, cuts off some drawn primitives (like bounding box for example) :
	//float scene_radius = this->scene->getSceneRadius();
	//this->setSceneRadius(qreal(scene_radius));

	glm::vec3 scene_center = this->scene->getSceneCenter();
	qglviewer::Vec cnt(scene_center.x, scene_center.y, scene_center.z);
	this->setSceneCenter(cnt);
	this->camera()->centerScene();
	this->update();
}

void Viewer::mousePressEvent(QMouseEvent* e) {
	QGLViewer::mousePressEvent(e);
}

void Viewer::wheelEvent(QWheelEvent* _w) {
	QFlags keyMods = QGuiApplication::queryKeyboardModifiers();
	if (_w->pixelDelta().y() != 0) {
		if (keyMods.testFlag(Qt::KeyboardModifier::ControlModifier)) {
			std::cerr << "Would zoom instead !\n";
			return;
		}
	}
	QGLViewer::wheelEvent(_w);
	this->update();
}

QString Viewer::helpString() const {
	QString message("<h2>3D   Viewer</h2>");

	message += "<hr/>";

	message += "This viewer allows to view a stack of images issued from a <i>di-SPIM</i> acquisition in 3D.<br/>";
	message += "It has a few different modes of viewing, which are explained in more detail later on :";
	message += "<ul><li>Solid viewing</li><li>Regular volumetric viewing</li><li>Boxed volumetric viewing</li></ul>";
	message += "You can also control the cutting planes (color coded in red, green, and blue) on each axis to show ";
	message += "only a subset of the loaded image stack. You can also invert those planes, and hide them if you wish.";
	message += "<br/>";

	message += "<hr/><br/>";

	message += "<h4>Solid viewing</h4>";
	message += "Renders the acquisition projected onto a cube. This mode does not take a lot of computing power, ";
	message += "and is thus suited for a quick overview of the data. It does not allow to see through the volume, ";
	message += "but instead offers a way to quickly check nothing has gone wrong during the sample acquisition.<br/>";

	message += "<h4>Regular volumetric viewing</h4>";
	message += "Renders the acqusition in a volumetric manner, i.e. as a volume. It allows for the stack of ";
	message += "currently loaded images to be inspected more thouroughly. If some image intensities are not shown ";
	message += "by using the sliders at the bottom of the program, then this rendering mode will not show them. ";
	message += "This allows to see certain structures within the loaded image stack (see for yourself, play with ";
	message += "it !).<br/>";

	message += "<h4>Boxed volumetric viewing</h4>";
	message += "This mode is the same as the more traditional volumetric viewing mode, but only shows the data ";
	message += "contained within a bounding box. You can control this bounding box by clicking on \'File\', ";
	message += "then on \'Show visu box controller\'";
	message += "<br/>";

	message += "<hr/><br/>";
	message += "You can control the minimum and maximum intensities of the images displayed by the viewer using ";
	message += "both sliders located at the bottom of the program. This will have a direct impact on all viewers ";
	message += "currently located on the window.</br>";
	message += "For the 3D viewer, in \'solid\' mode, the intensity values under (resp. over) the user-defined";
	message += "values will simply be set to the minimum (resp. maximum) of those values. In \'regular\'/\'boxed\' ";
	message += "volumetric mode, those user-defined values define which intensities to show and which to discard.";

	return message;
}

QString Viewer::keyboardString() const {
	QString message("");
	message += "<ul>";
		message += "<li>3D viewer :";
			message += "<ul>";
				message += "<li><b>S</b> : set draw mode to \'Solid\'</li>";
				message += "<li><b>V</b> : set draw mode to \'Volumetric\'</li>";
				message += "<li><i>Shift</i>+<b>V</b> : set draw mode to \'Volumetric Boxed\'</li>";
				message += "<li><i>Ctrl</i>+<b>S</b> : Generate a grid, if any are loaded.</li>";
			message += "</ul>";
		message += "</li>";
		message += "<li> Planar viewer(s) :";
			message += "<ul>";
				message += "<li><b>R</b> : Reset size and position to default values</li>";
			message += "</ul>";
		message += "</li>";
		message += "<li>Developper options :";
		message += "<ul><li><b>F5</b> : Reload shaders</li></ul>";
		message += "</li>";
	message += "</ul>";

	return message;
}

QString Viewer::mouseString() const {
	QString message("");
	message += "<ul>";
		message += "<li>3D viewer :";
			message += "<ul>";
				message += "<li><i>Left-click & drag</i> : Rotate around the loaded image stack(s)</li>";
				message += "<li><i>Right-click & drag</i> : Pan the camera</li>";
				message += "<li><i>Scroll up/down</i> : zoom in/out (respectively)</li>";
			message += "</ul>";
		message += "</li>";
		message += "<li> Planar viewer(s) :";
			message += "<ul>";
				message += "<li><i>Right-click & drag</i> : pan the image</li>";
				message += "<li><i>Scroll up/down</i> : zoom in/out (respectively)</li>";
			message += "</ul>";
		message += "</li>";
	message += "</ul>";

	return message;
}

void Viewer::updateCameraPosition() {
}

void Viewer::loadGrid(const std::shared_ptr<InputGrid>& g) {
	this->makeCurrent();
	this->scene->addGrid(g, "");
	this->doneCurrent();

	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	float sceneSize = glm::length(bbDiag);

	this->setSceneRadius(sceneSize*sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
	this->updateCameraPosition();
	this->centerScene();
}

void Viewer::loadTwoGrids(const std::shared_ptr<InputGrid>& g1, const std::shared_ptr<InputGrid>& g2) {
	if (this->scene == nullptr) { return; }
	this->makeCurrent();
	this->scene->addTwoGrids(g1, g2, "");
	this->doneCurrent();

	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	float sceneSize = glm::length(bbDiag);

	this->setSceneRadius(sceneSize*sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
	this->updateCameraPosition();
	this->centerScene();
}
