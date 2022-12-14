#ifndef VIEWER_INCLUDE_SCENE_HPP_
#define VIEWER_INCLUDE_SCENE_HPP_

#include "QGLViewer/camera.h"
#include "../core/drawable/drawable_grid.hpp"
#include "scene_control.hpp"
#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>

// Helper structs and functions :
#include "legacy/viewer_structs.hpp"
// Qt headers :
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLFunctions_4_0_Core>
#include <QProgressBar>
#include <QStatusBar>
// libQGLViewer :
#include <QGLViewer/qglviewer.h>
// glm include :
#include <cstdint>
#include <glm/glm.hpp>
// STD headers :
#include <mutex>
#include <vector>

#include <thread>
#include "../legacy/image/utils/include/threaded_task.hpp"

// Tinytiff
#include <tinytiffreader.h>
#include <tinytiffwriter.h>

#include "../core/geometry/grid.hpp"
#include "../core/geometry/graph_mesh.hpp"
#include "../core/drawable/drawable_surface_mesh.hpp"
#include "../core/drawable/drawable_selection.hpp"
#include "../core/geometry/graph_mesh.hpp"
#include "../core/deformation/cage_surface_mesh.hpp"

#include "glm/gtx/string_cast.hpp"

#include <omp.h>

// Forward declaration
class ControlPanel;
namespace GL {
    class MeshManipulator;
}

class MeshManipulator;

// TODO: do not belong here
enum class CursorType {
    NORMAL,
    CROSS,
    OPEN_HAND,
    CLOSE_HAND,
    HOURGLASS,
    FAIL
};

enum class MeshManipulatorType {
    NONE,
    DIRECT,
    POSITION,
    ARAP,
    SLICE,
    MARKER
};

enum class SliceOrientation {
    X,
    Y,
    Z
};

enum ColorChannel {
    None = 0,
    RedOnly			= 1,
    GreenOnly		= 2,
    RedAndGreen		= 3,
    HandEColouring	= 4
};

enum ColorFunction {
    SingleChannel,
    HistologyHandE,
    HSV2RGB,
    ColorMagnitude
};

/**********************************************************************/
/**********************************************************************/

/**********************************************************************/
/**********************************************************************/

class Scene : public QObject, public QOpenGLFunctions_3_2_Core {
    Q_OBJECT

public:
    Scene();
    ~Scene(void);

    int h;
    int w;

private:
    QOpenGLContext* context;

    GLuint default_vao;
    GLuint default_vbo_Vertice;
    GLuint default_vbo_Normal;
    GLuint default_vbo_Id;
    GLuint frameBuffer;
    GLuint dualPass_program;

    std::unique_ptr<ShaderCompiler> shaderCompiler;

public:
    /* Open GL Utilities */
    void initGl(QOpenGLContext* context);

    GLuint uploadTexture1D(const TextureUpload& tex);
    GLuint uploadTexture2D(const TextureUpload& tex);
    GLuint uploadTexture3D(const TextureUpload& tex);
    GLuint newAPI_uploadTexture3D(const GLuint handle, const TextureUpload& tex, std::size_t s, std::vector<std::uint16_t>& data);
    GLuint newAPI_uploadTexture3D_allocateonly(const TextureUpload& tex);

    void recompileShaders(bool verbose = true);

    GLuint createUniformBuffer(std::size_t size_bytes, GLenum draw_mode);
    void setUniformBufferData(GLuint uniform_buffer, std::size_t begin_bytes, std::size_t size_bytes, GLvoid* data);
    void setRenderSize(int h, int w);

    GLuint updateFBOOutputs(glm::ivec2 dimensions, GLuint fb_handle, GLuint old_texture = 0);
    GLuint compileShaders(std::string vPath, std::string gPath, std::string fPath, bool verbose = false);
    /*************************************************/

    /* Draws */
    void drawScene(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, float near, float far);
    void activateCuttingPlaneLegacyOpenGL();
    void deactivateCuttingPlaneLegacyOpenGL();
    void createBuffers();// Only for selection TODO: remove

    /* Others */
    void updateUserColorScale();
    void updateMinMaxDisplayValues();

    glm::vec3 computePlanePositions();
    glm::vec3 computePlanePositionsWithActivation();
    /*************************************************/

private:
    /* Scene boolean */
    bool shouldUpdateUserColorScales;
    bool needUpdateMinMaxDisplayValues;

    /* Containers */
private:

    /* Color channel management */
    ColorChannel rgbMode;
    ColorFunction channels_r;
    GLuint selectedChannel_r;	 ///< The currently selected channel for greyscale mode.
    ColorFunction channels_g;
    GLuint selectedChannel_g;	 ///< The currently selected channel for greyscale mode.
public:
    /* Widgets */
    ControlPanel* controlPanel;
    QStatusBar* programStatusBar;

    /* Functions */
public:
    /* Widget interaction */
    void addStatusBar(QStatusBar* _s);
    void setControlPanel(ControlPanel* cp) { this->controlPanel = cp; }

    void addGrid();

    double getMinNumericLimit(size_t gridIndex) const { return Image::getMinNumericLimit(grids[gridIndex]->getInternalDataType()); }
    double getMaxNumericLimit(size_t gridIndex) const { return Image::getMaxNumericLimit(grids[gridIndex]->getInternalDataType()); }

    enum ValueType { MIN, MAX };
    // Set the min and max values to display on the grid
    void setDisplayRange(int gridIdx, ValueType type, double value);
    double getDisplayRange(int gridIdx, ValueType type);

    // Set the min and max values for the display range (to change the color map)
    void setMinMaxDisplayRange(int gridIdx, ValueType type, double value);
    double getMinMaxDisplayRange(int gridIdx, ValueType type);

    // Set colors for min and max values
    void setUserColorScale(int gridIdx, ValueType type, glm::vec3 color);// color of the beginning of the color segment for the segmented color scale

    void setColorFunction_r(ColorFunction _c);// Changes the texture coloration mode to the desired setting
    void setColorFunction_g(ColorFunction _c);

    /* Toggle things */
    void toggleAllPlaneVisibilities(void);

    template<typename MeshToolType>
    MeshToolType * getMeshTool();

public:
    void writeDeformedImage(const std::string& filename, const std::string& gridName, bool useColorMap);
    void writeDeformedImage(const std::string& filename, const std::string& gridName, bool useColorMap, const glm::vec3& voxelSize);
    void writeDeformedImage(const std::string& filename, const std::string& gridName, const glm::vec3& bbMin, const glm::vec3& bbMax, bool useColorMap, const glm::vec3& voxelSize);
    void writeDeformedImageGeneric(const std::string& filename, const std::string& gridName, const glm::vec3& bbMin, const glm::vec3& bbMax, Image::ImageDataType imgDataType, bool useColorMap, const glm::vec3& voxelSize);
    template<typename DataType>

    //! @brief Write a deformed image into a TIFF image file.
    //! This function uses the TinyTIFF library, and TIFF is the only format supported.
    //! The TinyTIFF library has been choosen after several failed attempts to implement a writer using the tedious C library libtiff.
    void writeDeformedImageTemplated(const std::string& filename, const std::string& gridName, const glm::vec3& bbMin, const glm::vec3& bbMax, int bit, Image::ImageDataType dataType, bool useColorMap, const glm::vec3& imageVoxelSize);

    //! @brief Write an image into a TIFF file.
    //! This function is currently unused but still usefull for further developement.
    void writeGreyscaleTIFFImage(const std::string& filename, const glm::vec3& imgDimensions, const std::vector<std::vector<uint16_t>>& data);


signals:
    // Signals to the meshManipulator tools
    void keyPressed(QKeyEvent* e);
    void keyReleased(QKeyEvent* e);
    void mousePressed(QMouseEvent* e);
    void mouseReleased(QMouseEvent* e);
    void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction);
    void needCastRay();
    void pointIsClickedInPlanarViewer(const glm::vec3& position);
    // Signals to the viewer
    void sceneCenterChanged(const glm::vec3& center);
    void sceneRadiusChanged(const float radius);
    void meshAdded(const std::string& name, bool grid, bool cage);
    void planesMoved(const glm::vec3& planesPosition);
    void needPushHandleButton();
    void cursorChanged(CursorType cursorType);
    void cursorChangedInPlanarView(CursorType cursorType);
    void selectedPointChanged(std::pair<int, glm::vec3> selectedPoint);
    void meshMoved();
    void activeMeshChanged();
    void colorChanged();
    void planeControlWidgetNeedUpdate(const glm::vec3& values);
    void planeControlWidgetNeedUpdateImageSize(const glm::vec3& values);
    void sceneRadiusOutOfDate();
    void needDisplayInfos(const std::string& infos);
    void needChangeCameraType(qglviewer::Camera::Type cameraType);

// All these indirections are important because for most of them they interacts with various components of the scene
// And it allow more flexibility as the scene control ALL the informations to transit from class to class
public slots:
    void init();

    void changeCurrentTool(MeshManipulatorType newTool);
    void changeSelectedPoint(std::pair<int, glm::vec3> selectedPoint);

public :
    glm::vec3 planeDirection;// Cutting plane directions (-1 or 1 on each axis)
    glm::vec3 planeDisplacement;
    glm::vec3 planeActivation;// If planes are used (0 = false, 1 = true)
    enum CuttingPlaneDirection {X, Y, Z, XYZ};

public slots:
    void slotSetNormalizedPlaneDisplacement(CuttingPlaneDirection direction, float scalar);
    void slotSetPlaneDisplacement(std::string gridName, CuttingPlaneDirection direction, float scalar);
    void slotTogglePlaneDirection(CuttingPlaneDirection direction);
    void slotToggleDisplayPlane(CuttingPlaneDirection direction, bool display);

    //void updatePlaneControlWidget() {
    //    Q_EMIT planeControlWidgetNeedUpdate(this->planeDisplacement);
    //}

    // *************** //
    // Connected to UI //
    // *************** //

    // Tool management
    void updateTools(MeshManipulatorType tool);

    // Mesh management
    void moveInHistory(bool backward = true, bool reset = false);
    void undo();
    void redo();
    void reset();
    glm::vec3 getTransformedPoint(const glm::vec3& inputPoint, const std::string& from, const std::string& to);
    void getValues(const std::string &gridName, const glm::vec3 &slice, const std::pair<glm::vec3, glm::vec3> &area, const glm::vec3 &resolution, std::vector<uint16_t> &data, Interpolation::Method interpolationMethod);
    void clear();

    // Move tool
    void moveTool_toggleEvenMode();
    void toggleBindMeshToCageMove();
    void toggleBindMeshToCageMove(const std::string& name);

    // ARAP
    void ARAPTool_toggleEvenMode(bool value);

    // Slice
    void computeProjection(const std::vector<int>& vertexIndices);

    // Display management
    void slotToggleDisplayGrid() { this->displayGrid = !this->displayGrid;}
    void toggleDisplayMesh() { this->displayMesh = !this->displayMesh;}
    void toggleDisplayTetmesh(bool value);
    void setGridsToDraw(std::vector<int> indices);
    void setMultiGridRendering(bool value);
    void setDrawSliceOnly(bool value);
    void setBlend(float value);
    void setOrthographicCamera() { Q_EMIT needChangeCameraType(qglviewer::Camera::ORTHOGRAPHIC); }
    void setPerspectiveCamera() { Q_EMIT needChangeCameraType(qglviewer::Camera::PERSPECTIVE); }
    void setDipslayTetSizeUnit(bool value) {
        this->displayTetSizeUnit = value;
        for(auto& grid : grids)
            grid->drawSizeTetUnit = value;
    };

    // Segmented display
    void resetRanges();
    void addRange(uint16_t min, uint16_t max, glm::vec3 color = glm::vec3(1., 0., 0.), bool visible = true, bool updateUBO = true);
    void getRanges(std::vector<std::pair<uint16_t, uint16_t>>& ranges);
    void getRangesColor(std::vector<glm::vec3>& colors);
    void getRangesVisu(std::vector<bool>& visu);

    // ************************ //

    // MeshManipulator slots
    void changeActiveMesh(const std::string& name);

    void selectSlice(SliceOrientation sliceOrientation);
    void changeSliceToSelect(SliceOrientation sliceOrientation);

    void setBindMeshToCageMove(const std::string& name, bool state);

    bool isRightTool(const MeshManipulatorType& typeToCheck);

    // Rendering slots
    void setColorChannel(ColorChannel mode);
    void updateTetmeshAllGrids(bool updateAllInfos = false);
    std::pair<uint16_t, uint16_t> sendGridValuesToGPU(int gridIdx);
    void setLightPosition(const glm::vec3& lighPosition);

    // Scene management
    void openAtlas();
    void openIRM();
    bool openMesh(const std::string& name, const std::string& filename, const glm::vec4& color = glm::vec4(0.1, 0.5, 1.,0.85));
    bool openGraph(const std::string& name, const std::string& filename, const glm::vec4& color = glm::vec4(0.1, 0.5, 1.,0.85));
    bool openCage(const std::string& name, const std::string& filename, BaseMesh * surfaceMeshToDeform, const bool MVC = true, const glm::vec4& color = glm::vec4(1., 0., 0., 0.1));
    bool openCage(const std::string& name, const std::string& filename, const std::string& surfaceMeshToDeformName, const bool MVC = true, const glm::vec4& color = glm::vec4(1., 0., 0., 0.1));
    bool linkCage(const std::string& cageName, BaseMesh * meshToDeform, const bool MVC);

    bool openGrid(const std::string& name, const std::vector<std::string>& imgFilenames, const int subsample, const glm::vec3& sizeVoxel, const glm::vec3& nbCubeGridTransferMesh = glm::vec3(5., 5., 5.));
    bool openGrid(const std::string& name, const std::vector<std::string>& imgFilenames, const int subsample, const glm::vec3& sizeVoxel, const std::string& transferMeshFileName);
    void addGridToScene(const std::string& name, Grid * newGrid);
    int autofitSubsample(int initialSubsample, const std::vector<std::string>& imgFilenames);
    SurfaceMesh * getMesh(const std::string& name);
    BaseMesh * getBaseMesh(const std::string& name);
    int getMeshIdx(const std::string& name);
    Cage * getCage(const std::string& name);
    glm::vec3 getGridImgSize(const std::string& name);
    glm::vec3 getGridVoxelSize(const std::string &name);
    std::pair<uint16_t, uint16_t> getGridMinMaxValues(const std::string &name);
    std::pair<uint16_t, uint16_t> getGridMinMaxValues();
    std::vector<bool> getGridUsageValues(int minValue = 1);
    int getGridIdx(const std::string& name);
    int getGridIdxLinkToCage(const std::string& name);
    std::pair<glm::vec3, glm::vec3> getBbox(const std::string& name);
    void updateTextureCoordinates(const std::string& name) {
        return this->grids[this->getGridIdx(name)]->updateTextureCoordinates();
    }

    std::vector<std::string> getAllNonTetrahedralMeshesName();
    std::vector<std::string> getAllBaseMeshesName();
    std::vector<std::string> getAllCagesName();
    std::vector<std::string> getAllGridsName();

    bool isGrid(const std::string& name);
    bool isCage(const std::string& name);
    bool hasTwoOrMoreGrids();
    void changeSceneRadius(float sceneRadius);
    float getSceneRadius();
    std::pair<glm::vec3, glm::vec3> getSceneBBox();
    void updateSceneCenter();
    void updateSceneRadius();
    void updateManipulatorRadius();
    glm::vec3 getSceneCenter();
    bool saveMesh(const std::string& name, const std::string& filename);
    bool saveActiveCage(const std::string& filename);
    bool saveActiveMesh(const std::string& filename);
    void applyCage(const std::string& name, const std::string& filename);
    bool isSelecting() {return false;};
    // This is connect directly to selection in meshManipulator
    void redrawSelection(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec4& color = glm::vec4(1., 0., 0., 0.5));
    void changeCursor(CursorType cursorType) { Q_EMIT cursorChanged(cursorType); };
    void changeCursorInPlanarView(CursorType cursorType) { Q_EMIT cursorChangedInPlanarView(cursorType); };

    //void addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, bool onSurface);
public:

    MeshManipulator* meshManipulator;
    GL::Selection * glSelection;

    int maximumTextureSize;// Set by the viewer
    int activeGrid = -1;
    std::vector<int> gridsToDraw;

    glm::vec3 cameraPosition;
    float distanceFromCamera;
    float alphaBlend;

    bool funnyRender;

    bool registrationRendering;
    bool displayGrid;
    bool displayMesh;
    bool displayGridBBox;
    bool displayXRayManipulators;
    bool displayTetSizeUnit;
    bool previewCursorInPlanarView;

    std::string activeMesh;
    MeshManipulatorType currentTool;

    std::vector<std::pair<std::string, std::string>> cageToGrid;

    std::vector<std::string> grids_name;

    std::vector<Grid*> grids;
    std::vector<std::pair<GraphMesh*, std::string>> graph_meshes;
    std::vector<std::pair<SurfaceMesh*, std::string>> meshes;

    // Usefull to draw boxes in the scene
    // Used to preview the zone to save in the export image widget
    using Box = std::pair<glm::vec3, glm::vec3>;
    std::vector<Box> boxes;
    void drawBox(const Box& box);
    void addBox(const Box& box);
    void clearBoxes();
};

#endif	  // VIEWER_INCLUDE_SCENE_HPP_
