#include "../include/arap_controller.hpp"

#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "../../viewer/include/scene.hpp"

ARAPController::ARAPController(Viewer* _v, Scene* _s) {
	this->viewer = _v;
	this->scene	 = _s;

	this->init();
	this->initSignals();
}

void ARAPController::init() {
	//
}

void ARAPController::initSignals() {
	//
}

void ARAPController::addImageConstraint(glm::vec3 position) {
	this->image_constraints.push_back(glm::vec4{position, 1.f});
}

void ARAPController::addMeshConstraint(std::size_t vtx_idx) {
	this->mesh_constraints.push_back(vtx_idx);
}

void ARAPController::launchARAPInScene() {
	if (this->image_constraints.empty()) {
		std::cerr << "Error : no image constraints\n";
		return;
	}
	if (this->mesh_constraints.empty()) {
		std::cerr << "Error : no mesh constraints\n";
		return;
	}
	if (this->mesh_constraints.size() != this->image_constraints.size()) {
		std::cerr << "Error : not the same number of constraints.\n";
		return;
	}

	//
}