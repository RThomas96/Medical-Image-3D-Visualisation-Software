#include "../include/arap_controller.hpp"

#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "../../viewer/include/scene.hpp"

#include <QFileDialog>

ARAPController::ARAPController(Viewer* _v, Scene* _s) {
	this->viewer = _v;
	this->scene	 = _s;

	// Initialize the fields to a null value :
	this->mesh = nullptr;
	this->curve = nullptr;
	this->mesh_interface = nullptr;
	this->arapManipulator = nullptr;
	this->rectangleSelection = nullptr;

	this->mesh_constraints.clear();
	this->image_constraints.clear();

	this->listview_image_constraints = nullptr;
	this->listview_mesh_constraints = nullptr;

	this->button_load_image = nullptr;
	this->button_load_mesh = nullptr;
	this->button_load_constraints = nullptr;
	this->button_load_curve = nullptr;

	this->button_save_mesh = nullptr;
	this->button_save_curve = nullptr;

	this->button_align_arap = nullptr;
	this->button_scale_arap = nullptr;
	this->button_start_arap = nullptr;

	this->label_mesh_name = nullptr;
	this->label_mesh_info = nullptr;
	this->label_grid_name = nullptr;
	this->label_grid_info = nullptr;

	this->dir_last_accessed = QDir::homePath();

	this->widget_layout = nullptr;

	this->init();
}

void ARAPController::init() {
	if (this->arapManipulator != nullptr) {
		return;
	}

	this->button_load_image = new QPushButton("Load image");
	this->button_load_mesh = new QPushButton("Load mesh");
	this->button_load_constraints = new QPushButton("Load constraints");
	this->button_load_curve = new QPushButton("Load curve");

	this->button_save_mesh = new QPushButton("Save mesh");
	this->button_save_curve = new QPushButton("Save curve");

	this->button_align_arap = new QPushButton("Align constraints");
	this->button_scale_arap = new QPushButton("Scale constraints");
	this->button_start_arap = new QPushButton("Perform deformation");

	this->initLayout();
	this->initSignals();
}

const std::shared_ptr<SimpleManipulator>& ARAPController::getARAPManipulator() const { return this->arapManipulator; }
const std::shared_ptr<MMInterface<glm::vec3>>& ARAPController::getMeshInterface() const { return this->mesh_interface; }
const std::shared_ptr<RectangleSelection>& ARAPController::getRectangleSelection() const { return this->rectangleSelection; }

const std::size_t ARAPController::getCurrentlyEditedConstraint() const { return 0; }
const std::vector<glm::vec3>& ARAPController::getImageConstraints() const { return this->image_constraints; }
const std::vector<std::size_t>& ARAPController::getMeshConstraints() const { return this->mesh_constraints; }

void ARAPController::loadMeshFromFile() {
	// Launch a file picker to get the name of an OFF file :
	QString file_name = QFileDialog::getOpenFileName(nullptr, "Open a Mesh file (OFF)", this->dir_last_accessed, "OFF files (*.off)");
	if (file_name.isEmpty() || not QFileInfo::exists(file_name)) {
		std::cerr << "Error : nothing to open.\nFile path given : \"" << file_name.toStdString() << "\"\n";
		return;
	}

	// Update directory last accessed :
	QFileInfo mesh_file_info(file_name);
	this->dir_last_accessed = mesh_file_info.absolutePath();

	// If mesh was already set to something, remove it :
	if (this->mesh != nullptr) {
		// Note : the mesh data is the first one to be loaded in, so loading in a new mesh
		// effectively results in the whole process being reset. Delete everything here :
		std::cerr << "Deleting all previous data from the ARAP controller ...\n";
		this->deleteMeshData();
		this->deleteCurveData();
		this->deleteGridData();
		this->resetMeshInterface();
		std::cerr << "Done resetting the ARAP controller.\n";
	}

	// Create the mesh, load it and create the kd-tree structure :
	this->mesh = std::make_shared<Mesh>();
	FileIO::openOFF(file_name.toStdString(), this->mesh->getVertices(), this->mesh->getTriangles());
	this->mesh->update();

	// Attempt to find a local constraint file next to it :
	QDir mesh_root(mesh_file_info.absolutePath());
	QDirIterator mesh_root_folder_iterator(mesh_root, QDirIterator::IteratorFlag::NoIteratorFlags);
	QString target_file_name(file_name + ".constraints");

	while (mesh_root_folder_iterator.hasNext()) {
		QString current_filename = mesh_root_folder_iterator.next();
		QFileInfo current_file_info(current_filename);
		if (not current_file_info.isFile() &&
			current_file_info.fileName().contains(target_file_name, Qt::CaseSensitivity::CaseInsensitive)) {
			std::cerr << "Found mesh constraint file\n";
			// TODO : load constraints here !
		}
	}

	// Upload data to the scene and update the viewer's camera and data :
	this->uploadMeshToScene();
	this->initializeMeshInterface();

	// Update Scene BB
	this->scene->updateBoundingBox();
	// Update sphere size for handles

	// TODO : this only toggles the control panel and sets a boolean to false. Change it to include all of it here.
	this->viewer->resetDeformation();

	this->viewer->updateInfoFromScene();
	auto scene_radius = this->viewer->camera()->sceneRadius();
}

void ARAPController::loadImageFromFile() {
	// TODO : import some of the LoaderWidget code, or make another importer widget
}

void ARAPController::loadConstraintsFromFile() {
	// check if mesh loaded
	if (this->mesh == nullptr) {
		// Show error modal dialog :
		QMessageBox::critical(this, "Error : mesh not initialized",
		  "Error : you cannot load mesh constraints without loading a mesh first !",
		  QMessageBox::StandardButton::Ok);
		return;
	}

	// get file from user :
	QString file_name = QFileDialog::getOpenFileName(this, "Load a constraint file", this->dir_last_accessed, "Constraint files (*.constraint)");
	if (file_name.isEmpty()) {
		std::cerr << "Error : file name for constraint loading was empty.\n";
		return;
	}

	// get file info :
	QFileInfo file_info(file_name);
	if (not file_info.exists()) {
		std::cerr << "Error : cannot open file chosen because it does not exist !";
	}
	// Update last directory accessed :
	this->dir_last_accessed = file_info.absolutePath();

	// check if constraints not already set in place, in which case ask to append or replace
	if (not this->mesh_constraints.empty()) {
		auto msgBoxChoice = QMessageBox::question(this, "Override constraints loaded ?",
		  "Do you really want to load that constraint file ?\nThis will override the currently loaded constraints.",
		  QMessageBox::StandardButton::Ok|QMessageBox::StandardButton::Cancel, QMessageBox::StandardButton::Ok);
		if (msgBoxChoice != QMessageBox::StandardButton::Ok) {
			std::cerr << "User choice for overriding constraints was either cancel or escape.\n";
			return;
		}
	}

	// Load constraints :
	std::vector<glm::vec3> positions;
	std::ifstream constraints(file_name.toStdString().c_str(), std::ios_base::in | std::ios_base::binary);
	if (not constraints.is_open()) {
		std::cerr << "Error : could not open the " << file_name.toStdString() << " file for constraints.\n";
		return;
	}

	// Read the file like an off file :
	while (constraints && not constraints.eof()) {
		std::size_t constraint = 0;
		constraints >> constraint;
		std::cerr << "Adding constraint " << constraint << " to mesh ...";
		this->mesh_constraints.emplace_back(constraint);
	}

	// Debug output :
	std::cerr << "After constraint reading, constraints are : {";
	for (auto constraint : this->mesh_constraints) {
		std::cerr << constraint << " , ";
	}
	std::cerr << "}\n";

	constraints.close();

	return;
}

void ARAPController::deleteMeshData() {
	this->viewer->makeCurrent();
	this->scene->arap_delete_mesh_drawable();
	this->viewer->doneCurrent();
	this->mesh.reset();
}

void ARAPController::deleteCurveData() {
	this->viewer->makeCurrent();
	this->scene->arap_delete_curve_drawable();
	this->viewer->doneCurrent();
	this->curve.reset();
}

void ARAPController::deleteGridData() {
	this->viewer->makeCurrent();
	this->scene->arap_delete_grid_data();
	this->viewer->doneCurrent();
	this->image.reset();
}
