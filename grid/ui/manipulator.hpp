#ifndef MANIPULATOR_HPP_
#define MANIPULATOR_HPP_

#include "../../qt/viewers/include/scene.hpp"
#include <QGLViewer/manipulatedFrame.h>
#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>
#include <qobject.h>

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
}	 // namespace UITool
#endif
