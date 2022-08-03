#include "mesh_manipulator.hpp"
#include "manipulator.hpp"
#include "../deformation/mesh_deformer.hpp"
#include "../deformation/cage_surface_mesh.hpp"
#include "../utils/PCATools.h"
#include <fstream>

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
        this->meshIsModified = false;
        this->kid_manip = nullptr;
        this->manipulators.reserve(positions.size());
		for (int i = 0; i < positions.size(); ++i) {
			this->manipulators.push_back(Manipulator(positions[i]));
            //QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, [this, i](){Q_EMIT needChangeSelectedPoints(std::make_pair(i, this->manipulators[i].getManipPosition()));});
            //QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, [this, i](){Q_EMIT needChangeSelectedPoints(std::make_pair(-1, this->manipulators[i].getManipPosition()));});
            QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, [this, i](){Q_EMIT needChangeSelectedPoints(std::make_pair(i, this->manipulators[i].getManipPosition()));});
            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, [this, i](){Q_EMIT needChangeSelectedPoints(std::make_pair(i, this->manipulators[i].getManipPosition()));});

            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &DirectManipulator::selectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &DirectManipulator::deselectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, &DirectManipulator::moveManipulator);

            this->selectedManipulators.push_back(false);
            this->manipulatorsToDisplay.push_back(true);
			this->manipulators[i].setCustomConstraint();
            this->manipulators[i].enable();
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
        // Since vectors are organized sequentially, you can get an index by subtracting pointer to initial element 
        // from the pointer to the element you search
        ptrdiff_t index = manipulatorToDisplay - &(this->manipulators[0]);
        this->manipulatorsToDisplay[index] = true;
    }

    void DirectManipulator::hideManipulator(Manipulator * manipulatorToDisplay) {
        if(manipulatorToDisplay->isSelected)
            return;
        // Since vectors are organized sequentially, you can get an index by subtracting pointer to initial element 
        // from the pointer to the element you search
        ptrdiff_t index = manipulatorToDisplay - &(this->manipulators[0]);
        this->manipulatorsToDisplay[index] = false;
    }

    void DirectManipulator::moveManipulator(Manipulator * manipulator) {
        ptrdiff_t index = manipulator - &(this->manipulators[0]);
        this->mesh->movePoint(index, manipulator->getManipPosition());
        Q_EMIT needSendTetmeshToGPU();
        this->meshIsModified = true;
    }

    void DirectManipulator::selectManipulator(Manipulator * manipulator) {
        ptrdiff_t index = manipulator - &(this->manipulators[0]);
        if(!this->selectedManipulators[index]) {
            this->selectedManipulators[index] = true;
        }
        std::cout << "Selection vertex: [" << index << "]" << std::endl;
    }

    void DirectManipulator::deselectManipulator(Manipulator * manipulator) {
        for(int i = 0; i < this->selectedManipulators.size(); ++i) {
            this->selectedManipulators[i] = false;
        }
    }

    void DirectManipulator::keyPressed(QKeyEvent* e) {};

    void DirectManipulator::keyReleased(QKeyEvent* e) {};

    void DirectManipulator::mousePressed(QMouseEvent*) {};

    void DirectManipulator::mouseReleased(QMouseEvent*) {
        if(this->meshIsModified) {
            mesh->addStateToHistory();
            this->meshIsModified = false;
        }
    };

    /***/

	FreeManipulator::FreeManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh), manipulator(Manipulator(glm::vec3(0., 0., 0.))) {
		this->active = false;
        this->kid_manip = nullptr;
        QObject::connect(&(this->manipulator), &Manipulator::mouseRightButtonPressed, this, &FreeManipulator::selectManipulator);
        QObject::connect(&(this->manipulator), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &FreeManipulator::deselectManipulator);
        QObject::connect(&(this->manipulator), &Manipulator::isManipulated, this, &FreeManipulator::moveManipulator);

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

    void FreeManipulator::moveManipulator(Manipulator * manipulator) {
        this->mesh->movePoint(0, manipulator->getManipPosition());
        Q_EMIT needSendTetmeshToGPU();
    }

    void FreeManipulator::selectManipulator(Manipulator * manipulator) {
    }

    void FreeManipulator::deselectManipulator(Manipulator * manipulator) {
        this->setActivation(false);
    }

    void FreeManipulator::addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) {
        glm::vec3 manipulatorPosition;
        if(this->mesh->getPositionOfRayIntersection(origin, direction, minValue, maxValue, planePos, manipulatorPosition))
            this->addManipulator(manipulatorPosition);
    }

    void FreeManipulator::keyPressed(QKeyEvent* e) {}

    void FreeManipulator::keyReleased(QKeyEvent* e) {
        if(e->key() == Qt::Key_Q && !e->isAutoRepeat()) {
            setActivation(false);
        }
    }

    void FreeManipulator::mousePressed(QMouseEvent*) {}

    void FreeManipulator::mouseReleased(QMouseEvent*) {
        mesh->addStateToHistory();
    }

    /***/

	PositionManipulator::PositionManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh) {
        this->meshIsModified = false;
        this->selection.disable();
        this->evenMode = false;
        this->kid_manip = new RotationManipulator();
        this->kid_manip->setOrigine(qglviewer::Vec(mesh->getOrigin()[0], mesh->getOrigin()[1], mesh->getOrigin()[2]));
        this->kid_manip->setRepX(qglviewer::Vec(mesh->coordinate_system[0].x, mesh->coordinate_system[0].y, mesh->coordinate_system[0].z));
        this->kid_manip->setRepY(qglviewer::Vec(mesh->coordinate_system[1].x, mesh->coordinate_system[1].y, mesh->coordinate_system[1].z));
        this->kid_manip->setRepZ(qglviewer::Vec(mesh->coordinate_system[2].x, mesh->coordinate_system[2].y, mesh->coordinate_system[2].z));
        for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
            this->kid_manip->addPoint(i, qglviewer::Vec(this->mesh->getVertice(i)[0], this->mesh->getVertice(i)[1], this->mesh->getVertice(i)[2]));
        }
        QObject::connect(this->kid_manip, &RotationManipulator::moved, this, [this]() {this->moveManipulator(nullptr);});
	}

	void PositionManipulator::setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) {
        //Do nothing, because we don't want to move these manipulator
	}

    void PositionManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
    }

    void PositionManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
    }

    void PositionManipulator::getManipulatorsState(std::vector<State>& states) const {
    }

    void PositionManipulator::moveManipulator(Manipulator * manipulator) {
        std::vector<glm::vec3> newPoints(this->mesh->getNbVertices());
        for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
            qglviewer::Vec deformedPoint;
            int trueIndex;
            this->kid_manip->getTransformedPoint(i, trueIndex, deformedPoint);
            newPoints[i] = glm::vec3(deformedPoint[0], deformedPoint[1], deformedPoint[2]);
        }
        this->mesh->movePoints(newPoints);
        this->meshIsModified = true;
        Q_EMIT needSendTetmeshToGPU();
    }

    void PositionManipulator::toggleEvenMode() {
        if(this->evenMode) {
            this->evenMode = false;
            this->kid_manip->evenMode = false;
        } else {
            this->evenMode = true;
            this->kid_manip->evenMode = true;
        }
    }

    void PositionManipulator::keyPressed(QKeyEvent* e) {
        //if(e->modifiers() == Qt::ControlModifier && !e->isAutoRepeat()) {
        //    this->evenMode = true;
        //    this->kid_manip->evenMode = true;
        //}
    }

    void PositionManipulator::keyReleased(QKeyEvent* e) {
        //this->evenMode = false;
        //this->kid_manip->evenMode = false;
    }

    void PositionManipulator::mousePressed(QMouseEvent* e) {}

    void PositionManipulator::mouseReleased(QMouseEvent* e) {
        if(this->meshIsModified) {
            emit needRedraw();
            emit needUpdateSceneCenter();
            this->mesh->coordinate_system[0] = glm::vec3(kid_manip->RepX.x, kid_manip->RepX.y, kid_manip->RepX.z);
            this->mesh->coordinate_system[1] = glm::vec3(kid_manip->RepY.x, kid_manip->RepY.y, kid_manip->RepY.z);
            this->mesh->coordinate_system[2] = glm::vec3(kid_manip->RepZ.x, kid_manip->RepZ.y, kid_manip->RepZ.z);
            mesh->addStateToHistory();
            this->meshIsModified = false;
        }
    }

    PositionManipulator::~PositionManipulator() {
        delete this->kid_manip;
    }

    /***/

	CompManipulator::CompManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh) {
        this->hasAMeshToRegister = false;
        this->isOnSelectionMode = false;
        this->isSelectingFirstPoint = false;
        this->isSelectingSecondPoint = false;
        this->oneManipulatorWasAlreadyAdded = false;
        this->oneManipulatorIsAtRangeForGrab = false;
        this->currentPairToSelect = -1;
        this->kid_manip = nullptr;
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

        for(int i = 0; i < this->meshToRegister->getNbVertices(); ++i) {
            //this->meshToRegister->vertices[i] = this->previousPositions.back()[i];
            this->manipulators[i].setManipPosition(this->meshToRegister->getVertice(i));
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

        std::vector<glm::vec3> vertices = meshToRegister->getVertices();
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

    void CompManipulator::addManipulator(const glm::vec3& position) {
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
            this->previousPositions.push_back(this->meshToRegister->getVertices());
            deformer->fitToPointList(verticesToFit, newPositions);
        }
    }

    void CompManipulator::addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) {
        glm::vec3 manipulatorPosition;
        if(this->mesh->getPositionOfRayIntersection(origin, direction, minValue, maxValue, planePos, manipulatorPosition)) {
            this->addManipulator(manipulatorPosition);
        }
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

    void CompManipulator::keyPressed(QKeyEvent* e) {}

    void CompManipulator::keyReleased(QKeyEvent* e) {}

    void CompManipulator::mousePressed(QMouseEvent* e) {}

    void CompManipulator::mouseReleased(QMouseEvent* e) {}

    /***/


	ARAPManipulator::ARAPManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh) {
        this->kid_manip = new RotationManipulator();
        QObject::connect(this->kid_manip, &RotationManipulator::moved, this, [this]() {this->moveGuizmo();});
        this->kid_manip->disable();
        this->manipulators.reserve(positions.size());
        /// NEW
		for (int i = 0; i < positions.size(); ++i) {
			this->manipulators.push_back(Manipulator(positions[i]));
			this->manipulators[i].lockPosition();
            this->manipulators[i].enable();

            this->fixedVertices.push_back(false);
            this->selectedVertices.push_back(false);
            this->manipulatorsAtRangeForGrab.push_back(false);

            QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, [this, i](){this->manipulatorsAtRangeForGrab[i] = true;});
            QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, [this, i](){this->manipulatorsAtRangeForGrab[i] = false;});
		}
        this->selectionMode = INACTIVE;
        QObject::connect(&this->selection, &Selection::isSelecting, [this](){this->updateSelection();});
	}

    void ARAPManipulator::moveGuizmo() {
        RotationManipulator * manipulator = this->kid_manip;
        unsigned int n_points = manipulator->n_points();
        qglviewer::Vec p;
        int idx;

        std::vector<glm::vec3> positions;
        this->getAllPositions(positions);

        for(unsigned int i = 0; i < n_points; ++i)
        {
            manipulator->getTransformedPoint(i, idx, p);
            positions[idx] = glm::vec3(p[0], p[1], p[2]);
        }
        ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
        if(!deformer) {
            std::cout << "WARNING: ARAP manipulator can be used only with the ARAP deformer !" << std::endl;
            return;
        }
        std::vector<Vec3D<float>> ptsAsVec3D;
        for(int i = 0; i < positions.size(); ++i) {
            glm::vec3 pt = positions[i];
            ptsAsVec3D.push_back(Vec3D(pt[0], pt[1], pt[2]));
        }

        print_debug("Compute deformation");
        deformer->arap.compute_deformation(ptsAsVec3D);

        for(int i = 0; i < positions.size(); ++i)
            positions[i] = glm::vec3(ptsAsVec3D[i][0], ptsAsVec3D[i][1], ptsAsVec3D[i][2]);

        this->setPositions(positions);
        this->meshIsModified = true;
    }

    void ARAPManipulator::setPositions(std::vector<glm::vec3>& positions) {
        mesh->useNormal = true;
        mesh->movePoints(positions);
        mesh->useNormal = false;
        for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
            this->manipulators[i].setManipPosition(this->mesh->getVertice(i));
        }
        print_debug("Set positions");
        Q_EMIT needSendTetmeshToGPU();
    }

    void ARAPManipulator::updateSelection() {
        print_debug("Update selection");
        this->kid_manip->disable();
        std::vector<glm::vec3> positions;
        this->getAllPositions(positions);
        std::vector<bool> inSelection = this->selection.areInSelection(positions);

        for(int i = 0; i < inSelection.size(); ++i) {
            if(inSelection[i]) {
                if(this->selectionMode == ADD_MOVING) {
                    this->selectedVertices[i] = true;
                    this->fixedVertices[i] = false;
                } else if(this->selectionMode == ADD_FIXED) {
                    this->selectedVertices[i] = false;
                    this->fixedVertices[i] = true;
                } else if(this->selectionMode == REMOVE) {
                    this->selectedVertices[i] = false;
                    this->fixedVertices[i] = false;
                }
            }
        }
    }

    void ARAPManipulator::computeManipulatorFromSelection() {
        RotationManipulator * manipulator = this->kid_manip;
        manipulator->resetScales();
        manipulator->clear();

        std::vector<glm::vec3> positions;
        this->getAllPositions(positions);

        int nb=0;
        qglviewer::Vec oo( 0.f , 0.f , 0.f );
        for( unsigned int v = 0 ; v < positions.size() ; ++v )
        {
            if( selectedVertices[v] )
            {
                glm::vec3 const & p = positions[v];
                oo += qglviewer::Vec( p[0] , p[1] , p[2] );
                ++nb;
            }
        }
        oo /= nb;

        PCATools::PCASolver3f< qglviewer::Vec , qglviewer::Vec > solver;
        solver.setOrigine( oo );

        for( unsigned int v = 0 ; v < positions.size() ; ++v )
        {
            if( selectedVertices[v] )
            {
                glm::vec3 const & p = positions[v];
                solver.addPoint( qglviewer::Vec( p[0] , p[1] , p[2] ) );
            }
        }

        solver.compute();

        manipulator->setOrigine( oo );
        manipulator->setRepX( solver.RepX() );
        manipulator->setRepY( solver.RepY() );
        manipulator->setRepZ( solver.RepZ() );

        glm::vec3 center = glm::vec3(oo[0], oo[1], oo[2]);
        glm::vec3 min = glm::vec3(1000000., 1000000., 1000000.);
        glm::vec3 max = glm::vec3(0., 0., 0.);
        for( unsigned int v = 0 ; v < positions.size() ; ++v )
        {
            if( selectedVertices[v] )
            {
                glm::vec3 const & p = positions[v];
                manipulator->addPoint( v , qglviewer::Vec( p[0] , p[1] , p[2] ) );

                if(glm::distance(min, p) > glm::distance(center, p))
                    min = p;
                if(glm::distance(max, p) < glm::distance(center, p))
                    max = p;
            }
        }

        //manipulator->activate();
        manipulator->enable();
        //Q_EMIT needChangeKidManipulatorRadius(glm::length(max - min));
        print_debug("Show guizmo");


        ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
        if(!deformer) {
            std::cout << "WARNING: ARAP manipulator can be used only with the ARAP deformer !" << std::endl;
            return;
        }
        deformer->arap.setHandles(this->getHandles());
        print_debug("Update handles");
    }

    std::vector<bool> ARAPManipulator::getHandles() {
        std::vector<bool> handles_vertices(manipulators.size(), false);
        for( unsigned int i = 0; i < manipulators.size(); i++)
            if(fixedVertices[i] || selectedVertices[i])
                handles_vertices[i] = true;
        return handles_vertices;
    }

    void ARAPManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
        positions.clear();
        positions.reserve(this->manipulators.size());
		for (int i = 0; i < this->manipulators.size(); ++i) {
            positions.push_back(this->manipulators[i].getManipPosition());
		}
    }

    void ARAPManipulator::keyPressed(QKeyEvent* e) {
        print_debug("Key pressed: [");
        if(e->modifiers() & Qt::ShiftModifier) {
            print_debug("Switch to ACTIVE");
            this->selection.activate();
            this->selection.enterSelectionMode();
            this->selectionMode = ACTIVE;
        }
        print_debug("]");
    }

    void ARAPManipulator::print_debug(const char * text) {
        if(this->debug_mode)
            std::cout << text << std::endl;
    }

    void ARAPManipulator::keyReleased(QKeyEvent* e) {
        print_debug("Key released: [");
        if(e->key() == Qt::Key_Shift) {
            print_debug("Switch to INACTIVE");
            this->selection.exitSelectionMode();
            this->selection.deactivate();
            this->selectionMode = INACTIVE;
        }
        print_debug("]");
    }

    void ARAPManipulator::mousePressed(QMouseEvent* e) {
        print_debug("Mouse pressed: [");
        //if(this->selectionMode == INACTIVE) {
        //    print_debug("Selection mode is: INACTIVE");
        //    print_debug("Do nothing");
        //    return;
        //}

        if(e->modifiers() & Qt::ShiftModifier ) {
            if ((e->button() == Qt::LeftButton) && (e->modifiers() & Qt::ControlModifier))
            {
                print_debug("Switch to REMOVE");
                this->selectionMode = REMOVE;
            } else if ((e->button() == Qt::LeftButton) && (e->modifiers() & Qt::AltModifier))
            {
                print_debug("Switch to ADD_FIXED");
                this->selectionMode = ADD_FIXED;
            }
            else if ((e->button() == Qt::LeftButton))
            {
                print_debug("Switch to ADD_MOVING");
                this->selectionMode = ADD_MOVING;
            }
            else if ((e->button() == Qt::RightButton))
            {
                print_debug("Switch to INACTIVE");
                this->selectionMode = INACTIVE;
                this->computeManipulatorFromSelection();
            }
            print_debug("]");
            this->selection.mousePressEvent(e, nullptr);
            return;
        }

        RotationManipulator * manipulator = this->kid_manip;
        if(e->modifiers() & Qt::ControlModifier) {
            if (e->button() == Qt::MidButton)
            {
                manipulator->disable();
                this->makeSelecteFixedHandles();
                return;
            }
            int idxManipatorAtRange = -1;
            for(int i = 0; i < this->manipulatorsAtRangeForGrab.size(); ++i) {
                if(this->manipulatorsAtRangeForGrab[i]) {
                    idxManipatorAtRange = i;
                    break;
                }
            }
            if(idxManipatorAtRange == -1)
                return;
            if (e->button() == Qt::LeftButton)
            {
                manipulator->disable();
                this->fixedVertices[idxManipatorAtRange] = true;
                this->selectedVertices[idxManipatorAtRange] = false;
            }
            else if (e->button() == Qt::RightButton)
            {
                manipulator->disable();
                this->fixedVertices[idxManipatorAtRange] = false;
                this->selectedVertices[idxManipatorAtRange] = true;
            }
        }
    }

    void ARAPManipulator::makeSelecteFixedHandles() {
        for(unsigned int i = 0; i < selectedVertices.size(); i++) {
            if(selectedVertices[i]) {
                fixedVertices[i] = true;
                selectedVertices[i] = false;
            }
        }
    }

    void ARAPManipulator::mouseReleased(QMouseEvent* e) {
        print_debug("Mouse released: [");
        if(this->selectionMode == INACTIVE) {
            print_debug("Selection mode is: INACTIVE");
            print_debug("Do nothing");
            return;
        }
        print_debug("Switch to ACTIVE");
        this->selectionMode = ACTIVE;
        if(this->meshIsModified) {
            mesh->addStateToHistory();
            this->meshIsModified = false;
        }
        print_debug("]");
        this->selection.mouseReleaseEvent(e, nullptr);
    }

    void ARAPManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
		for (int i = 0; i < this->selectedVertices.size(); ++i) {
            toDisplay.push_back(this->selectedVertices[i] || this->fixedVertices[i] || this->manipulatorsAtRangeForGrab[i]);
        }
    }

    void ARAPManipulator::getManipulatorsState(std::vector<State>& states) const {
        states.clear();
        for(int i = 0; i < this->selectedVertices.size(); ++i) {
            State currentState = State::NONE;
            if(this->selectedVertices[i])
                currentState = State::AT_RANGE;
            if(this->fixedVertices[i])
                currentState = State::LOCK;
            if(this->manipulatorsAtRangeForGrab[i])
                currentState = State::HIGHLIGHT;
            states.push_back(currentState);
        }
    }

    void ARAPManipulator::toggleEvenMode(bool value) {
        this->kid_manip->evenMode = value;
    }

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

    ARAPManipulator::~ARAPManipulator() {
        ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
        if(deformer) {
            for(int i = 0; i < this->manipulators.size(); ++i)
                deformer->unsetHandle(i);
        }
        delete this->kid_manip;
    }

/***/

SliceManipulator::SliceManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions): MeshManipulator(mesh) {
    this->kid_manip = new RotationManipulator();
    QObject::connect(this->kid_manip, &RotationManipulator::moved, this, [this]() {this->moveKidManip();});
    this->manipulators.reserve(positions.size());
	for (int i = 0; i < positions.size(); ++i) {
		this->manipulators.push_back(Manipulator(positions[i]));
        QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, &SliceManipulator::displayManipulator);
        QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, &SliceManipulator::hideManipulator);

        //QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &SliceManipulator::deselectManipulator);
        QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &SliceManipulator::selectManipulator);
        QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, &SliceManipulator::moveManipulator);

        this->selectedManipulators.push_back(false);
        this->manipulatorsToDisplay.push_back(true);
        this->handles.push_back(false);
		this->manipulators[i].setCustomConstraint();
        this->manipulators[i].enable();
	}

    QObject::connect(&this->selection, &Selection::isSelecting, this, &SliceManipulator::checkSelectedManipulators);

    this->currentSelectedSlice = SliceOrientation::X;
    this->slicesPositions = glm::vec3(0., 0., 0.);
    this->selectSlice(this->currentSelectedSlice);
}

void SliceManipulator::checkSelectedManipulators() {
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

void SliceManipulator::setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) {
	if (positions.size() == this->manipulators.size()) {
		for (int i = 0; i < this->manipulators.size(); ++i) {
			this->manipulators[i].setManipPosition(positions[i]);
			this->manipulators[i].setLastPosition(positions[i]);
		}
	} else {
		std::cerr << "WARNING: try to set [" << this->manipulators.size() << "] manipulators positions with a position vector of size [" << positions.size() << "]" << std::endl;
	}
}

void SliceManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
	for (int i = 0; i < this->manipulators.size(); ++i) {
        positions.push_back(this->manipulators[i].getManipPosition());
	}
}

void SliceManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
	for (int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
        toDisplay.push_back(this->manipulatorsToDisplay[i]);
    }
}

void SliceManipulator::getManipulatorsState(std::vector<State>& states) const {
    states.clear();
    for(int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
        State currentState = State::NONE;
        if(this->manipulators[i].isAtRangeForGrab)
            currentState = State::AT_RANGE;
        if(this->selectedManipulators[i])
            currentState = State::AT_RANGE;
        if(this->manipulators[i].isSelected)
            currentState = State::MOVE;
        if(this->handles[i])
            currentState = State::LOCK;
        states.push_back(currentState);
    }
}

void SliceManipulator::displayManipulator(Manipulator * manipulatorToDisplay) {
    // Since vectors are organized sequentially, you can get an index by subtracting pointer to initial element 
    // from the pointer to the element you search
    ptrdiff_t index = manipulatorToDisplay - &(this->manipulators[0]);
    this->manipulatorsToDisplay[index] = true;
}

void SliceManipulator::hideManipulator(Manipulator * manipulatorToDisplay) {
    if(manipulatorToDisplay->isSelected)
        return;
    // Since vectors are organized sequentially, you can get an index by subtracting pointer to initial element 
    // from the pointer to the element you search
    ptrdiff_t index = manipulatorToDisplay - &(this->manipulators[0]);
    this->manipulatorsToDisplay[index] = false;
}

void SliceManipulator::moveManipulator(Manipulator * manipulator) {
    ptrdiff_t index = manipulator - &(this->manipulators[0]);
    this->mesh->movePoint(index, manipulator->getManipPosition());
    Q_EMIT needSendTetmeshToGPU();
}

void SliceManipulator::selectManipulator(Manipulator * manipulator) {
    ptrdiff_t index = manipulator - &(this->manipulators[0]);
    if(!this->selectedManipulators[index]) {
        this->selectedManipulators[index] = true;
    }
}

void SliceManipulator::deselectManipulator(Manipulator * manipulator) {
    this->deselectAllManipulators(true);
}

void SliceManipulator::deselectAllManipulators(bool keepHandles) {
    for(int i = 0; i < this->selectedManipulators.size(); ++i) {
        this->selectedManipulators[i] = false;
        if(keepHandles && this->handles[i]) {
            ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
            if(deformer) {
                deformer->setHandle(i);
            }
        }
    }
}

void SliceManipulator::keyPressed(QKeyEvent* e) {};

void SliceManipulator::keyReleased(QKeyEvent* e) {};

void SliceManipulator::mousePressed(QMouseEvent*) {};

void SliceManipulator::mouseReleased(QMouseEvent*) {};

void SliceManipulator::movePlanes(const glm::vec3& planesPosition) {
    this->deselectAllManipulators(true);
    this->slicesPositions = planesPosition;
    this->selectSlice(this->currentSelectedSlice);
}

void SliceManipulator::selectSlice(SliceOrientation sliceOrientation) {
    int valueIdxToCheck = 0;
    switch(sliceOrientation) {
        case SliceOrientation::X:
            valueIdxToCheck = 0;
            break;
        case SliceOrientation::Y:
            valueIdxToCheck = 1;
            break;
        case SliceOrientation::Z:
            valueIdxToCheck = 2;
            break;
    };

    glm::vec3 mean = glm::vec3(0., 0., 0.);
    float nbSelectedPts = 0;
    glm::vec3 selectionMin = glm::vec3(10000000, 10000000, 10000000);
    glm::vec3 selectionMax = glm::vec3(-10000000, -10000000, -10000000);
    for(int i = 0; i < this->manipulators.size(); ++i) {
        if(std::abs(this->manipulators[i].getManipPosition()[valueIdxToCheck] - this->slicesPositions[valueIdxToCheck]) < this->selectionRadius) {
            this->selectManipulator(&this->manipulators[i]);

            glm::vec3 glmManipPos = this->manipulators[i].getManipPosition();
            mean += glmManipPos;
            nbSelectedPts += 1;
            for(int j = 0; j < 3; ++j) {
                if(selectionMin[j] > glmManipPos[j])
                    selectionMin[j] = glmManipPos[j];
                if(selectionMax[j] < glmManipPos[j])
                    selectionMax[j] = glmManipPos[j];
            }
        }
    }

    if(nbSelectedPts > 0) {
        mean /= nbSelectedPts;
        this->kid_manip->reset();
        this->kid_manip->isVisible = true;
        this->kid_manip->setOrigine(qglviewer::Vec(mean[0], mean[1], mean[2]));
        Q_EMIT needChangeKidManipulatorRadius(glm::length(selectionMax - selectionMin));
        int idxInTransf = 0;
        for(int i = 0; i < this->selectedManipulators.size(); ++i) {
            if(this->selectedManipulators[i]) {
                glm::vec3 glmManipPos = this->manipulators[i].getManipPosition();
                this->kid_manip->addPoint(idxInTransf, qglviewer::Vec(glmManipPos[0], glmManipPos[1], glmManipPos[2]));
                idxInTransf += 1;
            }
        }
    } else {
        this->kid_manip->setOrigine(qglviewer::Vec(-1000., -1000., -1000.));
        this->kid_manip->isVisible = false;
    }
}

void SliceManipulator::updateSliceToSelect(SliceOrientation sliceOrientation) {
    this->deselectAllManipulators(true);
    this->currentSelectedSlice = sliceOrientation;
    this->selectSlice(this->currentSelectedSlice);
}

void SliceManipulator::assignAsHandle() {
    ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
    if(!deformer) {
        std::cout << "WARNING: can't assign handles if the deformer isn't ARAP." << std::endl;
        return;
    }
    for(int i = 0; i < this->selectedManipulators.size(); ++i) {
        if(this->selectedManipulators[i]) {
            this->handles[i] = true;
            deformer->setHandle(i);
        }
    }
}

void SliceManipulator::assignAllHandlesBeforePlane() {
    int valueIdxToCheck = 0;
    switch(this->currentSelectedSlice) {
        case SliceOrientation::X:
            valueIdxToCheck = 0;
            break;
        case SliceOrientation::Y:
            valueIdxToCheck = 1;
            break;
        case SliceOrientation::Z:
            valueIdxToCheck = 2;
            break;
    };

    ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
    if(!deformer) {
        std::cout << "WARNING: can't assign handles if the deformer isn't ARAP." << std::endl;
        return;
    }

    for(int i = 0; i < this->manipulators.size(); ++i) {
        if(this->manipulators[i].getManipPosition()[valueIdxToCheck] > this->slicesPositions[valueIdxToCheck]) {
            this->handles[i] = true;
            deformer->setHandle(i);
        }
    }
}

void SliceManipulator::removeAllHandles() {
    ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
    if(!deformer) {
        std::cout << "WARNING: can't assign handles if the deformer isn't ARAP." << std::endl;
        return;
    }
    for(int i = 0; i < this->handles.size(); ++i) {
        this->handles[i] = false;
        deformer->unsetHandle(i);
    }
}

void SliceManipulator::moveKidManip() {
    std::vector<int> indices;
    std::vector<glm::vec3> targetPoints;
    int idxInTransf = 0;
    for(int i = 0; i < this->selectedManipulators.size(); ++i) {
        if(this->selectedManipulators[i]) {
            qglviewer::Vec deformedPoint;
            int trueIndex;
            this->kid_manip->getTransformedPoint(idxInTransf, trueIndex, deformedPoint);
            idxInTransf += 1;
            if(!std::isnan(deformedPoint[0]) && !std::isnan(deformedPoint[1]) && !std::isnan(deformedPoint[2])) {
                indices.push_back(this->selectedManipulators[i]);
                targetPoints.push_back(glm::vec3(deformedPoint[0], deformedPoint[1], deformedPoint[2]));
            }
        }
    }
    this->mesh->movePoints(indices, targetPoints);
    Q_EMIT needSendTetmeshToGPU();
}

/***/

FixedRegistrationManipulator::FixedRegistrationManipulator(BaseMesh * mesh, Grid * gridToRegister, const std::vector<glm::vec3>& positions): MeshManipulator(mesh), gridToRegister(gridToRegister) {
    this->kid_manip = nullptr;
    this->manipulators.reserve(positions.size());
    this->toolState = FixedRegistrationManipulatorState::NONE;
    this->selectedIndex = -1;
    //this->fixed = std::vector<int>{206, 36, 17, 97, 39, 68, 106, 58, 74};
    this->fixed = std::vector<int>();
    std::ifstream ptsFile ("RegistrationPoints.txt");
    if (ptsFile.is_open()) {
        std::string line;
        while(std::getline(ptsFile, line)) {
            int value = std::atoi(line.c_str());
            if(value != 0) {
                this->fixed.push_back(value);
            }
        }
        ptsFile.close();
    } else {
        this->fixed = std::vector<int>{291, 416, 332, 206, 45, 46, 133, 700, 53};
    }
    this->nbNotAssociatedPoints = fixed.size();
    for (int i = 0; i < fixed.size(); ++i) {
        this->manipulators.push_back(Manipulator(positions[fixed[i]]));

        //QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, [this, i](){Q_EMIT needChangeSelectedPoints(std::make_pair(this->fixed[i], this->manipulators[i].getManipPosition()));});
        //QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, [this, i](){Q_EMIT needChangeSelectedPoints(std::make_pair(-1, this->manipulators[i].getManipPosition()));});
        QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, [this, i](){Q_EMIT needChangeSelectedPoints(std::make_pair(this->fixed[i], this->manipulators[i].getManipPosition()));});
        QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, [this, i](){Q_EMIT needChangeSelectedPoints(std::make_pair(this->fixed[i], this->manipulators[i].getManipPosition()));});

        QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &FixedRegistrationManipulator::selectManipulator);
        QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &FixedRegistrationManipulator::deselectManipulator);
        QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, &FixedRegistrationManipulator::moveManipulator);
        QObject::connect(this, &FixedRegistrationManipulator::pointIsClickedInPlanarViewer, this, &FixedRegistrationManipulator::addManipulator);

        QObject::connect(this, &FixedRegistrationManipulator::rayIsCasted, this, &FixedRegistrationManipulator::addManipulatorFromRay);

        this->associatedManipulator.push_back(-1);
        this->isFixed.push_back(true);
        this->selectedManipulators.push_back(false);
        this->manipulatorsToDisplay.push_back(true);
        this->manipulators[i].lockPosition();
        this->manipulators[i].enable();
    }
}

void FixedRegistrationManipulator::addManipulator(const glm::vec3& position) {
    if(this->toolState == FixedRegistrationManipulatorState::SELECTING_SECOND_POINT) {
        this->toolState = FixedRegistrationManipulatorState::NONE;

        this->manipulators.push_back(Manipulator(position));
        this->isFixed.push_back(false);
        this->manipulatorsToDisplay.push_back(true);
        this->manipulators.back().lockPosition();
        this->manipulators.back().disable();

        if(this->associatedManipulator[this->selectedIndex] == -1)
            this->nbNotAssociatedPoints -= 1;
        this->associatedManipulator[this->selectedIndex] = this->manipulators.size()-1;
        this->selectedManipulators[this->selectedIndex] = false;
        Q_EMIT needChangeCursor(CursorType::NORMAL);
        Q_EMIT needChangeCursorInPlanarView(CursorType::NORMAL);
        Q_EMIT needChangeActivatePreviewPoint(false);
        std::cout << "Associate vertex [" << selectedIndex << "] with [" << this->manipulators.back().getManipPosition() << "]" << std::endl;
    }
}

void FixedRegistrationManipulator::addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) {
    glm::vec3 manipulatorPosition;
    if(this->gridToRegister->getPositionOfRayIntersection(origin, direction, minValue, maxValue, planePos, manipulatorPosition))
        this->addManipulator(manipulatorPosition);
}

void FixedRegistrationManipulator::setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) {
    if (positions.size() == this->manipulators.size()) {
        for (int i = 0; i < this->manipulators.size(); ++i) {
            this->manipulators[i].setManipPosition(positions[i]);
            this->manipulators[i].setLastPosition(positions[i]);
        }
    } else {
        std::cerr << "WARNING: try to set [" << this->manipulators.size() << "] manipulators positions with a position vector of size [" << positions.size() << "]" << std::endl;
    }
}

void FixedRegistrationManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
    for (int i = 0; i < this->manipulators.size(); ++i) {
        positions.push_back(this->manipulators[i].getManipPosition());
    }
}

void FixedRegistrationManipulator::getManipulatorsToDisplay(std::vector<bool>& toDisplay) const {
    for (int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
        toDisplay.push_back(this->manipulatorsToDisplay[i]);
    }
}

void FixedRegistrationManipulator::getManipulatorsState(std::vector<State>& states) const {
    states.clear();
    for(int i = 0; i < this->manipulatorsToDisplay.size(); ++i) {
        State currentState = State::NONE;
        if(this->isFixed[i]) {
            if(this->manipulators[i].isAtRangeForGrab)
                currentState = State::AT_RANGE;
            if(this->selectedManipulators[i])
                currentState = State::AT_RANGE;
            if(this->associatedManipulator[i] != -1)
                currentState = State::HIGHLIGHT;
        } else {
            currentState = State::LOCK;
        }
        states.push_back(currentState);
    }
}

void FixedRegistrationManipulator::moveManipulator(Manipulator * manipulator) {
}

void FixedRegistrationManipulator::selectManipulator(Manipulator * manipulator) {
    ptrdiff_t index = manipulator - &(this->manipulators[0]);
    if(this->toolState == FixedRegistrationManipulatorState::NONE && this->isFixed[index]) {
        if(this->associatedManipulator[index] != -1) {
            std::cout << "Replace associated point" << std::endl;
            int previousIdx = this->associatedManipulator[index];
            std::cout << "Previous: " << previousIdx << std::endl;
            this->associatedManipulator[index] = -1;
            this->manipulators[previousIdx].lockPosition();
            this->manipulators[previousIdx].disable();
            this->manipulatorsToDisplay[previousIdx] = false;
            this->isFixed[previousIdx] = false;
        }
        this->selectedManipulators[index] = true;
        this->toolState = FixedRegistrationManipulatorState::SELECTING_SECOND_POINT;
        this->selectedIndex = index;
        Q_EMIT needChangeCursor(CursorType::CROSS);
        Q_EMIT needChangeCursorInPlanarView(CursorType::CROSS);
        Q_EMIT needChangeActivatePreviewPoint(true);
        std::cout << "Select vertex: [" << index << "]" << std::endl;
    }
}

void FixedRegistrationManipulator::apply() {
    std::vector<int> verticesToFit;
    std::vector<glm::vec3> newPositions;
    std::cout << "Selected pairs are:" << std::endl;
    for(int i = 0; i < this->associatedManipulator.size(); ++i) {
        if(this->associatedManipulator[i] >= 0) {
            verticesToFit.push_back(this->fixed[i]);
            newPositions.push_back(this->manipulators[this->associatedManipulator[i]].getManipPosition());
        }
    }
    this->mesh->setARAPDeformationMethod();
    ARAPMethod * deformer = dynamic_cast<ARAPMethod*>(this->mesh->meshDeformer);
    if(deformer) {
        deformer->fitToPointList(verticesToFit, newPositions);
    }

    for(int i = 0; i < this->fixed.size(); ++i) {
        this->manipulators[i].setManipPosition(this->mesh->getVertice(this->fixed[i]));
    }
}

void FixedRegistrationManipulator::clear() {
    int nbAddedManipulators = 0;
    for(int i = 0; i < this->isFixed.size(); ++i) {
        if(!this->isFixed[i]) {
            nbAddedManipulators += 1;
        }
    }
    for(int i = 0; i < nbAddedManipulators; ++i) {
        this->isFixed.pop_back();
        this->manipulators.pop_back();
        this->manipulatorsToDisplay.pop_back();
    }
    if(this->toolState == FixedRegistrationManipulatorState::NONE) {
        std::fill(this->associatedManipulator.begin(), this->associatedManipulator.end(), -1);
    }
}

void FixedRegistrationManipulator::deselectManipulator(Manipulator * manipulator) {}

void FixedRegistrationManipulator::keyPressed(QKeyEvent* e) {};

void FixedRegistrationManipulator::keyReleased(QKeyEvent* e) {};

void FixedRegistrationManipulator::mousePressed(QMouseEvent*) {};

void FixedRegistrationManipulator::mouseReleased(QMouseEvent*) {};

}

