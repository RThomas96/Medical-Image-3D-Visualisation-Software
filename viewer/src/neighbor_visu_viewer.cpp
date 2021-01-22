#include "../include/neighbor_visu_viewer.hpp"
#include "../../image/include/writer.hpp"
#include "../../features.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QCoreApplication>
#include <QProgressDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QtPlatformHeaders/QGLXNativeContext>
#include <QGuiApplication>

#include <fstream>
#include <dlfcn.h>

template<class T>
std::remove_reference_t<T> const& as_const(T&&t){return t;}

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

void Viewer::addGrid() {
	// create input grid pointer :
	std::shared_ptr<InputGrid> inputGrid = std::make_shared<InputGrid>();

	// create reader :
	IO::GenericGridReader* reader = nullptr;
	IO::GenericGridReader::data_t threshold = IO::GenericGridReader::data_t(0);

	// create message box to ask user :
	QMessageBox* msgBox = new QMessageBox();
	msgBox->setText("Choose your input data type");
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	QPushButton* dimButton = msgBox->addButton("DIM", QMessageBox::ActionRole);
	QPushButton* tiffButton = msgBox->addButton("TIFF", QMessageBox::ActionRole);
	QString lastPath = "";

	// show the msgbox :
	msgBox->exec();

	if (msgBox->clickedButton() == dimButton) {
		reader = new IO::DIMReader(threshold);
		QString filename = QFileDialog::getOpenFileName(this, "Open a DIM/IMA image (Blue channel)", "../../", "BrainVISA DIM Files (*.dim)");
		if (filename.isEmpty() == false) {
			// update last path :
			lastPath = QFileInfo(filename).path();
			std::vector<std::string> f;
			f.push_back(filename.toStdString());
			reader->setFilenames(f);
		} else {
			QMessageBox* fileDialog = new QMessageBox();
			fileDialog->critical(this, "Error", "Did not provide any filename !");
			fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->show();
			return;
		}
	} else if (msgBox->clickedButton() == tiffButton) {
		reader = new IO::Reader::TIFF(threshold);
		QStringList filenames = QFileDialog::getOpenFileNames(this, "Open multiple TIFF images (Blue channel)","../../", "TIFF Files (*.tiff, *.tif)");
		if (filenames.isEmpty() == false) {
			std::vector<std::string> f;
			for (const QString& fn : as_const(filenames)) {
				f.push_back(fn.toStdString());
				// update last path :
				if (lastPath.isEmpty() == true) {
					if (fn.isEmpty() == false) {
						lastPath = QFileInfo(fn).path();
					}
				}
			}
			reader->setFilenames(f);
		} else {
			QMessageBox* fileDialog = new QMessageBox();
			fileDialog->critical(this, "Error", "Did not provide any filenames !");
			fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->show();
			return;
		}
	} else {
		std::cerr << "No button was pressed." << '\n';
		throw std::runtime_error("error : no button pressed");
	}

	std::string meshpath; // filepath for the MESH file
	QString filename = QFileDialog::getOpenFileName(this, "Open a MESH", lastPath, "Mesh (*.MESH)");
	if (filename.isEmpty() == false) {
		meshpath = filename.toStdString();
	} else {
		delete reader;
		QMessageBox* fileDialog = new QMessageBox();
		fileDialog->critical(this, "Error", "Did not provide any filename !");
		fileDialog->setAttribute(Qt::WA_DeleteOnClose);
		fileDialog->show();
		return;
	}

	// Set reader properties :
	reader->setDataThreshold(threshold);
	// Load the data :
	reader->loadImage();

	// Update data from the grid reader :
	inputGrid = std::make_shared<InputGrid>();
	inputGrid->fromGridReader(*reader);

	// free up the reader's resources :
	delete reader;

	this->makeCurrent();
	this->scene->addGrid(inputGrid, meshpath);
	this->doneCurrent();

	std::cerr << "Added input grid to scene\n";

	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	float sceneSize = glm::length(bbDiag);

	this->setSceneRadius(sceneSize*sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
}
