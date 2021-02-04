#include "../include/neighbor_visu_viewer.hpp"
#include "../../image/include/writer.hpp"
#include "../../features.hpp"
#include "../../qt/include/user_settings_widget.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	glm::vec3 scene_center = this->scene->getSceneCenter();
	float scene_radius = this->scene->getSceneRadius();

	this->setSceneRadius(qreal(scene_radius));
	qglviewer::Vec cnt(scene_center.x, scene_center.y, scene_center.z);
	this->setSceneCenter(cnt);
	this->camera()->centerScene();
	this->update();
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

void Viewer::loadGrid(std::shared_ptr<InputGrid>& g) {
	this->makeCurrent();
	this->scene->addGrid(g, "");
	this->doneCurrent();

	std::cerr << "Added input grid to scene\n";

	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	float sceneSize = glm::length(bbDiag);

	this->setSceneRadius(sceneSize*sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
}

void Viewer::loadTwoGrids(std::shared_ptr<InputGrid>& g1, std::shared_ptr<InputGrid>& g2) {
	this->makeCurrent();
	this->scene->addTwoGrids(g1, g2, "");
	this->doneCurrent();

	std::cerr << "Added 2 input grids to scene\n";

	glm::vec3 bbDiag = this->scene->getSceneBoundaries();
	float sceneSize = glm::length(bbDiag);

	this->setSceneRadius(sceneSize*sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
}
#if 0
void Viewer::addGrid() {
	// create input grid pointer :
	std::shared_ptr<InputGrid> inputGrid = std::make_shared<InputGrid>();

	// create reader :
	std::shared_ptr<IO::GenericGridReader> reader = nullptr;
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
		reader = std::make_shared<IO::DIMReader>(threshold);
		QString filename = QFileDialog::getOpenFileName(this, "Open a DIM/IMA image (Blue channel)", "../../", "BrainVISA DIM Files (*.dim)");
		if (filename.isEmpty() == false) {
			// update last path :
			lastPath = QFileInfo(filename).path();
			std::vector<std::string> f;
			f.push_back(filename.toStdString());
			reader->setFilenames(f);
		} else {
			QMessageBox* fileDialog = new QMessageBox();
			fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->critical(this, "Error", "Did not provide any filename !");
			return;
		}
	} else if (msgBox->clickedButton() == tiffButton) {
		reader = std::make_shared<IO::Reader::TIFF>(threshold);
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
			fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->critical(this, "Error", "Did not provide any filenames !");
			return;
		}
	} else {
		std::cerr << "No button was pressed." << '\n';
		throw std::runtime_error("error : no button pressed");
	}

	reader->preComputeImageData();
	std::size_t sizebits = reader->getGridSizeBytes() * 8;
	std::size_t settings = UserSettings::getInstance().getUserRemainingBitSize();

	bool askUser = ((settings / 2) < sizebits);
	std::cerr << "Bits available : " << settings/2 << " bits taken by loading " << sizebits << '\n';

	levelBox->exec();

	if (askUser) {
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
	}

	if (levelBox->clickedButton() == lowButton) { reader->enableDownsampling(IO::DownsamplingLevel::Low); }
	if (levelBox->clickedButton() == lowerButton) { reader->enableDownsampling(IO::DownsamplingLevel::Lower); }
	if (levelBox->clickedButton() == lowestButton) { reader->enableDownsampling(IO::DownsamplingLevel::Lowest); }

	if (levelBox->clickedButton() != originalButton) {
	// Set the interpolation method :
		std::shared_ptr<Interpolators::genericInterpolator<IO::GenericGridReader::data_t>> interpolator =
				std::make_shared<Interpolators::meanValue<IO::GenericGridReader::data_t>>();
		reader->setInterpolationMethod(interpolator);
	}

	sizebits = reader->getGridSizeBytes() * 8;
	// add the image size to loaded size
	UserSettings::getInstance().loadImageSize(sizebits);

	// Set reader properties :
	reader->setDataThreshold(threshold);
	// Load the data :
	reader->loadImage();
	reader->getBoundingBox().printInfo("After reader");
	// get texture bounds :
	glm::vec<2, IO::GenericGridReader::data_t, glm::defaultp> limits = reader->getTextureLimits();
	this->scene->slotSetMinColorValue(limits.x);
	this->scene->slotSetMaxColorValue(limits.y);

	// Update data from the grid reader :
	inputGrid->setGridReader(reader);
	inputGrid->fromGridReader();

	this->makeCurrent();
	this->scene->addGrid(inputGrid, "");
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
	std::shared_ptr<InputGrid> otherGrid = std::make_shared<InputGrid>();

	// create reader :
	std::shared_ptr<IO::GenericGridReader> readerR = nullptr;
	std::shared_ptr<IO::GenericGridReader> readerG = nullptr;
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
		readerR = std::make_shared<IO::DIMReader>(threshold);
		QString filenameR = QFileDialog::getOpenFileName(this, "Open a DIM/IMA image (Red channel)", "../../", "BrainVISA DIM Files (*.dim)");
		if (filenameR.isEmpty() == false) {
			// update last path :
			lastPath = QFileInfo(filenameR).path();
			std::vector<std::string> f;
			f.push_back(filenameR.toStdString());
			readerR->setFilenames(f);
		} else {
			QMessageBox* fileDialog = new QMessageBox();
            fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->critical(this, "Error", "Did not provide any filename !");
			return;
		}

		// Reader Green (or blue) channel :
		readerG = std::make_shared<IO::DIMReader>(threshold);
		QString filenameG = QFileDialog::getOpenFileName(this, "Open a DIM/IMA image (Blue channel)", lastPath, "BrainVISA DIM Files (*.dim)");
		if (filenameG.isEmpty() == false) {
			// update last path :
			lastPath = QFileInfo(filenameG).path();
			std::vector<std::string> f;
			f.push_back(filenameG.toStdString());
			readerG->setFilenames(f);
		} else {
			QMessageBox* fileDialog = new QMessageBox();
            fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->critical(this, "Error", "Did not provide any filename !");
			return;
		}
	} else if (msgBox->clickedButton() == tiffButton) {
		readerR = std::make_shared<IO::Reader::TIFF>(threshold);
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
            fileDialog->setAttribute(Qt::WA_DeleteOnClose);
			fileDialog->critical(this, "Error", "Did not provide any filenames !");
			return;
		}

		readerG = std::make_shared<IO::Reader::TIFF>(threshold);
		QStringList filenamesG = QFileDialog::getOpenFileNames(this, "Open multiple TIFF images (Blue channel)", lastPath, "TIFF Files (*.tiff, *.tif)");
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

	readerR->preComputeImageData();
	readerG->preComputeImageData();
	std::size_t sizebits = readerR->getGridSizeBytes() * 8 + readerG->getGridSizeBytes() * 8;
	std::size_t settings = UserSettings::getInstance().getUserRemainingBitSize();
	bool askUser = ((settings / 2) < sizebits);

	levelBox->exec();

	if (askUser) {
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
	if (levelBox->clickedButton() != originalButton) {
		// Set the interpolation method :
		std::shared_ptr<Interpolators::genericInterpolator<IO::GenericGridReader::data_t>> interpolator =
				std::make_shared<Interpolators::meanValue<IO::GenericGridReader::data_t>>();
		readerR->setInterpolationMethod(interpolator);
		readerG->setInterpolationMethod(interpolator);
	}

	sizebits = readerR->getGridSizeBytes() * 8 + readerG->getGridSizeBytes() * 8;
	// add the image size to loaded size
	UserSettings::getInstance().loadImageSize(sizebits);

	// Set reader properties :
	readerR->setDataThreshold(threshold);
	readerG->setDataThreshold(threshold);
	// Load the data :
	readerR->loadImage();
	readerG->loadImage();
	readerR->getBoundingBox().printInfo("After readerR");
	readerG->getBoundingBox().printInfo("After readerG");
	// get texture bounds :
	glm::vec<2, IO::GenericGridReader::data_t, glm::defaultp> limitsR = readerR->getTextureLimits();
	glm::vec<2, IO::GenericGridReader::data_t, glm::defaultp> limitsG = readerG->getTextureLimits();
	glm::vec<2, IO::GenericGridReader::data_t, glm::defaultp> limits{ std::min(limitsR.x, limitsG.x), std::max(limitsR.y, limitsG.y) };
	this->scene->slotSetMinColorValue(limits.x);
	this->scene->slotSetMaxColorValue(limits.y);

	// Update data from the grid reader :
	inputGrid->setGridReader(readerR);
	inputGrid->fromGridReader();
	otherGrid->setGridReader(readerG);
	otherGrid->fromGridReader();

	this->makeCurrent();
	this->scene->addTwoGrids(inputGrid, otherGrid, "");
	this->doneCurrent();

	std::cerr << "Added input grid to scene\n";

	glm::vec3 bbDiag = this->scene->getSceneCenter();
	float sceneSize = this->scene->getSceneRadius();

	this->setSceneRadius(sceneSize*sceneRadiusMultiplier);
	// center scene on center of grid
	this->setSceneCenter(qglviewer::Vec(bbDiag.x/2., bbDiag.y/2., bbDiag.z/2.));
	this->showEntireScene();
}
#endif
