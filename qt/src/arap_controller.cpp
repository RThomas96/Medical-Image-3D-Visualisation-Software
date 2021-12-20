#include "../include/arap_controller.hpp"

#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "../../viewer/include/scene.hpp"

#include <glm/gtx/io.hpp>

#include <QFileDialog>
#include <QVBoxLayout>

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

	this->init();
}

void ARAPController::init() {
	if (this->arapManipulator != nullptr) {
		return;
	}

	this->viewer->setARAPController(this);

	this->button_load_image = new QPushButton("Load image");
	this->button_load_mesh = new QPushButton("Load mesh");
	this->button_load_constraints = new QPushButton("Load constraints");
	this->button_load_curve = new QPushButton("Load curve");

	this->button_save_mesh = new QPushButton("Save mesh");
	this->button_save_curve = new QPushButton("Save curve");

	this->button_align_arap = new QPushButton("Align constraints");
	this->button_scale_arap = new QPushButton("Scale constraints");
	this->button_start_arap = new QPushButton("Perform deformation");

	this->button_load_constraints->setEnabled(false);
	this->button_load_curve->setEnabled(false);
	this->button_load_image->setEnabled(false);

	this->button_align_arap->setEnabled(false);
	this->button_scale_arap->setEnabled(false);
	this->button_start_arap->setEnabled(false);
	this->button_save_mesh->setEnabled(false);
	this->button_save_curve->setEnabled(false);

	this->initLayout();
	this->initSignals();
}

void ARAPController::initLayout() {
	auto label_header = new QLabel("Deformation controller");
	auto label_loading = new QLabel("Loading data");
	auto label_deformation = new QLabel("Deformation");
	auto label_save = new QLabel("Save data");

	auto separator_header = new QFrame;
	separator_header->setFrameShape(QFrame::Shape::HLine);
	separator_header->setFrameShadow(QFrame::Shadow::Sunken);
	auto separator_deformation = new QFrame;
	separator_deformation->setFrameShape(QFrame::Shape::HLine);
	separator_deformation->setFrameShadow(QFrame::Shadow::Sunken);
	auto separator_save = new QFrame;
	separator_save->setFrameShape(QFrame::Shape::HLine);
	separator_save->setFrameShadow(QFrame::Shadow::Sunken);

	auto widget_layout = new QVBoxLayout;
	widget_layout->addWidget(label_header);
	widget_layout->addWidget(separator_header);
	widget_layout->addWidget(label_loading);
	widget_layout->addWidget(this->button_load_mesh);
	widget_layout->addWidget(this->button_load_constraints);
	widget_layout->addWidget(this->button_load_curve);
	widget_layout->addWidget(this->button_load_image);
	widget_layout->addWidget(separator_deformation);
	widget_layout->addWidget(label_deformation);
	widget_layout->addWidget(this->button_align_arap);
	widget_layout->addWidget(this->button_scale_arap);
	widget_layout->addWidget(this->button_start_arap);
	widget_layout->addWidget(separator_save);
	widget_layout->addWidget(label_save);
	widget_layout->addWidget(this->button_save_mesh);
	widget_layout->addWidget(this->button_save_curve);

	widget_layout->setSpacing(0);
	this->setLayout(widget_layout);
}

void ARAPController::initSignals() {
	//
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
		if (current_file_info.isFile() &&
			current_file_info.fileName().contains(target_file_name, Qt::CaseInsensitive)) {
			std::cerr << "Found mesh constraint file : " << current_file_info.fileName().toStdString() << "\n";
			std::string file_name_std = current_file_info.absoluteFilePath().toStdString();
			this->loadConstraintDataFromFile(file_name_std);
			break;
		}
	}

	// Upload data to the scene and update the viewer's camera and data :
	this->uploadMeshToScene();
	this->initializeMeshInterface();

	// Update Scene BB & sphere size for handles
	this->scene->updateBoundingBox();
	auto scene_radius = this->viewer->camera()->sceneRadius();

	// TODO : this only toggles the control panel and sets a boolean to false. Change it to include all of it here.
	this->viewer->resetDeformation();

	this->viewer->updateInfoFromScene();

	emit this->meshIsLoaded();
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

	// load constraints :
	this->loadConstraintDataFromFile(file_name.toStdString());
}

void ARAPController::loadCurveFromFile() {
	// Launch a file picker to get the name of an OFF file :
	QString file_name = QFileDialog::getOpenFileName(nullptr, "Open a Curve file (OBJ)", this->dir_last_accessed, "OBJ files (*.obj)");
	if (file_name.isEmpty() || not QFileInfo::exists(file_name)) {
		std::cerr << "Error : nothing to open.\nFile path given : \"" << file_name.toStdString() << "\"\n";
		return;
	}
	QFileInfo fi(file_name);
	this->dir_last_accessed = fi.absolutePath();

	if (this->mesh != nullptr) {
		// Reset the potentially already-loaded curve :
		if (this->curve != nullptr) {
			this->deleteCurveData();
		}

		// Load the curve :
		auto fname= file_name.toStdString();
		this->curve = openCurveFromOBJ(fname, this->mesh);
		this->uploadCurveToScene();
		emit curveIsLoaded();
	} else {
		QMessageBox* msg = new QMessageBox;
		msg->setAttribute(Qt::WA_DeleteOnClose);
		msg->critical(this, "Cannot load curve by itself.",
					  "Error : no meshes were loaded previously.\nWe cannot open a curve all by its lonesome.");
	}
}

void ARAPController::loadConstraintDataFromFile(const std::string& file_name) {
	// Load constraints :
	std::vector<glm::vec3> positions;
	std::ifstream constraints(file_name.c_str(), std::ios_base::in | std::ios_base::binary);
	if (not constraints.is_open()) {
		std::cerr << "Error : could not open the " << file_name << " file for constraints.\n";
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

void ARAPController::uploadMeshToScene() {
	this->viewer->makeCurrent();
	this->scene->arap_load_mesh_data(this->mesh);
	this->viewer->doneCurrent();
	this->scene->updateBoundingBox();
}

void ARAPController::uploadCurveToScene() {
	this->viewer->makeCurrent();
	this->scene->arap_load_curve_data(this->curve);
	this->viewer->doneCurrent();
	this->scene->updateBoundingBox();
}

void ARAPController::updateMeshDrawable() {
	this->viewer->makeCurrent();
	// update mesh data
	this->viewer->doneCurrent();
	this->scene->updateBoundingBox();
}

void ARAPController::updateCurveDrawable() {
	this->viewer->makeCurrent();
	// update curve data
	this->viewer->doneCurrent();
	this->scene->updateBoundingBox();
}

void ARAPController::initializeMeshInterface() {
	if (this->mesh_interface == nullptr) {
		this->arapManipulator = std::make_shared<SimpleManipulator>();
		this->rectangleSelection = std::make_shared<RectangleSelection>();
		this->mesh_interface = std::make_shared<MMInterface<glm::vec3>>();

		// connect signals from the rectangle selection and the arap manipulator :
		QObject::connect(this->rectangleSelection.get(), &RectangleSelection::add, this->viewer, &Viewer::rectangleSelection_add);
		QObject::connect(this->rectangleSelection.get(), &RectangleSelection::remove, this->viewer, &Viewer::rectangleSelection_remove);
		QObject::connect(this->rectangleSelection.get(), &RectangleSelection::apply, this->viewer, &Viewer::rectangleSelection_apply);

		QObject::connect(this->arapManipulator.get(), &SimpleManipulator::moved, this->viewer, &Viewer::arapManipulator_moved);
		QObject::connect(this->arapManipulator.get(), &SimpleManipulator::mouseReleased, this->viewer, &Viewer::arapManipulator_released);

		this->viewer->initializeARAPManipulationInterface();

		this->mesh_interface->setMode(MeshModificationMode::REALTIME);
	}

	if (this->mesh != nullptr) {
		this->mesh_interface->clear();
		this->mesh_interface->loadAndInitialize(this->mesh->getVertices(), this->mesh->getTriangles());
	}
}

void ARAPController::resetMeshInterface() {
	if (this->mesh_interface != nullptr) {
		this->arapManipulator->disconnect();
		this->rectangleSelection->disconnect();

		this->arapManipulator.reset();
		this->rectangleSelection.reset();
		this->mesh_interface.reset();
	}
	this->initializeMeshInterface();
}

void ARAPController::arap_performAlignment() {
	if (this->mesh == nullptr) {
		std::cerr << "Error : no meshes loaded.\n";
		return;
	}

	std::vector<glm::vec3> transforms;	  // estimated translations between current point position and ARAP handle on the image
	auto current_transform = this->scene->getDrawableMesh()->getTransformation();

	// Note : mesh is not transformed in visu only anymore. Once loaded with an image, the transformation is immediately*
	// applied, so no need to apply it again to the target positions.
	std::cerr << "Generating 'best' estimated transform for the mesh ..." << '\n';
	for (std::size_t i = 0; i < this->mesh_constraints.size(); ++i) {
		auto constraint = this->mesh_constraints[i];
		auto position	= this->image_constraints[i];
		// Get current position :
		auto mesh_original_position = this->mesh->getVertices()[constraint];
		// Guess the best translation between this current position and the image-bound position :
		glm::vec3 estimated_transform = position - mesh_original_position;
		transforms.push_back(estimated_transform);
	}

	// Compute 'best' translation (avg translation) :
	glm::vec3 best_guess_transform = glm::vec3{.0f};
	for (auto& transform : transforms) {
		best_guess_transform += transform;
	}
	best_guess_transform /= static_cast<glm::vec3::value_type>(transforms.size());
	// Sanity check for NaNs in the components of the vector :
	if (std::isnan(best_guess_transform.x) || std::isnan(best_guess_transform.y) || std::isnan(best_guess_transform.z)) {
		std::cerr << "Error : one or more components of best_guess_transform was NaN !\n";
		std::cerr << "Alignment not performed.\n";
		return;
	}

	current_transform[3][0] += best_guess_transform[0];
	current_transform[3][1] += best_guess_transform[1];
	current_transform[3][2] += best_guess_transform[2];
	std::cerr << "Generated 'best' estimated transform for the mesh : " << best_guess_transform << '\n';

	// Apply the transformation to the mesh before ARAP !!! We want to place the mesh around the center of
	// the image in order for ARAP to have less guesswork to do.
	this->mesh->applyTransformation(current_transform);
	// ... but the drawing of the mesh doesn't need to have it anymore :
	this->scene->getDrawableMesh()->setTransformation(glm::mat4(1.f));
	//
	this->mesh_interface->loadAndInitialize(this->mesh->getVertices(), this->mesh->getTriangles());
	/*
	// TODO : see how to import these functions in the ARAPController !!!
	this->updateMeshAndCurve_No_Image_Resizing();
	this->updateMeshInterface();
	 */
	if (this->curve) {
		// not covered in updateMeshAndCurve() :
		this->scene->getDrawableCurve()->setTransformation(this->scene->getDrawableMesh()->getTransformation());
	}
}

void ARAPController::arap_performScaling() {
	if (this->mesh == nullptr) {
		std::cerr << "Error : no meshes loaded.\n";
		return;
	}

	if (this->mesh_constraints.empty() || this->image_constraints.empty()) {
		std::cerr << "Error : no constraints applied\n";
		return;
	}

	std::shared_ptr<Mesh> _mesh = this->mesh;
	Image::bbox_t bb_img;
	Image::bbox_t bb_mesh;

	// Compute size differential :
	for (const auto& constraint : this->mesh_constraints) {
		bb_mesh.addPoint(this->mesh->getVertices()[constraint]);
	}
	for (const auto& constraint : this->image_constraints) {
		bb_img.addPoint(constraint);
	}
	float size_diff = glm::length(bb_img.getDiagonal()) / glm::length(bb_mesh.getDiagonal());

	// Compute centroid of the mesh for translation :
	glm::vec3 mesh_centroid;
	for (const auto& v : this->mesh->getVertices()) {
		mesh_centroid += v / static_cast<float>(this->mesh->getVertices().size());
	}

	// Apply transfo(s) to mesh :
	glm::mat4 base_transfo(1.f);
	glm::mat4 translate_to_origin = glm::translate(base_transfo, -mesh_centroid);
	glm::mat4 translate_to_centroid = glm::translate(base_transfo, mesh_centroid);
	glm::mat4 scaling_matrix = glm::scale(base_transfo, glm::vec3{size_diff, size_diff, size_diff});

	// Apply transfos :
	this->mesh->applyTransformation(translate_to_origin);
	this->mesh->applyTransformation(scaling_matrix);
	this->mesh->applyTransformation(translate_to_centroid);

	// Update data :
	/*
	this->mesh_draw->setTransformation(glm::mat4(1.f));
	this->updateMeshAndCurve_No_Image_Resizing();
	this->updateMeshInterface();
	if (this->curve) {
		// not covered in updateMeshAndCurve() :
		this->curve_draw->setTransformation(this->mesh_draw->getTransformation());
	}
	 */
}

void ARAPController::arap_computeDeformation() {
	if (this->mesh == nullptr) {
		std::cerr << "Error : no meshes loaded.\n";
		return;
	}
	if (this->mesh_interface == nullptr) {
		std::cerr << "Error : no manip interface initialized !!!\n";
		return;
	}

	std::cerr << "Generating vertex handles ..." << '\n';
	if (this->mesh_constraints.size() != this->image_constraints.size()) {
		std::cerr << "Error : cannot perform ARAP, not the same number of constraints.\n";
		return;
	}
	std::vector<std::pair<int, glm::vec3>> arap_handles;
	for (std::size_t i = 0; i < this->mesh_constraints.size(); ++i) {
		arap_handles.emplace_back(this->mesh_constraints[i], this->image_constraints[i]);
	}
	std::cerr << "Generated vertex handles." << '\n';

	std::cerr << "Setting handles on ARAP ...\n";
	std::cerr << "Computing constrained ARAP ...\n";
	this->mesh_interface->changedConstraints(arap_handles);
	std::cerr << "Computed constrained ARAP. Propagating vertex positions ...\n";

	this->mesh->setNewVertexPositions(this->mesh_interface->get_modified_vertices());
	this->mesh->update();

	this->scene->updateBoundingBox();
	std::cerr << "Finished.\n";

	/*
	 * TODO figure out how to port this here :
	this->updateMeshAndCurve_No_Image_Resizing();
	 */
}

void ARAPController::enableDeformation() {
	// TODO
}

void ARAPController::disableDeformation() {
	// TODO
}

void ARAPController::applyTransformation_Mesh() {
	// TODO
}

void ARAPController::updateCurveFromMesh() {
	// TODO
}

void ARAPController::saveMesh() {
	if (this->mesh == nullptr) { return; }
	// Ask the user for the save file name & its path :
	QString home_path = QDir::homePath();
	QString selected;
	QString q_file_name = "";
	q_file_name = QFileDialog::getSaveFileName(nullptr, "Save OFF file", home_path, "OFF files (*.off)", &selected, QFileDialog::DontUseNativeDialog);
	// Check if the user didn't cancel the dialog :
	if (q_file_name.isEmpty()) {
		std::cerr << "Error : no filename chosen.\n";
		return;
	}

	std::ofstream myfile;
	std::string filename = q_file_name.toStdString();
	myfile.open(filename.c_str());
	if (!myfile.is_open())
	{
		std::cout << filename << " cannot be opened" << std::endl;
		return;
	}

	auto vertices = this->mesh->getVertices();
	auto triangles = this->mesh->getTriangles();

	myfile << "OFF" << std::endl;
	myfile << (vertices.size()) << " " << triangles.size() << " 0" << std::endl;

	for( unsigned int v = 0 ; v < vertices.size() ; ++v ) {
		myfile << vertices[v].x << " " << vertices[v].y << " " << vertices[v].z << std::endl;
	}
	for( unsigned int t = 0 ; t < triangles.size() ; ++t ) {
		myfile << "3 " << (triangles[t][0]) << " " << (triangles[t][1]) << " " << (triangles[t][2]) << std::endl;
	}

	myfile.close();
	return;
}

void ARAPController::saveCurve() {
	if (this->curve == nullptr) { return; }
	// Ask the user for the save file name & its path :
	QString home_path = QDir::homePath();
	QString selected;
	QString q_file_name = "";
	q_file_name = QFileDialog::getSaveFileName(nullptr, "Save OBJ file", home_path, "Wavefront OBJ files (*.obj)", &selected, QFileDialog::DontUseNativeDialog);
	// Check if the user didn't cancel the dialog :
	if (q_file_name.isEmpty()) {
		std::cerr << "Error : no filename chosen.\n";
		return;
	}

	std::ofstream myfile;
	std::string filename = q_file_name.toStdString();
	myfile.open(filename.c_str());
	if(!myfile.is_open()) {
		std::cout << filename << " cannot be opened" << std::endl;
	}

	// Only have vertices in the curve !
	auto vertices = this->curve->getPositions();
	for( unsigned int v = 0 ; v < vertices.size() ; ++v ) {
		myfile << "v " << vertices[v].x << " " << vertices[v].y << " " << vertices[v].z << std::endl;
	}

	myfile.close();
	return;
}
