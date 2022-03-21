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

    enum State {
        NONE,
        AT_RANGE,
        SELECTED,
        LOCK,
        MOVE,
        WAITING
    };

    class Selection : public Manipulator {
        Q_OBJECT
    public:
        const qglviewer::Camera * camera;

        glm::ivec2 screenP0;
        glm::ivec2 screenP1;
        glm::vec3 p0;
        glm::vec3 p1;
        glm::vec3 p2;
        glm::vec3 p3;

        bool isTouchPressed;

        bool isInScreenSelection(glm::ivec2 p) {
            return (p[0] > screenP0[0] && p[1] > screenP0[1] && p[0] < screenP1[0] && p[1] < screenP1[1]);
        }

        bool isInSelection(const glm::vec3& position) {
            qglviewer::Vec pVec = camera->projectedCoordinatesOf(qglviewer::Vec(position[0], position[1], position[2]));
            glm::ivec2 screenPosition(pVec[0], pVec[1]);
            bool isVisible = true;
            bool found = false;
            qglviewer::Vec ptOnSurfaceVec = camera->pointUnderPixel(QPoint(screenPosition[0], screenPosition[1]), found);
            glm::vec3 ptOnSurface = glm::vec3(ptOnSurfaceVec[0], ptOnSurfaceVec[1], ptOnSurfaceVec[2]);
            if(found)
                std::cout << "found" << std::endl;
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

        Selection() : Manipulator(glm::vec3(0., 0., 0.)), p0(glm::vec3(0., 0., 0.)), p1(glm::vec3(0., 0., 0.)), p2(glm::vec3(0., 0., 0.)), p3(glm::vec3(0., 0., 0.)), screenP0(glm::ivec2(0., 0.)), screenP1(glm::ivec2(0, 0)), isTouchPressed(false) {};

    signals:
        void needToRedrawSelection(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
        void isSelecting();
        void resetSelection();
        void beginSelection();

    public slots:
        void keyPressed(QKeyEvent* e){
            if(!this->isTouchPressed)
                Q_EMIT beginSelection();
            this->isTouchPressed = true;
        };

        void keyReleased(QKeyEvent* e){
            if(this->isTouchPressed)
                Q_EMIT resetSelection();
            this->isTouchPressed = false;
        };

        void mouseReleaseEvent(QMouseEvent* const e, qglviewer::Camera* const camera) override {
            this->isSelected = false;
            Q_EMIT needToRedrawSelection(glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.));
        };

        void mousePressEvent( QMouseEvent* const e, qglviewer::Camera* const camera) override {
            this->isSelected = true;
        };

        void mouseMoveEvent(QMouseEvent *const event, qglviewer::Camera *const camera) override {
            if(this->isSelected && this->isTouchPressed) {
                Q_EMIT isSelecting();
            }
        };

        void checkIfGrabsMouse(int x, int y, const qglviewer::Camera *const camera) override {
            qglviewer::Vec pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(x, y, 0));
            //std::cout << "x: " << x << " - y: " << y << std::endl;
            glm::vec3 p = glm::vec3(pVec[0], pVec[1], pVec[2]);
            this->camera = camera;
            if(!this->isSelected) {
                this->screenP0 = glm::ivec2(x, y);
                this->screenP1 = glm::ivec2(x, y);
            }
            if(this->isSelected && this->isTouchPressed) {
                this->screenP1 = glm::ivec2(x, y);
                this->updateSelection(camera);
            }
            //std::cout << "P0: " << this->screenP0 << "P1: " << this->screenP1 << std::endl;
            setGrabsMouse(this->isSelected || this->isTouchPressed);
        }

        void updateSelection(const qglviewer::Camera *const camera) {
            glm::ivec2 screenMin;
            glm::ivec2 screenMax;
            screenMin[0] = std::min(this->screenP0[0], this->screenP1[0]);
            screenMin[1] = std::min(this->screenP0[1], this->screenP1[1]);
            screenMax[0] = std::max(this->screenP0[0], this->screenP1[0]);
            screenMax[1] = std::max(this->screenP0[1], this->screenP1[1]);
            this->screenP0 = screenMin;
            this->screenP1 = screenMax;

            qglviewer::Vec pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(screenMax[0], screenMax[1], 0.1));
            this->p0 = glm::vec3(pVec[0], pVec[1], pVec[2]);
            pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(screenMax[0], screenMin[1], 0.1));
            this->p1 = glm::vec3(pVec[0], pVec[1], pVec[2]);
            pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(screenMin[0], screenMin[1], 0.1));
            this->p2 = glm::vec3(pVec[0], pVec[1], pVec[2]);
            pVec = camera->unprojectedCoordinatesOf(qglviewer::Vec(screenMin[0], screenMax[1], 0.1));
            this->p3 = glm::vec3(pVec[0], pVec[1], pVec[2]);
            Q_EMIT needToRedrawSelection(p0, p1, p2, p3);
        }
    };

	//! @ingroup uitools
    class MeshManipulator {
    public:
        BaseMesh * mesh;

        MeshManipulator(BaseMesh * mesh): mesh(mesh) {}

        float getManipulatorSize(bool side = false);

        virtual bool isActive() = 0;

        // These functions are used from the exterior
        virtual void setActivation(bool isActive) = 0;

        virtual void removeManipulator(Manipulator * manipulatorToDisplay) = 0;

        virtual bool isWireframeDisplayed() = 0;

        // These functions are used only in glMeshManipulator in the prepare function
        virtual void getAllPositions(std::vector<glm::vec3>& positions) = 0;
        virtual void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const = 0;
        virtual void getManipulatorsState(std::vector<State>& states) const = 0;
        virtual void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) = 0;

        virtual ~MeshManipulator() {};
    //public slots:
        // These are connected to the manipulators
        virtual void displayManipulator(Manipulator * manipulatorToDisplay) = 0;
        virtual void hideManipulator(Manipulator * manipulatorToDisplay) = 0;

        virtual void moveManipulator(Manipulator * manipulator) = 0;
        virtual void selectManipulator(Manipulator * manipulator) = 0;
        virtual void deselectManipulator(Manipulator * manipulator) = 0;

        virtual void addManipulator(const glm::vec3& position) = 0;
        virtual void keyPressed(QKeyEvent* e) = 0;
        virtual void keyReleased(QKeyEvent* e) = 0;

    //signal:
        virtual void needRedraw() = 0;
        // These signals are trigerred from the scene
        virtual void keyQReleased() = 0;
        virtual void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) = 0;
        virtual void pointIsClickedInPlanarViewer(const glm::vec3& position) = 0;

        //This signal is used to trigger a function in the scene
        //This should be removed when the grid will have its own "Drawable" class
        virtual void needSendTetmeshToGPU() = 0;
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

        bool isActive() override { return this->active; };
        void setActivation(bool isActive) override;

        void addManipulator(const glm::vec3& position) override;

        void removeManipulator(Manipulator * manipulatorToDisplay) override;

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

        bool isWireframeDisplayed() override;

        void checkSelectedManipulators();

    public slots:
        void displayManipulator(Manipulator * manipulatorToDisplay) override;
        void hideManipulator(Manipulator * manipulatorToDisplay) override;

        void moveManipulator(Manipulator * manipulator) override;
        void selectManipulator(Manipulator * manipulator) override;
        void deselectManipulator(Manipulator * manipulator) override;
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;

    signals:
        void needRedraw() override;
        void keyQReleased() override;
        void needSendTetmeshToGPU() override;
        void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) override;
        void pointIsClickedInPlanarViewer(const glm::vec3& position) override;
    public:
        Selection selection;
	private:
		std::vector<Manipulator> manipulators;
        std::vector<bool> manipulatorsToDisplay;
        std::vector<bool> selectedManipulators;


		bool active;
	};

    //! @ingroup uitools
	class FreeManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		FreeManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions);

        bool isActive() override { return this->active; };
        void setActivation(bool isActive) override;

        void addManipulator(const glm::vec3& position) override;

        void removeManipulator(Manipulator * manipulatorToDisplay) override;

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;

        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;
        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

        bool isWireframeDisplayed() override;

    public slots:
        void displayManipulator(Manipulator * manipulatorToDisplay) override;
        void hideManipulator(Manipulator * manipulatorToDisplay) override;

        void moveManipulator(Manipulator * manipulator) override;
        void selectManipulator(Manipulator * manipulator) override;
        void deselectManipulator(Manipulator * manipulator) override;
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;

    signals:
        void needRedraw() override;
        void keyQReleased() override;
        void needSendTetmeshToGPU() override;
        void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) override;
        void pointIsClickedInPlanarViewer(const glm::vec3& position) override;
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

        bool isActive() override { return this->active; };
        void setActivation(bool isActive) override;

        void addManipulator(const glm::vec3& position) override;

        void removeManipulator(Manipulator * manipulatorToDisplay) override;

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;

        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;
        void getManipulatorsState(std::vector<State>& states) const override;

        bool isWireframeDisplayed() override;

		RotationManipulator kid_manip;


    public slots:
        void displayManipulator(Manipulator * manipulatorToDisplay) override;
        void hideManipulator(Manipulator * manipulatorToDisplay) override;

        void moveManipulator(Manipulator * manipulator) override;
        void selectManipulator(Manipulator * manipulator) override;
        void deselectManipulator(Manipulator * manipulator) override;
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;
        void mousePressed(QMouseEvent* e);
        void mouseReleased(QMouseEvent* e);

    signals:
        void needRedraw() override;
        void keyQReleased() override;
        void needSendTetmeshToGPU() override;
        void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) override;
        void pointIsClickedInPlanarViewer(const glm::vec3& position) override;

	private:
		Manipulator manipulator;

        bool evenMode;
		bool active;
	};

	/// @ingroup uitools
	class CompManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		CompManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions);

        bool isActive() override { return this->active; };
        void setActivation(bool isActive) override;

        void addManipulator(const glm::vec3& position) override;

        void removeManipulator(Manipulator * manipulatorToDisplay) override;

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

        bool isWireframeDisplayed() override;

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
        void displayManipulator(Manipulator * manipulatorToDisplay) override;
        void hideManipulator(Manipulator * manipulatorToDisplay) override;

        void moveManipulator(Manipulator * manipulator) override;
        void selectManipulator(Manipulator * manipulator) override;
        void deselectManipulator(Manipulator * manipulator) override;
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;

    signals:
        void needRedraw() override;
        void keyQReleased() override;
        void needSendTetmeshToGPU() override;
        void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) override;
        void pointIsClickedInPlanarViewer(const glm::vec3& position) override;

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

		bool active;
	};

    //! @ingroup uitools
	class ARAPManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		ARAPManipulator(BaseMesh * mesh, const std::vector<glm::vec3>& positions);

        bool isActive() override { return this->active; };
        void setActivation(bool isActive) override;

        void addManipulator(const glm::vec3& position) override;

        void removeManipulator(Manipulator * manipulatorToDisplay) override;

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsState(std::vector<State>& states) const override;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

        bool isWireframeDisplayed() override;

    public slots:
        void displayManipulator(Manipulator * manipulatorToDisplay) override;
        void hideManipulator(Manipulator * manipulatorToDisplay) override;

        void moveManipulator(Manipulator * manipulator) override;
        void selectManipulator(Manipulator * manipulator) override;
        void deselectManipulator(Manipulator * manipulator) override;
        void toggleMode();
        void keyPressed(QKeyEvent* e) override;
        void keyReleased(QKeyEvent* e) override;
        void checkSelectedManipulators();

    signals:
        void needRedraw() override;
        void keyQReleased() override;
        void needSendTetmeshToGPU() override;
        void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) override;
        void pointIsClickedInPlanarViewer(const glm::vec3& position) override;

    public:
        Selection selection;
	private:
		std::vector<Manipulator> manipulators;
        std::vector<bool> manipulatorsToDisplay;
        std::vector<bool> selectedManipulators;
        std::vector<bool> handles;
        std::vector<bool> alreadySwitch;

        bool isSelecting;
		bool active;
        bool moveMode;
	};
}	 // namespace UITool
#endif
