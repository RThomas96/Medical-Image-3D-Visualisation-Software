#include "../include/manipulator.hpp"

namespace UITool {

	glm::vec3 Manipulator::getPosition() {
		double x = 0;
		double y = 0;
		double z = 0;
		this->manipulatedFrame.getPosition(x, y, z);
		return glm::vec3(x, y, z);
	}

	void Manipulator::setPosition(glm::vec3 position) {
		this->manipulatedFrame.setPosition(position[0], position[1], position[2]);
	}

	CustomConstraint::CustomConstraint() {
		this->constraintx = new qglviewer::LocalConstraint();
		this->constraintx->setTranslationConstraintType(qglviewer::AxisPlaneConstraint::AXIS);
		this->constraintx->setTranslationConstraintDirection(qglviewer::Vec(1., 0., 0.));

		this->constrainty = new qglviewer::LocalConstraint();
		this->constrainty->setTranslationConstraintType(qglviewer::AxisPlaneConstraint::AXIS);
		this->constrainty->setTranslationConstraintDirection(qglviewer::Vec(0., 1., 0.));

		this->constraintz = new qglviewer::LocalConstraint();
		this->constraintz->setTranslationConstraintType(qglviewer::AxisPlaneConstraint::AXIS);
		this->constraintz->setTranslationConstraintDirection(qglviewer::Vec(0., 0., 1.));
	}

	void CustomConstraint::constrainTranslation(qglviewer::Vec& t, qglviewer::Frame* const fr) {
		if (std::abs(t.x) > std::abs(t.y) && std::abs(t.x) > std::abs(t.z)) {
			this->constraintx->constrainTranslation(t, fr);
		} else if (std::abs(t.y) > std::abs(t.x) && std::abs(t.y) > std::abs(t.z)) {
			this->constrainty->constrainTranslation(t, fr);
		} else if (std::abs(t.z) > std::abs(t.x) && std::abs(t.z) > std::abs(t.y)) {
			this->constraintz->constrainTranslation(t, fr);
		}
	}

	void LockConstraint::constrainTranslation(qglviewer::Vec& t, qglviewer::Frame* const fr) {
		t[0] = 0;
		t[1] = 0;
		t[2] = 0;
	}

	MeshManipulator::MeshManipulator(int nbManipulators) :
		nbManipulators(nbManipulators), manipulators(std::vector<VertexManipulator>(nbManipulators)) {
		this->activeManipulator = 0;
		this->isActive			= false;

		this->commonConstraint = new CustomConstraint();
		this->lockConstraint   = new LockConstraint();

		for (int i = 0; i < nbManipulators; ++i) {
			this->manipulators[i].setAssignedIdx(215-i);
			this->lock(i);
		}
	}

	void MeshManipulator::setConstraint(int idx) {
		assert(idx < this->manipulators.size());
		this->manipulators[idx].getManipulatedFrame().setConstraint(this->commonConstraint);
	}

	void MeshManipulator::lock(int idx) {
		assert(idx < this->manipulators.size());
		this->manipulators[idx].getManipulatedFrame().setConstraint(this->lockConstraint);
	}

	int MeshManipulator::getGrabbedManipulator() {
		for (int i = 0; i < this->manipulators.size(); ++i) {
			if (this->manipulators[i].getManipulatedFrame().grabsMouse())
				return i;
		}
		return -1;
	}

	void MeshManipulator::getAllPositions(std::vector<glm::vec3>& vec) {
		for (int i = 0; i < this->manipulators.size(); ++i) {
			vec.push_back(this->manipulators[i].getPosition());
		}
	}

	void MeshManipulator::setAllPositions(std::vector<glm::vec3>& positions) {
		if (positions.size() == this->manipulators.size()) {
			for (int i = 0; i < this->manipulators.size(); ++i) {
				int assignedIdx = this->manipulators[i].getAssignedIdx();
				this->manipulators[i].setPosition(positions[assignedIdx]);
			}
		} else {
			std::cerr << "WARNING: try to set [" << this->manipulators.size() << "] manipulators positions with a position vector of size [" << positions.size() << "]" << std::endl;
		}
	}

	int MeshManipulator::setAssignedIdx(int idx, int i) {
		assert(idx < this->manipulators.size());
		assert(i < this->manipulators.size());
		manipulators[idx].setAssignedIdx(i);
	}

	int MeshManipulator::getAssignedIdx(int idx) {
		assert(idx < this->manipulators.size());
		return manipulators[idx].getAssignedIdx();
	}

	glm::vec3 MeshManipulator::getActiveManipulatorPos() {
		assert(this->activeManipulator >= 0);
		assert(this->activeManipulator < this->nbManipulators);
		return this->manipulators[this->activeManipulator].getPosition();
	}

	bool MeshManipulator::isActiveManipulatorManipuled() {
		assert(this->activeManipulator >= 0);
		//assert(this->activeManipulator < this->nbManipulators); Because this function is used even when MeshManipulator hasn't any manipulator
		this->updateActiveManipulator();
		return this->isActive && this->manipulators[this->activeManipulator].getManipulatedFrame().isManipulated();
	}

	Manipulator& MeshManipulator::getActiveManipulator() {
		assert(this->activeManipulator >= 0);
		assert(this->activeManipulator < this->nbManipulators);
		return this->manipulators[this->activeManipulator];
	}

	void MeshManipulator::setActiveManipulator(int idx) {
		assert(idx < this->manipulators.size());
		this->activeManipulator = idx;
	}

	int MeshManipulator::getNbManipulators() {
		return this->nbManipulators;
	}

	bool MeshManipulator::updateActiveManipulator() {
		int newActiveManipulator = this->getGrabbedManipulator();
		if (this->isActive && newActiveManipulator != -1) {
			this->setActiveManipulator(newActiveManipulator);
			return true;
		} else {
			return false;
		}
	}

	int MeshManipulator::getActiveManipulatorAssignedIdx() {
		return this->manipulators[this->activeManipulator].getAssignedIdx();
	}

    void MeshManipulator::getActiveManipulatorPosition(glm::vec3& res) {
        res = this->manipulators[this->activeManipulator].getPosition();
    }

	void MeshManipulator::toggleActivation() {
		if (this->isActive) {
			for (int i = 0; i < this->nbManipulators; ++i) {
				this->lock(i);
			}
		} else {
			for (int i = 0; i < this->nbManipulators; ++i) {
				this->setConstraint(i);
			}
		}
		this->isActive = ! this->isActive;
	}
}	 // namespace UITool
