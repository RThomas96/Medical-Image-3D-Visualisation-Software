#include "../include/planar_viewer.hpp"

#include "../../qt/widgets/include/neighbor_visu_main_widget.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QProgressDialog>

PlanarViewer::PlanarViewer(Scene* const _scene, planes _p, QStatusBar* _sb, planeHeading _h, QWidget* parent) :
	QGLViewer(parent), sceneToShow(_scene), planeToShow(_p), planeOrientation(_h) {
	this->setGridIsDrawn(false);
	this->setAxisIsDrawn(false);
	this->setCameraIsEdited(false);

	this->viewerController = nullptr;
	this->status_bar	   = _sb;

	// Default render texture is not initialized :
	this->renderTarget	  = 0;
	this->minZoomRatio	  = .1f;
	this->zoomRatio		  = 1.f;
	this->maxZoomRatio	  = 500.f;
	this->offset		  = glm::vec2(0.f, 0.f);
	this->mouse_isPressed = false;
	this->ctrl_pressed	  = false;
	this->planeDepth	  = .0f;
	this->posRequest	  = glm::ivec2{-1, -1};
	this->tempOffset	  = glm::vec2{.0f, .0f};

	this->refreshTimer = new QTimer();
	// ~7 ms for 144fps, ~16ms for 60fps and ~33ms for 30 FPS
	this->refreshTimer->setInterval(std::chrono::milliseconds(500));	// 1/2 second when not updated by the viewer
	this->refreshTimer->setSingleShot(false);
	connect(this->refreshTimer, &QTimer::timeout, this, &PlanarViewer::updateView);
}

void PlanarViewer::addParentStatusBar(QStatusBar* main) {
	this->status_bar = main;
}

PlanarViewer::~PlanarViewer(void) {
	// Nothing here yet.
	if (this->viewerController != nullptr) {
		this->viewerController->unregisterPlaneViewer();
	}
	this->refreshTimer->disconnect();
	delete this->refreshTimer;
}

void PlanarViewer::init(void) {
	if (this->sceneToShow == nullptr) {
		throw std::runtime_error("[ERROR] Scene was nullptr when initialized.");
	}
	this->setUpdateBehavior(UpdateBehavior::NoPartialUpdate);

	this->makeCurrent();

	this->sceneToShow->initGl(this->context());

	this->refreshTimer->start();
}

void PlanarViewer::draw(void) {
	float white_shade = 245. / 255.;

	glClearColor(white_shade, white_shade, white_shade, .0);

	QSize viewerSize = this->size();
	glm::vec2 fbDims = glm::vec2(static_cast<float>(viewerSize.width()), static_cast<float>(viewerSize.height()));

	glm::vec2 fullOffset = this->offset + this->tempOffset;
	this->sceneToShow->drawGridMonoPlaneView(fbDims, this->planeToShow, this->planeOrientation, this->zoomRatio, fullOffset);

	if (this->posRequest.x > -1 && this->status_bar != nullptr) {
		glm::vec4 pixelValue = this->sceneToShow->readFramebufferContents(this->defaultFramebufferObject(), this->posRequest);
		if (pixelValue.w > .01f) {
			glm::vec4 p = pixelValue;
			std::cerr << "Value in fbo : {" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << "}\n";
			// TODO : old API
			//auto all_input = this->sceneToShow->getInputGrids();
			//for (const auto& grid : all_input) {
			//	using sizevec3 = glm::vec<3, std::size_t, glm::defaultp>;
			//	sizevec3 index = sizevec3(0, 0, 0);
			//	QString msg = "Message from plane ";
			//	if (this->planeToShow == planes::x) { msg += "X "; }
			//	else if (this->planeToShow == planes::y) { msg += "Y "; }
			//	else if (this->planeToShow == planes::z) { msg += "Z "; }
			//	else { msg += "<unknown> "; }
			//	if (grid->indexFromWorldSpace(p, index)) {
			//		msg += "Index in picture : " + QString::number(index.x) + ", " + QString::number(index.y) + ", " + QString::number(index.z);
			//		this->statusbar->showMessage(msg, 5000);
			//	}
			//}
		}
		this->posRequest = glm::vec2{-1, -1};
	}
}

void PlanarViewer::guessScenePosition(void) {
	// Get framebuffer size, and (current) relative mouse position to the widget origin :
	QSize wSize			   = this->size();
	glm::ivec2 fbDims	   = glm::ivec2(wSize.width(), wSize.height());
	glm::ivec2 rawMousePos = glm::ivec2(this->cursorPosition_current.x(), this->cursorPosition_current.y());
	// If already outside of the framebuffer, return and do nothing :
	if (rawMousePos.x < 0 || rawMousePos.x > fbDims.x || rawMousePos.y < 0 || rawMousePos.y > fbDims.y) {
		return;
	}

	// OpenGL FBO coordinates have their 0 at the bottom left. Flip the
	// Y coordinate so it reflects the OpenGL framebuffer coordinates :
	this->posRequest = glm::convert_to<int>(glm::vec2(rawMousePos.x, fbDims.y - rawMousePos.y));
	this->makeCurrent();
	glm::vec4 pixelValue = this->sceneToShow->readFramebufferContents(this->defaultFramebufferObject(), this->posRequest);
	if (pixelValue.w > .01f) {
		glm::vec4 p = pixelValue;
		std::cerr << "Value in fbo : {" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << "}\n";
		this->sceneToShow->setPositionResponse(pixelValue);
		// TODO: new API
		// auto inputs = this->sceneToShow->getInputGrids();
		// for (const auto& grid : inputs) {
		// 	if (grid->includesPointWorldSpace(pixelValue)) {
		// 		IO::GenericGridReader::sizevec3 index = grid->worldPositionToIndex(p);
		// 		QString msg							  = "Position in image space : " + QString::number(index.x) + ", " +
		// 					  QString::number(index.y) + ", " + QString::number(index.z) + ", in grid " +
		// 					  QString::fromStdString(grid->getGridName());
		// 		std::cerr << "Message from plane viewer : " << msg.toStdString() << "\n";
		// 		this->status_bar->showMessage(msg, 10000);
		// 	}
		// }
	}
	this->doneCurrent();
}

void PlanarViewer::keyPressEvent(QKeyEvent* _e) {
	switch (_e->key()) {
		/*
		SHADER PROGRAMS
		*/
		case Qt::Key::Key_F5:
			this->sceneToShow->recompileShaders();
			this->update();
			break;
		case Qt::Key::Key_P:
			this->sceneToShow->printVAOStateNext();
			this->update();
			break;

		/*
		MOUSE MOVEMENT
		*/
		case Qt::Key::Key_R:
			if (not this->mouse_isPressed) {
				this->offset	= glm::vec2(.0, .0);
				this->zoomRatio = 1.f;
			}
			this->update();
			break;
		default:
			QGLViewer::keyPressEvent(_e);
			this->update();
			break;
	}
}

void PlanarViewer::mousePressEvent(QMouseEvent* _e) {
	// update both positions to the same position (interaction began) :
	this->cursorPosition_last	 = _e->pos();
	this->cursorPosition_current = this->cursorPosition_last;
	if (_e->buttons().testFlag(Qt::MouseButton::LeftButton)) {
		// Start "tracking" the mouse
		this->mouse_isPressed = 1;
	}
	if (_e->buttons().testFlag(Qt::MouseButton::RightButton)) {
		this->guessScenePosition();
	}

	QGLViewer::mousePressEvent(_e);
	this->update();
}

void PlanarViewer::mouseMoveEvent(QMouseEvent* _m) {
	if (this->mouse_isPressed >= 1u) {
		// Gather current viewport dimensions :
		QSize viewerSize	   = this->size();
		glm::vec2 viewportSize = glm::convert_to<float>(glm::ivec2(viewerSize.width(), viewerSize.height()));
		// Gather current mouse coordinates, relative to the viewer's origin point :
		QPoint currentPos			 = _m->pos();
		glm::vec2 mousePosAbs		 = glm::convert_to<float>(glm::ivec2(currentPos.x(), currentPos.y()));
		glm::vec2 mousePosLast		 = glm::convert_to<float>(glm::ivec2(this->cursorPosition_last.x(), this->cursorPosition_last.y()));
		glm::vec2 mousePosNormalized = (mousePosAbs - mousePosLast) / viewportSize;
		// OpenGL NDC are in [-1; 1], so :
		this->tempOffset = mousePosNormalized * 2.f;
		this->mouse_isPressed += 1u;
	}
	this->cursorPosition_current = _m->pos();
	if (_m->buttons().testFlag(Qt::MouseButton::RightButton)) {
		this->guessScenePosition();
	}
	QGLViewer::mouseMoveEvent(_m);
	return;
}

void PlanarViewer::mouseReleaseEvent(QMouseEvent* _m) {
	// If left button is not pressed anymore :
	if (_m->buttons().testFlag(Qt::MouseButton::LeftButton) == false) {
		this->mouse_isPressed = 0;
		this->offset += this->tempOffset;
		// Reset temp offset :
		this->tempOffset			 = glm::vec2{.0f, .0f};
		this->cursorPosition_last	 = _m->pos();
		this->cursorPosition_current = this->cursorPosition_last;
	}
	QGLViewer::mouseReleaseEvent(_m);
	this->update();
}

void PlanarViewer::wheelEvent(QWheelEvent* _w) {
	if (_w->angleDelta().y() > 0) {
		this->zoomRatio *= 1.1;
	} else {
		this->zoomRatio *= .9;
	}

	if (this->zoomRatio > this->maxZoomRatio) {
		this->zoomRatio = this->maxZoomRatio;
	}
	if (this->zoomRatio < this->minZoomRatio) {
		this->zoomRatio = this->minZoomRatio;
	}

	QGLViewer::wheelEvent(_w);
	this->update();
}

void PlanarViewer::resizeGL(int w, int h) {
	// First, call the superclass' function
	QGLViewer::resizeGL(w, h);

	// Is the scene initialized ? (might not on first call to this function)
	if (this->sceneToShow->isSceneInitialized()) {
		this->renderTarget = this->sceneToShow->updateFBOOutputs(glm::ivec2{w, h},
		  this->defaultFramebufferObject(),
		  this->renderTarget);
	}
}

void PlanarViewer::setController(ViewerHeader* _header) {
	this->viewerController = _header;
}

void PlanarViewer::updateView() {
	// Done because for some reason we coundn't connect the signal
	// from the timer's timeout to the update slot directly. Dumb.
	this->update();
}

void PlanarViewer::updatePlaneDepth(int newVal) {
	float scalar	 = static_cast<float>(newVal) / 1000.f;
	this->planeDepth = scalar;
	if (this->planeToShow == planes::x) {
		this->sceneToShow->slotSetPlaneDisplacementX(scalar);
	}
	if (this->planeToShow == planes::y) {
		this->sceneToShow->slotSetPlaneDisplacementY(scalar);
	}
	if (this->planeToShow == planes::z) {
		this->sceneToShow->slotSetPlaneDisplacementZ(scalar);
	}
	this->update();
}

void PlanarViewer::flipPlaneDirection() {
	// The plane inversion doesn't happen here (only visible in 3D view) however the buttons to control it
	// are contained in the headers of the planar viewers. Thus, it must be placed here.
	if (this->planeToShow == planes::x) {
		this->sceneToShow->slotTogglePlaneDirectionX();
	}
	if (this->planeToShow == planes::y) {
		this->sceneToShow->slotTogglePlaneDirectionY();
	}
	if (this->planeToShow == planes::z) {
		this->sceneToShow->slotTogglePlaneDirectionZ();
	}
	this->update();
}

void PlanarViewer::rotatePlaneClockwise() {
	if (this->planeOrientation == planeHeading::North) {
		this->planeOrientation = planeHeading::East;
	} else if (this->planeOrientation == planeHeading::East) {
		this->planeOrientation = planeHeading::South;
	} else if (this->planeOrientation == planeHeading::South) {
		this->planeOrientation = planeHeading::West;
	} else if (this->planeOrientation == planeHeading::West) {
		this->planeOrientation = planeHeading::North;
	}
	this->sceneToShow->setPlaneHeading(this->planeToShow, this->planeOrientation);
	this->update();
}

void PlanarViewer::rotatePlaneCounterClockwise() {
	if (this->planeOrientation == planeHeading::North) {
		this->planeOrientation = planeHeading::West;
	} else if (this->planeOrientation == planeHeading::West) {
		this->planeOrientation = planeHeading::South;
	} else if (this->planeOrientation == planeHeading::South) {
		this->planeOrientation = planeHeading::East;
	} else if (this->planeOrientation == planeHeading::East) {
		this->planeOrientation = planeHeading::North;
	}
	this->sceneToShow->setPlaneHeading(this->planeToShow, this->planeOrientation);
	this->update();
}

void PlanarViewer::togglePlaneVisibility() {
	this->sceneToShow->togglePlaneVisibility(this->planeToShow);
}

QString PlanarViewer::helpString() const {
	QString message("<h2>P l a n a r   v i e w e r</h2>");

	return message;
}

QString PlanarViewer::keyboardString() const {
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
	message += "<ul><li><b>F5</b> : Reload shaders (from any viewer)</li></ul>";
	message += "</li>";
	message += "</ul>";

	return message;
}

QString PlanarViewer::mouseString() const {
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
