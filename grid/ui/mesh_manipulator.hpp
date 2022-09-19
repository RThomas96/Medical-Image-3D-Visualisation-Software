#ifndef MESH_MANIPULATOR_HPP_
#define MESH_MANIPULATOR_HPP_

#include <QGLViewer/manipulatedCameraFrame.h>
#include "manipulator.hpp"
#include "kid_manipulator.h"

//! @defgroup uitools UI
//! @brief Group of tools used to interact with the application.
//! @details It include static components like sliders or double sliders, as well as
//! dynamic components used to directly interact with a 3D scene, like manipulators.
namespace UITool {

    enum class State {
        NONE,
        AT_RANGE,
        SELECTED,
        LOCK,
        MOVE,
        WAITING,
        HIGHLIGHT
    };

    class Selection : public Manipulator {
        Q_OBJECT
    public:
        const qglviewer::Camera * camera;

        glm::vec4 color;
        glm::ivec2 screenP0;
        glm::ivec2 screenP1;
        glm::vec3 p0;
        glm::vec3 p1;
        glm::vec3 p2;
        glm::vec3 p3;

        bool isActive;
        bool isInSelectionMode;

        bool isInScreenSelection(glm::ivec2 p) {
            glm::ivec2 screenMin;
            glm::ivec2 screenMax;
            screenMin[0] = std::min(this->screenP0[0], this->screenP1[0]);
            screenMin[1] = std::min(this->screenP0[1], this->screenP1[1]);
            screenMax[0] = std::max(this->screenP0[0], this->screenP1[0]);
            screenMax[1] = std::max(this->screenP0[1], this->screenP1[1]);
            return (p[0] > screenMin[0] && p[1] > screenMin[1] && p[0] < screenMax[0] && p[1] < screenMax[1]);
        }

        bool isInSelection(const glm::vec3& position) {
            qglviewer::Vec pVec = camera->projectedCoordinatesOf(qglviewer::Vec(position[0], position[1], position[2]));
            glm::ivec2 screenPosition(pVec[0], pVec[1]);
            bool isVisible = true;
            bool found = false;
            qglviewer::Vec ptOnSurfaceVec = camera->pointUnderPixel(QPoint(screenPosition[0], screenPosition[1]), found);
            glm::vec3 ptOnSurface = glm::vec3(ptOnSurfaceVec[0], ptOnSurfaceVec[1], ptOnSurfaceVec[2]);
            if(found && glm::distance(ptOnSurface, position) > 0.01) {
                isVisible = false;
            }
            return (isVisible && isInScreenSelection(screenPosition));
        }

        std::vector<bool> areInSelection(const std::vector<glm::vec3>& positions) {
            std::vector<bool> res;
            for(int i = 0; i < positions.size(); ++i) {
                res.push_back(this->isInSelection(positions[i]));
            }
            return res;
        }

        Selection() : Manipulator(glm::vec3(0., 0., 0.)), p0(glm::vec3(0., 0., 0.)), p1(glm::vec3(0., 0., 0.)), p2(glm::vec3(0., 0., 0.)), p3(glm::vec3(0., 0., 0.)), screenP0(glm::ivec2(0., 0.)), screenP1(glm::ivec2(0, 0)), isInSelectionMode(false), color(glm::vec4(1., 0., 0., 0.5)), isActive(true) {this->enable();};

        void activate() {this->isActive = true;}
        void deactivate() {this->isActive = false;}

    signals:
        void needToRedrawSelection(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec4& color);
        void beginSelection();
        void endSelection();
        void isSelecting();

    public slots:
        void setColor(const glm::vec4& color) {
            this->color = color;
        };

        void enterSelectionMode() {
            if(!this->isActive)
                return;
            this->isInSelectionMode = true;
        }

        void exitSelectionMode() {
            this->isInSelectionMode = false;
            this->isSelected = false;
            Q_EMIT needToRedrawSelection(glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.), this->color);
            Q_EMIT endSelection();
        }

        void keyPressed(QKeyEvent* e){
        };

        void keyReleased(QKeyEvent* e){
        };

        void mousePressEvent( QMouseEvent* const e, qglviewer::Camera* const camera) override {
            std::cout << "Mouse press event" << std::endl;
            if(this->isInSelectionMode) {
                this->isSelected = true;
                std::cout << "IsSelected is activated" << std::endl;
                Q_EMIT beginSelection();
            }
        };

        void mouseReleaseEvent(QMouseEvent* const e, qglviewer::Camera* const camera) override {
            this->isSelected = false;
            Q_EMIT needToRedrawSelection(glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.), this->color);
            Q_EMIT endSelection();
        };

        void mouseMoveEvent(QMouseEvent *const event, qglviewer::Camera *const camera) override {
            if(this->isInSelectionMode && this->isSelected) {
                Q_EMIT isSelecting();
            }
        };

        void checkIfGrabsMouse(int x, int y, const qglviewer::Camera *const camera) override {
            qglviewer::Vec pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(x, y, 0));
            glm::vec3 p = glm::vec3(pVec[0], pVec[1], pVec[2]);
            this->camera = camera;
            if(!this->isSelected) {
                this->screenP0 = glm::ivec2(x, y);
                this->screenP1 = glm::ivec2(x, y);
            }
            if(this->isInSelectionMode && this->isSelected) {
                this->screenP1 = glm::ivec2(x, y);
                this->updateSelection(camera);
            }
            setGrabsMouse(this->isInSelectionMode);
        }

        void updateSelection(const qglviewer::Camera *const camera) {
            glm::ivec2 screenMin;
            glm::ivec2 screenMax;
            screenMin[0] = std::min(this->screenP0[0], this->screenP1[0]);
            screenMin[1] = std::min(this->screenP0[1], this->screenP1[1]);
            screenMax[0] = std::max(this->screenP0[0], this->screenP1[0]);
            screenMax[1] = std::max(this->screenP0[1], this->screenP1[1]);

            qglviewer::Vec pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(screenMax[0], screenMax[1], 0.1));
            this->p0 = glm::vec3(pVec[0], pVec[1], pVec[2]);
            pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(screenMax[0], screenMin[1], 0.1));
            this->p1 = glm::vec3(pVec[0], pVec[1], pVec[2]);
            pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(screenMin[0], screenMin[1], 0.1));
            this->p2 = glm::vec3(pVec[0], pVec[1], pVec[2]);
            pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(screenMin[0], screenMax[1], 0.1));
            this->p3 = glm::vec3(pVec[0], pVec[1], pVec[2]);
            Q_EMIT needToRedrawSelection(p0, p1, p2, p3, this->color);
        }
    };

	//! @ingroup uitools
    class MeshManipulator {
    public:
        // These are needed here as there drawing functions are directly in the class
        RotationManipulator * kid_manip;
        Selection selection;
        BaseMesh * mesh;

        MeshManipulator(BaseMesh * mesh): mesh(mesh) {}

        float getManipulatorSize(bool side = false);
        void removeKidManip();

        // These functions are used only in glMeshManipulator in the prepare function
        virtual void getAllPositions(std::vector<glm::vec3>& positions) = 0;
        virtual void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const = 0;
        virtual void getManipulatorsState(std::vector<State>& states) const = 0;
        virtual void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) = 0;
        virtual void updateWithMeshVertices() = 0;

        virtual ~MeshManipulator() {};
    //public slots:
        // These are connected to the manipulators

        virtual void keyPressed(QKeyEvent* e) = 0;
        virtual void keyReleased(QKeyEvent* e) = 0;
        virtual void mousePressed(QMouseEvent*e) = 0;
        virtual void mouseReleased(QMouseEvent*e) = 0;

    //signal:
        virtual void needRedraw() = 0;
        // These signals are trigerred from the scene

        //This signal is used to trigger a function in the scene
        //This should be removed when the grid will have its own "Drawable" class
        virtual void needSendTetmeshToGPU() = 0;
        virtual void needChangeKidManipulatorRadius(float radius) = 0;
        virtual void needChangeCursor(UITool::CursorType cursorType) = 0;
        virtual void needChangeCursorInPlanarView(UITool::CursorType cursorType) = 0;
        virtual void needChangeSelectedPoints(std::pair<int, glm::vec3> selectedPoint) = 0;
        virtual void needUpdateSceneCenter() = 0;
    };
}
Q_DECLARE_INTERFACE(UITool::MeshManipulator, "MeshManipulator")
namespace UITool {
	/// @ingroup uitools
	/// @brief The DirectManipulator class represents a set of vertex manipulators used to manipulate each mesh's vertex.
	/// @details The active manipulator indicates the manipulator at range for being grabbed by the mouse.
	/// The commonConstraint is a custom translation constraint allowing to simplify vertex manipulation. See UITool::CustomConstraint.
	/// The lockConstraint allow to prevent manipulator to move when the feature is inactive.
	class DirectManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		DirectManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions);

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void updateWithMeshVertices() override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

        void checkSelectedManipulators();

    public slots:
        void displayManipulator(Manipulator * manipulatorToDisplay);
        void hideManipulator(Manipulator * manipulatorToDisplay);

        void moveManipulator(Manipulator * manipulator);
        void selectManipulator(Manipulator * manipulator);
        void deselectManipulator(Manipulator * manipulator);
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;
        void mousePressed(QMouseEvent* e) override;
        void mouseReleased(QMouseEvent* e) override;

    signals:
        void needRedraw() override;
        void needSendTetmeshToGPU() override;
        void needChangeKidManipulatorRadius(float radius) override;
        void needChangeCursor(UITool::CursorType cursorType) override;
        void needChangeCursorInPlanarView(UITool::CursorType cursorType) override;
        void needChangeSelectedPoints(std::pair<int, glm::vec3> selectedPoint) override;
        void needUpdateSceneCenter() override;
    private:
		std::vector<Manipulator> manipulators;
        std::vector<bool> manipulatorsToDisplay;
        std::vector<bool> selectedManipulators;
        bool meshIsModified;
    };

    //! @ingroup uitools
	class FreeManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		FreeManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions);

        void addManipulator(const glm::vec3& position);
        void setActivation(bool isActive);

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void updateWithMeshVertices() override;

        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;
        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

    public slots:

        void moveManipulator(Manipulator * manipulator);
        void selectManipulator(Manipulator * manipulator);
        void deselectManipulator(Manipulator * manipulator);
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;
        void mousePressed(QMouseEvent*e) override;
        void mouseReleased(QMouseEvent*e) override;

    signals:
        void needRedraw() override;
        void needSendTetmeshToGPU() override;
        void needChangeKidManipulatorRadius(float radius) override;
        void needChangeCursor(UITool::CursorType cursorType) override;
        void needChangeCursorInPlanarView(UITool::CursorType cursorType) override;
        void needChangeSelectedPoints(std::pair<int, glm::vec3> selectedPoint) override;
        void needUpdateSceneCenter() override;
        void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos);
	private:
        void addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos);

		Manipulator manipulator;
        bool active;
    };

    //! @ingroup uitools
	class PositionManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		PositionManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions);
        ~PositionManipulator();

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void updateWithMeshVertices() override;

        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;
        void getManipulatorsState(std::vector<State>& states) const override;

    public slots:
        void toggleEvenMode();
        void moveManipulator(Manipulator * manipulator);
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;
        void mousePressed(QMouseEvent* e) override;
        void mouseReleased(QMouseEvent* e) override;

    signals:
        void needRedraw() override;
        void needSendTetmeshToGPU() override;
        void needChangeKidManipulatorRadius(float radius) override;
        void needChangeCursor(UITool::CursorType cursorType) override;
        void needChangeCursorInPlanarView(UITool::CursorType cursorType) override;
        void needChangeSelectedPoints(std::pair<int, glm::vec3> selectedPoint) override;
        void needUpdateSceneCenter() override;

	private:
        bool evenMode;
        bool meshIsModified;
	};

	/// @ingroup uitools
	class CompManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		CompManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions);

        void addManipulator(const glm::vec3& position);

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void updateWithMeshVertices() override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

        void addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos);
        void displayErrorNoMeshAssigned();
        void assignMeshToRegister(BaseMesh * meshToRegister);
        void switchToSelectionMode();
        void validate();
        void apply();
        void clearSelectedPoints();
        void assignPreviousSelectedPoints(const std::vector<std::pair<int, std::pair<int, glm::vec3>>>& previousSelectedPoints, const std::vector<std::vector<glm::vec3>>& previousPositions, std::vector<int> previousNumberOfSelectedPoints);
        void undo();

    public slots:
        void selectManipulator(Manipulator * manipulator);
        void deselectManipulator(Manipulator * manipulator);
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;
        void mousePressed(QMouseEvent*e) override;
        void mouseReleased(QMouseEvent*e) override;

    signals:
        void needRedraw() override;
        void needSendTetmeshToGPU() override;
        void needChangeKidManipulatorRadius(float radius) override;
        void needChangeCursor(UITool::CursorType cursorType) override;
        void needChangeCursorInPlanarView(UITool::CursorType cursorType) override;
        void needChangeSelectedPoints(std::pair<int, glm::vec3> selectedPoint) override;
        void needUpdateSceneCenter() override;
        void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos);
        void pointIsClickedInPlanarViewer(const glm::vec3& position);

    public:
        std::vector<std::pair<int, std::pair<int, glm::vec3>>> selectedPoints;
        std::vector<std::vector<glm::vec3>> previousPositions;
        std::vector<int> previousNumberOfSelectedPoints;
	private:
        BaseMesh * meshToRegister;
        int currentPairToSelect;

        // states
        // 0: not assigned to a point (neutral)(on mesh)
        // 1: assigned to a point (neutral)(on mesh)
        // 2: waiting to be selected (on mesh)
        // 3: selected, waiting for second point (on mesh)
                // 4: not selected :'( (on mesh)
        // 4: not selected :'( (on mesh) (invisible)
        // 5: waiting for validation (on grid)
        // 6: not assigned to a point (neutral)(on grid)
        // 7: assigned to a point (neutral)(on grid)

        std::vector<Manipulator> manipulators;
        std::vector<bool> manipulatorsIsOnMesh;
        std::vector<bool> manipulatorsToDisplay;
        std::vector<int> manipulatorsState;

        bool hasAMeshToRegister;

        bool isOnSelectionMode;
        bool isSelectingFirstPoint;
        bool isSelectingSecondPoint;

        bool oneManipulatorWasAlreadyAdded;
        bool oneManipulatorIsAtRangeForGrab;

	};

    //! @ingroup uitools
	class ARAPManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)
        /// NEW
    public:

        ARAPManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions);
        ~ARAPManipulator();

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void updateWithMeshVertices() override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

    public slots:
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;
        void mousePressed(QMouseEvent*e) override;
        void mouseReleased(QMouseEvent*e) override;
        void toggleEvenMode(bool value);

    signals:
        void needRedraw() override;
        void needSendTetmeshToGPU() override;
        void needChangeKidManipulatorRadius(float radius) override;
        void needChangeCursor(UITool::CursorType cursorType) override;
        void needChangeCursorInPlanarView(UITool::CursorType cursorType) override;
        void needChangeSelectedPoints(std::pair<int, glm::vec3> selectedPoint) override;
        void needUpdateSceneCenter() override;

    protected:
        enum SelectionMode { INACTIVE , ACTIVE , ADD_FIXED , ADD_MOVING , REMOVE };
        bool debug_mode = true;
        void print_debug(const char * text);
        void updateSelection();
        void computeManipulatorFromSelection();
        void setPositions(std::vector<glm::vec3>& positions);
        void moveGuizmo();
        void makeSelecteFixedHandles();
        void moveOneManipulator();

        std::vector<bool> getHandles();

        std::vector<bool> selectedVertices;
        std::vector<bool> fixedVertices;
		std::vector<Manipulator> manipulators;

        std::vector<bool> manipulatorsAtRangeForGrab;

    private:
        SelectionMode selectionMode;
        bool meshIsModified;
    };

	class SliceManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		SliceManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions);

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void updateWithMeshVertices() override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

    public slots:
        void displayManipulator(Manipulator * manipulatorToDisplay);
        void hideManipulator(Manipulator * manipulatorToDisplay);

        void updateSliceToSelect(SliceOrientation sliceOrientation);
        void moveGuizmo();

        void computeManipulatorFromSelection();
        void selectSlice(SliceOrientation sliceOrientation);
        void movePlanes(const glm::vec3& planesPosition);
        std::vector<bool> getHandles();
        void setPositions(std::vector<glm::vec3>& positions);

        void moveManipulator(Manipulator * manipulator);
        void selectManipulator(Manipulator * manipulator);
        void deselectManipulator(Manipulator * manipulator);
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;
        void mousePressed(QMouseEvent* e) override;
        void mouseReleased(QMouseEvent* e) override;

    signals:
        void needRedraw() override;
        void needSendTetmeshToGPU() override;
        void needChangeKidManipulatorRadius(float radius) override;
        void needChangeCursor(UITool::CursorType cursorType) override;
        void needChangeCursorInPlanarView(UITool::CursorType cursorType) override;
        void needChangeSelectedPoints(std::pair<int, glm::vec3> selectedPoint) override;
        void needUpdateSceneCenter() override;
        void needChangePointsToProject(std::vector<int> selectedPoints);
    private:
		std::vector<Manipulator> manipulators;

        std::vector<bool> selectedVertices;
        std::vector<bool> fixedVertices;

        SliceOrientation currentSelectedSlice;
        glm::vec3 slicesPositions;

        bool guizmoUpToDate;
        bool meshIsModified;

        float selectionRadius;
        float selectionRange;
        float incrementRadius;
        float incrementRange;
    };

    enum class FixedRegistrationManipulatorState {
        NONE,
        SELECTING_FIRST_POINT,
        SELECTING_SECOND_POINT
    };

	class FixedRegistrationManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		FixedRegistrationManipulator(BaseMesh * mesh, Grid * gridToRegister, const std::vector<glm::vec3>& positions);

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void updateWithMeshVertices() override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

    public slots:
        void moveManipulator(Manipulator * manipulator);
        void selectManipulator(Manipulator * manipulator);
        void deselectManipulator(Manipulator * manipulator);
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;
        void mousePressed(QMouseEvent* e) override;
        void mouseReleased(QMouseEvent* e) override;
        void addManipulator(const glm::vec3& position);
        void addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos);
        void apply();
        void clear();

    signals:
        void needRedraw() override;
        void needSendTetmeshToGPU() override;
        void needChangeKidManipulatorRadius(float radius) override;
        void needChangeCursor(UITool::CursorType cursorType) override;
        void needChangeCursorInPlanarView(UITool::CursorType cursorType) override;
        void needChangeSelectedPoints(std::pair<int, glm::vec3> selectedPoint) override;
        void needUpdateSceneCenter() override;
        void pointIsClickedInPlanarViewer(const glm::vec3& position);
        void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos);
        void needChangeActivatePreviewPoint(bool preview);
	private:
        Grid * gridToRegister;
        FixedRegistrationManipulatorState toolState;
        int selectedIndex;
        int nbNotAssociatedPoints;
		std::vector<Manipulator> manipulators;
        std::vector<bool> manipulatorsToDisplay;
        std::vector<bool> selectedManipulators;
        std::vector<bool> isFixed;
        std::vector<int> associatedManipulator;
        std::vector<int> fixed;
	};

}	 // namespace UITool
#endif
