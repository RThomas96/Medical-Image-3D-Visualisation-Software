#ifndef MESH_MANIPULATOR_HPP_
#define MESH_MANIPULATOR_HPP_

#include "manipulator.hpp"

/// @defgroup uitools UITools
/// @brief Group of tools used to interact with the application.
/// @details It include static components like sliders or double sliders, as well as
/// dynamic components used to directly interact with a 3D scene, like manipulators.
namespace UITool {
    class MeshManipulator {
    public:
        // This should not be here
        // Unfortunatly, actually the scene inherite from QOpenGL_Core
        // So the scene canno't use the Slot/Signal mecanisme
        // Therefore, a pointer to the scene is mandatory to "connect" the functions
        Scene * scene;

        MeshManipulator(Scene * scene): scene(scene) {}

        virtual bool isActive() = 0;

        // These functions are used from the exterior
        virtual void setActivation(bool isActive) = 0;

        virtual void removeManipulator(Manipulator * manipulatorToDisplay) = 0;

        virtual bool isWireframeDisplayed() = 0;

        // These functions are used only in glMeshManipulator in the prepare function
        virtual void getAllPositions(std::vector<glm::vec3>& positions) = 0;
        virtual void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const = 0;
        virtual void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) = 0;

        virtual ~MeshManipulator() {};
    //public slots:
        // These are connected to the manipulators
        virtual void displayManipulator(Manipulator * manipulatorToDisplay) = 0;
        virtual void hideManipulator(Manipulator * manipulatorToDisplay) = 0;

        virtual void moveManipulator(Manipulator * manipulator) = 0;
        virtual void selectManipulator(Manipulator * manipulator) = 0;
        virtual void deselectManipulator(Manipulator * manipulator) = 0;

        // These are connected to the scene
        virtual void addManipulator(const glm::vec3& position) = 0;

    //signal:
        virtual void needRedraw() = 0;
        // This signal is trigerred from the scene
        virtual void keyQReleased() = 0;
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
		DirectManipulator(Scene * scene, const std::vector<glm::vec3>& positions);

        bool isActive() override { return this->active; };
        void setActivation(bool isActive) override;

        void addManipulator(const glm::vec3& position) override;

        void removeManipulator(Manipulator * manipulatorToDisplay) override;

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

        bool isWireframeDisplayed() override;

    public slots:
        void displayManipulator(Manipulator * manipulatorToDisplay) override;
        void hideManipulator(Manipulator * manipulatorToDisplay) override;

        void moveManipulator(Manipulator * manipulator) override;
        void selectManipulator(Manipulator * manipulator) override;
        void deselectManipulator(Manipulator * manipulator) override;

    signals:
        void needRedraw() override;
        void keyQReleased() override;

	private:
		std::vector<Manipulator> manipulators;
        std::vector<bool> manipulatorsToDisplay;

		bool active;
	};

	class FreeManipulator : public QObject, public MeshManipulator {
        Q_OBJECT
        Q_INTERFACES(UITool::MeshManipulator)

	public:
		FreeManipulator(Scene * scene, const std::vector<glm::vec3>& positions);

        bool isActive() override { return this->active; };
        void setActivation(bool isActive) override;

        void addManipulator(const glm::vec3& position) override;

        void removeManipulator(Manipulator * manipulatorToDisplay) override;

        void setAllManipulatorsPosition(const std::vector<glm::vec3>& positions) override;

        void getAllPositions(std::vector<glm::vec3>& positions) override;
        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

        bool isWireframeDisplayed() override;

    public slots:
        void displayManipulator(Manipulator * manipulatorToDisplay) override;
        void hideManipulator(Manipulator * manipulatorToDisplay) override;

        void moveManipulator(Manipulator * manipulator) override;
        void selectManipulator(Manipulator * manipulator) override;
        void deselectManipulator(Manipulator * manipulator) override;

    signals:
        void needRedraw() override;
        void keyQReleased() override;

	private:
		Manipulator manipulator;

		bool active;
	};
}	 // namespace UITool
#endif
