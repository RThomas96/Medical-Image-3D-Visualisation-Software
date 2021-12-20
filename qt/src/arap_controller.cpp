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
		this->mesh.reset();
		if (this->curve) {
			this->curve.reset();
		}
		// TODO : Delete all scene data here !
	}

	FileIO::openOFF(file_name.toStdString(), this->mesh->getVertices(), this->mesh->getTriangles());
	this->mesh->update();

	this->viewer->makeCurrent();
	this->scene->loadMesh();
	this->viewer->doneCurrent();
	this->viewer->resetDeformation();
	this->viewer->updateInfoFromScene();
}

void ARAPController::loadImageFromFile() {
	// TODO : import some of the LoaderWidget code, or make another importer widget
}

void ARAPController::loadConstraintsFromFile() {
	//
}
