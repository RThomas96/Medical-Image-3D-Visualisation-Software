#ifndef MANIPULATOR_HPP_
#define MANIPULATOR_HPP_

#include <QGLViewer/manipulatedFrame.h>
#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>

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
	class Manipulator {
	public:
		Manipulator(const glm::vec3& position);
        ~Manipulator() {};

		void lockPosition();
		void setCustomConstraint();

		void setPosition(const glm::vec3& position);
		glm::vec3 getPosition() const;
        glm::vec3 getLastPosition() const { return this->lastPosition; };
    	void setLastPosition(const glm::vec3& position);

        void updateLastPosition() { this->lastPosition = this->getPosition(); };

        void disable() { this->manipulatedFrame.removeFromMouseGrabberPool(); };
        void enable() { this->manipulatedFrame.addInMouseGrabberPool(); };

        void preventToSpin() { this->manipulatedFrame.setSpinningSensitivity(100.0); };
        void preventToRotate() { this->manipulatedFrame.setRotationSensitivity(0.0); };

        bool isManipulated() const { return this->manipulatedFrame.isManipulated(); }

	protected:
		qglviewer::ManipulatedFrame manipulatedFrame;
        glm::vec3 lastPosition;
	};

    class MeshManipulator {
    public:
        virtual bool isActive() = 0;
        virtual void setActivation(bool isActive) = 0;

        virtual void getMovement(glm::vec3& origin, glm::vec3& target) = 0;
        virtual bool hasBeenMoved() const = 0;

        virtual void addManipulator(const glm::vec3& position) = 0;

        virtual void removeManipulator(const glm::vec3& position) = 0;
        virtual void removeManipulator(int idx) = 0;

        virtual int getManipulatorIdx(const glm::vec3& position) const = 0;
        virtual int getNbManipulator() const = 0;

        virtual void setAllPositions(const std::vector<glm::vec3>& positions) = 0;
        virtual void getAllPositions(std::vector<glm::vec3>& positions) = 0;

        virtual void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const = 0;

        virtual ~MeshManipulator() {};
    };

	/// @ingroup uitools
	/// @brief The DirectManipulator class represents a set of vertex manipulators used to manipulate each mesh's vertex.
	/// @details The active manipulator indicates the manipulator at range for being grabbed by the mouse.
	/// The commonConstraint is a custom translation constraint allowing to simplify vertex manipulation. See UITool::CustomConstraint.
	/// The lockConstraint allow to prevent manipulator to move when the feature is inactive.
	class DirectManipulator : public MeshManipulator {
	public:
		DirectManipulator(const std::vector<glm::vec3>& positions);

        bool isActive() override { return this->active; };
        void setActivation(bool isActive) override;

        void getMovement(glm::vec3& origin, glm::vec3& target) override;
        bool hasBeenMoved() const override;

        void addManipulator(const glm::vec3& position) override;

        void removeManipulator(const glm::vec3& position) override;
        void removeManipulator(int idx) override;

        int getManipulatorIdx(const glm::vec3& position) const override;
        int getNbManipulator() const override;

        void setAllPositions(const std::vector<glm::vec3>& positions) override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;

        int getMovedManipulatorIdx() const;

        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

	private:
		std::vector<Manipulator> manipulators;
        std::vector<bool> manipulatorsToDisplay;

		bool active;
	};

	class FreeManipulator : public MeshManipulator {
	public:
		FreeManipulator(const std::vector<glm::vec3>& positions);

        bool isActive() override { return this->active; };
        void setActivation(bool isActive) override;

        void getMovement(glm::vec3& origin, glm::vec3& target) override;
        bool hasBeenMoved() const override;

        void addManipulator(const glm::vec3& position) override;

        void removeManipulator(const glm::vec3& position) override;
        void removeManipulator(int idx) override;

        int getManipulatorIdx(const glm::vec3& position) const override;
        int getNbManipulator() const override;

        void setAllPositions(const std::vector<glm::vec3>& positions) override;
        void getAllPositions(std::vector<glm::vec3>& positions) override;

        int getMovedManipulatorIdx() const;
        void getManipulatorsToDisplay(std::vector<bool>& toDisplay) const override;

	private:
		Manipulator manipulator;

		bool active;
	};

}	 // namespace UITool
#endif
