#include "mesh_manipulator.hpp"
#include "glm/trigonometric.hpp"
#include "grid/drawable/drawable.hpp"
#include "grid/geometry/surface_mesh.hpp"
#include "grid/utils/GLUtilityMethods.h"
#include "manipulator.hpp"
//#include "../deformation/mesh_deformer.hpp"
#include "../deformation/cage_surface_mesh.hpp"
#include "../utils/PCATools.h"
#include "qnamespace.h"
#include "qobjectdefs.h"
#include "qt/scene.hpp"
#include <fstream>
#include <glm/gtx/closest_point.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

namespace UITool {

    void MeshManipulator::zoom(float newSceneRadius) {
        GL::DrawableUI::zoom(newSceneRadius);
        if(this->guizmo)
            this->guizmo->setDisplayScale(this->getGuizmoRadius());
    };

    void MeshManipulator::drawGuizmo() {
        if(this->guizmo && this->guizmo->isVisible)
            this->guizmo->draw();
    }


    glm::vec3 stateToColor(State state) {
        switch(state) {
            case State::NONE:
                return glm::vec3(1., 0., 0.);
                break;
            case State::AT_RANGE:
                return glm::vec3(0., 0.9, 0.);
                break;
            case State::SELECTED:
                return glm::vec3(0.2, 1., 0.2);
                break;
            case State::LOCK:
                return glm::vec3(100./255., 100./255., 100./255.);
                break;
            case State::MOVE:
                return glm::vec3(0., 1., 0.);
                break;
            case State::WAITING:
                return glm::vec3(0.1, 1., 0.1);
                break;
            case State::HIGHLIGHT:
                return glm::vec3(1., 215./255., 0.);
                break;
            default:
                return glm::vec3(0., 0., 0.);
        }
    }

    DirectManipulator::DirectManipulator(BaseMesh * mesh): MeshManipulator(mesh) {
        this->meshIsModified = false;
        this->guizmo = nullptr;
        std::vector<glm::vec3> positions = mesh->getVertices();
        this->manipulators.reserve(positions.size());
        this->defaultManipulatorColor = stateToColor(State::NONE);
		for (int i = 0; i < positions.size(); ++i) {
			this->manipulators.push_back(Manipulator(positions[i]));
            QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, [this, i](){Q_EMIT needDisplayVertexInfo(std::make_pair(i, this->manipulators[i].getManipPosition()));});
            QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, [this, i](){Q_EMIT needDisplayVertexInfo(std::make_pair(i, this->manipulators[i].getManipPosition()));});
            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, [this, i](){Q_EMIT needDisplayVertexInfo(std::make_pair(i, this->manipulators[i].getManipPosition()));});

            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &DirectManipulator::selectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &DirectManipulator::deselectManipulator);
            QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, &DirectManipulator::moveManipulator);

            this->selectedManipulators.push_back(false);
            this->manipulatorsToDisplay.push_back(true);
			this->manipulators[i].setCustomConstraint();
            this->manipulators[i].enable();
		}

        QObject::connect(&this->selection, &Selection::isSelecting, this, &DirectManipulator::checkSelectedManipulators);

        this->instructions = std::string("Right-click : Move vertex.");
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

    void DirectManipulator::updateWithMeshVertices() {
        for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
            this->manipulators[i].setLastPosition(this->mesh->getVertice(i));
            this->manipulators[i].setManipPosition(this->mesh->getVertice(i));
        }
    }

    void DirectManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
		for (int i = 0; i < this->manipulators.size(); ++i) {
            positions.push_back(this->manipulators[i].getManipPosition());
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
        this->mesh->movePoints({int(index)}, {manipulator->getManipPosition()});
        Q_EMIT needSendTetmeshToGPU();
        this->meshIsModified = true;
    }

    void DirectManipulator::selectManipulator(Manipulator * manipulator) {
        ptrdiff_t index = manipulator - &(this->manipulators[0]);
        if(!this->selectedManipulators[index]) {
            this->selectedManipulators[index] = true;
        }
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

    void DirectManipulator::draw() {
        glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
        glColor3f(0.2,0.2,0.9);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_DEPTH);
        for(int i = 0; i < this->manipulators.size(); ++i){
            State currentState = State::NONE;
            if(this->manipulators[i].isAtRangeForGrab)
                currentState = State::AT_RANGE;
            if(this->selectedManipulators[i])
                currentState = State::AT_RANGE;
            if(this->manipulators[i].isSelected)
                currentState = State::MOVE;

            glColor3fv(glm::value_ptr(stateToColor(currentState)));
            BasicGL::drawSphere(this->manipulators[i].getManipPosition().x, this->manipulators[i].getManipPosition().y, this->manipulators[i].getManipPosition().z, this->sphereRadius, 15,15);
        }
    }

    /***/

    GlobalManipulator::GlobalManipulator(BaseMesh * mesh): MeshManipulator(mesh) {
        this->meshIsModified = false;
        this->selection.disable();
        this->evenMode = false;
        this->guizmo = new RotationManipulator();
        this->guizmo->setOrigine(qglviewer::Vec(mesh->getOrigin()[0], mesh->getOrigin()[1], mesh->getOrigin()[2]));
        this->guizmo->setRepX(qglviewer::Vec(mesh->coordinate_system[0].x, mesh->coordinate_system[0].y, mesh->coordinate_system[0].z));
        this->guizmo->setRepY(qglviewer::Vec(mesh->coordinate_system[1].x, mesh->coordinate_system[1].y, mesh->coordinate_system[1].z));
        this->guizmo->setRepZ(qglviewer::Vec(mesh->coordinate_system[2].x, mesh->coordinate_system[2].y, mesh->coordinate_system[2].z));
        for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
            this->guizmo->addPoint(i, qglviewer::Vec(this->mesh->getVertice(i)[0], this->mesh->getVertice(i)[1], this->mesh->getVertice(i)[2]));
        }
        QObject::connect(this->guizmo, &RotationManipulator::moved, this, [this]() {this->moveManipulator(nullptr);});

        this->instructions = std::string("Wheel : Change guizmo mode.");
    }

    void GlobalManipulator::updateWithMeshVertices() {
        delete this->guizmo;
        this->guizmo = new RotationManipulator();
        this->guizmo->evenMode = this->evenMode;
        this->guizmo->setOrigine(qglviewer::Vec(mesh->getOrigin()[0], mesh->getOrigin()[1], mesh->getOrigin()[2]));
        this->guizmo->setRepX(qglviewer::Vec(mesh->coordinate_system[0].x, mesh->coordinate_system[0].y, mesh->coordinate_system[0].z));
        this->guizmo->setRepY(qglviewer::Vec(mesh->coordinate_system[1].x, mesh->coordinate_system[1].y, mesh->coordinate_system[1].z));
        this->guizmo->setRepZ(qglviewer::Vec(mesh->coordinate_system[2].x, mesh->coordinate_system[2].y, mesh->coordinate_system[2].z));
        for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
            this->guizmo->addPoint(i, qglviewer::Vec(this->mesh->getVertice(i)[0], this->mesh->getVertice(i)[1], this->mesh->getVertice(i)[2]));
        }
        QObject::connect(this->guizmo, &RotationManipulator::moved, this, [this]() {this->moveManipulator(nullptr);});

        this->meshIsModified = false;
    }

    void GlobalManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
    }

    void GlobalManipulator::moveManipulator(Manipulator * manipulator) {
        std::vector<glm::vec3> newPoints(this->mesh->getNbVertices());
        for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
            qglviewer::Vec deformedPoint;
            int trueIndex;
            this->guizmo->getTransformedPoint(i, trueIndex, deformedPoint);
            newPoints[i] = glm::vec3(deformedPoint[0], deformedPoint[1], deformedPoint[2]);
        }
        this->mesh->movePoints(newPoints);
        this->meshIsModified = true;
        Q_EMIT needSendTetmeshToGPU();
    }

    void GlobalManipulator::toggleEvenMode() {
        if(this->evenMode) {
            this->evenMode = false;
            this->guizmo->evenMode = false;
        } else {
            this->evenMode = true;
            this->guizmo->evenMode = true;
        }
    }

    void GlobalManipulator::keyPressed(QKeyEvent* e) {
        //if(e->modifiers() == Qt::ControlModifier && !e->isAutoRepeat()) {
        //    this->evenMode = true;
        //    this->kid_manip->evenMode = true;
        //}
    }

    void GlobalManipulator::keyReleased(QKeyEvent* e) {
        //this->evenMode = false;
        //this->kid_manip->evenMode = false;
    }

    void GlobalManipulator::mousePressed(QMouseEvent* e) {}

    void GlobalManipulator::mouseReleased(QMouseEvent* e) {
        if(this->meshIsModified) {
            emit needUpdateSceneCenter();
            this->mesh->coordinate_system[0] = glm::vec3(guizmo->RepX.x, guizmo->RepX.y, guizmo->RepX.z);
            this->mesh->coordinate_system[1] = glm::vec3(guizmo->RepY.x, guizmo->RepY.y, guizmo->RepY.z);
            this->mesh->coordinate_system[2] = glm::vec3(guizmo->RepZ.x, guizmo->RepZ.y, guizmo->RepZ.z);
            mesh->addStateToHistory();
            this->meshIsModified = false;
        }
    }

    GlobalManipulator::~GlobalManipulator() {
        delete this->guizmo;
    }

    /***/

    ARAPManipulator::ARAPManipulator(BaseMesh * mesh): MeshManipulator(mesh) {
        this->guizmo = new RotationManipulator();
        QObject::connect(this->guizmo, &RotationManipulator::moved, this, [this]() {this->moveGuizmo();});
        this->guizmo->disable();
        std::vector<glm::vec3> positions = mesh->getVertices();
        this->manipulators.reserve(positions.size());
        /// NEW
		for (int i = 0; i < positions.size(); ++i) {
			this->manipulators.push_back(Manipulator(positions[i]));
			this->manipulators[i].lockPosition();
            //this->manipulators[i].enable();
            this->manipulators[i].disable();

            this->fixedVertices.push_back(false);
            this->selectedVertices.push_back(false);
            this->manipulatorsAtRangeForGrab.push_back(false);

            QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, [this, i](){this->manipulatorsAtRangeForGrab[i] = true;});
            QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, [this, i](){this->manipulatorsAtRangeForGrab[i] = false;});
		}
        this->selectionMode = INACTIVE;
        QObject::connect(&this->selection, &Selection::isSelecting, [this](){this->updateSelection();});

        dynamic_cast<SurfaceMesh*>(this->mesh)->initARAPDeformer();

        this->instructions = std::string("Maj+Left-click : Select MOVING vertices. \n"
                                         "Maj+Alt+Left-click : Select FIXED vertices. \n"
                                         "Maj+Right-click : Show guizmo \n"
                                         "Guizmo"
                                         "Wheel : Change guizmo mode. \n"
                                         "");
    }

    void ARAPManipulator::moveGuizmo() {
        RotationManipulator * manipulator = this->guizmo;
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

        dynamic_cast<SurfaceMesh*>(this->mesh)->deformARAP(positions);
        this->setPositions(positions);
        this->meshIsModified = true;
    }

    void ARAPManipulator::setPositions(std::vector<glm::vec3>& positions) {
        for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
            this->manipulators[i].setManipPosition(this->mesh->getVertice(i));
        }
        print_debug("Set positions");
        Q_EMIT needSendTetmeshToGPU();
    }

    void ARAPManipulator::updateSelection() {
        print_debug("Update selection");
        this->guizmo->disable();
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
        RotationManipulator * manipulator = this->guizmo;
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
        //manipulator->setRepX( solver.RepX() );
        //manipulator->setRepY( solver.RepY() );
        //manipulator->setRepZ( solver.RepZ() );

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
        print_debug("Show guizmo");

        dynamic_cast<SurfaceMesh*>(this->mesh)->setHandlesARAP(this->getHandles());
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
            Q_EMIT needChangeCursor(UITool::CursorType::CROSS);
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
            Q_EMIT needChangeCursor(UITool::CursorType::NORMAL);
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

        RotationManipulator * manipulator = this->guizmo;
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

        if(this->meshIsModified) {
            print_debug("Add to history");
            mesh->addStateToHistory();
            this->meshIsModified = false;
        }
        if(this->selectionMode == INACTIVE) {
            print_debug("Selection mode is: INACTIVE");
            print_debug("Do nothing");
            return;
        }
        print_debug("Switch to ACTIVE");
        this->selectionMode = ACTIVE;
        print_debug("]");
        this->selection.mouseReleaseEvent(e, nullptr);
    }

    void ARAPManipulator::toggleEvenMode(bool value) {
        this->guizmo->evenMode = value;
    }

    void ARAPManipulator::updateWithMeshVertices() {
        for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
            this->manipulators[i].setLastPosition(this->mesh->getVertice(i));
            this->manipulators[i].setManipPosition(this->mesh->getVertice(i));
        }
        this->computeManipulatorFromSelection();
    }

    ARAPManipulator::~ARAPManipulator() {
        delete this->guizmo;
    }

    void ARAPManipulator::draw() {
        glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
        glColor3f(0.2,0.2,0.9);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_DEPTH);
        for(int i = 0; i < this->manipulators.size(); ++i){
            State currentState = State::NONE;
            if(this->selectedVertices[i])
                currentState = State::AT_RANGE;
            if(this->fixedVertices[i])
                currentState = State::LOCK;
            if(this->manipulatorsAtRangeForGrab[i])
                currentState = State::HIGHLIGHT;
            glColor3fv(glm::value_ptr(stateToColor(currentState)));
            BasicGL::drawSphere(this->manipulators[i].getManipPosition().x, this->manipulators[i].getManipPosition().y, this->manipulators[i].getManipPosition().z, this->sphereRadius, 15,15);
        }
    }

/***/

SliceManipulator::SliceManipulator(BaseMesh * mesh): MeshManipulator(mesh) {
    this->guizmo = new RotationManipulator();
    QObject::connect(this->guizmo, &RotationManipulator::moved, this, [this]() {this->moveGuizmo();});
    std::vector<glm::vec3> positions = mesh->getVertices();
    this->manipulators.reserve(positions.size());
	for (int i = 0; i < positions.size(); ++i) {
		this->manipulators.push_back(Manipulator(positions[i]));
        this->manipulators[i].lockPosition();

        this->fixedVertices.push_back(false);
        this->selectedVertices.push_back(false);

        //QObject::connect(&(this->manipulators[i]), &Manipulator::enterAtRangeForGrab, this, &SliceManipulator::displayManipulator);
        //QObject::connect(&(this->manipulators[i]), &Manipulator::exitFromRangeForGrab, this, &SliceManipulator::hideManipulator);

        //QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonReleasedAndCtrlIsNotPressed, this, &SliceManipulator::deselectManipulator);
        //QObject::connect(&(this->manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &SliceManipulator::selectManipulator);
        //QObject::connect(&(this->manipulators[i]), &Manipulator::isManipulated, this, &SliceManipulator::moveManipulator);
	}

    //QObject::connect(&this->selection, &Selection::isSelecting, this, &SliceManipulator::checkSelectedManipulators);

    this->currentSelectedSlice = SliceOrientation::X;
    this->slicesPositions = glm::vec3(0., 0., 0.);
    this->slicesNormals[0] = glm::vec3(1., 0., 0.);
    this->slicesNormals[1] = glm::vec3(0., 1., 0.);
    this->slicesNormals[2] = glm::vec3(0., 0., 1.);

    this->selectionRadius = mesh->getDimensions().y / 10.;
    this->selectionRange = this->selectionRadius*5.;

    glm::vec3 dimensions = mesh->getDimensions();
    float smallestDimension = std::min(dimensions.x, std::min(dimensions.y, dimensions.z));
    this->incrementRadius = smallestDimension / 50.;
    this->incrementRange = incrementRadius;

    this->selectSlice(this->currentSelectedSlice);

    this->guizmoUpToDate = false;
    this->meshIsModified = false;

    this->lastModifiedSlice = 0;

    dynamic_cast<SurfaceMesh*>(this->mesh)->initARAPDeformer();
}

void SliceManipulator::updateWithMeshVertices() {
    for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
        this->manipulators[i].setLastPosition(this->mesh->getVertice(i));
        this->manipulators[i].setManipPosition(this->mesh->getVertice(i));
    }
    this->selectSlice(this->currentSelectedSlice);
    this->computeManipulatorFromSelection();
}

void SliceManipulator::getAllPositions(std::vector<glm::vec3>& positions) {
	for (int i = 0; i < this->manipulators.size(); ++i) {
        positions.push_back(this->manipulators[i].getManipPosition());
	}
}

void SliceManipulator::keyPressed(QKeyEvent* e) {
    if(e->key() == Qt::Key_Plus) {
        if(e->modifiers() & Qt::ControlModifier)
            this->selectionRadius += incrementRadius;
        else
            this->selectionRange += incrementRange;

        this->selectSlice(this->currentSelectedSlice);
        this->computeManipulatorFromSelection();
    }

    if(e->key() == Qt::Key_Minus) {
        if(e->modifiers() & Qt::ControlModifier)
            this->selectionRadius -= incrementRadius;
        else
            this->selectionRange -= incrementRange;
        this->selectSlice(this->currentSelectedSlice);
        this->computeManipulatorFromSelection();
    }

    if(e->key() == Qt::Key_R) {
        if(e->modifiers() & Qt::ControlModifier)
            this->rotateLastModifiedSlice(-1);
        else
            this->rotateLastModifiedSlice(1);
        this->selectSlice(this->currentSelectedSlice);
        this->computeManipulatorFromSelection();
    }

    if(e->key() == Qt::Key_M) {
        std::cout << "Start apss projection" << std::endl;
        std::vector<int> ptToProject;
        for(int i = 0; i < this->selectedVertices.size(); ++i) {
            if(this->selectedVertices[i]) {
                ptToProject.push_back(i);
            }
        }
        Q_EMIT needChangePointsToProject(ptToProject); // TODO
    }
};

void SliceManipulator::keyReleased(QKeyEvent* e) {};

void SliceManipulator::mousePressed(QMouseEvent* e) {
};

void SliceManipulator::mouseReleased(QMouseEvent*) {
    if(this->meshIsModified) {
        mesh->addStateToHistory();
        this->meshIsModified = false;
    }
};

void SliceManipulator::movePlanes(const glm::vec3& planesPosition) {
    glm::vec3 diff = planesPosition - this->slicesPositions;
    for(int i = 0; i < 3; ++i)
        if(diff[i] != 0)
            this->lastModifiedSlice = i;
    this->slicesPositions = planesPosition;
    this->selectSlice(this->currentSelectedSlice);
    this->computeManipulatorFromSelection();
}

void SliceManipulator::computeManipulatorFromSelection() {
    RotationManipulator * manipulator = this->guizmo;
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

    for( unsigned int v = 0 ; v < positions.size() ; ++v )
    {
        if( selectedVertices[v] )
        {
            glm::vec3 const & p = positions[v];
            manipulator->addPoint( v , qglviewer::Vec( p[0] , p[1] , p[2] ) );
        }
    }

    //manipulator->activate();
    manipulator->enable();
    this->guizmoUpToDate = false;
}

std::vector<bool> SliceManipulator::getHandles() {
    std::vector<bool> handles_vertices(manipulators.size(), false);
    for( unsigned int i = 0; i < manipulators.size(); i++)
        if(fixedVertices[i] || selectedVertices[i])
            handles_vertices[i] = true;
    return handles_vertices;
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

    std::fill(this->selectedVertices.begin(), this->selectedVertices.end(), false);
    std::fill(this->fixedVertices.begin(), this->fixedVertices.end(), false);

    std::vector<glm::vec3> positions;
    this->getAllPositions(positions);
    for(int i = 0; i < positions.size(); ++i) {
        for(int j = 0; j < 2; ++j) {
            valueIdxToCheck = j;
            //if(std::abs(positions[i][valueIdxToCheck] - this->slicesPositions[valueIdxToCheck]) < this->selectionRadius) {
            //    this->selectedVertices[i] = true;
            //}

            glm::vec3 normal = this->slicesNormals[valueIdxToCheck];

            glm::vec3 point = (this->mesh->bbMax + this->mesh->bbMin)/glm::vec3(2., 2., 2.);
            point[valueIdxToCheck] = this->slicesPositions[valueIdxToCheck];

            float dist = std::abs(glm::dot(normal, (positions[i] - point)));

            if(dist < this->selectionRadius) {
                this->selectedVertices[i] = true;
            }

            if(dist >= this->selectionRange) {
                this->fixedVertices[i] = true;
            }
        }
    }
}

void SliceManipulator::updateSliceToSelect(SliceOrientation sliceOrientation) {
    this->currentSelectedSlice = sliceOrientation;
    this->selectSlice(this->currentSelectedSlice);
    this->computeManipulatorFromSelection();
}

void SliceManipulator::moveGuizmo() {
    if(!guizmoUpToDate) {
        dynamic_cast<SurfaceMesh*>(this->mesh)->setHandlesARAP(this->getHandles());
        this->guizmoUpToDate = true;
    }

    RotationManipulator * manipulator = this->guizmo;
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
    dynamic_cast<SurfaceMesh*>(this->mesh)->deformARAP(positions);
    this->setPositions(positions);
    this->meshIsModified = true;
}

void SliceManipulator::setPositions(std::vector<glm::vec3>& positions) {
    for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
        this->manipulators[i].setManipPosition(this->mesh->getVertice(i));
    }
    Q_EMIT needSendTetmeshToGPU();
}

void SliceManipulator::rotateLastModifiedSlice(float angle) {
    std::cout << "Rotate plane" << std::endl;
    glm::vec3 axis = glm::vec3(0., 0., 0.);
    axis[(this->lastModifiedSlice+1)%3] = 1;
    this->slicesNormals[this->lastModifiedSlice] = glm::normalize(glm::rotate(this->slicesNormals[this->lastModifiedSlice], static_cast<float>(glm::radians(angle)), axis));
}

void SliceManipulator::draw() {
    glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
    glColor3f(0.2,0.2,0.9);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH);
    for(int i = 0; i < this->manipulators.size(); ++i){
        State currentState = State::NONE;
        if(this->selectedVertices[i])
            currentState = State::AT_RANGE;
        if(this->fixedVertices[i])
            currentState = State::LOCK;
        glColor3fv(glm::value_ptr(stateToColor(currentState)));
        BasicGL::drawSphere(this->manipulators[i].getManipPosition().x, this->manipulators[i].getManipPosition().y, this->manipulators[i].getManipPosition().z, this->sphereRadius, 15,15);
    }
}

/***/

MarkerManipulator::MarkerManipulator(BaseMesh * mesh, Grid * grid): MeshManipulator(mesh) {
    this->step = Step::SELECT_VERTICE_ON_MESH;
    this->grid = grid;
    this->guizmo = nullptr;

    std::vector<glm::vec3> positions = mesh->getVertices();
    this->mesh_manipulators.reserve(positions.size());
    for (int i = 0; i < positions.size(); ++i) {
        this->mesh_manipulators.push_back(Manipulator(positions[i]));
        this->mesh_manipulators[i].lockPosition();
        QObject::connect(&(this->mesh_manipulators[i]), &Manipulator::mouseRightButtonPressed, this, &MarkerManipulator::selectManipulator);
        QObject::connect(&(this->mesh_manipulators[i]), &Manipulator::enterAtRangeForGrab, this, [this, i](){Q_EMIT needDisplayVertexInfo(std::make_pair(i, this->mesh_manipulators[i].getManipPosition()));});
        QObject::connect(&(this->mesh_manipulators[i]), &Manipulator::isManipulated, this, [this, i](){Q_EMIT needDisplayVertexInfo(std::make_pair(i, this->mesh_manipulators[i].getManipPosition()));});
    }
    dynamic_cast<SurfaceMesh*>(this->mesh)->initARAPDeformer();
}

void MarkerManipulator::selectManipulator(Manipulator * manipulator) {
    std::cout << "SELECT" << std::endl;
    if(this->step == Step::SELECT_VERTICE_ON_MESH) {
        ptrdiff_t index = manipulator - &(this->mesh_manipulators[0]);
        this->switchToPlaceMarkerStep(index);
    }
}

void MarkerManipulator::switchToPlaceMarkerStep(int manipulatorId) {
    this->manipulator_association.push_back(std::make_pair(manipulatorId, -1));
    this->step = Step::PLACE_MARKER;
    for(auto& manipulator : this->mesh_manipulators) {
        manipulator.disable();
    }
}

void MarkerManipulator::undoSwitchToPlaceMarkerStep() {
    this->manipulator_association.pop_back();
    this->step = Step::SELECT_VERTICE_ON_MESH;
    for(auto& manipulator : this->mesh_manipulators) {
        manipulator.enable();
    }
}

void MarkerManipulator::switchToSelectManipulatorStep(glm::vec3 markerPlaced) {
    for(int i = 0; i < this->manipulator_association.size()-1; ++i) {
        if(this->manipulator_association[i].first == this->manipulator_association.back().first) {
            this->marker_manipulators.pop_back();
            this->manipulator_association.erase(this->manipulator_association.begin()+i);
        }
    }
    this->manipulator_association.back().second = this->marker_manipulators.size();
    this->marker_manipulators.push_back(Manipulator(markerPlaced));
    this->marker_manipulators.back().lockPosition();
    this->marker_manipulators.back().disable();
    this->step = Step::SELECT_VERTICE_ON_MESH;
    for(auto& manipulator : this->mesh_manipulators) {
        manipulator.enable();
    }
    this->applyDeformation();
}

void MarkerManipulator::updateWithMeshVertices() {
    for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
        this->mesh_manipulators[i].setLastPosition(this->mesh->getVertice(i));
        this->mesh_manipulators[i].setManipPosition(this->mesh->getVertice(i));
    }
}

void MarkerManipulator::getAllPositions(std::vector<glm::vec3>& positions) {}

void MarkerManipulator::keyPressed(QKeyEvent* e) {
    if(e->key() == Qt::Key_Q && !e->isAutoRepeat()) {
        if(this->step == Step::PLACE_MARKER) {
            Q_EMIT needChangeCursor(UITool::CursorType::HOURGLASS);
            // This signal will cast a ray in the scene
            // The function that cast a ray in the scene is bind to the placeManipulator function in the marker manipulator
            Q_EMIT needCastRay();
        }
    }
    if(e->key() == Qt::Key_Escape && this->step == PLACE_MARKER) {
        this->undoSwitchToPlaceMarkerStep();
    }
}

void MarkerManipulator::placeManipulator(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos) {
    glm::vec3 position(0., 0., 0.);
    bool found = grid->getPositionOfRayIntersection(origin, direction, visibilityMap, planePos, position);
    if(found) {
        Q_EMIT needChangeCursor(UITool::CursorType::NORMAL);
        this->switchToSelectManipulatorStep(position);
    } else {
        Q_EMIT needChangeCursor(UITool::CursorType::FAIL);
        sleep(1);
        Q_EMIT needChangeCursor(UITool::CursorType::NORMAL);
    }
}

void MarkerManipulator::keyReleased(QKeyEvent* e) {}

void MarkerManipulator::mousePressed(QMouseEvent* e) {}

void MarkerManipulator::mouseReleased(QMouseEvent* e) {}

void MarkerManipulator::draw() {
    glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
    glColor3f(0.2,0.2,0.9);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH);
    for(int i = 0; i < this->marker_manipulators.size(); ++i){
        State currentState = State::NONE;
        glColor3fv(glm::value_ptr(stateToColor(currentState)));
        BasicGL::drawSphere(this->marker_manipulators[i].getManipPosition().x, this->marker_manipulators[i].getManipPosition().y, this->marker_manipulators[i].getManipPosition().z, this->sphereRadius, 15,15);
    }

    if(this->step == Step::SELECT_VERTICE_ON_MESH) {
        for(int i = 0; i < this->mesh_manipulators.size(); ++i){
            State currentState = State::AT_RANGE;
            if(this->mesh_manipulators[i].isAtRangeForGrab)
                currentState = State::HIGHLIGHT;
            glColor3fv(glm::value_ptr(stateToColor(currentState)));
            BasicGL::drawSphere(this->mesh_manipulators[i].getManipPosition().x, this->mesh_manipulators[i].getManipPosition().y, this->mesh_manipulators[i].getManipPosition().z, this->sphereRadius, 15,15);
        }
    }

    if(this->step == Step::PLACE_MARKER) {
        glColor3fv(glm::value_ptr(stateToColor(State::HIGHLIGHT)));
        int idx = this->manipulator_association.back().first;
        BasicGL::drawSphere(this->mesh_manipulators[idx].getManipPosition().x, this->mesh_manipulators[idx].getManipPosition().y, this->mesh_manipulators[idx].getManipPosition().z, this->sphereRadius, 15,15);
    }

    glBegin(GL_LINES);
    for(int i = 0; i < this->manipulator_association.size(); ++i) {
        if(this->manipulator_association[i].second != -1) {
            glColor3fv(glm::value_ptr(glm::vec3(0., 0., 1.)));
            glVertex3fv(glm::value_ptr(this->mesh_manipulators[this->manipulator_association[i].first].getManipPosition()));
            glColor3fv(glm::value_ptr(glm::vec3(0., 0., 1.)));
            glVertex3fv(glm::value_ptr(this->marker_manipulators[this->manipulator_association[i].second].getManipPosition()));
        }
    }
    glEnd();
}

void MarkerManipulator::applyDeformation() {
    std::vector<bool> handles(this->mesh_manipulators.size(), false);
    for(int i = 0; i < this->manipulator_association.size(); ++i) {
        if(this->manipulator_association[i].second != -1) {
            mesh->movePoints({this->manipulator_association[i].first},
                             {this->marker_manipulators[this->manipulator_association[i].second].getManipPosition()});
            handles[this->manipulator_association[i].first] = true;
        }
    }

    AsRigidAsPossible * deformer = dynamic_cast<SurfaceMesh*>(this->mesh)->arapDeformer;
    if(!deformer) {
        std::cout << "WARNING: ARAP manipulator can be used only with the ARAP deformer !" << std::endl;
        return;
    }
    std::vector<glm::vec3> positions;
    positions = mesh->getVertices();

    std::vector<Vec3D<float>> ptsAsVec3D;
    for(int i = 0; i < positions.size(); ++i) {
        glm::vec3 pt = positions[i];
        ptsAsVec3D.push_back(Vec3D(pt[0], pt[1], pt[2]));
    }
    dynamic_cast<SurfaceMesh*>(this->mesh)->setHandlesARAP(handles);
    deformer->compute_deformation(ptsAsVec3D);

    for(int i = 0; i < positions.size(); ++i)
        positions[i] = glm::vec3(ptsAsVec3D[i][0], ptsAsVec3D[i][1], ptsAsVec3D[i][2]);

    mesh->movePoints(positions);
    for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
        this->mesh_manipulators[i].setManipPosition(this->mesh->getVertice(i));
    }
    Q_EMIT needSendTetmeshToGPU();
    mesh->addStateToHistory();
}

}
