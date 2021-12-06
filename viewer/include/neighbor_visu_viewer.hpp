#ifndef VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
#define VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_

#include "../../features.hpp"

#include "./scene.hpp"

#include <QGLViewer/qglviewer.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QTimer>

// #include <renderdoc_app.h>

#include <memory>

struct Manipulator {

    Manipulator(): direction_control(glm::vec3(1., 1., 1.)), assignedPoint(-1) {}

    qglviewer::ManipulatedFrame manipulatedFrame;
    glm::vec3 direction_control;

    int assignedPoint;

    glm::vec3 getPosition() {
        double x = 0;
        double y = 0;
        double z = 0;
        this->manipulatedFrame.getPosition(x, y, z);
        return glm::vec3(x, y, z);
    }

    void setPosition(glm::vec3 position) {
        this->manipulatedFrame.setPosition(position[0], position[1], position[2]);
    }
};

class CustomConstraint : public qglviewer::Constraint
{
public:

    CustomConstraint() {
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

    virtual void constrainTranslation(qglviewer::Vec& t, qglviewer::Frame * const fr)
    {
        if(std::abs(t.x) > std::abs(t.y) && std::abs(t.x) > std::abs(t.z)) {
            this->constraintx->constrainTranslation(t, fr);
        } else if(std::abs(t.y) > std::abs(t.x) && std::abs(t.y) > std::abs(t.z)) {
            this->constrainty->constrainTranslation(t, fr);
        } else if(std::abs(t.z) > std::abs(t.x) && std::abs(t.z) > std::abs(t.y)) {
            this->constraintz->constrainTranslation(t, fr);
        }
    }

    qglviewer::AxisPlaneConstraint * constraintx;
    qglviewer::AxisPlaneConstraint * constrainty;
    qglviewer::AxisPlaneConstraint * constraintz;
};

struct ManipulatorManager {

    ManipulatorManager(int nbManipulators):nbManipulators(nbManipulators), manipulators(std::vector<Manipulator>(nbManipulators)) {
        activeManipulator = &this->manipulators[0];

        this->constraintx = new CustomConstraint();
        //this->constraintx = new qglviewer::LocalConstraint();
        //this->constraintx->setTranslationConstraintType(qglviewer::AxisPlaneConstraint::AXIS);
        //this->constraintx->setTranslationConstraintDirection(qglviewer::Vec(1., 0., 0.));

        //this->constrainty = new qglviewer::LocalConstraint();
        //this->constrainty->setTranslationConstraintType(qglviewer::AxisPlaneConstraint::AXIS);
        //this->constrainty->setTranslationConstraintDirection(qglviewer::Vec(0., 1., 0.));

        //this->constraintz = new qglviewer::LocalConstraint();
        //this->constraintz->setTranslationConstraintType(qglviewer::AxisPlaneConstraint::AXIS);
        //this->constraintz->setTranslationConstraintDirection(qglviewer::Vec(0., 0., 1.));

        for(int i = 0; i < nbManipulators; ++i) {
            this->manipulators[i].assignedPoint = i;
            this->applyConstraint(i);
        }

    }

    //void setManipulatedPos(int idx, glm::vec3 position) { manipulators[idx].position = position * manipulators[idx].direction_control; }
    //glm::vec3 getManipulatedPos(int idx) { return manipulators[idx].getPosition();}

    Manipulator& getManipulor(int idx) { return manipulators[idx];}

    int getAssignedPoint(int idx) { return manipulators[idx].assignedPoint; }

    bool hasConstrainedDirection(int idx) { return (manipulators[idx].direction_control != glm::vec3(0., 0., 0.)); }

    bool isActiveManipulatorManipuled() { return this->activeManipulator->manipulatedFrame.isManipulated(); }
    glm::vec3 getActiveManipulatedPos() { return this->activeManipulator->getPosition();}

    void applyConstraint(int idx) {
        this->manipulators[idx].manipulatedFrame.setConstraint(this->constraintx);
        //this->manipulators[idx].manipulatedFrame.setConstraint(this->constrainty);
        //this->manipulators[idx].manipulatedFrame.setConstraint(this->constraintz);
    }

    void print() {
        glm::vec3 pos = this->activeManipulator->getPosition();
        std::cerr << "Frame pos: " << pos.x  << " - " << pos.y << " - " << pos.z << " - " << std::endl;
    }

    int getActiveFrame() {
        for(int i = 0; i < this->manipulators.size(); ++i) {
            if(this->manipulators[i].manipulatedFrame.grabsMouse())
                return i;
        }
        return -1;
    }

    void getPositionVector(std::vector<glm::vec3>& vec) {
        for(int i = 0; i < this->manipulators.size(); ++i) {
            vec.push_back(this->manipulators[i].getPosition());
        }
    }

    void populatePositions(std::vector<glm::vec3>& positions) {
        if(positions.size() > this->manipulators.size()) {
            for(int i = 0; i < this->manipulators.size(); ++i) {
                int assignedPoint = this->manipulators[i].assignedPoint;
                this->manipulators[i].setPosition(positions[assignedPoint]);
            }
        } else {
            std::cerr << "WARNING: try to update [" << this->manipulators.size() << "] manipulators positions with a position vector of size [" << positions.size() << "]" << std::endl;
        }
    }

    int nbManipulators;
    std::vector<Manipulator> manipulators;
    Manipulator* activeManipulator;

    // Common constraint
    qglviewer::Constraint * constraintx;
    //qglviewer::AxisPlaneConstraint * constraintx;
    //qglviewer::AxisPlaneConstraint * constrainty;
    //qglviewer::AxisPlaneConstraint * constraintz;
};


/// @ingroup graphpipe
/// @brief A viewer that displays a scene, either in real space or in initial space
class Viewer : public QGLViewer {
	Q_OBJECT
public:
	/// @brief Default constructor for the viewer.
	Viewer(Scene* const scene, QStatusBar* program_bar, QWidget* parent = nullptr);
	~Viewer();
	/// @brief Multiplier to apply to scene radii for the scene's view.
	static float sceneRadiusMultiplier;

	/// @brief Updates info from the scene, binding its context for rendering.
	void updateInfoFromScene();

protected:
	/// @brief Initializes the scene, and the viewer's variables.
	virtual void init() override;
	/// @brief Draws the scene, in the space the viewer is supposed to show.
	virtual void draw() override;
	/// @brief Handles key events from the user.
	virtual void keyPressEvent(QKeyEvent* e) override;
	/// @brief Mouse press event generated by the widget.
	virtual void mousePressEvent(QMouseEvent* e) override;
	/// @brief Mouse press event generated by the widget.
	virtual void mouseMoveEvent(QMouseEvent* e) override;
	/// @brief Mouse press event generated by the widget.
	virtual void mouseReleaseEvent(QMouseEvent* e) override;
	/// @brief Wheel event for the mouse.
	virtual void wheelEvent(QWheelEvent* _w) override;
	/// @brief Defines the 'Help'/'About' string defined for this viewer.
	virtual QString helpString(void) const override;
	/// @brief Defines the 'Help'/'About' string for the keyboard for this viewer.
	virtual QString keyboardString(void) const override;
	/// @brief Defines the 'Help'/'About' string for the mouse for this viewer.
	virtual QString mouseString(void) const override;
	/// @brief Overrides the function to resize the widget.
	virtual void resizeGL(int w, int h) override;
	/// @brief Resets and removes the local point query
	void resetLocalPointQuery();

private:
	/// @brief The scene to control.
	Scene* const scene;
	/// @brief Should we draw it in volumetric mode ?
	bool drawVolumetric;
	/// @brief A refresh timer for the viewer, to update in real time.
	QTimer* refreshTimer;
	/// @brief Should we capture a frame ?
	bool shouldCapture;
	/// @brief Is the Ctrl key pressed down ?
	bool keyboard_CtrlDown;
	/// @brief The texture attached to the secondary framebuffer output.
	GLuint renderTarget;
	/// @brief Are we in "select mode" ? (for selecting coordinates in rasterized/volumetric view)
	bool selectMode;
	/// @brief Framebuffer dimensions, updated once per resize event
	glm::ivec2 fbSize;
	/// @brief Current cursor position relative to the widget's geometry origin
	glm::ivec2 cursorPos_current;
	/// @brief Last known cursor position relative to the widget's geometry origin
	glm::ivec2 cursorPos_last;
	/// @brief The number of frames a modifier (RMB,LMB,Ctrl ...) has been held for.
	std::size_t framesHeld;
	/// @brief Request to read a fragment"s position
	glm::ivec2 posRequest;
	/// @brief Program's status bar
	QStatusBar* statusBar;
	bool drawAxisOnTop;

    ManipulatorManager manipulatorManager;

public slots:
	/// @brief Update the view, as a slot without any arguments (currently only used by QTimer)
	void updateView() { this->update(); }
	/// @brief Updates the camera position once one or two grids are loaded in the scene.
	void updateCameraPosition(void);
	/// @brief Asks the scene to load a grid into itself.
	// void loadGrid(const std::shared_ptr<InputGrid>& g);
	/// @brief Asks the scene to load two grids into itself.
	// void loadTwoGrids(const std::shared_ptr<InputGrid>& g1, const std::shared_ptr<InputGrid>& g2);
	void newAPI_loadGrid(Image::Grid::Ptr ptr);
	/// @brief Re-centers the camera around the scene-defined center point
	void centerScene(void);
	void guessMousePosition(void);

    void updateManipulatorsPositions(void);
};

#endif	  // VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
