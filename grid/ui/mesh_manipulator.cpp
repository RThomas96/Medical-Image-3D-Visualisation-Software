#include "mesh_manipulator.hpp"
#include "manipulator.hpp"
#include "../deformation/mesh_deformer.hpp"
#include "../deformation/cage_surface_mesh.hpp"


namespace UITool {
    
    float MeshManipulator::getManipulatorSize(bool side) {
        if(this->mesh)
            if(side)
                return std::max(std::max(this->mesh->getDimensions()[0], this->mesh->getDimensions()[1]), this->mesh->getDimensions()[2])/130.;     
            else
                return std::min(std::min(this->mesh->getDimensions()[0], this->mesh->getDimensions()[1]), this->mesh->getDimensions()[2])/130.;     
        else
            return 10.;
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

            this->selectedManipulators.push_back(false);
            this->manipulatorsToDisplay.push_back(true);
			this->manipulators[i].lockPosition();
            this->manipulators[i].disable();
		}

        QObject::connect(&this->selection, &Selection::isSelecting, this, &DirectManipulator::checkSelectedManipulators);
	}

    void DirectManipulator::checkSelectedManipulators() {
        this->deselectManipulator(nullptr);
        std::vector<glm::vec3> positions;
        this->getAllPositions(positions);
        std::vector<bool> inSelection = this->selection.areInSelection(positions);
        for(int i = 0; i < inSelection.size(); ++i) {
            if(inSelection[i]) {
                this->selectManipulator(&this->manipulators[i]);
            }
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
            if(this->selectedManipulators[i])
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
        ptrdiff_t index = manipulator - &(this->manipulators[0]);
        if(!this->selectedManipulators[index]) {
            this->mesh->selectPts(manipulator->getManipPosition());
            this->selectedManipulators[index] = true;
        }
    }

    void DirectManipulator::deselectManipulator(Manipulator * manipulator) {
        this->mesh->deselectAllPts();
        for(int i = 0; i < this->selectedManipulators.size(); ++i) {
            this->selectedManipulators[i] = false;
        }
    }

    void DirectManipulator::keyPressed(QKeyEvent* e) {
        this->selection.keyPressed(e);
    }

    void DirectManipulator::keyReleased(QKeyEvent* e) {
        this->selection.keyReleased(e);
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

    void FreeManipulator::addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) {
        glm::vec3 manipulatorPosition;
        if(this->mesh->getPositionOfRayIntersection(origin, direction, minValue, maxValue, planePos, manipulatorPosition))
            this->addManipulator(manipulatorPosition);
    }

    void FreeManipulator::keyPressed(QKeyEvent* e) {

    }

    void FreeManipulator::keyReleased(QKeyEvent* e) {

    }

    /***/

	PositionManipulator::PositionManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh), manipulator(Manipulator(glm::vec3(0., 0., 0.))) {
		this->active = false;
        this->evenMode = false;
        QObject::connect(&(this->manipulator), &Manipulator::mouseRightButtonPressed, this, &PositionManipulator::selectManipulator);
        QObject::connect(&(this->manipulator), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &PositionManipulator::deselectManipulator);
        QObject::connect(&(this->manipulator), &Manipulator::isManipulated, this, &PositionManipulator::moveManipulator);
        QObject::connect(&(this->kid_manip), &RotationManipulator::moved, this, [this]() {this->moveManipulator(nullptr);});

        QObject::connect(this, &PositionManipulator::keyPressed, this, &PositionManipulator::keyPressed);

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
            if(this->kid_manip.mode_modification >= 7 && this->kid_manip.mode_modification <= 9 || this->kid_manip.mode_modification <= -7 && this->kid_manip.mode_modification >= -9) {
                glm::vec3 scale = this->kid_manip.getScaleVector();
                int axeToScale = -1;
                switch(this->kid_manip.mode_modification) {
                    case 7:
                        axeToScale = 0;
                        break;
                    case -7:
                        axeToScale = 0;
                        break;
                    case 8:
                        axeToScale = 1;
                        break;
                    case -8:
                        axeToScale = 1;
                        break;
                    case 9:
                        axeToScale = 2;
                        break;
                    case -9:
                        axeToScale = 2;
                        break;
                }
                if(this->evenMode) {
                    this->mesh->scale(glm::vec3(scale[axeToScale], scale[axeToScale], scale[axeToScale]));
                } else {
                    glm::vec3 finalScale = glm::vec3(1., 1., 1.);
                    finalScale[axeToScale] = scale[axeToScale];
                    this->mesh->scale(finalScale);
                }

                this->mesh->setOrigin(glm::vec3(this->kid_manip.Origine[0], this->kid_manip.Origine[1], this->kid_manip.Origine[2]));
                this->mesh->computeNormals();
                this->mesh->updatebbox();
            }
        }
    }

    void PositionManipulator::selectManipulator(Manipulator * manipulator) {
    }

    void PositionManipulator::deselectManipulator(Manipulator * manipulator) {
    }

    void PositionManipulator::keyPressed(QKeyEvent* e) {
        if(e->modifiers() == Qt::ControlModifier && !e->isAutoRepeat()) {
            this->evenMode = true;
        }
    }

    void PositionManipulator::keyReleased(QKeyEvent* e) {
        this->evenMode = false;
    }

    void PositionManipulator::mousePressed(QMouseEvent* e) {

    }

    void PositionManipulator::mouseReleased(QMouseEvent* e) {
        emit needRedraw();
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

    void CompManipulator::clearSelectedPoints() {
        int nbOfPointsToUndo = this->selectedPoints.size();
        std::cout << nbOfPointsToUndo << " points deleted" << std::endl;
        for(int nb = 0; nb < nbOfPointsToUndo; ++nb) {
            this->manipulatorsState[this->selectedPoints.back().first] = 0;
            this->manipulators.pop_back();
            this->manipulatorsToDisplay.pop_back();
            this->manipulatorsIsOnMesh.pop_back();
            this->manipulatorsState.pop_back();
            this->selectedPoints.pop_back();
        }
    }

    void CompManipulator::undo() {

        if(this->previousNumberOfSelectedPoints.size() == 0) {
            std::cout << "Nothing to undo" << std::endl;
            return;
        }

        int nbOfPointsToUndo = this->selectedPoints.size();
        if(this->previousNumberOfSelectedPoints.size() > 1) {
            int prevNumberOfSelectedPoint = this->previousNumberOfSelectedPoints[this->previousNumberOfSelectedPoints.size()-2];
            int nbSelectedPoint = this->selectedPoints.size();
            nbOfPointsToUndo = nbSelectedPoint - prevNumberOfSelectedPoint;
        }

        std::cout << nbOfPointsToUndo << " points deleted" << std::endl;
        for(int nb = 0; nb < nbOfPointsToUndo; ++nb) {
            this->manipulatorsState[this->selectedPoints.back().first] = 0;
            this->manipulators.pop_back();
            this->manipulatorsToDisplay.pop_back();
            this->manipulatorsIsOnMesh.pop_back();
            this->manipulatorsState.pop_back();
            this->selectedPoints.pop_back();
        }

        for(int i = 0; i < this->meshToRegister->vertices.size(); ++i) {
            this->meshToRegister->vertices[i] = this->previousPositions.back()[i];
            this->manipulators[i].setManipPosition(this->meshToRegister->vertices[i]);
        }
        this->previousPositions.pop_back();
        this->meshToRegister->computeNormals();
        this->meshToRegister->updatebbox();
    }

    void CompManipulator::assignPreviousSelectedPoints(const std::vector<std::pair<int, std::pair<int, glm::vec3>>>& previousSelectedPoints, const std::vector<std::vector<glm::vec3>>& previousPositions, std::vector<int> previousNumberOfSelectedPoints) {
        this->previousPositions = previousPositions;
        this->previousNumberOfSelectedPoints = previousNumberOfSelectedPoints;
        for(int i = 0; i < previousSelectedPoints.size(); ++i) {
            this->selectedPoints.push_back(std::make_pair(previousSelectedPoints[i].first, std::make_pair(previousSelectedPoints[i].second.first, previousSelectedPoints[i].second.second)));
            int addedIndex = this->selectedPoints.back().second.first;
            if(addedIndex >= this->manipulators.size()) {
                this->manipulators.push_back(Manipulator(this->selectedPoints.back().second.second));
                this->manipulators.back().setCustomConstraint();
                this->manipulatorsIsOnMesh.push_back(false);
                this->manipulatorsState.push_back(7);
                this->manipulatorsToDisplay.push_back(true);
                if(addedIndex != this->manipulators.size() - 1) {
                    std::cout << "Error in selected points, this will bug !!" << std::endl;
                    std::cout << "Added index: " << addedIndex << std::endl;
                    std::cout << "Size: " << this->manipulators.size() << std::endl;
                }
            }
        }
    }

    void CompManipulator::switchToSelectionMode() {
        if(!hasAMeshToRegister) {
            this->displayErrorNoMeshAssigned();
            return;
        }
        this->isOnSelectionMode = true;
        this->isSelectingFirstPoint = true;
        this->currentPairToSelect = this->selectedPoints.size();
        this->selectedPoints.push_back(std::make_pair(-1, std::make_pair(-1, glm::vec3(0., 0., 0.))));
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
            this->selectedPoints[this->currentPairToSelect].second.first = this->manipulators.size() - 1;
            this->selectedPoints[this->currentPairToSelect].second.second = this->manipulators.back().getManipPosition();
        }
    }

    void CompManipulator::validate() {
        if(this->selectedPoints[this->currentPairToSelect].first == -1) {
            std::cout << "No point selected on the mesh" << std::endl;
        }

        if(this->selectedPoints[this->currentPairToSelect].second.first == -1) {
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
            this->manipulatorsState[this->selectedPoints[i].second.first] = 7;
            this->manipulatorsToDisplay[this->selectedPoints[i].first] = true;
            this->manipulatorsToDisplay[this->selectedPoints[i].second.first] = true;
        }
    }

    void CompManipulator::apply() {
        if(this->isOnSelectionMode) {
            std::cout << "You are currently selected some points.\n Please Validate your selection before apply the transformation." << std::endl;
            return;
        }
        std::vector<int> verticesToFit;
        std::vector<glm::vec3> newPositions;
        std::cout << "Selected pairs are:" << std::endl;
        for(int i = 0; i < this->selectedPoints.size(); ++i) {
            std::cout << "Mesh: " << this->selectedPoints[i].first << std::endl;
            std::cout << "Grid: " << this->selectedPoints[i].second.first << std::endl;
            verticesToFit.push_back(this->selectedPoints[i].first);
            newPositions.push_back(this->selectedPoints[i].second.second);
        }
        this->meshToRegister->setARAPDeformationMethod();
        ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->meshToRegister->meshDeformer);
        if(deformer) {
            this->previousPositions.push_back(this->meshToRegister->vertices);
            deformer->fitToPointList(verticesToFit, newPositions);
        }
    }

    void CompManipulator::addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) {
        glm::vec3 manipulatorPosition;
        if(this->mesh->getPositionOfRayIntersection(origin, direction, minValue, maxValue, planePos, manipulatorPosition)) {
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
        return false;
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
                this->selectedPoints[this->currentPairToSelect].second.first = index;
                this->selectedPoints[this->currentPairToSelect].second.second = this->manipulators[index].getManipPosition();
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

    void CompManipulator::keyPressed(QKeyEvent* e) {

    }

    void CompManipulator::keyReleased(QKeyEvent* e) {

    }

    /***/

	ARAPManipulator::ARAPManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh) {
		this->active = false;
        this->moveMode = true;
        this->isSelecting = false;
        this->manipulators.reserve(positions.size());
		for (int i = 0; i < positions.size(); ++i) {
			this->manipulators.push_back(Manipulator(positions[i]));
            QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, &ARAPManipulator::displayManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, &ARAPManipulator::hideManipulator);

            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &ARAPManipulator::selectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonReleased, this, &ARAPManipulator::deselectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, &ARAPManipulator::moveManipulator);

            this->manipulatorsToDisplay.push_back(false);
            this->handles.push_back(false);
			this->manipulators[i].lockPosition();
            this->manipulators[i].disable();
            this->selectedManipulators.push_back(false);
		}
        QObject::connect(&this->selection, &Selection::isSelecting, this, &ARAPManipulator::checkSelectedManipulators);
        QObject::connect(&this->selection, &Selection::beginSelection, this, [this](){this->isSelecting = true; this->mesh->deselectAllPts(); std::fill(this->selectedManipulators.begin(), this->selectedManipulators.end(), false);});
        QObject::connect(&this->selection, &Selection::resetSelection, this, [this](){this->isSelecting = false;});
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

    void ARAPManipulator::checkSelectedManipulators() {
        std::vector<glm::vec3> positions;
        this->getAllPositions(positions);
        std::vector<bool> inSelection = this->selection.areInSelection(positions);
        for(int i = 0; i < inSelection.size(); ++i) {
            if(inSelection[i]) {
                this->selectManipulator(&this->manipulators[i]);
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
            if(this->selectedManipulators[i])
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
        ptrdiff_t index = manipulator - &(this->manipulators[0]);
        if(this->moveMode) {
            if(!this->selectedManipulators[index]) {
                this->mesh->selectPts(manipulator->getManipPosition());
                this->selectedManipulators[index] = true;
            }
        } else {
            ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
            if(!deformer) {
                std::cout << "WARNING: ARAP manipulator can be used only with the ARAP deformer !" << std::endl;
                return;
            }
            if(!this->handles[index]) {
                deformer->setHandle(index);
                this->handles[index] = true;
            } else {
                if(isSelecting)
                    return;
                deformer->unsetHandle(index);
                this->handles[index] = false;
            }
        }

    }

    void ARAPManipulator::deselectManipulator(Manipulator * manipulator) {
        if(this->moveMode) {
            this->mesh->deselectAllPts();
            for(int i = 0; i < this->selectedManipulators.size(); ++i) {
                this->selectedManipulators[i] = false;
            }
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

    void ARAPManipulator::keyPressed(QKeyEvent* e) {
        this->selection.keyPressed(e);
    }

    void ARAPManipulator::keyReleased(QKeyEvent* e) {
        this->selection.keyReleased(e);
    }
}
