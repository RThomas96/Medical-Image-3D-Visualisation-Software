#include "../include/manipulator.hpp"
#include <limits>
#include "glm/gtx/string_cast.hpp" 

namespace UITool {

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

    /***/

    Manipulator::Manipulator(const glm::vec3& position) { 
        this->setPosition(position); 
        this->lastPosition = position;
        this->enable(); 
    }

	glm::vec3 Manipulator::getPosition() const {
		double x = 0;
		double y = 0;
		double z = 0;
		this->manipulatedFrame.getPosition(x, y, z);
		return glm::vec3(x, y, z);
	}

	void Manipulator::setPosition(const glm::vec3& position) {
		this->manipulatedFrame.setPosition(position[0], position[1], position[2]);
	}

	void Manipulator::setLastPosition(const glm::vec3& position) {
		this->lastPosition = position;
	}

	void Manipulator::lockPosition() {
		this->manipulatedFrame.setConstraint(new LockConstraint());
    }

	void Manipulator::setCustomConstraint() {
		this->manipulatedFrame.setConstraint(new CustomConstraint());
    }

    /***/

	DirectManipulator::DirectManipulator(const std::vector<glm::vec3>& positions) {
		this->active = false;
		for (int i = 0; i < positions.size(); ++i) {
			this->manipulators.push_back(Manipulator(positions[i]));
            this->manipulatorsToDisplay.push_back(false);
			this->manipulators[i].lockPosition();
            this->manipulators[i].disable();
		}
	}

	void DirectManipulator::setActivation(bool isActive) {
        this->active = isActive;
		if (this->active) {
			for (int i = 0; i < this->manipulators.size(); ++i) {
				this->manipulators[i].setCustomConstraint();
                this->manipulatorsToDisplay[i] = true;
                this->manipulators[i].enable();
			}
		} else {
			for (int i = 0; i < this->manipulators.size(); ++i) {
				this->manipulators[i].lockPosition();
                this->manipulatorsToDisplay[i] = false;
                this->manipulators[i].disable();
			}
		}
	}

    void DirectManipulator::getMovement(glm::vec3& origin, glm::vec3& target) {
        if(this->hasBeenMoved()) {
            int movedManipulatorIdx = this->getMovedManipulatorIdx();
            origin = this->manipulators[movedManipulatorIdx].getLastPosition();
            target = this->manipulators[movedManipulatorIdx].getPosition();
            std::cout << glm::to_string(origin) << "->" << glm::to_string(target) << std::endl;
            this->manipulators[movedManipulatorIdx].updateLastPosition();
        } else {
            std::cout << "Warning: try to request a movement when manipulator do not move" << std::endl;
            origin = glm::vec3(0., 0., 0.);
            target = glm::vec3(0., 0., 0.);
        }
    }

	bool DirectManipulator::hasBeenMoved() const {
		for (int i = 0; i < this->manipulators.size(); ++i) {
			//if (this->manipulators[i].getManipulatedFrame().grabsMouse()) // This check if the manipulator is grabbed
			if (this->manipulators[i].isManipulated())
				return true;
		}
		return false;
	}

    void DirectManipulator::addManipulator(const glm::vec3& position) {
        this->manipulators.push_back(Manipulator(position));
        if(this->active) {
            this->manipulatorsToDisplay.push_back(true);
        } else {
            this->manipulatorsToDisplay.push_back(false);
        }
    }

    void DirectManipulator::removeManipulator(const glm::vec3& position) {
        this->manipulators.erase(this->manipulators.begin()+this->getManipulatorIdx(position));
        this->manipulatorsToDisplay.erase(this->manipulatorsToDisplay.begin()+this->getManipulatorIdx(position));
    }

    void DirectManipulator::removeManipulator(int idx) {
        this->manipulators.erase(this->manipulators.begin()+idx);
        this->manipulatorsToDisplay.erase(this->manipulatorsToDisplay.begin()+idx);
    }

    int DirectManipulator::getManipulatorIdx(const glm::vec3& position) const {
        float distance = std::numeric_limits<float>::max();
        int res = 0;
        for(int i = 0; i < this->manipulators.size(); ++i) {
            const glm::vec3& p = this->manipulators[i].getPosition();
            float currentDistance = glm::distance(p, position);
            if(currentDistance < distance) {
                distance = currentDistance;
                res = i;
            }
        }
        return res;
    }

	int DirectManipulator::getNbManipulator() const {
		return this->manipulators.size();
	}

	void DirectManipulator::setAllPositions(const std::vector<glm::vec3>& positions) {
		if (positions.size() == this->manipulators.size()) {
			for (int i = 0; i < this->manipulators.size(); ++i) {
				this->manipulators[i].setPosition(positions[i]);
			}
		} else {
			std::cerr << "WARNING: try to set [" << this->manipulators.size() << "] manipulators positions with a position vector of size [" << positions.size() << "]" << std::endl;
		}
	}

    int DirectManipulator::getMovedManipulatorIdx() const {
		for (int i = 0; i < this->manipulators.size(); ++i) {
			if (this->manipulators[i].isManipulated())
				return i;
		}
		return -1;
    }

    void DirectManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
		for (int i = 0; i < this->manipulators.size(); ++i) {
            positions.push_back(this->manipulators[i].getPosition());
		}
    }

    void DirectManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
		for (int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
            toDisplay.push_back(this->manipulatorsToDisplay[i]);
        }
    }

    /***/

	FreeManipulator::FreeManipulator(const std::vector<glm::vec3>& positions): manipulator(Manipulator(glm::vec3(0., 0., 0.))) {
		this->active = false;
	}

    void FreeManipulator::setActivation(bool isActive) {
        this->active = isActive;
        if (this->active) {
            this->manipulator.setCustomConstraint();
        } else {
            this->manipulator.lockPosition();
        }
    }

    void FreeManipulator::getMovement(glm::vec3& origin, glm::vec3& target) {
        if(this->hasBeenMoved()) {
            origin = this->manipulator.getLastPosition();
            target = this->manipulator.getPosition();
            //std::cout << glm::to_string(origin) << "->" << glm::to_string(target) << std::endl;
            this->manipulator.updateLastPosition();
        } else {
            std::cout << "Warning: try to request a movement when manipulator->do not move" << std::endl;
            origin = glm::vec3(0., 0., 0.);
            target = glm::vec3(0., 0., 0.);
        }
    }

	bool FreeManipulator::hasBeenMoved() const {
		if (this->manipulator.isManipulated())
			return true;
		return false;
	}

    void FreeManipulator::addManipulator(const glm::vec3& position) {
        this->manipulator.setPosition(position);
        this->manipulator.setLastPosition(position);
        this->setActivation(true);
    }

    void FreeManipulator::removeManipulator(const glm::vec3& position) {}

    void FreeManipulator::removeManipulator(int idx) {
        this->setActivation(false);
    }

    int FreeManipulator::getManipulatorIdx(const glm::vec3& position) const {
        return 0;
    }

	int FreeManipulator::getNbManipulator() const {
        return 1;
	}

	void FreeManipulator::setAllPositions(const std::vector<glm::vec3>& positions) {
        //Do nothing, becaues we don't want to move these manipulator
	}

    int FreeManipulator::getMovedManipulatorIdx() const {
		return 0;
    }

    void FreeManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
        positions.push_back(this->manipulator.getPosition());
    }

    void FreeManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
        toDisplay.push_back(this->active);
    }
}	 // namespace UITool
