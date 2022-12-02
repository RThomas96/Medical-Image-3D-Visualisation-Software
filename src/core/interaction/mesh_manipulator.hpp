#ifndef MESH_MANIPULATOR_HPP_
#define MESH_MANIPULATOR_HPP_

#include <QGLViewer/manipulatedCameraFrame.h>
#include <map>
#include "src/core/drawable/drawable.hpp"
#include "manipulator.hpp"
#include "kid_manipulator.h"

//! \defgroup tools Edition tools
//! @brief 
//! All classes that manage edition tool and interaction widget.
//! Main classes are:
//! - TODO

enum class State {
    NONE,
    AT_RANGE,
    SELECTED,
    LOCK,
    MOVE,
    WAITING,
    HIGHLIGHT
};

//! \ingroup tools
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

//! \ingroup tools
class MeshManipulator : public GL::DrawableUI {
public:
    // These are needed here as there drawing functions are directly in the class
    RotationManipulator * guizmo;
    Selection selection;
    BaseMesh * mesh;

    std::string instructions;

    MeshManipulator(BaseMesh * mesh): mesh(mesh) {
        this->sphereRatio = 0.004;
        instructions = std::string("");
    }

    void zoom(float newSceneRadius) override;

    virtual void draw() = 0;
    void drawGuizmo();

    // These functions are used only in glMeshManipulator in the prepare function
    virtual void getAllPositions(std::vector<glm::vec3>& positions) = 0;
    virtual void updateWithMeshVertices() = 0;

    virtual ~MeshManipulator() {};
//public slots:
    // These are connected to the manipulators

    virtual void keyPressed(QKeyEvent* e) = 0;
    virtual void keyReleased(QKeyEvent* e) = 0;
    virtual void mousePressed(QMouseEvent*e) = 0;
    virtual void mouseReleased(QMouseEvent*e) = 0;

//signal:
    // These signals are trigerred from the scene

    //This signal is used to trigger a function in the scene
    //This should be removed when the grid will have its own "Drawable" class
    virtual void needDisplayInfos(const std::string& infos) = 0;
    virtual void meshIsModified() = 0;
};

Q_DECLARE_INTERFACE(MeshManipulator, "MeshManipulator")
//! @details The active manipulator indicates the manipulator at range for being grabbed by the mouse.
//! The commonConstraint is a custom translation constraint allowing to simplify vertex manipulation. See CustomConstraint.
//! The lockConstraint allow to prevent manipulator to move when the feature is inactive.
//! \ingroup tools
class DirectManipulator : public MeshManipulator {
    Q_OBJECT
    Q_INTERFACES(MeshManipulator)

public:
    DirectManipulator(BaseMesh * mesh);

    void updateWithMeshVertices() override;
    void getAllPositions(std::vector<glm::vec3>& positions) override;

    void checkSelectedManipulators();
    void setDefaultManipulatorColor(glm::vec3 color) { this->defaultManipulatorColor = color; };

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

    void draw() override;

signals:
    void needDisplayVertexInfo(std::pair<int, glm::vec3> selectedPoint);
    void needDisplayInfos(const std::string& infos) override;
    void meshIsModified() override;
private:
    std::vector<Manipulator> manipulators;
    std::vector<bool> manipulatorsToDisplay;
    std::vector<bool> selectedManipulators;
    bool meshHasBeenModified;
    glm::vec3 defaultManipulatorColor;
};

//! \ingroup tools
class GlobalManipulator : public MeshManipulator {
    Q_OBJECT
    Q_INTERFACES(MeshManipulator)

public:
    GlobalManipulator(BaseMesh * mesh);
    ~GlobalManipulator();

    void updateWithMeshVertices() override;

    void getAllPositions(std::vector<glm::vec3>& positions) override;

public slots:
    void toggleEvenMode();
    void moveManipulator(Manipulator * manipulator);
    void keyPressed(QKeyEvent* e) override;
    void keyReleased(QKeyEvent* e) override;
    void mousePressed(QMouseEvent* e) override;
    void mouseReleased(QMouseEvent* e) override;

    void draw() override {
        this->guizmo->draw();
    }

signals:
    void needUpdateSceneCenter();
    void needDisplayInfos(const std::string& infos) override;
    void meshIsModified() override;

private:
    bool evenMode;
    bool meshHasBeenModified;
};

//! \ingroup tools
class ARAPManipulator : public MeshManipulator {
    Q_OBJECT
    Q_INTERFACES(MeshManipulator)
    /// NEW
public:

    ARAPManipulator(BaseMesh * mesh);
    ~ARAPManipulator();

    void updateWithMeshVertices() override;
    void getAllPositions(std::vector<glm::vec3>& positions) override;

public slots:
    void keyPressed(QKeyEvent* e) override;
    void keyReleased(QKeyEvent* e) override;
    void mousePressed(QMouseEvent*e) override;
    void mouseReleased(QMouseEvent*e) override;

    void draw() override;

    void toggleEvenMode(bool value);

signals:
    void needChangeCursor(CursorType cursorType);
    void needChangeCursorInPlanarView(CursorType cursorType);
    void needDisplayInfos(const std::string& infos) override;
    void meshIsModified() override;

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
    bool meshHasBeenModified;
};

//! \ingroup tools
class SliceManipulator : public MeshManipulator {
    Q_OBJECT
    Q_INTERFACES(MeshManipulator)

public:
    SliceManipulator(BaseMesh * mesh);

    void updateWithMeshVertices() override;
    void getAllPositions(std::vector<glm::vec3>& positions) override;

public slots:
    void updateSliceToSelect(SliceOrientation sliceOrientation);
    void moveGuizmo();

    void computeManipulatorFromSelection();
    void selectSlice(SliceOrientation sliceOrientation);
    void movePlanes(const glm::vec3& planesPosition);
    std::vector<bool> getHandles();
    void setPositions(std::vector<glm::vec3>& positions);
    void rotateLastModifiedSlice(float angle);

    void keyPressed(QKeyEvent* e) override;
    void keyReleased(QKeyEvent* e) override;
    void mousePressed(QMouseEvent* e) override;
    void mouseReleased(QMouseEvent* e) override;

    void draw() override;

signals:
    void needChangePointsToProject(std::vector<int> selectedPoints);
    void needDisplayInfos(const std::string& infos) override;
    void meshIsModified() override;
private:
    std::vector<Manipulator> manipulators;

    std::vector<bool> selectedVertices;
    std::vector<bool> fixedVertices;

    SliceOrientation currentSelectedSlice;
    glm::vec3 slicesPositions;
    glm::vec3 slicesNormals[3];

    bool guizmoUpToDate;
    bool meshHasBeenModified;

    float selectionRadius;
    float selectionRange;
    float incrementRadius;
    float incrementRange;

    int lastModifiedSlice;
};

//! \ingroup tools
class MarkerManipulator : public MeshManipulator {
    Q_OBJECT
    Q_INTERFACES(MeshManipulator)

public:

    enum Step {
        SELECT_VERTICE_ON_MESH,
        PLACE_MARKER
    };

    Step step;

    MarkerManipulator(BaseMesh * mesh, Grid * grid);

    void updateWithMeshVertices() override;
    void getAllPositions(std::vector<glm::vec3>& positions) override;

public slots:

    void placeManipulator(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos);

    void keyPressed(QKeyEvent* e) override;
    void keyReleased(QKeyEvent* e) override;
    void mousePressed(QMouseEvent* e) override;
    void mouseReleased(QMouseEvent* e) override;

    void selectManipulator(Manipulator * manipulator);
    void switchToPlaceMarkerStep(int manipulatorId);
    void undoSwitchToPlaceMarkerStep();
    void switchToSelectManipulatorStep(glm::vec3 markerPlaced);
    void applyDeformation();

    void draw() override;

signals:
    void needCastRay();
    void needChangeCursor(CursorType cursorType);
    void needDisplayVertexInfo(std::pair<int, glm::vec3> selectedPoint);
    void needDisplayInfos(const std::string& infos) override;
    void meshIsModified() override;

private:
    Grid * grid;
    std::vector<Manipulator> marker_manipulators;
    std::vector<Manipulator> mesh_manipulators;

    std::vector<std::pair<int, int>> manipulator_association;
};

#endif
