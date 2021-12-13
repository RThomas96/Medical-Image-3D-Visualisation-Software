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
		Manipulator() {
			manipulatedFrame.setSpinningSensitivity(100.0);	   // Prevent the manipulator to spin
			manipulatedFrame.setRotationSensitivity(0.0);	 // Prevent the manipulator to rotate
		}

		qglviewer::ManipulatedFrame& getManipulatedFrame() { return this->manipulatedFrame; };

		void setPosition(glm::vec3 position);
		glm::vec3 getPosition();

	protected:
		qglviewer::ManipulatedFrame manipulatedFrame;
	};

	/// @ingroup uitools
	/// @brief The VertexManipulator class represents a manipulator associed with a mesh's vertex.
	/// @details The associed mesh vertex is simply an index.
	class VertexManipulator : public Manipulator {
	public:
		VertexManipulator() :
			assignedIdx(-1) {}

		int getAssignedIdx() { return assignedIdx; };
		void setAssignedIdx(int i) { this->assignedIdx = i; };

	protected:
		int assignedIdx;
	};

	/// @ingroup uitools
	/// @brief The MeshManipulator class represents a set of vertex manipulators used to manipulate each mesh's vertex.
	/// @details The active manipulator indicates the manipulator at range for being grabbed by the mouse.
	/// The commonConstraint is a custom translation constraint allowing to simplify vertex manipulation. See UITool::CustomConstraint.
	/// The lockConstraint allow to prevent manipulator to move when the feature is inactive.
	class MeshManipulator {
	public:
		MeshManipulator(int nbManipulators);

		Manipulator& getActiveManipulator();
		void setActiveManipulator(int idx);
		glm::vec3 getActiveManipulatorPos();
		bool isActiveManipulatorManipuled();

		int setAssignedIdx(int idx, int i);
		int getAssignedIdx(int idx);

		void setAllPositions(std::vector<glm::vec3>& positions);
		void getAllPositions(std::vector<glm::vec3>& vec);

		void setConstraint(int idx);

		bool updateActiveManipulator();
		int getActiveManipulatorAssignedIdx();

		int getNbManipulators();

		void toggleActivation();

		void lock(int idx);

	private:
		int nbManipulators;
		int activeManipulator;
		std::vector<VertexManipulator> manipulators;

		qglviewer::Constraint* commonConstraint;
		qglviewer::Constraint* lockConstraint;

		int getGrabbedManipulator();

		bool isActive;
	};

}	 // namespace UITool
#endif
