#include "../include/neighbor_visu_viewer.hpp"
#include "../../features.hpp"
//#include "../../grid/include/discrete_grid_writer.hpp"
#include "../../qt/widgets/include/user_settings_widget.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>

#include <QGLViewer/manipulatedFrame.h>

#include <QCoreApplication>
#include <QFileDialog>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QProgressDialog>

#include <fstream>

Viewer::Viewer(Scene* const scene, QStatusBar* _program_bar, QWidget* parent) :
	QGLViewer(parent), scene(scene) {
	this->statusBar	   = _program_bar;
	this->refreshTimer = new QTimer();
	this->refreshTimer->setInterval(std::chrono::milliseconds(7));	  // ~7 ms for 144fps, ~16ms for 60fps and ~33ms for 30 FPS
	this->refreshTimer->setSingleShot(false);
	connect(this->refreshTimer, &QTimer::timeout, this, &Viewer::updateView);
    //connect(this, &QGLViewer::interpolated, this, [this](){std::cout << "I MOVE !" << std::endl;});


	this->drawVolumetric	= true;
	this->shouldCapture		= false;
	this->renderTarget		= 0;
	this->selectMode		= false;
	this->fbSize			= glm::ivec2{0, 0};
	this->cursorPos_current = glm::ivec2{0, 0};
	this->cursorPos_last	= glm::ivec2{0, 0};
	this->framesHeld		= 0;
	this->posRequest		= glm::ivec2{-1, -1};
	this->drawAxisOnTop		= false;

	// Setup the alt key binding to move an object
	setMouseBinding(Qt::AltModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::ROTATE);
	setMouseBinding(Qt::AltModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
	setMouseBinding(Qt::AltModifier, Qt::MidButton, QGLViewer::FRAME, QGLViewer::ZOOM);

    setShortcut(QGLViewer::STEREO, Qt::ALT+Qt::Key_M);
	//setWheelBinding(Qt::AltModifier, QGLViewer::FRAME, QGLViewer::ZOOM);

	//setManipulatedFrame(&this->meshManipulator.getActiveManipulator().getManipulatedFrame());
	//updateManipulatorsPositions();
	//this->scene->glMeshManipulator->bind(&this->meshManipulator);
	//this->scene->bindMeshManipulator(&this->meshManipulator);
    this->cursor = new QCursor();
    this->setCursor(*this->cursor);
    this->setFPSIsDisplayed(true);
}

Viewer::~Viewer() {
	disconnect(this->refreshTimer, &QTimer::timeout, this, &Viewer::updateView);
	delete this->refreshTimer;
}

void Viewer::init() {
	this->makeCurrent();
	this->setMouseTracking(true);

	this->scene->initGl(this->context());

    QObject::connect(this, &Viewer::keyPressed, this->scene, &Scene::keyPressed);
    QObject::connect(this, &Viewer::keyReleased, this->scene, &Scene::keyReleased);
    QObject::connect(this, &Viewer::mousePressed, this->scene, &Scene::mousePressed);
    QObject::connect(this, &Viewer::mouseReleased, this->scene, &Scene::mouseReleased);

    QObject::connect(this, &Viewer::sceneRadiusChanged, this->scene, &Scene::changeSceneRadius);

    QObject::connect(this->scene, &Scene::sceneCenterChanged, this, &Viewer::setCenter);
    QObject::connect(this->scene, &Scene::sceneRadiusChanged, this, &Viewer::setRadius);

    QObject::connect(this->scene, &Scene::cursorChanged, this, &Viewer::setCursorType);

    //QObject::connect(this->scene, &Scene::needRedraw3DScene, this, &Viewer::draw);

	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	float sceneSize	 = glm::length(bbDiag);

	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x / 2., bbDiag.y / 2., bbDiag.z / 2.));
	this->showEntireScene();

	this->refreshTimer->start();	// Update every 'n' milliseconds from here on out

    this->scene->init();
    Q_EMIT sceneRadiusChanged(this->camera()->distanceToSceneCenter());
}

void Viewer::addStatusBar(QStatusBar* bar) {
	this->statusBar = bar;
}

void Viewer::draw() {
	GLfloat mvMat[16];
	GLfloat pMat[16];

	float white_shade = 245. / 255.;

	glClearColor(white_shade, white_shade, white_shade, .0);

	this->camera()->getModelViewMatrix(mvMat);
	this->camera()->getProjectionMatrix(pMat);

	qglviewer::Vec cam = this->camera()->worldCoordinatesOf(qglviewer::Vec(0., 0., 0.));
	glm::vec3 camPos   = glm::vec3(static_cast<float>(cam.x), static_cast<float>(cam.y), static_cast<float>(cam.z));

	this->scene->draw3DView(mvMat, pMat, camPos, false);
	this->scene->drawPositionResponse(this->sceneRadius() / 10., this->drawAxisOnTop);
}

void Viewer::keyReleaseEvent(QKeyEvent* e) {
	QGLViewer::keyReleaseEvent(e);
    Q_EMIT keyReleased(e);
}

void Viewer::keyPressEvent(QKeyEvent* e) {
	switch (e->key()) {
        case Qt::Key::Key_Q:
            if(!e->isAutoRepeat())
                this->castRay();
            break;
		case Qt::Key::Key_F5:
			this->scene->recompileShaders();
			this->update();
			break;
	}
	QGLViewer::keyPressEvent(e);
    Q_EMIT keyPressed(e);
}

void Viewer::mousePressEvent(QMouseEvent* e) {
	QGLViewer::mousePressEvent(e);
    Q_EMIT mousePressed(e);
}

void Viewer::mouseMoveEvent(QMouseEvent* e) {
	QGLViewer::mouseMoveEvent(e);
    this->mousePos = e->pos();
}

void Viewer::mouseReleaseEvent(QMouseEvent* e) {
	QGLViewer::mouseReleaseEvent(e);
    Q_EMIT mouseReleased(e);
}

void Viewer::wheelEvent(QWheelEvent* _w) {
	QGLViewer::wheelEvent(_w);
	this->update();
    Q_EMIT sceneRadiusChanged(this->camera()->distanceToSceneCenter());
}

void Viewer::resizeGL(int w, int h) {
	// First, call the superclass' function
	QGLViewer::resizeGL(w, h);

	this->fbSize = glm::ivec2{w, h};

	// Is the scene initialized ? (might not on first call to this function)
	if (this->scene->isSceneInitialized()) {
		// Update the texture accompanying the framebuffer to reflect its size change
		this->renderTarget = this->scene->updateFBOOutputs(this->fbSize,
		  this->defaultFramebufferObject(),
		  this->renderTarget);
	}
}

void Viewer::castRay() {
    glm::vec3 origin;
    glm::vec3 direction;
    this->castRayFromMouse(origin, direction);
    emit this->scene->rayIsCasted(origin, direction);
}

void Viewer::castRayFromMouse(glm::vec3& origin, glm::vec3& direction) {
    qglviewer::Vec originVec;
    qglviewer::Vec directionVec;
    this->camera()->convertClickToLine(this->mousePos, originVec, directionVec); 
    origin = glm::vec3(originVec.x, originVec.y, originVec.z);
    direction = glm::vec3(directionVec.x, directionVec.y, directionVec.z);
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

void Viewer::newAPI_loadGrid(const std::string& name, const std::vector<std::string>& filenames, const std::string& tetMeshFileName, const int subsample, const glm::vec3& sizeTetmesh, glm::vec3& sizeVoxel, const std::pair<glm::vec3, glm::vec3>& bbox) {
	if (this->scene == nullptr) {
		return;
	}
	this->makeCurrent();
    if(tetMeshFileName.empty()) {
	    this->scene->openGrid(name, filenames, subsample, sizeVoxel, sizeTetmesh);
    } else {
	    this->scene->openGrid(name, filenames, subsample, tetMeshFileName);
    }
	this->doneCurrent();
}

void Viewer::setCenter(const glm::vec3& center) {
	this->setSceneCenter(qglviewer::Vec(center[0], center[1], center[2]));
    std::cout << "Set center" << std::endl;
    //this->showEntireScene();
}

void Viewer::setRadius(const float radius) {
    this->setSceneRadius(radius);
    this->showEntireScene();
    this->scene->distanceFromCamera = this->camera()->distanceToSceneCenter();
}

void Viewer::setCursorType(UITool::CursorType cursorType) {
    switch(cursorType) {
        case UITool::CursorType::NORMAL:
            this->cursor->setShape(Qt::ArrowCursor); 
            this->setCursor(*this->cursor);
            break;

        case UITool::CursorType::CROSS:
            this->cursor->setShape(Qt::CrossCursor); 
            this->setCursor(*this->cursor);
            break;
    }
}
