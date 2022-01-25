#ifndef MANIPULATOR_HPP_
#define MANIPULATOR_HPP_

#include "../../viewer/include/scene.hpp"
#include <QGLViewer/manipulatedFrame.h>
#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>
#include <qobject.h>

/// @defgroup uitools UITools
/// @brief Group of tools used to interact with the application.
/// @details It include static components like sliders or double sliders, as well as
/// dynamic components used to directly interact with a 3D scene, like manipulators.
namespace UITool {

	/// @ingroup uitools
	/// @brief The CustomConstraint class can be applied to a manipulator to constrain its movements.
	/// @details This constraint ensure that the manipulator's translation always follow x, y and z axis.
	/// It enhances the user interactions.
	class CustomConstraint : public qglviewer::Constraint {
	public:
		CustomConstraint();

		virtual void constrainTranslation(qglviewer::Vec& t, qglviewer::Frame* const fr);

	private:
		qglviewer::AxisPlaneConstraint* constraintx;
		qglviewer::AxisPlaneConstraint* constrainty;
		qglviewer::AxisPlaneConstraint* constraintz;
	};

	/// @ingroup uitools
	/// @brief The LockConstraint class prevent a manipulator to move.
	class LockConstraint : public qglviewer::Constraint {
	public:
		virtual void constrainTranslation(qglviewer::Vec& t, qglviewer::Frame* const fr);
	};

	/// @ingroup uitools
	/// @brief The Manipulator class represents a coordinate system, defined by a position and an orientation,
	/// that can be move/rotate with the mouse.
	/// @details This class contain a qglviewer::ManipulatedFrame that can detect when it is grabbed by the mouse.
	/// @note It's a simple wrapper around qglviewer::ManipulatedFrame to be used with glm::vec3.
	class Manipulator : public qglviewer::ManipulatedFrame {
        Q_OBJECT
	public:
		Manipulator(const glm::vec3& position);
        ~Manipulator() {};

		void lockPosition();
		void setCustomConstraint();

		void setManipPosition(const glm::vec3& position);
		glm::vec3 getManipPosition() const;
        glm::vec3 getLastPosition() const { return this->lastPosition; };
    	void setLastPosition(const glm::vec3& position);

        void updateLastPosition() { this->lastPosition = this->getManipPosition(); };

        void disable() { this->removeFromMouseGrabberPool(); };
        void enable() { this->addInMouseGrabberPool(); };

        void preventToSpin() { this->setSpinningSensitivity(100.0); };
        void preventToRotate() { this->setRotationSensitivity(0.0); };

        void mouseReleaseEvent(QMouseEvent* const e, qglviewer::Camera* const camera);

        void mousePressEvent( QMouseEvent* const e, qglviewer::Camera* const camera);

        void mouseMoveEvent(QMouseEvent *const event, qglviewer::Camera *const camera);

        void checkIfGrabsMouse(int x, int y, const qglviewer::Camera *const camera);

        void slotMovePoint();

        glm::vec3 lastPosition;

    signals:
        void enterAtRangeForGrab(Manipulator*);
        void exitFromRangeForGrab(Manipulator*);

        // Those signals are emitted when the mouse is at range for grab
        void mouseRightButtonPressed(Manipulator*);
        void mouseRightButtonReleased(Manipulator*);
        void mouseRightButtonReleasedAndCtrlIsNotPressed(Manipulator*);

        // This signal is already present in the default ManipulatedFrame, but it doesn't not return its adress
        void isManipulated(Manipulator*);// The mouse right button is pressed and the mouse is moved

	public:
        bool isSelected;
        bool isAtRangeForGrab;
	};

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
