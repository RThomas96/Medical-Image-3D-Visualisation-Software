#include "../include/planar_viewer.hpp"

#include <QCoreApplication>
#include <QProgressDialog>
#include <QMouseEvent>
#include <QGuiApplication>

PlanarViewer::PlanarViewer(Scene* const _scene, planes _p, planeHeading _h, QWidget* parent) :
		QGLViewer(parent), sceneToShow(_scene), planeToShow(_p), planeOrientation(_h) {
	this->setGridIsDrawn(false);
	this->setAxisIsDrawn(false);
	this->setCameraIsEdited(false);

	this->viewerController = nullptr;

	// Default render texture is not initialized :
	this->renderTarget = 0;
	this->renderTargetCopy = 0;
	this->minZoomRatio = .1f;
	this->zoomRatio = 1.f;
	this->maxZoomRatio = 500.f;
	this->offset = glm::vec2(0.f, 0.f);
	this->mouse_isPressed = false;
	this->ctrl_pressed = false;
	this->planeDepth = -1.f;

	this->refreshTimer = new QTimer();
	// ~7 ms for 144fps, ~16ms for 60fps and ~33ms for 30 FPS
	this->refreshTimer->setInterval(std::chrono::milliseconds(500)); // 1/2 second when not updated by the viewer
	this->refreshTimer->setSingleShot(false);
	connect(this->refreshTimer, &QTimer::timeout, this, &PlanarViewer::updateView);
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
	glClearColor(.8, .8, .8, 1.);

	QSize viewerSize = this->size();
	glm::vec2 fbDims = glm::vec2(static_cast<float>(viewerSize.width()), static_cast<float>(viewerSize.height()));

	glm::vec4 p = this->sceneToShow->drawPlaneView(fbDims, this->planeToShow, this->planeOrientation, this->zoomRatio, this->offset, this->renderTargetCopy, this->posRequest);

	if (this->posRequest.x > -1) {
		std::cerr << "Value from scene : {" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << "}\n";
	}
	this->posRequest = glm::vec2{-1, -1};
}

void PlanarViewer::guessScenePosition(void) {
	// If the plane depth is invalid, return :
	if (this->planeDepth < .0f) { return; }

	// Get framebuffer size, and (current) relative mouse position to the widget origin :
	QSize wSize = this->size();
	glm::vec2 fbDims = glm::convert_to<float>(glm::ivec2(wSize.width(), wSize.height()));
	glm::vec2 rawMousePos = glm::convert_to<float>(glm::ivec2(
								this->cursorPosition_current.x(),
								this->cursorPosition_current.y()
							));
	// If already outside of the framebuffer, return and do nothing :
	if (rawMousePos.x < .0f || rawMousePos.x > fbDims.x || rawMousePos.y < .0f || rawMousePos.y > fbDims.y) {
		return;
	}

	std::cerr << "[LOG] Framebuffer size   : [" << fbDims.x << ", " << fbDims.y << "]\n";
	std::cerr << "[LOG] Raw mouse position : [" << rawMousePos.x << ", " << rawMousePos.y << "]\n";

	// OpenGL NDC coordinates have their 0 at the center, negative coordinates
	// on the left and bottom and positive ones on the top and right. Flip the
	// mouse position, normalize it and scale to [-1; 1] :
	glm::vec2 flippedMousePos = glm::vec2(rawMousePos.x, fbDims.y - rawMousePos.y);	// Flip on Y
	this->posRequest = flippedMousePos;
	/*
	glm::vec2 normalizedMousePos = (flippedMousePos / fbDims) * 2.f;				// Normalize
	glm::vec2 ndcMousePos = normalizedMousePos - glm::vec2{1.f, 1.f};				// Set in [-1;1]

	std::cerr << "[LOG] NDC mouse position : [" << ndcMousePos.x << ", " << ndcMousePos.y << "]\n";

	// Get the bounding box size to use for this particular plane :
	DiscreteGrid::bbox_t sceneBox = this->sceneToShow->getSceneBoundingBox();
	glm::vec3 bbSceneDims = glm::convert_to<float>(sceneBox.getDiagonal());

	// Determine the dimensions of the plane to show :
	glm::vec2 bbDims{};
	if (this->planeToShow == planes::x) {
		std::cerr << "[LOG] Plane shown : X\n";
		bbDims.x = bbSceneDims.y;
		bbDims.y = bbSceneDims.z;
	} else if (this->planeToShow == planes::y) {
		std::cerr << "[LOG] Plane shown : Y\n";
		bbDims.x = bbSceneDims.x;
		bbDims.y = bbSceneDims.z;
	} else if (this->planeToShow == planes::z) {
		std::cerr << "[LOG] Plane shown : Z\n";
		bbDims.x = bbSceneDims.x;
		bbDims.y = bbSceneDims.y;
	} else {
		std::cerr << "[ERROR] Cannot guess plane position of plane other than X, Y or Z\n";
		return;
	}

	if (this->planeOrientation != planeHeading::Right && this->planeOrientation != planeHeading::Left) {
		std::cerr << "[LOG] Swapping plane dimensions relative to orientation ...\n";
		bbDims = glm::vec2(bbDims.y, bbDims.x);
	}

	std::cerr << "[LOG] Bouding box size : {" << bbDims.x << ", " << bbDims.y << "}\n";

	// The plane multiplier :
	glm::vec2 multiplier{1., 1.};
	// The ratios of the framebuffer, and of the bounding box :
	float fbRatio = fbDims.x / fbDims.y;
	float bbRatio = bbDims.x / bbDims.y;

	// Guess the ratio used to 'compress' the plane to fit the bb in the frame :
	if (bbRatio > fbRatio) {
		multiplier.y = fbRatio / bbRatio;
	} else {
		float fbRatioInv = fbDims.y / fbDims.x;
		float bbRatioInv = bbDims.y / bbDims.x;
		multiplier.x = fbRatioInv / bbRatioInv;
	}

	// Multiplier represents how much of the current frame buffer the projected
	// quad takes.
	std::cerr << "[LOG] Multiplier computed : {" << multiplier.x << ", " << multiplier.y << "}\n";

	// Simulate the fact the plane will be shrunk by 'multiplier' and centered :
	glm::vec2 bbPosInFb = glm::vec2{-1.f,-1.f} * multiplier;
	glm::vec2 bbPosInFbMax = glm::vec2{1.f,1.f} * multiplier;
	glm::vec2 bbSizeInFb = bbPosInFbMax - bbPosInFb;

	std::cerr << "[LOG] Bounding box size in framebuffer : {" << bbSizeInFb.x << ", " << bbSizeInFb.y << "}\n";
	std::cerr << "[LOG] Bounding box plane coordinates in NDC : {" << bbPosInFb.x << ", " << bbPosInFb.y << "}\n";
	std::cerr << "[LOG] Bounding box plane coordinates in NDC : {" << bbPosInFbMax.x << ", " << bbPosInFbMax.y << "}\n";
	if (ndcMousePos.x > bbPosInFb.x && ndcMousePos.x < bbPosInFbMax.x &&
			ndcMousePos.y > bbPosInFb.x && ndcMousePos.y < bbPosInFbMax.y) {
		glm::vec2 relativeBBPos = (ndcMousePos - bbPosInFb) / bbSizeInFb;
		glm::vec2 realBBPos = bbDims * relativeBBPos;
		std::cerr << "[LOG] Found a point. Normalized coordinates : {" << relativeBBPos.x << ", " << relativeBBPos.y << "}\n";
		std::cerr << "[LOG] Found a point. Real coordinates : {" << realBBPos.x << ", " << realBBPos.y << "}\n";
	}
	std::cerr << '\n';
	*/
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
				this->offset = glm::vec2(.0, .0);
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
	if (_e->buttons().testFlag(Qt::MouseButton::LeftButton)) {
		this->cursorPosition_last = _e->pos();
		this->cursorPosition_current = this->cursorPosition_last;
		this->mouse_isPressed = 1;
	}
	if (_e->buttons().testFlag(Qt::MouseButton::RightButton) && !_e->buttons().testFlag(Qt::MouseButton::LeftButton)) {
		this->cursorPosition_last = _e->pos();
		this->cursorPosition_current = this->cursorPosition_last;
		this->guessScenePosition();
	}

	QGLViewer::mousePressEvent(_e);
	this->update();
}

void PlanarViewer::mouseMoveEvent(QMouseEvent* _m) {
	if (this->mouse_isPressed >= 1u) {
		QPoint currentPos = _m->pos();
		QSize viewerSize = this->size();
		QPoint viewerPos = this->pos();
		glm::vec2 minViewer = glm::convert_to<float>(glm::ivec2(viewerPos.x(), viewerPos.y()));
		glm::vec2 maxViewer = minViewer + glm::convert_to<float>(glm::ivec2(viewerSize.width(), viewerSize.height()));
		// absolute positions of the mouse in last pos and current pos :
		glm::vec2 absPosMouse = glm::convert_to<float>(glm::ivec2(currentPos.x(), currentPos.y()));
		glm::vec2 absPosLastPos = glm::convert_to<float>(glm::ivec2(this->cursorPosition_last.x(), this->cursorPosition_last.y()));
		this->offset += (absPosMouse - absPosLastPos) / (maxViewer - minViewer);
		this->cursorPosition_last = this->cursorPosition_current;
		this->cursorPosition_current = currentPos;
		this->mouse_isPressed += 1u;
	}
	QGLViewer::mouseMoveEvent(_m);
	return;
}

void PlanarViewer::mouseReleaseEvent(QMouseEvent* _m) {
	// If left button is not pressed anymore :
	if (_m->buttons().testFlag(Qt::MouseButton::LeftButton) == false) {
		this->mouse_isPressed = 0;
		this->cursorPosition_last = _m->pos();
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

	if (this->zoomRatio > this->maxZoomRatio) { this->zoomRatio = this->maxZoomRatio; }
	if (this->zoomRatio < this->minZoomRatio) { this->zoomRatio = this->minZoomRatio; }

	QGLViewer::wheelEvent(_w);
	this->update();
}

void PlanarViewer::resizeGL(int w, int h) {
	// First, call the superclass' function
	QGLViewer::resizeGL(w,h);

	// Is the scene initialized ? (might not on first call to this function)
	if (this->sceneToShow->isSceneInitialized()) {
		glm::ivec2 s{w,h}; // framebuffer size
		this->renderTarget = this->sceneToShow->createRenderTexture(s, this->defaultFramebufferObject(), this->renderTarget);
		this->renderTargetCopy = this->sceneToShow->createRenderTextureCopy(s, this->renderTargetCopy);
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
	float scalar = static_cast<float>(newVal) / 1000.f;
	this->planeDepth = scalar;
	if (this->planeToShow == planes::x) { this->sceneToShow->slotSetPlaneDisplacementX(scalar); }
	if (this->planeToShow == planes::y) { this->sceneToShow->slotSetPlaneDisplacementY(scalar); }
	if (this->planeToShow == planes::z) { this->sceneToShow->slotSetPlaneDisplacementZ(scalar); }
	this->update();
}

void PlanarViewer::flipPlaneDirection() {
	// The plane inversion doesn't happen here (only visible in 3D view) however the buttons to control it
	// are contained in the headers of the planar viewers. Thus, it must be placed here.
	if (this->planeToShow == planes::x) { this->sceneToShow->slotTogglePlaneDirectionX(); }
	if (this->planeToShow == planes::y) { this->sceneToShow->slotTogglePlaneDirectionY(); }
	if (this->planeToShow == planes::z) { this->sceneToShow->slotTogglePlaneDirectionZ(); }
	this->update();
}

void PlanarViewer::rotatePlaneClockwise() {
	if (this->planeOrientation == planeHeading::North) { this->planeOrientation = planeHeading::East; }
	else if (this->planeOrientation == planeHeading::East) { this->planeOrientation = planeHeading::South; }
	else if (this->planeOrientation == planeHeading::South) { this->planeOrientation = planeHeading::West; }
	else if (this->planeOrientation == planeHeading::West) { this->planeOrientation = planeHeading::North; }
	this->sceneToShow->setPlaneHeading(this->planeToShow, this->planeOrientation);
	this->update();
}

void PlanarViewer::rotatePlaneCounterClockwise() {
	if (this->planeOrientation == planeHeading::North) { this->planeOrientation = planeHeading::West; }
	else if (this->planeOrientation == planeHeading::West) { this->planeOrientation = planeHeading::South; }
	else if (this->planeOrientation == planeHeading::South) { this->planeOrientation = planeHeading::East; }
	else if (this->planeOrientation == planeHeading::East) { this->planeOrientation = planeHeading::North; }
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
