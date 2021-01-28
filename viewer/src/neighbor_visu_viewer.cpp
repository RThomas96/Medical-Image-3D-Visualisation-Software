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
#include <QGuiApplication>

#include <fstream>

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

	glm::vec3 scene_center = this->scene->getSceneCenter();
	float scene_radius = this->scene->getSceneRadius();

	this->setSceneRadius(qreal(scene_radius));
	qglviewer::Vec cnt(scene_center.x, scene_center.y, scene_center.z);
	this->setSceneCenter(cnt);

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

	QMessageBox* levelBox = new QMessageBox();
	levelBox->setText("Choose your downsampling level");
	//levelBox->setAttribute(Qt::WA_DeleteOnClose);
	QPushButton* originalButton = levelBox->addButton("Original Size", QMessageBox::ButtonRole::ActionRole);
	QPushButton* lowButton = levelBox->addButton("Low Resolution", QMessageBox::ButtonRole::ActionRole);
	QPushButton* lowerButton = levelBox->addButton("Lower Resolution", QMessageBox::ButtonRole::ActionRole);
	QPushButton* lowestButton = levelBox->addButton("Lowest Resolution", QMessageBox::ButtonRole::ActionRole);

	QMessageBox* confirmBox = new QMessageBox();
	confirmBox->setText("This will be a considerably large image. Do you really wish to do this ?");
	QPushButton* confirm_Accept = confirmBox->addButton("Yes", QMessageBox::ButtonRole::AcceptRole);
	QPushButton* confirm_Deny = confirmBox->addButton("No, take me back", QMessageBox::ButtonRole::RejectRole);

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

	levelBox->exec();

	while (levelBox->clickedButton() == originalButton) {
		confirmBox->exec();
		if (confirmBox->clickedButton() == confirm_Accept) {
			std::cerr << "Accepted full-res\n";
			break;
		}
		else {
			std::cerr << "Re-showing the dialog box\n";
			levelBox->exec();
		}
	}

	if (levelBox->clickedButton() == lowButton) { reader->enableDownsampling(IO::DownsamplingLevel::Low); }
	if (levelBox->clickedButton() == lowerButton) { reader->enableDownsampling(IO::DownsamplingLevel::Lower); }
	if (levelBox->clickedButton() == lowestButton) { reader->enableDownsampling(IO::DownsamplingLevel::Lowest); }

	// Set the interpolation method :
	std::shared_ptr<Interpolators::genericInterpolator<IO::GenericGridReader::data_t>> interpolator =
			std::make_shared<Interpolators::meanValue<IO::GenericGridReader::data_t>>();
	reader->setInterpolationMethod(interpolator);

	std::string meshpath; // filepath for the MESH file
	QString filename = QFileDialog::getOpenFileName(this, "Open a MESH", lastPath, "Mesh (*.MESH)");
	if (filename.isEmpty() == false) {
		meshpath = filename.toStdString();
	} else {
		meshpath = "";
	}

	// Set reader properties :
	reader->setDataThreshold(threshold);
	// Load the data :
	reader->loadImage();
	// get texture bounds :
	glm::vec<2, IO::GenericGridReader::data_t, glm::defaultp> limits = reader->getTextureLimits();
	this->scene->slotSetMinColorValue(limits.x);
	this->scene->slotSetMaxColorValue(limits.y);

	// Update data from the grid reader :
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

void Viewer::addTwoGrids() {
	// create input grid pointer :
	std::shared_ptr<InputGrid> inputGrid = std::make_shared<InputGrid>();
	std::shared_ptr<InputGrid>  otherGrid = std::make_shared<InputGrid>();

	// create reader :
	IO::GenericGridReader* readerR = nullptr;
	IO::GenericGridReader* readerG = nullptr;
	IO::GenericGridReader::data_t threshold = IO::GenericGridReader::data_t(0);

	// create message box to ask user :
	QMessageBox* msgBox = new QMessageBox();
	msgBox->setText("Choose your input data type");
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	QPushButton* dimButton = msgBox->addButton("DIM", QMessageBox::ActionRole);
	QPushButton* tiffButton = msgBox->addButton("TIFF", QMessageBox::ActionRole);
	QString lastPath = "";

	// NOTE : Downsampling level will be used for both loaded grids.
	QMessageBox* levelBox = new QMessageBox();
	levelBox->setText("Choose your downsampling level");
	QPushButton* originalButton = levelBox->addButton("Original Size", QMessageBox::ButtonRole::ActionRole);
	QPushButton* lowButton = levelBox->addButton("Low Resolution", QMessageBox::ButtonRole::ActionRole);
	QPushButton* lowerButton = levelBox->addButton("Lower Resolution", QMessageBox::ButtonRole::ActionRole);
	QPushButton* lowestButton = levelBox->addButton("Lowest Resolution", QMessageBox::ButtonRole::ActionRole);

	QMessageBox* confirmBox = new QMessageBox();
	confirmBox->setText("This will be a considerably large image. Do you really wish to do this ?");
	QPushButton* confirm_Accept = confirmBox->addButton("Yes", QMessageBox::ButtonRole::AcceptRole);
	QPushButton* confirm_Deny = confirmBox->addButton("No, take me back", QMessageBox::ButtonRole::RejectRole);

	// show the msgbox :
	msgBox->exec();

	if (msgBox->clickedButton() == dimButton) {
		// Reader for Red channel :
		readerR = new IO::DIMReader(threshold);
		QString filenameR = QFileDialog::getOpenFileName(this, "Open a DIM/IMA image (Red channel)", "../../", "BrainVISA DIM Files (*.dim)");
		if (filenameR.isEmpty() == false) {
			// update last path :
			lastPath = QFileInfo(filenameR).path();
			std::vector<std::string> f;
			f.push_back(filenameR.toStdString());
			readerR->setFilenames(f);
		} else {
			QMessageBox* fileDialog = new QMessageBox();
			fileDialog->critical(this, "Error", "Did not provide any filename !");
			fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->show();
			return;
		}

		// Reader Green (or blue) channel :
		readerG = new IO::DIMReader(threshold);
		QString filenameG = QFileDialog::getOpenFileName(this, "Open a DIM/IMA image (Blue channel)", lastPath, "BrainVISA DIM Files (*.dim)");
		if (filenameG.isEmpty() == false) {
			// update last path :
			lastPath = QFileInfo(filenameG).path();
			std::vector<std::string> f;
			f.push_back(filenameG.toStdString());
			readerG->setFilenames(f);
		} else {
			QMessageBox* fileDialog = new QMessageBox();
			fileDialog->critical(this, "Error", "Did not provide any filename !");
			fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->show();
			return;
		}
	} else if (msgBox->clickedButton() == tiffButton) {
		readerR = new IO::Reader::TIFF(threshold);
		QStringList filenamesR = QFileDialog::getOpenFileNames(this, "Open multiple TIFF images (Red channel)","../../", "TIFF Files (*.tiff, *.tif)");
		if (filenamesR.isEmpty() == false) {
			std::vector<std::string> f;
			for (const QString& fn : as_const(filenamesR)) {
				f.push_back(fn.toStdString());
				// update last path :
				if (lastPath.isEmpty() == true) {
					if (fn.isEmpty() == false) {
						lastPath = QFileInfo(fn).path();
					}
				}
			}
			readerR->setFilenames(f);
		} else {
			QMessageBox* fileDialog = new QMessageBox();
			fileDialog->critical(this, "Error", "Did not provide any filenames !");
			fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->show();
			return;
		}

		readerG = new IO::Reader::TIFF(threshold);
		QStringList filenamesG = QFileDialog::getOpenFileNames(this, "Open multiple TIFF images (Red channel)", lastPath, "TIFF Files (*.tiff, *.tif)");
		if (filenamesG.isEmpty() == false) {
			std::vector<std::string> f;
			for (const QString& fn : as_const(filenamesG)) {
				f.push_back(fn.toStdString());
				// update last path :
				if (lastPath.isEmpty() == true) {
					if (fn.isEmpty() == false) {
						lastPath = QFileInfo(fn).path();
					}
				}
			}
			readerG->setFilenames(f);
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

	levelBox->exec();

	while (levelBox->clickedButton() == originalButton) {
		confirmBox->exec();
		if (confirmBox->clickedButton() == confirm_Accept) {
			std::cerr << "Accepted full-res\n";
			break;
		}
		else {
			std::cerr << "Re-showing the dialog box\n";
			levelBox->exec();
		}
	}

	if (levelBox->clickedButton() == lowButton) {
		readerR->enableDownsampling(IO::DownsamplingLevel::Low);
		readerG->enableDownsampling(IO::DownsamplingLevel::Low);
	}
	if (levelBox->clickedButton() == lowerButton) {
		readerR->enableDownsampling(IO::DownsamplingLevel::Lower);
		readerG->enableDownsampling(IO::DownsamplingLevel::Lower);
	}
	if (levelBox->clickedButton() == lowestButton) {
		readerR->enableDownsampling(IO::DownsamplingLevel::Lowest);
		readerG->enableDownsampling(IO::DownsamplingLevel::Lowest);
	}

	// Set the interpolation method :
	std::shared_ptr<Interpolators::genericInterpolator<IO::GenericGridReader::data_t>> interpolator =
			std::make_shared<Interpolators::meanValue<IO::GenericGridReader::data_t>>();
	readerR->setInterpolationMethod(interpolator);
	readerG->setInterpolationMethod(interpolator);

	std::string meshpath; // filepath for the MESH file
	QString filename = QFileDialog::getOpenFileName(this, "Open a MESH", lastPath, "Mesh (*.MESH)");
	if (filename.isEmpty() == false) {
		meshpath = filename.toStdString();
	} else {
		meshpath = "";
	}

	// Set reader properties :
	readerR->setDataThreshold(threshold);
	readerG->setDataThreshold(threshold);
	// Load the data :
	readerR->loadImage();
	readerG->loadImage();
	// get texture bounds :
	glm::vec<2, IO::GenericGridReader::data_t, glm::defaultp> limitsR = readerR->getTextureLimits();
	glm::vec<2, IO::GenericGridReader::data_t, glm::defaultp> limitsG = readerG->getTextureLimits();
	glm::vec<2, IO::GenericGridReader::data_t, glm::defaultp> limits{ std::min(limitsR.x, limitsG.x), std::max(limitsR.y, limitsG.y) };
	this->scene->slotSetMinColorValue(limits.x);
	this->scene->slotSetMaxColorValue(limits.y);

	// Update data from the grid reader :
	inputGrid->fromGridReader(*readerR);
	otherGrid->fromGridReader(*readerG);

	// free up the reader's resources :
	delete readerR;
	delete readerG;

	this->makeCurrent();
	this->scene->addTwoGrids(inputGrid, otherGrid, meshpath);
	this->doneCurrent();

	std::cerr << "Added input grid to scene\n";

	glm::vec3 bbDiag = this->scene->getSceneCenter();
	float sceneSize = this->scene->getSceneRadius();

	this->setSceneRadius(sceneSize*sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
}
