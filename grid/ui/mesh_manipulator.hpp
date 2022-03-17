#ifndef MESH_MANIPULATOR_HPP_
#define MESH_MANIPULATOR_HPP_

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

    signals:
        void needRedraw() override;
        void keyQReleased() override;
        void needSendTetmeshToGPU() override;
        void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos) override;
        void pointIsClickedInPlanarViewer(const glm::vec3& position) override;

	private:
		std::vector<Manipulator> manipulators;
        std::vector<bool> manipulatorsToDisplay;
        std::vector<bool> selectedManipulators;
        std::vector<bool> handles;

		bool active;
        bool moveMode;
	};
}	 // namespace UITool
#endif
