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
		case Qt::Key::Key_R:
			this->scene->recompileShaders();
			this->update();
		break;
		case Qt::Key::Key_P:
			this->scene->printVAOStateNext();
		break;
		case Qt::Key::Key_V:
			this->scene->setDrawMode(DrawMode::Volumetric);
			this->update();
		break;
		case Qt::Key::Key_S:
			this->scene->setDrawMode(DrawMode::Solid);
			this->update();
		break;
		/*
		GRID VISIBILITY
		*/
		case Qt::Key::Key_I:
			this->scene->toggleInputGridVisible();
			this->update();
		break;
		case Qt::Key::Key_O:
			this->scene->toggleOutputGridVisible();
			this->update();
		break;
		case Qt::Key::Key_A:
			this->addGrid();
			this->update();
		break;
		/*
		INTERPOLATION
		*/
		case Qt::Key::Key_T:
			this->scene->fillTrilinear();
			this->update();
		break;
		case Qt::Key::Key_N:
			this->scene->fillNearestNeighbor();
			this->update();
		break;
		case Qt::Key::Key_W:
			this->scene->writeGridDIM("outputGrid");
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
	QGLViewer::wheelEvent(_w);
	this->update();
}

void Viewer::addGrid() {
	// create input grid pointer :
	std::shared_ptr<InputGrid> inputGrid = std::make_shared<InputGrid>();

	IO::GenericGridReader* reader = nullptr;
	IO::GenericGridReader::data_t threshold = IO::GenericGridReader::data_t(0);

	QMessageBox* msgBox = new QMessageBox();
	msgBox->setText("Choose your input data type");
	QPushButton* dimButton = msgBox->addButton("DIM", QMessageBox::ActionRole);
	QPushButton* tiffButton = msgBox->addButton("TIFF", QMessageBox::ActionRole);

	msgBox->exec();

	if (msgBox->clickedButton() == dimButton) {
		reader = new IO::DIMReader(threshold);
		QString filename = QFileDialog::getOpenFileName(nullptr, "Open a DIM/IMA image (Blue channel)", "../../", "BrainVISA DIM Files (*.dim)");
		std::vector<std::string> f;
		f.push_back(filename.toStdString());
		reader->setFilenames(f);
	} else if (msgBox->clickedButton() == tiffButton) {
		// do nothing :
		reader = new IO::Reader::TIFF(threshold);
		QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Open multiple TIFF images (Blue channel)","../../", "TIFF Files (*.tiff, *.tif)");
		std::vector<std::string> f;
		for (const QString& fn : as_const(filenames)) {
			f.push_back(fn.toStdString());
		}
		reader->setFilenames(f);
	} else {
		std::cerr << "No button was pressed." << '\n';
		throw std::runtime_error("error : no button pressed");
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

	QString filename = QFileDialog::getOpenFileName(nullptr, "Open a MESH", "../../", "Mesh (*.MESH)");
	std::string meshpath = filename.toStdString();

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
