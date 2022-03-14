#include "mesh_manipulator.hpp"
#include "manipulator.hpp"
#include "../deformation/mesh_deformer.hpp"
#include "../deformation/cage_surface_mesh.hpp"


namespace UITool {
    
    float MeshManipulator::getManipulatorSize() {
        return std::min(std::min(this->mesh->getDimensions()[0], this->mesh->getDimensions()[1]), this->mesh->getDimensions()[2])/130.;     
    }

	DirectManipulator::DirectManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh) {
		this->active = false;
        this->manipulators.reserve(positions.size());
		for (int i = 0; i < positions.size(); ++i) {
			this->manipulators.push_back(Manipulator(positions[i]));
            QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, &DirectManipulator::displayManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, &DirectManipulator::hideManipulator);

            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &DirectManipulator::selectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &DirectManipulator::deselectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, &DirectManipulator::moveManipulator);

            this->manipulatorsToDisplay.push_back(true);
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

    void DirectManipulator::addManipulator(const glm::vec3& position) {
        // Nothing to do here
        // We deactivate the add manipulator functionnality because we need a MeshDeformer to create a manipulator
        // And its hard to get from here
        //this->manipulators.push_back(Manipulator(position));
        //if(this->active) {
        //    this->manipulatorsToDisplay.push_back(true);
        //} else {
        //    this->manipulatorsToDisplay.push_back(false);
        //}
    }

    void DirectManipulator::removeManipulator(Manipulator * manipulatorToDisplay) {
        //this->manipulators.erase(this->manipulators.begin()+idx);
        //this->manipulatorsToDisplay.erase(this->manipulatorsToDisplay.begin()+idx);
    }

	void DirectManipulator::setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) {
		if (positions.size() == this->manipulators.size()) {
			for (int i = 0; i < this->manipulators.size(); ++i) {
				this->manipulators[i].setManipPosition(positions[i]);
				this->manipulators[i].setLastPosition(positions[i]);
			}
		} else {
			std::cerr << "WARNING: try to set [" << this->manipulators.size() << "] manipulators positions with a position vector of size [" << positions.size() << "]" << std::endl;
		}
	}

    void DirectManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
		for (int i = 0; i < this->manipulators.size(); ++i) {
            positions.push_back(this->manipulators[i].getManipPosition());
		}
    }

    void DirectManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
		for (int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
            toDisplay.push_back(this->manipulatorsToDisplay[i]);
        }
    }

    void DirectManipulator::getManipulatorsState(std::vector<State>& states) const {
        states.clear();
        for(int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
            State currentState = State::NONE;
            if(this->manipulators[i].isAtRangeForGrab)
                currentState = State::AT_RANGE;
            if(this->manipulators[i].isSelected)
                currentState = State::MOVE;
            states.push_back(currentState);
        }
    }

    void DirectManipulator::displayManipulator(Manipulator * manipulatorToDisplay) {
        if(!this->active)
            return;
        // Since vectors are organized sequentially, you can get an index by subtracting pointer to initial element 
        // from the pointer to the element you search
        ptrdiff_t index = manipulatorToDisplay - &(this->manipulators[0]);
        this->manipulatorsToDisplay[index] = true;
    }

    void DirectManipulator::hideManipulator(Manipulator * manipulatorToDisplay) {
        if(!this->active || manipulatorToDisplay->isSelected)
            return;
        // Since vectors are organized sequentially, you can get an index by subtracting pointer to initial element 
        // from the pointer to the element you search
        ptrdiff_t index = manipulatorToDisplay - &(this->manipulators[0]);
        this->manipulatorsToDisplay[index] = false;
    }

    bool DirectManipulator::isWireframeDisplayed() {
        return this->active;
    }

    void DirectManipulator::moveManipulator(Manipulator * manipulator) {
        this->mesh->movePoint(manipulator->lastPosition, manipulator->getManipPosition());
        Q_EMIT needSendTetmeshToGPU();
    }

    void DirectManipulator::selectManipulator(Manipulator * manipulator) {
        this->mesh->selectPts(manipulator->getManipPosition());
    }

    void DirectManipulator::deselectManipulator(Manipulator * manipulator) {
        this->mesh->deselectAllPts();
        //this->hideManipulator(manipulator);
    }

    /***/

	FreeManipulator::FreeManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh), manipulator(Manipulator(glm::vec3(0., 0., 0.))) {
		this->active = false;
        QObject::connect(&(this->manipulator), &Manipulator::mouseRightButtonPressed, this, &FreeManipulator::selectManipulator);
        QObject::connect(&(this->manipulator), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &FreeManipulator::deselectManipulator);
        QObject::connect(&(this->manipulator), &Manipulator::isManipulated, this, &FreeManipulator::moveManipulator);

        QObject::connect(this, &FreeManipulator::keyQReleased, this, [this]{setActivation(false);});
        QObject::connect(this, &FreeManipulator::rayIsCasted, this, &FreeManipulator::addManipulatorFromRay);
	}

    void FreeManipulator::setActivation(bool isActive) {
        this->active = isActive;
        if (this->active) {
            this->manipulator.setCustomConstraint();
        } else {
            this->manipulator.lockPosition();
        }
    }

    void FreeManipulator::addManipulator(const glm::vec3& position) {
        this->manipulator.setManipPosition(position);
        this->manipulator.setLastPosition(position);
        this->setActivation(true);
    }

    void FreeManipulator::removeManipulator(Manipulator * manipulatorToDisplay) {
        this->setActivation(false);
    }

	void FreeManipulator::setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) {
        //Do nothing, because we don't want to move these manipulator
	}

    void FreeManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
        positions.push_back(this->manipulator.getManipPosition());
    }

    void FreeManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
        toDisplay.push_back(this->active);
    }

    void FreeManipulator::getManipulatorsState(std::vector<State>& states) const {
        states.clear();
        State currentState = State::NONE;
        if(this->manipulator.isAtRangeForGrab)
            currentState = State::AT_RANGE;
        if(this->manipulator.isSelected)
            currentState = State::MOVE;
        states.push_back(currentState);
    }

    void FreeManipulator::displayManipulator(Manipulator * manipulatorToDisplay) {
        //Do nothin because display is directly link to active variable
        return;
    }

    void FreeManipulator::hideManipulator(Manipulator * manipulatorToDisplay) {
        //Do nothin because display is directly link to active variable
        return;
    }

    bool FreeManipulator::isWireframeDisplayed() {
        return false;
    }

    void FreeManipulator::moveManipulator(Manipulator * manipulator) {
        this->mesh->movePoint(manipulator->lastPosition, manipulator->getManipPosition());
        Q_EMIT needSendTetmeshToGPU();
    }

    void FreeManipulator::selectManipulator(Manipulator * manipulator) {
        this->mesh->selectPts(manipulator->getManipPosition());
    }

    void FreeManipulator::deselectManipulator(Manipulator * manipulator) {
        this->mesh->deselectAllPts();
        this->removeManipulator(manipulator);
    }

    void FreeManipulator::addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue) {
        glm::vec3 manipulatorPosition;
        if(this->mesh->getPositionOfRayIntersection(origin, direction, minValue, maxValue, manipulatorPosition))
            this->addManipulator(manipulatorPosition);
    }

    /***/

	PositionManipulator::PositionManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh), manipulator(Manipulator(glm::vec3(0., 0., 0.))) {
		this->active = false;
        QObject::connect(&(this->manipulator), &Manipulator::mouseRightButtonPressed, this, &PositionManipulator::selectManipulator);
        QObject::connect(&(this->manipulator), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &PositionManipulator::deselectManipulator);
        QObject::connect(&(this->manipulator), &Manipulator::isManipulated, this, &PositionManipulator::moveManipulator);
        QObject::connect(&(this->kid_manip), &RotationManipulator::moved, this, [this]() {this->moveManipulator(nullptr);});

        this->manipulator.setCustomConstraint();
        this->manipulator.setManipPosition(mesh->getOrigin());
        this->kid_manip.setOrigine(qglviewer::Vec(mesh->getOrigin()[0], mesh->getOrigin()[1], mesh->getOrigin()[2]));
	}

    void PositionManipulator::setActivation(bool isActive) {
        this->active = isActive;
        if (this->active) {
            this->manipulator.setCustomConstraint();
        } else {
            this->manipulator.lockPosition();
        }
    }

    void PositionManipulator::addManipulator(const glm::vec3& position) {
    }

    void PositionManipulator::removeManipulator(Manipulator * manipulatorToDisplay) {
    }

	void PositionManipulator::setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) {
        //Do nothing, because we don't want to move these manipulator
	}

    void PositionManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
        positions.push_back(this->manipulator.getManipPosition());
    }

    void PositionManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
        toDisplay.push_back(this->active);
    }

    void PositionManipulator::getManipulatorsState(std::vector<State>& states) const {
        states.clear();
        State currentState = State::NONE;
        if(this->manipulator.isAtRangeForGrab)
            currentState = State::AT_RANGE;
        if(this->manipulator.isSelected)
            currentState = State::MOVE;
        states.push_back(currentState);
    }

    void PositionManipulator::displayManipulator(Manipulator * manipulatorToDisplay) {
        //Do nothin because display is directly link to active variable
        return;
    }

    void PositionManipulator::hideManipulator(Manipulator * manipulatorToDisplay) {
        //Do nothin because display is directly link to active variable
        return;
    }

    bool PositionManipulator::isWireframeDisplayed() {
        return false;
    }

    void PositionManipulator::moveManipulator(Manipulator * manipulator) {
        if(manipulator) {
            this->mesh->setOrigin(manipulator->getManipPosition());
        } else {
            // For translation
            if(this->kid_manip.mode_modification >= 1 && this->kid_manip.mode_modification <= 3) {
                this->mesh->setOrigin(glm::vec3(this->kid_manip.Origine[0], this->kid_manip.Origine[1], this->kid_manip.Origine[2]));
            }
            // For rotation
            if(this->kid_manip.mode_modification >= 4 && this->kid_manip.mode_modification <= 6) {
                this->mesh->rotate(this->kid_manip.getRotationMatrix());
                this->mesh->setOrigin(glm::vec3(this->kid_manip.Origine[0], this->kid_manip.Origine[1], this->kid_manip.Origine[2]));
            }
            // For scale
            if(this->kid_manip.mode_modification >= 7 && this->kid_manip.mode_modification <= 9) {
                glm::vec3 scale = this->kid_manip.getScaleVector();
                int maxIdx = 0;
                float max = 0;
                for(int i = 0; i < 3; ++i) {
                    if(std::abs(1 - scale[i]) > max) {
                        maxIdx = i;
                        max = scale[i];
                    }
                }
                scale = glm::vec3(scale[maxIdx], scale[maxIdx], scale[maxIdx]);
                this->mesh->scale(scale);
                this->mesh->setOrigin(glm::vec3(this->kid_manip.Origine[0], this->kid_manip.Origine[1], this->kid_manip.Origine[2]));
            }
        }
    }

    void PositionManipulator::selectManipulator(Manipulator * manipulator) {
    }

    void PositionManipulator::deselectManipulator(Manipulator * manipulator) {
    }

    /***/

	CompManipulator::CompManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh) {
		this->active = false;
        this->hasAMeshToRegister = false;
        this->isOnSelectionMode = false;
        this->isSelectingFirstPoint = false;
        this->isSelectingSecondPoint = false;
        this->oneManipulatorWasAlreadyAdded = false;
        this->oneManipulatorIsAtRangeForGrab = false;
        this->currentPairToSelect = -1;
        QObject::connect(this, &CompManipulator::pointIsClickedInPlanarViewer, this, &CompManipulator::addManipulator);
        QObject::connect(this, &CompManipulator::rayIsCasted, this, &CompManipulator::addManipulatorFromRay);
	}

    void CompManipulator::switchToSelectionMode() {
        if(!hasAMeshToRegister) {
            this->displayErrorNoMeshAssigned();
            return;
        }
        this->isOnSelectionMode = true;
        this->isSelectingFirstPoint = true;
        this->currentPairToSelect = this->selectedPoints.size();
        this->selectedPoints.push_back(std::make_pair(-1, -1));
        for(int i = 0; i < this->manipulators.size(); ++i) {
            if(this->manipulatorsIsOnMesh[i]) {
                this->manipulatorsState[i] = 2;
                this->manipulatorsToDisplay[i] = true;
            } else {
                this->manipulatorsToDisplay[i] = false;
            }
            this->manipulators[i].lockPosition();
        }
    }

    void CompManipulator::assignMeshToRegister(BaseMesh * meshToRegister) {
        this->meshToRegister = meshToRegister;
        this->hasAMeshToRegister = true;

        std::vector<glm::vec3> vertices = meshToRegister->vertices;
        this->manipulators.reserve(vertices.size()*2.);
		for (int i = 0; i < vertices.size(); ++i) {
			this->manipulators.push_back(Manipulator(vertices[i]));
            this->manipulatorsIsOnMesh.push_back(true);
            QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, [this](){this->oneManipulatorIsAtRangeForGrab = true;});
            QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, [this](){this->oneManipulatorIsAtRangeForGrab = false;});
            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &CompManipulator::selectManipulator);

            this->manipulatorsToDisplay.push_back(false);
            this->manipulatorsState.push_back(0.);
			this->manipulators[i].lockPosition();
            this->manipulators[i].enable();
		}
    }

	void CompManipulator::setActivation(bool isActive) {
        this->active = isActive;
		if (this->active) {
			for (int i = 0; i < this->manipulators.size(); ++i) {
				this->manipulators[i].lockPosition();
                this->manipulators[i].enable();
			}
		} else {
			for (int i = 0; i < this->manipulators.size(); ++i) {
				this->manipulators[i].lockPosition();
                this->manipulators[i].disable();
			}
		}
	}

    void CompManipulator::addManipulator(const glm::vec3& position) {
        if(!this->active)
            return;
        if(!hasAMeshToRegister) {
            this->displayErrorNoMeshAssigned();
            return;
        }
        if(this->isSelectingFirstPoint) {
            std::cout << "Please first select a point on the mesh to register" << std::endl;
        } 

        if(this->isSelectingSecondPoint) {
            if(this->oneManipulatorWasAlreadyAdded && !this->oneManipulatorIsAtRangeForGrab) {
                this->manipulators.back().setManipPosition(position);
                this->manipulators.back().setCustomConstraint();
            } else {
                this->manipulators.push_back(Manipulator(position));
                this->manipulators.back().setCustomConstraint();
                this->manipulatorsIsOnMesh.push_back(false);
                this->manipulatorsState.push_back(5);
                this->manipulatorsToDisplay.push_back(true);
                this->oneManipulatorWasAlreadyAdded = true;
            }
            this->selectedPoints[this->currentPairToSelect].second = this->manipulators.size() - 1;
        }
    }

    void CompManipulator::validate() {
        if(this->selectedPoints[this->currentPairToSelect].first == -1) {
            std::cout << "No point selected on the mesh" << std::endl;
        }

        if(this->selectedPoints[this->currentPairToSelect].second == -1) {
            std::cout << "No point selected on the grid" << std::endl;
        }

        this->isOnSelectionMode = false;
        this->isSelectingFirstPoint = false;
        this->isSelectingSecondPoint = false;
        this->oneManipulatorWasAlreadyAdded = false;
        this->currentPairToSelect = -1;

        for(int i = 0; i < this->manipulators.size(); ++i) {
            if(this->manipulatorsIsOnMesh[i]) {
                this->manipulatorsState[i] = 0;
            } else {
                this->manipulatorsState[i] = 6;
            }
        }

        for(int i = 0; i < this->selectedPoints.size(); ++i) {
            this->manipulatorsState[this->selectedPoints[i].first] = 1;
            this->manipulatorsState[this->selectedPoints[i].second] = 7;
            this->manipulatorsToDisplay[this->selectedPoints[i].first] = true;
            this->manipulatorsToDisplay[this->selectedPoints[i].second] = true;
        }
    }

    void CompManipulator::apply() {
        std::vector<int> verticesToFit;
        std::vector<glm::vec3> newPositions;
        std::cout << "Selected pairs are:" << std::endl;
        for(int i = 0; i < this->selectedPoints.size(); ++i) {
            std::cout << "Mesh: " << this->selectedPoints[i].first << std::endl;
            std::cout << "Grid: " << this->selectedPoints[i].second << std::endl;
            verticesToFit.push_back(this->selectedPoints[i].first);
            newPositions.push_back(this->manipulators[this->selectedPoints[i].second].getManipPosition());
        }
        this->meshToRegister->setARAPDeformationMethod();
        ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->meshToRegister->meshDeformer);
        if(deformer) {
            deformer->fitToPointList(verticesToFit, newPositions);
        }
    }

    void CompManipulator::addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue) {
        glm::vec3 manipulatorPosition;
        if(this->mesh->getPositionOfRayIntersection(origin, direction, minValue, maxValue, manipulatorPosition)) {
            this->addManipulator(manipulatorPosition);
        }
    }

    void CompManipulator::removeManipulator(Manipulator * manipulatorToDisplay) {
        if(!this->active)
            return;
        if(!hasAMeshToRegister) {
            this->displayErrorNoMeshAssigned();
            return;
        }
        //ptrdiff_t index = manipulatorToDisplay - &(this->markerOnGrid[0]);
        //this->markerOnGrid.erase(this->markerOnGrid.begin()+index);
    }

	void CompManipulator::setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) {
	}

    void CompManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
        positions.clear();
		for (int i = 0; i < this->manipulators.size(); ++i) {
            positions.push_back(this->manipulators[i].getManipPosition());
		}
    }

    void CompManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
        toDisplay.clear();
		for (int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
            toDisplay.push_back(this->manipulatorsToDisplay[i]);
        }
    }

    void CompManipulator::getManipulatorsState(std::vector<State>& states) const {
        states.clear();
        for(int i = 0; i < this->manipulatorsState.size(); ++i) {
            State currentState = State::NONE;
            switch(this->manipulatorsState[i]) {
                // 0: not assigned to a point (neutral)(on mesh)
                // 1: assigned to a point (neutral)(on mesh)
                // 2: waiting to be selected (on mesh)
                // 3: selected, waiting for second point (on mesh)
                // 5: waiting for validation (on grid)
                // 7: assigned to a point (neutral)(on grid)
                case 0:
                    currentState = State::NONE;
                    break;
                case 1:
                    currentState = State::SELECTED;
                    break;
                case 2:
                    currentState = State::WAITING;
                    break;
                case 3:
                    currentState = State::AT_RANGE;
                    break;
                case 5:
                    currentState = State::MOVE;
                    break;
                case 7:
                    currentState = State::LOCK;
                    break;
            }
            if(this->manipulators[i].isAtRangeForGrab)
                currentState = State::AT_RANGE;
            states.push_back(currentState);
        }
    }

    void CompManipulator::displayManipulator(Manipulator * manipulatorToDisplay) {
    }

    void CompManipulator::hideManipulator(Manipulator * manipulatorToDisplay) {
    }

    bool CompManipulator::isWireframeDisplayed() {
        return this->active;
    }

    void CompManipulator::moveManipulator(Manipulator * manipulator) {
    }

    void CompManipulator::selectManipulator(Manipulator * manipulator) {
        if(!hasAMeshToRegister) {
            this->displayErrorNoMeshAssigned();
            return;
        }
        ptrdiff_t index = manipulator - &(this->manipulators[0]);
        if(this->isSelectingFirstPoint) {
            if(!this->manipulatorsIsOnMesh[index]) {
                std::cout << "Select a point on the mesh please" << std::endl;
                return;
            }
            for(int i = 0; i < this->manipulators.size(); ++i) {
                if(this->manipulatorsIsOnMesh[i]) {
                    this->manipulatorsState[i] = 4;// Not selected
                    this->manipulatorsToDisplay[i] = false;
                }
            }

            this->manipulatorsState[index] = 3;// Selected
            this->manipulatorsToDisplay[index] = true;// Selected
            this->selectedPoints[this->currentPairToSelect].first = index;
            this->isSelectingFirstPoint = false;
            this->isSelectingSecondPoint = true;
            return;
        } 

        if(this->isSelectingSecondPoint) {
            if(this->manipulatorsIsOnMesh[index]) {
                std::cout << "A marker in the mesh has already been selected, please select a point on the grid now" << std::endl;
                return;
            }

            if(index == this->manipulators.size() -1) {
                this->selectedPoints[this->currentPairToSelect].second = index;
            } else {
                std::cout << "This marker has already been assigned" << std::endl;
            }
        }
    }

    void CompManipulator::deselectManipulator(Manipulator * manipulator) {
    }

    void CompManipulator::displayErrorNoMeshAssigned() {
        std::cout << "Error: no mesh assigned" << std::endl;
    }

    /***/

	ARAPManipulator::ARAPManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh) {
		this->active = false;
        this->moveMode = true;
        this->manipulators.reserve(positions.size());
		for (int i = 0; i < positions.size(); ++i) {
			this->manipulators.push_back(Manipulator(positions[i]));
            QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, &ARAPManipulator::displayManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, &ARAPManipulator::hideManipulator);

            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &ARAPManipulator::selectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &ARAPManipulator::deselectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, &ARAPManipulator::moveManipulator);

            this->manipulatorsToDisplay.push_back(false);
            this->handles.push_back(false);
			this->manipulators[i].lockPosition();
            this->manipulators[i].disable();
		}
	}

	void ARAPManipulator::setActivation(bool isActive) {
        this->active = isActive;
		if (this->active) {
			for (int i = 0; i < this->manipulators.size(); ++i) {
				this->manipulators[i].setCustomConstraint();
                this->manipulatorsToDisplay[i] = false;
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

    void ARAPManipulator::addManipulator(const glm::vec3& position) {}

    void ARAPManipulator::removeManipulator(Manipulator * manipulatorToDisplay) {}

	void ARAPManipulator::setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) {
		if (positions.size() == this->manipulators.size()) {
			for (int i = 0; i < this->manipulators.size(); ++i) {
				this->manipulators[i].setManipPosition(positions[i]);
				this->manipulators[i].setLastPosition(positions[i]);
			}
		} else {
			std::cerr << "WARNING: try to set [" << this->manipulators.size() << "] manipulators positions with a position vector of size [" << positions.size() << "]" << std::endl;
		}
	}

    void ARAPManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
		for (int i = 0; i < this->manipulators.size(); ++i) {
            positions.push_back(this->manipulators[i].getManipPosition());
		}
    }

    void ARAPManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
		for (int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
            toDisplay.push_back(this->manipulatorsToDisplay[i]);
        }
    }

    void ARAPManipulator::getManipulatorsState(std::vector<State>& states) const {
        states.clear();
        for(int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
            State currentState = State::NONE;
            if(this->handles[i])
                currentState = State::LOCK;
            if(this->manipulators[i].isAtRangeForGrab)
                currentState = State::AT_RANGE;
            if(this->manipulators[i].isSelected)
                currentState = State::MOVE;
            states.push_back(currentState);
        }
    }

    void ARAPManipulator::displayManipulator(Manipulator * manipulatorToDisplay) {
        if(!this->active)
            return;
        ptrdiff_t index = manipulatorToDisplay - &(this->manipulators[0]);
        this->manipulatorsToDisplay[index] = true;
    }

    void ARAPManipulator::hideManipulator(Manipulator * manipulatorToDisplay) {
        if(!this->active || manipulatorToDisplay->isSelected)
            return;
        ptrdiff_t index = manipulatorToDisplay - &(this->manipulators[0]);
        if(!this->handles[index])
            this->manipulatorsToDisplay[index] = false;
    }

    bool ARAPManipulator::isWireframeDisplayed() {
        return this->active;
    }

    void ARAPManipulator::moveManipulator(Manipulator * manipulator) {
        if(!moveMode)
            return;
        ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
        if(!deformer) {
            std::cout << "WARNING: ARAP manipulator can be used only with the ARAP deformer !" << std::endl;
            return;
        }
        this->mesh->movePoint(manipulator->lastPosition, manipulator->getManipPosition());
        Q_EMIT needSendTetmeshToGPU();
    }

    void ARAPManipulator::selectManipulator(Manipulator * manipulator) {
        if(this->moveMode) {
            this->mesh->selectPts(manipulator->getManipPosition());
        } else {
            ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
            if(!deformer) {
                std::cout << "WARNING: ARAP manipulator can be used only with the ARAP deformer !" << std::endl;
                return;
            }
            ptrdiff_t index = manipulator - &(this->manipulators[0]);
            deformer->setHandle(index);
            this->handles[index] = true;
        }

    }

    void ARAPManipulator::deselectManipulator(Manipulator * manipulator) {
        if(this->moveMode) {
            this->mesh->deselectPts(manipulator->getManipPosition());
        } else {
            ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
            if(!deformer) {
                std::cout << "WARNING: ARAP manipulator can be used only with the ARAP deformer !" << std::endl;
                return;
            }
            //ptrdiff_t index = manipulator - &(this->manipulators[0]);
            //deformer->unsetHandle(index);
            //this->handles[index] = false;
        }
    }

    void ARAPManipulator::toggleMode() {
        std::cout << "Move mode set to: " << this->moveMode << std::endl;
        this->moveMode = !this->moveMode;
        if(!this->moveMode) {
            for(int i = 0; i < this->manipulators.size(); ++i) {
                this->manipulators[i].lockPosition();
            }
        } else {
            for(int i = 0; i < this->manipulators.size(); ++i) {
                this->manipulators[i].setCustomConstraint();
            }
        }
    }
}
