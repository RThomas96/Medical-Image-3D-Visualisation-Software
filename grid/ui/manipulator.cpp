#include "manipulator.hpp"
#include <limits>
#include "glm/gtx/string_cast.hpp" 

namespace UITool {

	CustomConstraint::CustomConstraint() {
		this->constraintx = new qglviewer::LocalConstraint();
		this->constraintx->setTranslationConstraintType(qglviewer::AxisPlaneConstraint::AXIS);
		this->constraintx->setTranslationConstraintDirection(qglviewer::Vec(1., 0., 0.));

		this->constrainty = new qglviewer::LocalConstraint();
		this->constrainty->setTranslationConstraintType(qglviewer::AxisPlaneConstraint::AXIS);
		this->constrainty->setTranslationConstraintDirection(qglviewer::Vec(0., 1., 0.));

		this->constraintz = new qglviewer::LocalConstraint();
		this->constraintz->setTranslationConstraintType(qglviewer::AxisPlaneConstraint::AXIS);
		this->constraintz->setTranslationConstraintDirection(qglviewer::Vec(0., 0., 1.));
	}

	void CustomConstraint::constrainTranslation(qglviewer::Vec& t, qglviewer::Frame* const fr) {
		if (std::abs(t.x) > std::abs(t.y) && std::abs(t.x) > std::abs(t.z)) {
			this->constraintx->constrainTranslation(t, fr);
		} else if (std::abs(t.y) > std::abs(t.x) && std::abs(t.y) > std::abs(t.z)) {
			this->constrainty->constrainTranslation(t, fr);
		} else if (std::abs(t.z) > std::abs(t.x) && std::abs(t.z) > std::abs(t.y)) {
			this->constraintz->constrainTranslation(t, fr);
		}
	}

	void LockConstraint::constrainTranslation(qglviewer::Vec& t, qglviewer::Frame* const fr) {
		t[0] = 0;
		t[1] = 0;
		t[2] = 0;
	}

    /***/

    Manipulator::Manipulator(const glm::vec3& position) { 
        this->setManipPosition(position); 
        this->lastPosition = position;
        this->enable(); 
        this->isSelected = false;
        this->isAtRangeForGrab = false;
    }

	glm::vec3 Manipulator::getManipPosition() const {
		double x = 0;
		double y = 0;
		double z = 0;
		this->getPosition(x, y, z);
		return glm::vec3(x, y, z);
	}

	void Manipulator::setManipPosition(const glm::vec3& position) {
		this->setPosition(position[0], position[1], position[2]);
	}

	void Manipulator::setLastPosition(const glm::vec3& position) {
		this->lastPosition = position;
	}

	void Manipulator::lockPosition() {
		this->setConstraint(new LockConstraint());
    }

	void Manipulator::setCustomConstraint() {
		this->setConstraint(new CustomConstraint());
    }

    void Manipulator::slotMovePoint() { 
    };

    void Manipulator::mouseReleaseEvent(QMouseEvent* const e, qglviewer::Camera* const camera) { 
        ManipulatedFrame::mouseReleaseEvent(e, camera);
        if (e->button() == Qt::RightButton) {
            this->isSelected = false;
            Q_EMIT mouseRightButtonReleased(this);
            if (e->modifiers() != Qt::ControlModifier) {
                Q_EMIT mouseRightButtonReleasedAndCtrlIsNotPressed(this);
            }
        }
    };

    void Manipulator::mousePressEvent(QMouseEvent *const e, qglviewer::Camera *const camera) {
        ManipulatedFrame::mousePressEvent(e, camera);
        if (e->button() == Qt::RightButton) {
            Q_EMIT mouseRightButtonPressed(this);
            this->isSelected = true;
        }
    }

    void Manipulator::mouseMoveEvent(QMouseEvent *const event, qglviewer::Camera *const camera) {
        ManipulatedFrame::mouseMoveEvent(event, camera);
        if (action_ != QGLViewer::NO_MOUSE_ACTION) {
            Q_EMIT isManipulated(this);
            this->lastPosition = this->getManipPosition();
        }
    }

    void Manipulator::checkIfGrabsMouse(int x, int y, const qglviewer::Camera *const camera) {
        const int thresold = 5;
        const qglviewer::Vec proj = camera->projectedCoordinatesOf(position());
        bool isCurrentlyAtRangeForGrab =  ((fabs(x - proj.x) < thresold) && (fabs(y - proj.y) < thresold));
        setGrabsMouse(this->isSelected || isCurrentlyAtRangeForGrab);
        if(this->isAtRangeForGrab != isCurrentlyAtRangeForGrab) {
            this->isAtRangeForGrab = isCurrentlyAtRangeForGrab;
            if(this->isAtRangeForGrab) {
                Q_EMIT enterAtRangeForGrab(this);
            } else {
                Q_EMIT exitFromRangeForGrab(this);
            }
        }
    }
}	 // namespace UITool
