#include "../include/neighbor_visu_viewer.hpp"
#include "../../features.hpp"
#include "../../grid/include/discrete_grid_writer.hpp"
#include "../../qt/include/user_settings_widget.hpp"

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

float Viewer::sceneRadiusMultiplier{.5f};

Viewer::Viewer(Scene* const scene, QStatusBar* _program_bar, QWidget* parent) :
	QGLViewer(parent), scene(scene), meshManipulator(UITool::MeshManipulator(36)) {
	this->statusBar	   = _program_bar;
	this->refreshTimer = new QTimer();
	this->refreshTimer->setInterval(std::chrono::milliseconds(7));	  // ~7 ms for 144fps, ~16ms for 60fps and ~33ms for 30 FPS
	this->refreshTimer->setSingleShot(false);
	connect(this->refreshTimer, &QTimer::timeout, this, &Viewer::updateView);

	this->renderTarget		= 0;
	this->selectMode		= false;
	this->fbSize			= glm::ivec2{0, 0};
	this->cursorPos_current = glm::ivec2{0, 0};
	this->framesHeld		= 0;
	this->posRequest		= glm::ivec2{-1, -1};
	this->drawAxisOnTop		= false;

	this->deformation_enabled = false;

	// Setup the alt key binding to move an object
	setMouseBinding(Qt::AltModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::ROTATE);
	setMouseBinding(Qt::AltModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
	setMouseBinding(Qt::AltModifier, Qt::MidButton, QGLViewer::FRAME, QGLViewer::ZOOM);
	//setWheelBinding(Qt::AltModifier, QGLViewer::FRAME, QGLViewer::ZOOM);

	//setManipulatedFrame(&this->meshManipulator.getActiveManipulator().getManipulatedFrame());
	updateManipulatorsPositions();
	//this->scene->glMeshManipulator->bind(&this->meshManipulator);
	this->scene->bindMeshManipulator(&this->meshManipulator);
}

void Viewer::updateManipulatorsPositions() {
	std::vector<glm::vec3> vecpos;
	scene->getTetraMeshPoints(vecpos);
	this->meshManipulator.setAllPositions(vecpos);
}

Viewer::~Viewer() {
	disconnect(this->refreshTimer, &QTimer::timeout, this, &Viewer::updateView);
	delete this->refreshTimer;
}

void Viewer::init() {
	this->makeCurrent();
	this->setMouseTracking(true);

	this->scene->initGl(this->context());
	this->scene->setViewer(this);

	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	float sceneSize	 = glm::length(bbDiag);

	this->setSceneRadius(sceneSize * sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x / 2., bbDiag.y / 2., bbDiag.z / 2.));
	this->showEntireScene();
	this->refreshTimer->start();	// Update every 'n' milliseconds from here on out
	emit hasInitializedScene();

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
	this->scene->drawPositionResponse(this->sceneRadius() / 10., false);

	if (this->meshManipulator.isActiveManipulatorManipuled()) {
		this->scene->launchDeformation(this->meshManipulator.getActiveManipulatorAssignedIdx(), this->meshManipulator.getActiveManipulatorPos());
	}
}

void Viewer::enableControlPanel(bool should_enable) {
	emit this->enableImageControl(should_enable);
}

void Viewer::updateSceneTextureLimits() {
	emit this->overrideTextureLimits(this->scene->getMinTexValue(), this->scene->getMaxTexValue());
}

void Viewer::toggleSelectionMode() {
	QString msg = "";
	this->selectMode = not this->selectMode;
	msg				 = "Turned selection mode " + (this->selectMode ? QString("on") : QString("off"));
	this->statusBar->showMessage(msg, 5000);
	this->update();
}

void Viewer::printVAOStateNext() {
	this->scene->printVAOStateNext();
}

void Viewer::keyPressEvent(QKeyEvent* e) {
	// 'msg' allocated here not to have curly braces in
	// all case statements that need to show a message:
	QString msg = "";

	switch (e->key()) {
		/*
		VIEWER BEHAVIOUR
		*/
		case Qt::Key::Key_R:
			this->updateInfoFromScene();
			break;
		case Qt::Key::Key_T:
			this->drawAxisOnTop = not this->drawAxisOnTop;
			break;
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

		case Qt::Key::Key_G:
			this->scene->draft_tryAndSaveFirstGrid();
			this->update();
			break;
		case Qt::Key::Key_Space:
			// Done here in order to prevent a segfault.
			std::cerr << "Segfault avoided.\n";
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

glm::vec4 Viewer::readPositionFromFramebuffer() {
	glm::ivec2 rawMousePos = this->cursorPos_current;
	if (rawMousePos.x < 0 || rawMousePos.x > this->fbSize.x || rawMousePos.y < 0 || rawMousePos.y > this->fbSize.y) {
		return glm::vec4{-1.f, -1.f, -1.f, -1.f};
	}
	this->makeCurrent();
	this->posRequest = glm::ivec2(rawMousePos.x, this->fbSize.y - rawMousePos.y);
	glm::vec4 p = this->scene->readFramebufferContents(this->defaultFramebufferObject(), this->posRequest);
	this->doneCurrent();
	return p;
}

void Viewer::mousePressEvent(QMouseEvent* e) {
	this->cursorPos_current = glm::ivec2{e->pos().x(), e->pos().y()};

	// Ctrl+Shift+LClick is a selection in 3D mode, act accordingly :
	auto ctrl_shift = Qt::KeyboardModifier::ShiftModifier | Qt::KeyboardModifier::ControlModifier;
	if (not this->deformation_enabled && e->button() == Qt::MouseButton::LeftButton && (e->modifiers() & (ctrl_shift)) == (ctrl_shift)) {
		this->framesHeld = 1;
		this->guessMousePosition();
		e->accept();	// stop the event from propagating further !
		return;
	}

	// If not in select mode, process the event normally :
	QGLViewer::mousePressEvent(e);
}

void Viewer::mouseMoveEvent(QMouseEvent* e) {
	// If tracking the mouse, update its position :
	if (this->framesHeld > 0) {
		this->cursorPos_current = glm::ivec2{e->pos().x(), e->pos().y()};
		this->framesHeld += 1;
		this->guessMousePosition();
		e->accept();
		return;
	}
	QGLViewer::mouseMoveEvent(e);
}

void Viewer::mouseReleaseEvent(QMouseEvent* e) {
	if (not e->buttons().testFlag(Qt::MouseButton::RightButton)) {
		this->framesHeld = 0;
	}

	QGLViewer::mouseReleaseEvent(e);
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

void Viewer::guessMousePosition() {
	glm::vec4 p = this->readPositionFromFramebuffer();
	if (p.w > .01f) {
		this->scene->setPositionResponse(p);
		std::cerr << "[3D Viewer] FBO position contents : " << p << '\n';
		// TODO: new API
		//auto inputs = this->scene->getInputGrids();
		//for (const auto& grid : inputs) {
		//	if (grid->includesPointWorldSpace(p)) {
		//		IO::GenericGridReader::sizevec3 index = grid->worldPositionToIndex(p);
		//		QString msg							  = "Position in image space : " + QString::number(index.x) + ", " +
		//					  QString::number(index.y) + ", " + QString::number(index.z) + ", in grid " +
		//					  QString::fromStdString(grid->getGridName());
		//		this->statusBar->showMessage(msg, 10000);
		//	}
		//}
		std::function<void(const NewAPI_GridGLView::Ptr&)> findSuitablePoint =
		  [this, p](const NewAPI_GridGLView::Ptr& gridView) -> void {
			const Image::Grid::Ptr grid		  = gridView->grid;
			TransformStack::Ptr gridTransform = grid->getTransformStack();
			BoundingBox_General<float> bb	  = grid->getBoundingBox();
			glm::vec4 p_prime				  = gridTransform->to_image(p);
			if (bb.contains(p_prime)) {
				glm::vec3 voxdim			  = grid->getVoxelDimensions();
				glm::tvec3<std::size_t> index = p_prime / glm::vec4(voxdim, 1.f);
				QString msg					  = "Position in image space : " + QString::number(index.x) + ", " +
							  QString::number(index.y) + ", " + QString::number(index.z) + ", in grid " +
							  QString::fromStdString(grid->getImageName());
				this->statusBar->showMessage(msg, 10000);
			}
		};
		this->scene->lambdaOnGrids(findSuitablePoint);
	}
}

void Viewer::resetLocalPointQuery() {
	this->posRequest = glm::ivec2{-1, -1};
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
	auto bb		= this->scene->getSceneBoundingBox();
	auto center = bb.getMin() + (bb.getDiagonal() / 2.f);
	auto radius = glm::length(bb.getDiagonal());
	this->setSceneCenter(qglviewer::Vec(center.x, center.y, center.z));
	this->setSceneRadius(radius * sceneRadiusMultiplier);
	Mesh::Ptr mesh = nullptr;
	this->showEntireScene();
}

void Viewer::updateInfoFromScene() {
	this->updateCameraPosition();
}

void Viewer::newAPI_loadGrid(Image::Grid::Ptr ptr) {
	if (this->scene == nullptr) {
		return;
	}
	this->makeCurrent();
	this->scene->newAPI_addGrid(ptr);
	this->updateManipulatorsPositions();
	this->scene->prepareManipulators();
	this->doneCurrent();

	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	float sceneSize	 = glm::length(bbDiag);

	this->setSceneRadius(sceneSize * sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x / 2., bbDiag.y / 2., bbDiag.z / 2.));
	this->showEntireScene();
	this->updateCameraPosition();
	this->centerScene();
}

void Viewer::toggleManipulators() {
	this->meshManipulator.toggleActivation();
}
