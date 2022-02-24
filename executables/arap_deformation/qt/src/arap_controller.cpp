#include "../include/arap_controller.hpp"

#include "executables/arap_deformation/viewer/include/arap_viewer.hpp"
#include "viewer/include/scene.hpp"

#include "glm/gtx/io.hpp"

#include <QFileDialog>
#include <QVBoxLayout>

ARAPController::ARAPController(Viewer* _v, Scene* _s) {
	this->viewer = _v;
	this->scene	 = _s;

	// Initialize the fields to a null value :
	this->mesh = nullptr;
	this->curve = nullptr;
	this->image = nullptr;
	this->mesh_interface = nullptr;
	this->arapManipulator = nullptr;
	this->rectangleSelection = nullptr;

	this->currently_edited_constraint = 0; // for now, no constraints edited. set to 0 to mean that special case.
	this->mesh_constraints.clear();
	this->image_constraints.clear();
	this->compounded_constraints.clear();

	this->listview_image_constraints = nullptr;
	this->listview_mesh_constraints = nullptr;

	this->button_load_image = nullptr;
	this->button_load_mesh = nullptr;
	this->button_load_constraints = nullptr;
	this->button_load_curve = nullptr;
	this->button_load_second_curve = nullptr;

	this->button_save_mesh = nullptr;
	this->button_save_curve = nullptr;
	this->button_save_json = nullptr;
	this->button_save_image = nullptr;

	this->button_align_arap = nullptr;
	this->button_scale_arap = nullptr;
	this->button_start_arap = nullptr;

	this->label_mesh_name = nullptr;
	this->label_mesh_info = nullptr;
	this->label_grid_name = nullptr;
	this->label_grid_info = nullptr;

	this->button_manip_select_all = nullptr;
	this->button_manip_select_none = nullptr;
	this->checkbox_enable_deformation = nullptr;

	this->dir_last_accessed = QDir::homePath();

	this->mesh_file_name = "";
	this->mesh_file_path = "";
	this->curve_file_path = "";
	this->curve_file_path = "";
	this->output_image_file_name = "";
	this->output_image_file_path = "";
	this->generated_mesh_save_path = "";

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
	this->button_load_second_curve = new QPushButton("Resize curve from other curve");

	this->button_save_mesh = new QPushButton("Save mesh");
	this->button_save_curve = new QPushButton("Save curve");
	this->button_save_json = new QPushButton("Save curve as JSON");
	this->button_save_image = new QPushButton("Save image");

	this->button_align_arap = new QPushButton("Align constraints");
	this->button_scale_arap = new QPushButton("Scale constraints");
	this->button_start_arap = new QPushButton("Perform deformation");

	this->button_manip_select_all = new QPushButton("Select all vertices");
	this->button_manip_select_none = new QPushButton("Unselect all vertices");

	this->checkbox_enable_deformation = new QCheckBox("Enable deformation");
	this->checkbox_enable_deformation->setCheckState(Qt::CheckState::Unchecked);

	this->label_mesh_name = new QLabel("<No mesh loaded>");
	this->label_mesh_info = new QLabel("Mesh info : N/A");
	this->label_curve_name = new QLabel("<No curve loaded>");
	this->label_curve_info = new QLabel("Curve info : N/A");
	this->label_grid_name = new QLabel("<No image loaded>");
	this->label_grid_info = new QLabel("Image info : N/A");

	this->initLayout();
	this->initSignals();

	this->setDeformationButtonsState(States::Initialized);
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
	widget_layout->addWidget(this->label_mesh_name);
	widget_layout->addWidget(this->label_mesh_info);
	widget_layout->addWidget(this->button_load_constraints);
	widget_layout->addWidget(this->button_load_curve);
	widget_layout->addWidget(this->label_curve_name);
	widget_layout->addWidget(this->label_curve_info);
	widget_layout->addWidget(this->button_load_second_curve);
	widget_layout->addWidget(this->button_load_image);
	widget_layout->addWidget(this->label_grid_name);
	widget_layout->addWidget(this->label_grid_info);
	widget_layout->addStretch(2);
	widget_layout->addWidget(separator_deformation);
	widget_layout->addWidget(label_deformation);
	widget_layout->addWidget(this->checkbox_enable_deformation);
	widget_layout->addWidget(this->button_manip_select_all);
	widget_layout->addWidget(this->button_manip_select_none);
	widget_layout->addWidget(this->button_align_arap);
	widget_layout->addWidget(this->button_scale_arap);
	widget_layout->addWidget(this->button_start_arap);
	widget_layout->addStretch(2);
	widget_layout->addWidget(separator_save);
	widget_layout->addWidget(label_save);
	widget_layout->addWidget(this->button_save_mesh);
	widget_layout->addWidget(this->button_save_curve);
	widget_layout->addWidget(this->button_save_image);
	widget_layout->addWidget(this->button_save_json);

	this->setLayout(widget_layout);
}

void ARAPController::setDeformationButtonsState(States new_state) {
	this->state = new_state;
	this->updateButtonsActivated();
}

void ARAPController::updateButtonsActivated() {
	// Disable signals from the widgets we change the state of :
	this->checkbox_enable_deformation->blockSignals(true);

	this->button_load_mesh->setEnabled(false);
	this->button_load_constraints->setEnabled(false);
	this->button_load_curve->setEnabled(false);
	this->button_load_image->setEnabled(false);
	this->button_load_second_curve->setEnabled(false);
	this->button_align_arap->setEnabled(false);
	this->button_scale_arap->setEnabled(false);
	this->button_start_arap->setEnabled(false);
	this->button_save_mesh->setEnabled(false);
	this->button_save_curve->setEnabled(false);
	this->button_save_image->setEnabled(false);
	this->button_save_json->setEnabled(false);
	this->button_manip_select_all->setEnabled(false);
	this->button_manip_select_none->setEnabled(false);
	this->checkbox_enable_deformation->setEnabled(false);
	this->checkbox_enable_deformation->setCheckState(Qt::CheckState::Unchecked);
	this->viewer->setDeformation(false);

	this->button_load_mesh->setEnabled(true);	// This one's always on
	if (this->state >= States::MeshLoaded) {
		this->button_load_constraints->setEnabled(true);
		this->button_save_mesh->setEnabled(true);
		this->button_load_curve->setEnabled(true);
		this->checkbox_enable_deformation->setEnabled(true);
		this->viewer->setDeformation(true);
	}
	if (this->state >= States::CurveLoaded) {
		this->button_save_curve->setEnabled(true);
		this->button_load_image->setEnabled(true);
	}
	if (this->state >= States::ImageLoaded) {
		this->button_align_arap->setEnabled(true);
		this->button_scale_arap->setEnabled(true);
		this->button_start_arap->setEnabled(true);
		this->button_save_image->setEnabled(true);
	}
	if (this->state >= States::Deformed) {
		// This is after the automatic ARAP deformation, not after a hand-made deformation.
		this->button_load_second_curve->setEnabled(true);
		this->button_save_json->setEnabled(true);
	}
	// Re-enable signals for widgets we possibly changed the state of :
	this->checkbox_enable_deformation->blockSignals(false);
}

void ARAPController::initSignals() {
	// Buttons to load the data :
	QObject::connect(this->button_load_mesh, &QPushButton::pressed, this, &ARAPController::loadMeshFromFile);
	QObject::connect(this->button_load_constraints, &QPushButton::pressed, this, &ARAPController::loadConstraintsFromFile);
	QObject::connect(this->button_load_curve, &QPushButton::pressed, this, &ARAPController::loadCurveFromFile);
	QObject::connect(this->button_load_image, &QPushButton::pressed, this, &ARAPController::loadImageFromFile);
	QObject::connect(this->button_load_second_curve, &QPushButton::pressed, this, &ARAPController::resizeCurveWithSecondCurve);
	// Buttons to save the data :
	QObject::connect(this->button_save_mesh, &QPushButton::pressed, this, &ARAPController::saveMesh);
	QObject::connect(this->button_save_curve, &QPushButton::pressed, this, &ARAPController::saveCurve);
	QObject::connect(this->button_save_image, &QPushButton::pressed, this, &ARAPController::saveImageToBinaryFile);
	QObject::connect(this->button_save_json, &QPushButton::pressed, this, &ARAPController::saveCurveAsJSON);
	// Buttons to control the ARAP deformation :
	QObject::connect(this->button_align_arap, &QPushButton::pressed, this, &ARAPController::arap_performAlignment);
	QObject::connect(this->button_scale_arap, &QPushButton::pressed, this, &ARAPController::arap_performScaling);
	QObject::connect(this->button_start_arap, &QPushButton::pressed, this, &ARAPController::arap_computeDeformation);

	QObject::connect(this->checkbox_enable_deformation, &QCheckBox::stateChanged, this, [this](int state) -> void {
		if (state == Qt::CheckState::Unchecked) {
			this->disableDeformation();
		} else {
			this->enableDeformation();
		}
	});

	QObject::connect(this, &ARAPController::imageIsLoaded, this, &ARAPController::updateGridInfoLabel);
	QObject::connect(this, &ARAPController::meshIsLoaded, this, &ARAPController::updateMeshInfoLabel);

	// Enable or disable the control panel at will :
	QObject::connect(this, &ARAPController::imageIsLoaded, this, [this]() -> void {
		this->viewer->enableControlPanel(true);
	});
	QObject::connect(this, &ARAPController::meshIsLoaded, this, [this]() -> void {
	  this->viewer->enableControlPanel(false);
	});
	QObject::connect(this, &ARAPController::curveIsLoaded, this, [this]() -> void {
	  this->viewer->enableControlPanel(false);
	});
}

const Mesh::Ptr& ARAPController::getMesh() const { return this->mesh; }
const Curve::Ptr& ARAPController::getCurve() const { return this->curve; }
const Image::Grid::Ptr& ARAPController::getImage() const { return this->image; }

const std::shared_ptr<SimpleManipulator>& ARAPController::getARAPManipulator() const { return this->arapManipulator; }
const std::shared_ptr<MMInterface<glm::vec3>>& ARAPController::getMeshInterface() const { return this->mesh_interface; }
const std::shared_ptr<RectangleSelection>& ARAPController::getRectangleSelection() const { return this->rectangleSelection; }

const std::size_t ARAPController::getCurrentlyEditedConstraint() const { return this->currently_edited_constraint > this->mesh_constraints.size() ? 0 : this->currently_edited_constraint; }
const std::vector<glm::vec3>& ARAPController::getImageConstraints() const { return this->image_constraints; }
const std::vector<std::size_t>& ARAPController::getMeshConstraints() const { return this->mesh_constraints; }
const std::vector<glm::vec3>& ARAPController::getCompoundedConstraints() const { return this->compounded_constraints; }

const std::vector<glm::vec3> ARAPController::getMeshConstraintsAsPositions() const {
	// return default-initialized vectors if some pre-conditions aren't met :
	if (this->mesh_constraints.empty()) { return {}; }
	if (this->compounded_constraints.empty()) { return {}; }

	using constraints_diff_t = std::decay_t<decltype(this->compounded_constraints)>::iterator::difference_type;
	// return a vector spanning from the beginning of the compounded constraints to the end of the mesh ones, while keeping in
	// mind the last element (the second _Iter) is NOT included in the vector created, it's treated as a past-the-end iterator :
	return {this->compounded_constraints.cbegin(), this->compounded_constraints.cbegin()+static_cast<constraints_diff_t>(this->mesh_constraints.size())};
}

void ARAPController::updateMeshInfoLabel() {
	if (this->mesh == nullptr) {
		this->label_mesh_name->setText("<No mesh loaded>");
		this->label_mesh_info->setText("Mesh info :\nN/A");
		return;
	}
	QString default_name = "Mesh name : %1";
	QString default_info = "%1 vertices, %2 MB on disk";

	this->label_mesh_name->setText(default_name.arg(this->mesh_file_name));
	this->label_mesh_name->setToolTip(this->label_mesh_name->text());
	elideText(this->label_mesh_name, this->label_mesh_name->text());
	QFileInfo mesh_file_info(this->mesh_file_path, this->mesh_file_name);
	this->label_mesh_name->setWordWrap(true);
	this->label_mesh_info->setWordWrap(true);
	this->label_mesh_info->setText(default_info.arg(this->mesh->getVertices().size()).arg(static_cast<float>(mesh_file_info.size()) / 1024.f / 1024.f));
}

void ARAPController::updateCurveInfoLabel() {
	if (this->curve == nullptr) {
		this->label_curve_name->setText("<No curve loaded>");
		this->label_curve_info->setText("Curve info :\nN/A");
		return;
	}
	QString default_name = "Curve name : %1";
	QString default_info = "%1 control points";

	this->label_curve_name->setText(default_name.arg(this->curve_file_name));
	this->label_curve_name->setToolTip(this->label_curve_name->text());
	elideText(this->label_curve_name, this->label_curve_name->text());

	QFileInfo curve_file_info(this->curve_file_path, this->curve_file_name);
	this->label_curve_name->setWordWrap(true);
	this->label_curve_info->setWordWrap(true);
	this->label_curve_info->setText(default_info.arg(this->curve->getPositions().size()));
}

void ARAPController::updateGridInfoLabel() {
	if (this->image == nullptr) {
		this->label_grid_name->setText("<No image loaded>");
		this->label_grid_info->setText("Image info :\nN/A");
		return;
	}
	QString default_name = "Grid name : %1";
	this->label_grid_name->setText(default_name.arg(this->image->getImageName().c_str()));
	this->label_grid_name->setWordWrap(true);
	this->label_grid_info->setWordWrap(true);

	auto dims = this->image->getResolution();
	auto type = this->image->getInternalDataType();
	std::size_t byte_size = dims.x * dims.y * dims.z * this->image->getVoxelDimensionality();

	// attempt to read the size on disk from the internal type :
	if (type & Image::ImageDataType::Bit_8) { byte_size *= 1u; }
	else if (type & Image::ImageDataType::Bit_16) { byte_size *= 2u; }
	else if (type & Image::ImageDataType::Bit_32) { byte_size *= 4u; }
	else if (type & Image::ImageDataType::Bit_64) { byte_size *= 8u; }

	// get it in float as MB :
	float byte_size_f = static_cast<float>(byte_size) / 1024.f / 1024.f;

	QString default_info = "%1x%2x%3 voxels, %4 MB on disk.";
	this->label_grid_info->setText(default_info.arg(dims.x).arg(dims.y).arg(dims.z).arg(byte_size_f));
	this->label_grid_name->setToolTip(this->label_grid_name->text());
	elideText(this->label_grid_name, this->label_grid_name->text());
}

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
		this->updateMeshInfoLabel();
		this->updateCurveInfoLabel();
		this->updateGridInfoLabel();
		this->setDeformationButtonsState(States::Initialized);
		std::cerr << "Done resetting the ARAP controller.\n";
	}

	// Create the mesh, load it and create the kd-tree structure :
	this->mesh = std::make_shared<Mesh>();
	FileIO::openOFF(file_name.toStdString(), this->mesh->getVertices(), this->mesh->getTriangles());
	this->mesh->update();

	this->mesh_file_name = mesh_file_info.fileName();
	this->mesh_file_path = this->dir_last_accessed;

	// Attempt to find a local constraint file next to it :
	QDir mesh_root(mesh_file_info.absolutePath());
	QDirIterator mesh_root_folder_iterator(mesh_root, QDirIterator::IteratorFlag::NoIteratorFlags);
	QString target_file_name(mesh_file_info.fileName() + ".constraints");

	bool has_loaded_constraints = false;

	while (mesh_root_folder_iterator.hasNext()) {
		QString current_filename = mesh_root_folder_iterator.next();
		QFileInfo current_file_info(current_filename);
		if (current_file_info.isFile() &&
			current_file_info.fileName().contains(target_file_name, Qt::CaseInsensitive)) {
			std::cerr << "Found mesh constraint file : " << current_file_info.fileName().toStdString() << "\n";
			std::string file_name_std = current_file_info.absoluteFilePath().toStdString();
			this->loadConstraintDataFromFile(file_name_std);
			has_loaded_constraints = true;
			break;
		}
	}

	// Upload data to the scene and update the viewer's camera and data :
	this->uploadMeshToScene();

	// Update Scene BB & sphere size for handles
	this->scene->updateBoundingBox();
	auto scene_radius = this->viewer->camera()->sceneRadius();

	// TODO : this only toggles the control panel and sets a boolean to false. Change it to include all of it here.
	this->viewer->resetDeformation();

	this->viewer->updateInfoFromScene();

	emit this->meshIsLoaded();

	if (has_loaded_constraints) {
		this->setDeformationButtonsState(States::MeshLoadedWithConstraints);
	} else {
		this->setDeformationButtonsState(States::MeshLoaded);
	}
}

void ARAPController::loadImageFromFile() {
	// TODO : import some of the LoaderWidget code, or make another importer widget
	emit requestImageLoad();
}

void ARAPController::setImagePointer(Image::Grid::Ptr& grid) {
	// Load image into the grid :
	this->viewer->makeCurrent();
	if (this->image != nullptr) {
		this->deleteGridData();
		this->image = nullptr;
	}
	this->image = grid;
	this->scene->arap_load_image_data(grid);
	std::cerr << "Adding grid to scene from ARAPController !\n";
	Image::bbox_t selected_grid_bb				 = this->image->getBoundingBox();
	Image::bbox_t::vec selected_grid_bb_diagonal = selected_grid_bb.getDiagonal();	  // gets the scale factors on X, Y, Z
	Image::bbox_t::vec selected_grid_bb_center	 = selected_grid_bb.getMin() + (selected_grid_bb_diagonal / 2.f);

	// The scaling done here is _very_ approximate in order to get a rough estimate of the size of the image :
	float scaling_factor	 = glm::length(selected_grid_bb_diagonal) / glm::length(this->mesh->getBB()[1] - this->mesh->getBB()[0]) * .7f;
	glm::mat4 scaling_matrix = glm::scale(glm::mat4(1.f), glm::vec3(scaling_factor));
	// We apply the transformation here in order to get an updated bounding box.

	selected_grid_bb.printInfo("Image bounding box on loading : ", "[LOG]");
	std::cerr << "\nTransformation before shifting the mesh to the side :\n" << scaling_matrix << "\n";

	auto mesh_bb = this->mesh->getBB();
	auto mesh_bb_real = BoundingBox_General<float>(mesh_bb[0], mesh_bb[1]);

	// And base the computation of the translations from the scaled bounding box.
	auto scaled_bb				   = mesh_bb_real.transformTo(scaling_matrix);
	auto mesh_to_image_translation = (selected_grid_bb.getMin() - scaled_bb.getMin());
	auto shift_image_translation   = glm::vec3(-scaled_bb.getDiagonal().x, .0f, .0f) + mesh_to_image_translation;
	// Determine the best transformation to apply by shifting the mesh's BB to be aligned with the image's BB, and
	// let the user put points later on the mesh in order to get a first alignment of the image/mesh.
	// Then, translate that by the mesh's bounding box in order to place them one beside another :
	scaling_matrix[3][0] += shift_image_translation.x;
	scaling_matrix[3][1] += shift_image_translation.y;
	scaling_matrix[3][2] += shift_image_translation.z;

	scaled_bb.printInfo("Image bounding box after scaling : ", "[LOG]");
	std::cerr << "Mesh to image : " << mesh_to_image_translation << " \n";
	std::cerr << "Estimated scaling and rotation : " << shift_image_translation << " and transfo : " << scaling_matrix << '\n';

	this->scene->getDrawableMesh()->setTransformation(scaling_matrix);
	this->scene->getDrawableMesh()->updateBoundingBox();
	this->scene->getDrawableMesh()->updateOnNextDraw();
	if (this->curve) {
		this->scene->getDrawableCurve()->setTransformation(scaling_matrix);
		this->scene->getDrawableCurve()->updateBoundingBox();
		this->scene->getDrawableCurve()->updateOnNextDraw();
	}
	this->scene->updateBoundingBox();
	this->viewer->doneCurrent();
	this->viewer->updateInfoFromScene();
	this->setDeformationButtonsState(States::ImageLoaded);
	emit this->imageIsLoaded();
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

	if (this->state < States::MeshLoadedWithConstraints) {
		this->setDeformationButtonsState(States::MeshLoadedWithConstraints);
	}
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
		this->updateCurveInfoLabel();

		this->curve_file_name = fi.fileName();
		this->curve_file_path = fi.filePath();

		this->setDeformationButtonsState(States::CurveLoaded);
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
		this->mesh_constraints.emplace_back(constraint);
	}

	// Debug output :
	std::cerr << "After constraint reading, constraints are : {";
	for (auto constraint : this->mesh_constraints) {
		std::cerr << constraint << " , ";
	}
	std::cerr << "}\n";

	constraints.close();

	if (not this->mesh_constraints.empty()) {
		// enable the constraint editing if this is the first constraint added !
		this->currently_edited_constraint = 1;
	}
	this->updateCompoundedConstraints();
}

void ARAPController::addImageConstraint(glm::vec3 img_ctx_pos) {
	if (this->image == nullptr) {
		std::cerr << "Error : no image was present.\n";
		return;
	}

	this->image_constraints.emplace_back(img_ctx_pos);
	std::cerr << "Added constraint to the image at position " << img_ctx_pos << '\n';
	// increment edited constraint if there were some :
	if (currently_edited_constraint != 0) {
		this->currently_edited_constraint++;
		if (this->currently_edited_constraint > this->mesh_constraints.size()) {
			// set it to past-the-end, but no further.
			this->currently_edited_constraint = this->mesh_constraints.size()+1;
		} else {
			std::cerr << "Warning : adding more image constraints than there are mesh constraints.\n";
			std::cerr << "Constraints : " << this->mesh_constraints.size() << " and current one : " <<
				this->currently_edited_constraint << '\n';
		}
	} else {
		std::cerr << "Warning : added image constraint but no mesh constraints were present.\n";
	}
	this->updateCompoundedConstraints();
}

void ARAPController::addMeshConstraint(std::size_t mesh_ctx_idx) {
	if (this->mesh_constraints.empty()) {
		// enable the constraint editing if this is the first constraint added !
		this->currently_edited_constraint = 1;
	}
	this->mesh_constraints.emplace_back(mesh_ctx_idx);
	std::cerr << "Added constraint to the mesh for vertex " << mesh_ctx_idx << '\n';
	this->updateCompoundedConstraints();
}

void ARAPController::updateCompoundedConstraints() {
	this->compounded_constraints.resize(this->image_constraints.size() + this->mesh_constraints.size(), glm::vec3{.0f});
	// Copy the mesh constraints :
	const auto& vertices = this->mesh->getVertices();
	for (std::size_t i = 0; i < this->mesh_constraints.size(); ++i) {
		this->compounded_constraints[i] = vertices[this->mesh_constraints[i]];
	}

	/// Typedef to the difference type between two iterators for the vector type of 'compounded_constraints'
	using diff_t = std::decay_t<decltype(this->compounded_constraints)>::iterator::difference_type;

	// Copy the image constraints :
	std::copy(this->image_constraints.cbegin(), this->image_constraints.cend(), this->compounded_constraints.begin() + static_cast<diff_t>(this->mesh_constraints.size()));
}

void ARAPController::deleteMeshData() {
	this->viewer->makeCurrent();
	this->viewer->clearMeshData();
	this->compounded_constraints.clear();
	this->mesh_constraints.clear();
	this->image_constraints.clear();
	this->currently_edited_constraint = 0;
	this->mesh_file_name = this->mesh_file_path = "";
	this->viewer->doneCurrent();
	this->mesh.reset();
}

void ARAPController::deleteCurveData() {
	this->viewer->makeCurrent();
	this->viewer->clearCurveData(); // also clears img
	this->curve_file_name = this->curve_file_path = "";
	this->viewer->doneCurrent();
	this->curve.reset();
}

void ARAPController::deleteGridData() {
	this->viewer->makeCurrent();
	this->viewer->clearImageData();
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
	std::cerr << "Updating mesh drawable ...\n";
	/**
	 * Should :
	 * update mesh/curve drawables
	 * update curve positions
	 * update scene BB
	 */
	this->viewer->makeCurrent();
	this->scene->getDrawableMesh()->updateBoundingBox();
	this->scene->getDrawableMesh()->updateOnNextDraw();
	auto bb = this->scene->getDrawableMesh()->getBoundingBox();
	std::cerr << "After mesh drawable update ------\n";
	std::cerr << bb.first << " // " << bb.second << '\n';
	this->updateCompoundedConstraints();
	//if (this->curve) {
	//	this->curve->deformFromMeshData();
	//	this->updateCurveDrawable();
	//}
	this->viewer->doneCurrent();
	this->scene->updateBoundingBox();
}

void ARAPController::updateCurveDrawable() {
	if (this->curve) {
		this->scene->getDrawableCurve()->updateOnNextDraw();
		this->scene->getDrawableCurve()->updateBoundingBox();
	}
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

		QObject::connect(this->button_manip_select_all, &QPushButton::pressed, this, [this]() -> void {
			this->mesh_interface->select_all();
		});
		QObject::connect(this->button_manip_select_none, &QPushButton::pressed, this, [this]() -> void {
			this->mesh_interface->unselect_all();
			this->mesh_interface->unfixe_all();
			this->arapManipulator->clear();
			this->rectangleSelection->deactivate();
		});

		this->viewer->initializeARAPManipulationInterface();

		this->mesh_interface->setMode(MeshModificationMode::REALTIME);
	}

	if (this->mesh != nullptr) {
		this->mesh_interface->clear();
		this->mesh_interface->loadAndInitialize(this->mesh->getVertices(), this->mesh->getTriangles());
		if (not this->mesh_constraints.empty()) {
			// If constraints there are, constraints thar shall be :
			std::vector<std::pair<int, glm::vec3>> arap_constraints{};
			for (int i = 0; i < static_cast<int>(this->compounded_constraints.size()); ++i){
				arap_constraints.emplace_back(this->mesh_constraints[i], this->compounded_constraints[i]);
			}
			this->mesh_interface->set_locked_vertices(arap_constraints);
		}
	}
}

void ARAPController::resetMeshInterface() {
	if (this->mesh_interface != nullptr) {
		this->arapManipulator->disconnect();
		this->rectangleSelection->disconnect();

		this->arapManipulator.reset();
		this->rectangleSelection.reset();
		this->mesh_interface.reset();

		this->button_manip_select_all->disconnect();
		this->button_manip_select_none->disconnect();
	}
	this->initializeMeshInterface();
}

void ARAPController::arap_performAlignment() {
	if (this->mesh == nullptr) {
		std::cerr << "Error : no meshes loaded.\n";
		return;
	}

	// Check if mesh interface hasn't been created yet. If not, there might also be a transform
	// not yet applied to the mesh : apply it and create the mesh interface & manipulators :
	if (this->mesh_interface == nullptr) {
		this->applyTransformation_Mesh();
		this->initializeMeshInterface();
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

	this->mesh_interface->loadAndInitialize(this->mesh->getVertices(), this->mesh->getTriangles());

	this->updateMeshDrawable();
	if (this->curve) {
		// not covered in updateMeshAndCurve() :
		this->scene->getDrawableCurve()->setTransformation(glm::mat4(1.f));
	}
	this->updateCurveFromMesh();
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

	// Check if mesh interface hasn't been created yet. If not, there might also be a transform
	// not yet applied to the mesh : apply it and create the mesh interface & manipulators :
	if (this->mesh_interface == nullptr) {
		this->applyTransformation_Mesh();
		this->initializeMeshInterface();
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
	bb_img.printInfo("Image BB is : ");
	bb_mesh.printInfo("Mesh constraints BB is : ");

	// Compute centroid of the mesh for translation :
	glm::vec3 mesh_centroid{.0f, .0f, .0f};
	for (const auto& v : this->mesh->getVertices()) {
		mesh_centroid += v / static_cast<float>(this->mesh->getVertices().size());
	}

	// Apply transfo(s) to mesh :
	glm::mat4 base_transfo(1.f);
	glm::mat4 translate_to_origin = glm::translate(base_transfo, -mesh_centroid);
	glm::mat4 translate_to_centroid = glm::translate(base_transfo, mesh_centroid);
	glm::mat4 scaling_matrix = glm::scale(base_transfo, glm::vec3{size_diff, size_diff, size_diff});
	std::cerr << "scaling size diff : " << size_diff << "\n";

	std::cerr << "=========== TRANSFORMATIONS =============\n";
	std::cerr << translate_to_origin << '\n';
	std::cerr << translate_to_centroid << '\n';
	std::cerr << scaling_matrix << '\n';

	// Apply transfos :
	this->mesh->applyTransformation(translate_to_origin);
	this->mesh->applyTransformation(scaling_matrix);
	this->mesh->applyTransformation(translate_to_centroid);

	// Update data :
	this->updateMeshDrawable();
	this->updateCurveFromMesh();
}

void ARAPController::arap_computeDeformation() {
	if (this->mesh == nullptr) {
		std::cerr << "Error : no meshes loaded.\n";
		return;
	}
	if (this->mesh_constraints.empty()) {
		std::cerr << "Error : Cannot deform with no constraints.\n";
		return;
	}
	this->updateCompoundedConstraints(); // We'll need them later, better make sure they're updated.

	// Check if mesh interface hasn't been created yet. If not, there might also be a transform
	// not yet applied to the mesh : apply it and create the mesh interface & manipulators :
	if (this->mesh_interface == nullptr) {
		this->applyTransformation_Mesh();
		this->initializeMeshInterface();
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
	this->mesh_interface->set_locked_vertices(arap_handles);
	std::cerr << "Computing constrained ARAP ...\n";
	this->mesh_interface->changedConstraints(arap_handles);
	std::cerr << "Computed constrained ARAP. Propagating vertex positions ...\n";

	this->mesh->setNewVertexPositions(this->mesh_interface->get_modified_vertices());
	this->mesh->update();

	this->scene->updateBoundingBox();
	std::cerr << "Finished.\n";
	this->updateMeshDrawable();
	this->updateCurveFromMesh();
	this->updateCompoundedConstraints();
	this->setDeformationButtonsState(States::Deformed);
	this->updateButtonsActivated();
}

void ARAPController::resizeCurveWithSecondCurve() {
	if (this->mesh == nullptr || this->curve == nullptr || this->image == nullptr) {
		std::cerr << "Error : cannot resize a curve when none of a mesh, a curve or an image are loaded.\n";
		return;
	}
	std::cerr << "Loading another curve file ...\n";

	// load the other curve :
	QString file_name = QFileDialog::getOpenFileName(nullptr, "Open a Curve file (OBJ)", this->dir_last_accessed, "OBJ files (*.obj)");
	if (file_name.isEmpty() || not QFileInfo::exists(file_name)) {
		std::cerr << "Error : nothing to open.\nFile path given : \"" << file_name.toStdString() << "\"\n";
		return;
	}

	QFileInfo curve_file_info(file_name);
	this->dir_last_accessed = curve_file_info.absolutePath();

	// Read points :
	std::vector<glm::vec3> other_curve_positions;
	std::vector<Triangle> other_curve_triangles;
	FileIO::objLoader(file_name.toStdString(), other_curve_positions, other_curve_triangles);

	const auto positions = this->curve->getPositions();
	if (positions.size() != other_curve_positions.size()) {
		std::cerr << "Warning : both curves don't have the same number of points !\n";
	}

	// Get the curve lengths
	std::vector<float> current_curve_lengths(positions.size() - 1, .0f);
	std::vector<float> other_curve_lengths(other_curve_positions.size() - 1, .0f);
	float total_length_current = .0f, total_length_other = .0f;
	for (std::size_t i = 0; i < other_curve_lengths.size() - 1 || i < current_curve_lengths.size() - 1; ++i) {
		if (i < current_curve_lengths.size() - 1) { current_curve_lengths[i] = glm::length(positions[i+1] - positions[i]); total_length_current += current_curve_lengths[i]; }
		if (i < other_curve_lengths.size() - 1) { other_curve_lengths[i] = glm::length(other_curve_positions[i+1] - other_curve_positions[i]); total_length_other += other_curve_lengths[i]; }
	}
	float resize_factor = total_length_other / total_length_current;

	std::cerr << "Got both curves' lengths. Building new curve CPs ...\n";
	std::cerr << "Lengths : " << total_length_current << ", " << total_length_other << " // Diff : " << resize_factor << "\n";

	std::vector<glm::vec3> new_positions(positions.size(), glm::vec3{});
	new_positions[0] = positions[0];
	std::cerr << "Resizing segments .\n";
	std::cerr << "Old position : " << positions[0] << " // New position : " << new_positions[0] << '\n';
	// Resize them according to their tangent :
	for (std::size_t i = 1; i < positions.size(); ++i) {
		glm::vec3 prev = positions[i-1], current = positions[i];
		glm::vec3 current_tangent = current - prev;
		/*
		float factor = 1.f;
		if (i <= other_curve_lengths.size()) {
			factor = other_curve_lengths[i-1] / current_curve_lengths[i-1];
		}
		 */

		new_positions[i] = new_positions[i-1] + current_tangent * resize_factor;
		std::cerr << "Old position : " << positions[i] << " // New position : " << new_positions[i] << '\n';
	}

	this->curve->setPositions(new_positions);
	this->curve->computeWeightsFromMeshData();

	std::cerr << "Set new positions for the curve !\n";

	this->curve->update();
	this->scene->getDrawableCurve()->updateOnNextDraw();
	this->scene->getDrawableCurve()->updateBoundingBox();
	this->scene->updateBoundingBox();
}

void ARAPController::enableDeformation() {
	if (this->mesh == nullptr) { return; }
	std::cerr << "Enabled deformation !\n";
	this->applyTransformation_Mesh();
	this->updateMeshDrawable();
	this->updateCurveFromMesh();
	this->resetMeshInterface();
	this->button_manip_select_all->setEnabled(true);
	this->button_manip_select_none->setEnabled(true);
	this->viewer->setDeformation(true);
}

void ARAPController::disableDeformation() {
	if (this->mesh == nullptr) { return; }
	this->resetMeshInterface();
	this->button_manip_select_all->setEnabled(false);
	this->button_manip_select_none->setEnabled(false);
	this->viewer->setDeformation(false);
	std::cerr << "Disabled deformation !\n";
}

void ARAPController::applyTransformation_Mesh() {
	auto draw = this->scene->getDrawableMesh();
	auto transform = draw->getTransformation();

	this->mesh->applyTransformation(transform);
	draw->setTransformation(glm::mat4(1.f));
	draw->updateBoundingBox();
	draw->updateOnNextDraw();
	if (this->curve) {
		this->curve->deformFromMeshData();
		this->curve->update();
		auto curve_draw = this->scene->getDrawableCurve();
		if (curve_draw) {
			curve_draw->setTransformation(glm::mat4(1.f));
			curve_draw->updateBoundingBox();
			curve_draw->updateOnNextDraw();
		}
	}
}

void ARAPController::updateCurveFromMesh() {
	if (this->curve == nullptr) { return; }
	this->curve->deformFromMeshData();
	this->curve->update();

	auto curve_draw = this->scene->getDrawableCurve();
	if (curve_draw) {
		curve_draw->updateBoundingBox();
		curve_draw->updateOnNextDraw();
	}
}

void ARAPController::saveMesh() {
	if (this->mesh == nullptr) { return; }
	// Ask the user for the save file name & its path :
	QString selected;
	QString q_file_name = "";
	q_file_name = QFileDialog::getSaveFileName(nullptr, "Save OFF file", this->dir_last_accessed, "OFF files (*.off)", &selected, QFileDialog::DontUseNativeDialog);
	// Check if the user didn't cancel the dialog :
	if (q_file_name.isEmpty()) {
		std::cerr << "Error : no filename chosen.\n";
		return;
	}
	if (not q_file_name.endsWith(".off", Qt::CaseSensitivity::CaseInsensitive)) {
		q_file_name += ".off";
	}
	this->dir_last_accessed = QFileInfo(q_file_name).absolutePath();

	this->generated_mesh_save_path = QFileInfo(q_file_name).absoluteFilePath();

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
	QString selected;
	QString q_file_name = "";
	q_file_name = QFileDialog::getSaveFileName(nullptr, "Save OBJ file", this->dir_last_accessed, "Wavefront OBJ files (*.obj)", &selected, QFileDialog::DontUseNativeDialog);
	// Check if the user didn't cancel the dialog :
	if (q_file_name.isEmpty()) {
		std::cerr << "Error : no filename chosen.\n";
		return;
	}
	if (not q_file_name.endsWith(".obj", Qt::CaseSensitivity::CaseInsensitive)) {
		q_file_name += ".obj";
	}
	this->dir_last_accessed = QFileInfo(q_file_name).absolutePath();

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

void ARAPController::saveImageToBinaryFile() {
	if (this->image == nullptr) { return; }
	QString selected;
	QString q_file_name = "";
	q_file_name = QFileDialog::getSaveFileName(nullptr, "Save Image file", this->dir_last_accessed, "ASCII files (*.bin)", &selected, QFileDialog::DontUseNativeDialog);
	// Check if the user didn't cancel the dialog :
	if (q_file_name.isEmpty()) {
		std::cerr << "Error : no filename chosen.\n";
		return;
	}
	if (not q_file_name.endsWith(".bin", Qt::CaseSensitivity::CaseInsensitive)) {
		q_file_name += ".bin";
	}
	this->dir_last_accessed = QFileInfo(q_file_name).absolutePath();

	this->output_image_file_path = QFileInfo(q_file_name).absolutePath();
	this->output_image_file_name = QFileInfo(q_file_name).fileName();
	// Create CImg handle, dump image to raw binary format and call it a day
	auto img_dimensions = this->image->getResolution();
	auto d = this->image->getVoxelDimensionality();
	cimg_library::CImg<std::uint16_t> raw_file = cimg_library::CImg(img_dimensions.x, img_dimensions.y, img_dimensions.z, d);

	std::cout << "Going through the image and setting pixel values ...\n";
	std::vector<std::uint16_t> pixel_values;
	for (std::size_t z = 0; z < img_dimensions.z; ++z) {
		for (std::size_t y = 0; y < img_dimensions.y; ++y) {
			for (std::size_t x = 0; x < img_dimensions.x; ++x) {
				// read pixel for all color channels now :
				this->image->readPixel(svec3(x,y,z), pixel_values);
				// dump to CImg buffer :
				for (std::size_t c = 0; c < d; ++c) {
					raw_file(x, y, z, c) = pixel_values[c]; // TODO Maybe filter values here ?
				}
			}
		}
		std::cout << "\tWrote slice " << z << " ...\n";
	}

	QDir output_dir(this->output_image_file_path);
	std::string path = std::string(output_dir.absoluteFilePath(this->output_image_file_name).toStdString());
	raw_file.save_ascii(path.c_str());
	std::cout << "Saved file to " << output_dir.absoluteFilePath(this->output_image_file_name).toStdString() << " !\n";

	return;
}

void ARAPController::saveCurveAsJSON() {
	if (this->mesh == nullptr || this->curve == nullptr) {
		return;
	}

	if (this->generated_mesh_save_path.isEmpty()) {
		QMessageBox::information(this, "Warning", "Warning : you have not yet saved the mesh. In order to save this curve as a JSON file, you must first choose a save location for the mesh.");
		this->saveMesh();
		if (this->generated_mesh_save_path.isEmpty()) {
			QMessageBox::information(this, "Operation aborted", "Saving of the curve as a JSON file was aborted.");
			return;
		}
	}
	if (this->output_image_file_path.isEmpty()) {
		QMessageBox::information(this, "Warning", "Warning : you have not yet saved the image. In order to save this curve and mesh as a JSON file, you must first choose a save location for the image.");
		this->saveImageToBinaryFile();
		if (this->output_image_file_path.isEmpty()) {
			QMessageBox::information(this, "Operation aborted", "Saving of the curve as a JSON file was aborted.");
			return;
		}
	}

	// Ask for the file path to save it to :
	QString selected;
	QString q_file_name = "";
	q_file_name = QFileDialog::getSaveFileName(nullptr, "Save JSON file", this->dir_last_accessed, "JSON files (*.json)", &selected, QFileDialog::DontUseNativeDialog);
	// Check if the user didn't cancel the dialog :
	if (q_file_name.isEmpty()) {
		std::cerr << "Error : no filename chosen.\n";
		return;
	}
	if (not q_file_name.endsWith(".json", Qt::CaseSensitivity::CaseInsensitive)) {
		q_file_name += ".json";
	}
	this->dir_last_accessed = QFileInfo(q_file_name).absolutePath();

	std::function<void(QJsonArray&, glm::vec3)> vec3ToJSON = [](QJsonArray& parent, glm::vec3 v) -> void {
		QJsonArray vec_array;
		vec_array.push_back(v.x);
		vec_array.push_back(v.y);
		vec_array.push_back(v.z);
		parent.push_back(vec_array);
	};

	// Top level object :
	QJsonObject curve_object;
	// Object with "control points" id :
	QJsonArray ctrl_pts_array;
	// Add all points to the curve JSON array :
	for (const auto& v : this->curve->getPositions()) {
		vec3ToJSON(ctrl_pts_array, v);
	}
	QJsonObject img_obj;
	QJsonArray vox_dims;
	QJsonArray imgsize_array; // array of the image dimensions. ironically, this will be strings since integers are not supported in Qt's JSON headers.
	auto dims = this->image->getResolution();
	auto d = this->image->getVoxelDimensionality();
	imgsize_array.push_back(QString::number(dims.x));
	imgsize_array.push_back(QString::number(dims.y));
	imgsize_array.push_back(QString::number(dims.z));
	imgsize_array.push_back(QString::number(d));
	vox_dims.push_back(this->image->getVoxelDimensions().x);
	vox_dims.push_back(this->image->getVoxelDimensions().y);
	vox_dims.push_back(this->image->getVoxelDimensions().z);
	img_obj.insert("image_dimensions", imgsize_array);
	img_obj.insert("voxel_dimensionality", QString::number(this->image->getVoxelDimensionality()));
	img_obj.insert("voxel_dimensions", vox_dims);
	img_obj.insert("image_path", this->output_image_file_path+QDir::separator()+this->output_image_file_name);
	// Add ctrl points :
	curve_object.insert("control points", ctrl_pts_array);
	curve_object.insert("mesh file", QDir(this->dir_last_accessed).relativeFilePath(this->generated_mesh_save_path));
	// add image file and image size :
	curve_object.insert("image_file",
		QDir(this->dir_last_accessed).relativeFilePath(
			QDir(this->output_image_file_path).absoluteFilePath(this->output_image_file_name)
		)
	);
	curve_object.insert("image", img_obj);

	QJsonDocument doc(curve_object);
	std::ofstream out_file(q_file_name.toStdString());
	if (not out_file.is_open()) {
		std::cerr << "Error : cannot open out JSON file.\n";
		return;
	}
	out_file << doc.toJson().toStdString();
	out_file.close();

	std::cerr << "Wrote JSON file at location \"" << q_file_name.toStdString() << "\" ...\n";
	return;
}

void elideText(QLabel* label_to_elide, QString text_to_elide) {
	QFontMetrics font_metrics(label_to_elide->font());
	int width = label_to_elide->width() - 2;
	QString clipped_text = font_metrics.elidedText(text_to_elide, Qt::ElideRight, width);
	label_to_elide->setText(clipped_text);
}
