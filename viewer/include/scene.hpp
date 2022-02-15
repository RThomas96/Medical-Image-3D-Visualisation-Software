#ifndef VIEWER_INCLUDE_SCENE_HPP_
#define VIEWER_INCLUDE_SCENE_HPP_

// Program-wide features and macros
#include "../../features.hpp"
#include "../../macros.hpp"
// Scene control panel :
#include "../../qt/include/scene_control.hpp"
// Shader compiler :
#include "../../meshes/drawable/shaders.hpp"
// Meshes :
#include "../../meshes/base_mesh/Mesh.hpp"
#include "../../meshes/base_mesh/mesh_io.hpp"
#include "../../meshes/drawable/curve.hpp"
#include "../../meshes/drawable/mesh.hpp"
// Curve :
#include "../../meshes/deformable_curve/curve.hpp"
// UI elements :
//#include "../../qt/include/grid_control.hpp"
#include "../../qt/include/opengl_debug_log.hpp"
#include "../../qt/include/visu_box_controller.hpp"
//#include "../../grid/include/manipulator.hpp"
//#include "../../grid/include/mesh_manipulator.hpp"
// Helper structs and functions :
#include "./viewer_structs.hpp"
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
#include <glm/glm.hpp>
// STD headers :
#include <mutex>
#include <vector>

#include <thread>
#include "../../image/utils/include/threaded_task.hpp"

// Tinytiff
#include <tinytiffreader.h>
#include <tinytiffwriter.h>

#include "../../grid/include/grid.hpp"
#include "../../grid/include/drawable_surface_mesh.hpp"

#include "glm/gtx/string_cast.hpp"

/// @defgroup graphpipe Graphics pipeline
/// @brief This group contains all classes closely or loosely related to the graphics pipeline.
/// @details There are very few classes in this group, but that's only because Scene is a god-object. Some attempt was
/// made to move much of the code for displaying a grid over to the GridViewer class, altough that has not been tested.
/// @warning Spaghetti code ahead.

class ICP;

// Forward declaration
class ControlPanel;
namespace UITool {
	namespace GL {
		class MeshManipulator;
	}
}

namespace UITool {
		class MeshManipulator;
}

enum DrawMode { 
    Solid,
	Volumetric,
	VolumetricBoxed 
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

enum planes { 
    x = 1,
	y			= 2,
	z			= 3 };

enum planeHeading { 
    North = 0,
	East				  = 1,
	South				  = 2,
	West				  = 3,
	Up					  = North,
	Right				  = East,
	Down				  = South,
	Left				  = West };

// Choose which data of the tetmesh to send to the GPU
enum InfoToSend {
    VERTICES  = 0b00000001,
    NORMALS   = 0b00000010,
    TEXCOORD  = 0b00000100,
    NEIGHBORS = 0b00001000
};

/**********************************************************************/
/**********************************************************************/

class SceneGL : public QOpenGLFunctions_3_2_Core {
public:
	SceneGL();
	~SceneGL(void);

	void initGl(QOpenGLContext* context);

	GLuint uploadTexture1D(const TextureUpload& tex);
	GLuint uploadTexture2D(const TextureUpload& tex);

	QOpenGLContext* get_context() const { return this->context; }

	bool isSceneInitialized(void) const { return this->isInitialized; }

private:
	bool isInitialized;
	QOpenGLContext* context;
};

/**********************************************************************/
/**********************************************************************/

/// @ingroup graphpipe
/// @brief The Scene class is the gateway to the OpenGL functions attached to the GL context of the program.
/// @note As you might see, this kind of turned into a god-object. Although dismantling it is not that hard !
/// @details This class evolved from a simple scene representation at the start to a nearly all-encompassing OpenGL
/// gateway for any and all operations. It should be <b><i>heavily</i></b> refactored.
/// @warning Spaghetti code ahead.
class Scene : public QObject, public QOpenGLFunctions_3_2_Core {
    Q_OBJECT
	/// @brief typedef to omit glm:: from a 'uvec3'
	typedef glm::uvec3 uvec3;

public:
	Scene();	///< default constructor
	~Scene(void);	 ///< default destructor

    /**********/
    /* OpenGL */
    /**********/

    /* Attributes */
private:
	QOpenGLContext* context;
	OpenGLDebugLog* glOutput;
	QOpenGLDebugLogger* debugLog;

	/* Program */
	GLuint program_projectedTex;
	GLuint program_Plane3D;
	GLuint program_PlaneViewer;
	GLuint program_VolumetricViewer;
	GLuint program_BoundingBox;
	GLuint program_sphere;
    /*************************************************/
 
	/* VAO */
	GLuint vao;
	GLuint vao_VolumetricBuffers;
	GLuint vao_boundingBox;
	GLuint vao_spheres;
    /*************************************************/

	/* VBO */
	GLuint vbo_VertPos;
	GLuint vbo_VertNorm;
	GLuint vbo_VertTex;
	GLuint vbo_Element;	 ///< The vertex indices necessary to draw the tetrahedral mesh.
	GLuint vbo_PlaneElement;
	GLuint vbo_SinglePlaneElement;
	GLuint vbo_boundingBoxVertices;
	GLuint vbo_boundingBoxIndices;

	GLuint vbo_spherePositions;
	GLuint vbo_sphereNormals;
	GLuint vbo_sphereIndices;
	GLuint vbo_Texture3D_VertPos;
	GLuint vbo_Texture3D_VertNorm;
	GLuint vbo_Texture3D_VertTex;
	GLuint vbo_Texture3D_VertIdx;
    /*************************************************/


    /* Texture */
	GLuint tex_ColorScaleGrid;
	GLuint tex_ColorScaleGridAlternate;
	GLuint tex_colorScale_greyscale;
	GLuint tex_colorScale_hsv2rgb;
	GLuint tex_colorScale_user0;
	GLuint tex_colorScale_user1;
    /*************************************************/

	GLuint sphere_size_to_draw;
	std::unique_ptr<ShaderCompiler> shaderCompiler;
	void printAllUniforms(GLuint _shader_program);
	GLint gl_limit_max_texture_size;// Keeps track of the limits of the GL

    /* Functions */
public:
    /* Open GL Utilities */
	void initGl(QOpenGLContext* context);
	QOpenGLContext* get_context() const { return this->context; }

	GLuint uploadTexture1D(const TextureUpload& tex);
	GLuint uploadTexture2D(const TextureUpload& tex);
	GLuint uploadTexture3D(const TextureUpload& tex);
	GLuint newAPI_uploadTexture3D(const GLuint handle, const TextureUpload& tex, std::size_t s, std::vector<std::uint16_t>& data);
	GLuint newAPI_uploadTexture3D_allocateonly(const TextureUpload& tex);

	void recompileShaders(bool verbose = true);

	GLuint createUniformBuffer(std::size_t size_bytes, GLenum draw_mode);
	void setUniformBufferData(GLuint uniform_buffer, std::size_t begin_bytes, std::size_t size_bytes, GLvoid* data);

	GLuint updateFBOOutputs(glm::ivec2 dimensions, GLuint fb_handle, GLuint old_texture = 0);
	glm::vec4 readFramebufferContents(GLuint fb_handle, glm::ivec2 image_coordinates);
	GLuint compileShaders(std::string vPath, std::string gPath, std::string fPath, bool verbose = false);

	void newSHADERS_print_all_uniforms(GLuint program);
    /*************************************************/

    /* Draws */
	void draw3DView(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, bool showTexOnPlane);

	void drawGridVolumetricView(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const GridGLView::Ptr& grid);
	void drawGridPlaneView(GLfloat mvMat[], GLfloat pMat[], glm::mat4 baseMatrix, const GridGLView::Ptr& grid);
	void drawGridMonoPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading, float zoomRatio, glm::vec2 offset);
	void drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane = true);
	void drawBoundingBox(const Image::bbox_t& _box, glm::vec3 color, GLfloat* vMat, GLfloat* pMat);
    /*************************************************/

    /* Uniform preparation */
	void prepareUniformsGridVolumetricView(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, const GridGLView::Ptr& _grid);
	void prepareUniformsGridPlaneView(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, const GridGLView::Ptr& grid);// preps uniforms for a grid
	void prepareUniformsMonoPlaneView(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, glm::vec2 offset, const GridGLView::Ptr& _grid);// prep the plane uniforms to draw in space
	void prepareUniformsPlanes(GLfloat* mvMat, GLfloat* pMat, planes _plane, const GridGLView::Ptr& grid, bool showTexOnPlane = true);// preps uniforms for a given plane
    /*************************************************/

    /* VAO/VBO management */
	void createBuffers();
	void setupVBOData(const SimpleVolMesh& _mesh);// Generates the vertices, normals, and tex coordinates for a basic unit cube
	void setupVAOPointers();
	void createBoundingBoxBuffers();// Creates the VBO/VAO handles for bounding boxes
	void setupVAOBoundingBox();// Orders the VAO pointers for the bounding box
	void tex3D_bindVAO();// Bind the VAO created for the volumetric drawing method.
	void tex3D_buildBuffers(VolMesh& volMesh);// Build buffers to draw a single voxel (a cube)
    /*************************************************/

    /* Generate things */
	void generateSceneData();// Generate the unit cube used to draw the grid in non-volumetric mode, as well as the plane positions and bounding box buffers.
	void generateTexCube(SimpleVolMesh& _mesh);
	void generatePlanesArray(SimpleVolMesh& _mesh);
	void generateSphereData();
	void newSHADERS_generateColorScales(void);
	void generateColorScales();
    /*************************************************/

    /* Others */
	void newSHADERS_updateUserColorScales();
	void signal_updateUserColorScales();
	void newSHADERS_updateUBOData();
    // TODO: c koi ?
	uint colorFunctionToUniform(ColorFunction _c);// Returns an unsigned int (suitable for uniforms) from a color function
	void updateCVR();// Update colorChannelAttributes
	void updateBoundingBox(void);
	void updateVisuBoxCoordinates(void);
	glm::vec3 computePlanePositions();
    /*************************************************/

    /***********************************************/
    /* Computation                                 */
    /***********************************************/
private:
    /* Scene boolean */
	bool isInitialized;
	bool showVAOstate;
	bool shouldDeleteGrid;
	bool shouldUpdateUserColorScales;
	bool shouldUpdateUBOData;

    /* Containers */
	UITool::GL::MeshManipulator* glMeshManipulator;
public:
	std::vector<GridGLView::Ptr> grids;
	//std::vector<DeformableGrid*> grids;
private:
    // TODO: remove this

	std::vector<std::size_t> delGrid;	 ///< Grids to delete at next refresh
	qglviewer::Frame* posFrame;
	glm::vec4 posRequest;
	VolMesh volumetricMesh;

    /* Visualisation */
	std::array<glm::vec3, 8> lightPositions;	///< Scene lights (positionned at the corners of the scene BB)
	glm::bvec3 planeVisibility;
	glm::vec3 planeDirection;	 ///< Cutting plane directions (-1 or 1 on each axis)
	glm::vec3 planeDisplacement;
	Image::bbox_t sceneBB;
	Image::bbox_t sceneDataBB;
	float clipDistanceFromCamera;
	glm::uvec3 visuMin;
	glm::uvec3 visuMax;
	Image::bbox_t visuBox;	  ///< Used to restrict the view to a box with its coordinates
	DrawMode drawMode;


    /* Color channel management */
	ColorChannel rgbMode;
	GridGLView::data_2 textureBounds0;
	GridGLView::data_2 textureBounds1;
	GridGLView::data_2 colorBounds0;
	GridGLView::data_2 colorBounds1;
	ColorFunction channels_r;
	GLuint selectedChannel_r;	 ///< The currently selected channel for greyscale mode.
	ColorFunction channels_g;
	GLuint selectedChannel_g;	 ///< The currently selected channel for greyscale mode.
	glm::vec3 color0;	 ///< The color segment when approaching 0
	glm::vec3 color1;	 ///< The color segment when approaching 1
	glm::vec3 color0_second;	///< The color segment when approaching 0
	glm::vec3 color1_second;	///< The color segment when approaching 1

    /* Widgets */
	//GridControl* gridControl;	 ///< The controller for the grid 'save' feature (generation)
	ControlPanel* controlPanel;
	QStatusBar* programStatusBar;
	VisuBoxController* visuBoxController;

	std::size_t renderSize;	   ///< Number of primitives to render for the solid view mode.

	// Thread management 
	std::mutex mutexout;
	std::vector<Image::ThreadedTask::Ptr> tasks;
	std::vector<std::shared_ptr<std::thread>> runningThreads;
	QTimer* timer_refreshProgress;
	QProgressBar* pb_loadProgress;
	bool isFinishedLoading;

    /* Functions */
public:
    /* Widget interaction */

    void slotAddManipulator(const glm::vec3& position);
    //bool slotGetPositionFromRay(const glm::vec3& origin, const glm::vec3& direction, glm::vec3& res);

	void showVisuBoxController(VisuBoxController* _controller);
	void removeVisuBoxController();

	void prepareManipulators();

	void addOpenGLOutput(OpenGLDebugLog* _gldl);
	void addStatusBar(QStatusBar* _s);

	void setControlPanel(ControlPanel* cp) { this->controlPanel = cp; }
	void removeController();

	void updateProgressBar();

	void loadGridROI(void); // DEPRECATED
	void addGrid(const GridGL * gridLoaded);
	
	void launchSaveDialog();
	void printVAOStateNext() { this->showVAOstate = true; }

    /* Getters & setters */
	glm::vec3 getSceneBoundaries() const;

	double getMinNumericLimit(size_t gridIndex) const { return Image::getMinNumericLimit(grids[gridIndex]->grid->getInternalDataType()); }
	double getMaxNumericLimit(size_t gridIndex) const { return Image::getMaxNumericLimit(grids[gridIndex]->grid->getInternalDataType()); }

	double getMinTexValue(void) const { return static_cast<double>(this->textureBounds0.x); }
	double getMinTexValueAlternate(void) const { return static_cast<double>(this->textureBounds1.x); }
	double getMaxTexValue(void) const { return static_cast<double>(this->textureBounds0.y); }
	double getMaxTexValueAlternate(void) const { return static_cast<double>(this->textureBounds1.y); }

	/// These functions are used by the sliders to control what image values to display
	void slotSetMinTexValue(double val);
	void slotSetMinTexValueAlternate(double val);
	void slotSetMinColorValue(double val);
	void slotSetMinColorValueAlternate(double val);
	void slotSetMaxTexValue(double val);
	void slotSetMaxTexValueAlternate(double val);
	void slotSetMaxColorValue(double val);
	void slotSetMaxColorValueAlternate(double val);

	uint getMinColorValue(void) const { return this->colorBounds0.x; }
	uint getMinColorValueAlternate(void) const { return this->colorBounds1.x; }
	uint getMaxColorValue(void) const { return this->colorBounds0.y; }
	uint getMaxColorValueAlternate(void) const { return this->colorBounds1.y; }

	std::pair<glm::uvec3, glm::uvec3> getVisuBoxCoordinates(void);
	void setVisuBoxMinCoord(glm::uvec3 coor_min);
	void setVisuBoxMaxCoord(glm::uvec3 coor_max);
	void resetVisuBox();

	void setDrawMode(DrawMode _mode);

	float getSceneRadius();
	glm::vec3 getSceneCenter();
	Image::bbox_t getSceneBoundingBox() const;
	
	void setColorFunction_r(ColorFunction _c);// Changes the texture coloration mode to the desired setting
	void setColorFunction_g(ColorFunction _c);

	void setColor0(qreal r, qreal g, qreal b);// color of the beginning of the color segment for the segmented color scale
	void setColor1(qreal r, qreal g, qreal b);
	void setColor0Alternate(qreal r, qreal g, qreal b);
	void setColor1Alternate(qreal r, qreal g, qreal b);

	void slotSetPlaneDisplacementX(float scalar);
	void slotSetPlaneDisplacementY(float scalar);
	void slotSetPlaneDisplacementZ(float scalar);

	void setPlaneHeading(planes _plane, planeHeading _heading);
	bool isSceneInitialized(void) const { return this->isInitialized; }

    /* Toggle things */
	void togglePlaneVisibility(planes _plane);
	void toggleAllPlaneVisibilities(void);

	void slotTogglePlaneDirectionX();
	void slotTogglePlaneDirectionY();
	void slotTogglePlaneDirectionZ();
	void toggleAllPlaneDirections();

	void setPositionResponse(glm::vec4 _resp);// Position a qglviewer::Frame at the given position, in 3D space.
	void drawPositionResponse(float radius, bool drawOnTop = false);// Draw a set of arrows at the position designated by setPositionResponse()
	void resetPositionResponse(void);// Reset the axis positions.

	/// @brief Applies a user-defined function on grids, with the constraint it must be const.
	//void lambdaOnGrids(std::function<void(const GridGLView::Ptr&)>& callable) const {
	//	std::for_each(this->grids.cbegin(), this->grids.cend(), callable);
	//}

	void deleteGridNow();

	void setupGLOutput();
	void printOpenGLMessage(const QOpenGLDebugMessage& message);

    /***********************************************/
    /* ARAP branch section */
    /***********************************************/

	/// @brief Loads a mesh (OFF) and uploads it to the GL.
	void loadMesh();
	void loadCurve();

	/// @brief This performs ARAP deformation on the first mesh found.
	/// @note THIS IS A WIP/DRAFT FUNCTION, NOT DESIGNED FOR PRODUCTION RELEASE
	void dummy_perform_arap_on_first_mesh();
	/// @brief This performs ARAP deformation on the mesh associated with the first loaded image.
	/// @note THIS IS A WIP/DRAFT FUNCTION, NOT DESIGNED FOR PRODUCTION RELEASE
	void dummy_perform_constrained_arap_on_image_mesh();
	/// @brief This adds a constraint for image 'img_idx' at position 'img_pos'.
	/// @note THIS IS A WIP/DRAFT FUNCTION, NOT DESIGNED FOR PRODUCTION RELEASE
	void dummy_add_image_constraint(std::size_t img_idx, glm::vec3 img_pos);
	/// @brief This checks if the given query point is contained in the bounding box
	/// @param query The position to query
	/// @param mesh_index The index of the first mesh that contains it, INDEXED AT 1. If 0, no meshes contain it.
	/// @note THIS IS A WIP/DRAFT FUNCTION, NOT DESIGNED FOR PRODUCTION RELEASE
	void dummy_check_point_in_mesh_bb(glm::vec3 query, std::size_t& mesh_index);
	/// @brief Returns the i-th drawable loaded in the scene.
	/// @note THIS IS A WIP/DRAFT FUNCTION, NOT DESIGNED FOR PRODUCTION RELEASE
	DrawableBase::Ptr dummy_getDrawable(std::size_t idx);
	/// @brief Adds to the 'i-th' mesh a constraint at vertex 'n'
	/// @note THIS IS A WIP/DRAFT FUNCTION, NOT DESIGNED FOR PRODUCTION RELEASE
	void dummy_add_arap_constraint_mesh(std::size_t drawable, std::size_t vtx_idx);
	/// @brief Prints the ARAP constraints as they currently are set in the program.
	/// @note THIS IS A WIP/DRAFT FUNCTION, NOT DESIGNED FOR PRODUCTION RELEASE
	void dummy_print_arap_constraints();
	/// @brief Applies the mesh alignment before the ARAP solver
	void dummy_apply_alignment_before_arap();

	void drawPointSpheres_quick(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const std::vector<glm::vec3>& positions, float radius);
    /***********************************************/

private:

	std::queue<std::shared_ptr<DrawableBase>> to_init;
	std::vector<std::shared_ptr<DrawableBase>> drawables;	 ///< The drawables to display
	std::vector<Mesh::Ptr> meshes;
	Curve::Ptr curve;
	std::shared_ptr<DrawableCurve> curve_draw;

	///
	/// Only for ARAP integration testing :
	///
	std::vector<std::pair<std::size_t, std::size_t>> mesh_idx_constraints;	  ///< The mesh vertices considered constraints. Pair = <mesh_idx , vertex_idx>
	std::vector<glm::vec3> image_constraints;	 ///< The positions of those constraints explained abovepositions of those constraints explained above

public:
	SceneGL sceneGL;

signals:
    void keyQReleased();
    void rayIsCasted(const glm::vec3& origin, const glm::vec3& direction);

// All these indirections are important because for most of them they interacts with various components of the scene
// And it allow more flexibility as the scene control ALL the informations to transit from class to class
public slots:
    // MeshManipulator slots
    void createNewMeshManipulator(int i, bool onSurface);
	void toggleWireframe();
    void toggleManipulatorActivation();
    void setManipulatorRadius(float radius);

    // MeshDeformator slots
    void setNormalDeformationMethod();
    void setWeightedDeformationMethod(float radius);

    // Rendering slots
	void setColorChannel(ColorChannel mode);
    void sendTetmeshToGPU(int gridIdx, const InfoToSend infoToSend);
    void sendFirstTetmeshToGPU();
    uint16_t sendGridValuesToGPU(int gridIdx);

    //void addManipulatorFromRay(const glm::vec3& origin, const glm::vec3& direction, bool onSurface);
/*************/
/* Temporary */
/*************/
public:
    std::string filename = "";

    int gridToDraw = 0;

    SurfaceMesh * surfaceMesh;
    DrawableMeshV2 * drawableMesh;

    ICP * icp;

    void createNewICP();
    void ICPIteration();
    void ICPInitialize();

    void setL(float i);

    void setN(float i);

    void setS(float i);
};

/// @brief Type-safe conversion of enum values to unsigned ints.
inline unsigned int planeHeadingToIndex(planeHeading _heading);

class ICPMesh : public SurfaceMesh {

public:
    glm::mat4 originalTransformation;
    std::vector<glm::vec3> originalPoints;
    std::vector<glm::vec3> correspondence;
    std::vector<float> weights;

    ICPMesh(const std::vector<glm::vec3>& vertices, const std::vector<Triangle2>& triangles): SurfaceMesh(vertices, triangles) {
        this->originalTransformation = this->getModelTransformation();
        for(int i = 0; i < this->getNbVertices(); ++i) {
            this->correspondence.push_back(glm::vec3(0., 0., 0.));
            this->weights.push_back(1.f);
            this->originalPoints.push_back(this->getWorldVertice(i));
        }
    }

    ICPMesh(std::string const &filename) : SurfaceMesh(filename) {
        this->originalTransformation = this->getModelTransformation();
        for(int i = 0; i < this->getNbVertices(); ++i) {
            this->correspondence.push_back(glm::vec3(0., 0., 0.));
            this->weights.push_back(1.f);
            this->originalPoints.push_back(this->getWorldVertice(i));
        }
    }

    void setCorrespondence(glm::vec3 p, int i) {
        this->correspondence[i] = p;
    }

    glm::vec3 getCorrespondence(int i) const {
        return this->correspondence[i];
    }

    void setWeight(float p, int i) {
        this->weights[i] = p;
    }

    float getWeight(int i) const {
        return this->weights[i];
    }

    void getPoint0(float p[3], const unsigned int index) const {  for(unsigned int i=0;i<3;i++) p[i]=this->originalPoints[index][i]; }
    void getPoint(float p[3], const unsigned int index) const {  for(unsigned int i=0;i<3;i++) p[i]=this->getWorldVertice(index)[i]; }
    void getCorrespondence(float p[3], const unsigned int index) const {  for(unsigned int i=0;i<3;i++) p[i]=this->correspondence[index][i]; }
    float getWeight(const unsigned int index) const {  return this->weights[index]; }

};


class ICP {

public:
    //unsigned int Ni=20,No=20,S=10;
    //float l=0.4;

    unsigned int Ni=10,No=5,S=10;
    float l=0.02*100;

    int nbIt = 0;
    int metric = 1;

    glm::mat3 rotation;
    glm::vec3 origin;

    ICPMesh * surface;

    Grid * src;
    Grid * target;

    CImg<uint16_t> sourceProf;

    float A[3][3];
    float t[3];

    ICP(Grid * src, Grid * target, ICPMesh * surface): src(src), target(target), surface(surface) {
        this->rotation = glm::mat3(1.f);
        this->origin = glm::vec3(0., 0., 0.);
    }

    ICP(Grid * src, Grid * target, std::string const &filename): src(src), target(target), surface(new ICPMesh(filename)) {

        // Setup #1
        //this->surface->setScale(glm::vec3(35., 35., 35.));
        //this->surface->setOrigin(glm::vec3(137.871201, 115.300957, 142.449493));

        //this->src->setOrigin(glm::vec3(-50.-1.88574, -100.-1.13047, -60.-3.44723) + glm::vec3(110., 110., 110.));
 
        // Setup #2
        this->surface->setScale(glm::vec3(100., 100., 100.));
        //this->surface->setOrigin(glm::vec3(137.871201, 115.300957, 142.449493));
        //this->surface->setOrigin(((this->surface->bbMax - this->surface->bbMax)/2.f) + glm::vec3(110., 110., 110.));
        this->surface->setOrigin(glm::vec3(229.294800, -6.420403, 114.809021));

        this->src->setOrigin(glm::vec3(-50.-1.88574, -100.-1.13047, -60.-3.44723) + glm::vec3(110., 110., 110.));
        this->src->setScale(glm::vec3(1., 1., 3.));
        this->target->setOrigin(glm::vec3(-106.-2.3, -114.5-1., -50.-2.5) + glm::vec3(110., 110., 110.));
        this->target->setScale(glm::vec3(1.381, 1.29553, 3.90019));

        //this->rotation = glm::mat3(1.f);
        //this->origin = glm::vec3(0., 0., 0.);

        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < 3; ++j)
                this->A[i][j] = 0;

        A[0][0] = 1;
        A[1][1] = 1;
        A[2][2] = 1;

        t[0] = 0;
        t[1] = 0;
        t[2] = 0;
    }

    void draw() {
        if(this->surface) {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_DEPTH);

            float normalSize = 0.02*100. * 10;

            for(int i = 0; i < this->surface->getNbVertices(); ++i) {
                glm::vec3 p = this->surface->getWorldVertice(i);
                glm::vec3 p2 = p + this->surface->getVerticeNormal(i) * normalSize;
                glBegin(GL_LINES);
                glColor3f(1., 0., 0.);
                glVertex3f(p[0], p[1], p[2]);
                glColor3f(1., 1., 0.);
                glVertex3f(p2[0], p2[1], p2[2]);
                glEnd();
            }

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_DEPTH);
        }
    }

    void initialize() {
        this->surface->originalTransformation = this->surface->getModelTransformation();
        for(int i = 0; i < this->surface->getNbVertices(); ++i) {
            this->surface->correspondence.push_back(glm::vec3(0., 0., 0.));
            this->surface->weights.push_back(1.f);
            this->surface->originalPoints[i] = this->surface->getWorldVertice(i);
            //std::cout << glm::to_string(this->surface->originalPoints[i]);
        }
        this->sourceProf = computeProfiles(*this->surface, *this->src, Ni, No, l);
        this->sourceProf.print();
        this->sourceProf.save_bmp("srcProfil.bmp");
    }

    void computeCorrespondences(ICPMesh& mesh, const CImg<float> &dist, const float l, int metric) {
        unsigned int nbpoints=dist.height();
        unsigned int S=dist.width()/2;

        for(unsigned int ptIdx=0;ptIdx<nbpoints;ptIdx++) {
            glm::vec3 p = mesh.getWorldVertice(ptIdx);

            bool valueFound = (dist.get_row(ptIdx).mean() != 0.);
            if(valueFound) {
                std::vector<float> row(dist.get_shared_row(ptIdx).data(), dist.get_shared_row(ptIdx).data()+dist.width());
                int idx = 0;
                if(metric == 0)
                    idx = std::distance(row.begin(), std::min_element(row.begin(), row.end()));
                else
                    idx = std::distance(row.begin(), std::max_element(row.begin(), row.end()));

                float direction = 1.;
                if(idx < S)
                    direction = -1.;

                float distance = std::fabs(float(idx)-float(S)) * l;

                glm::vec3 currentNormal = mesh.getVerticeNormal(ptIdx);
                p += currentNormal * direction * distance;
            }

            mesh.setCorrespondence(p,ptIdx);
        }
    }
    
    CImg<uint16_t> computeProfiles(const ICPMesh& mesh, const Grid& img, const unsigned int Ni, const unsigned int No, const float l, bool print = false) {
        //for(int i = 0; i < 3; ++i) {
        //    currentPoint[i] = std::roundf(currentPoint[i] * 1000) / 1000.0;
        //    currentNormal[i] = std::roundf(currentNormal[i] * 1000) / 1000.0;
        //}
        CImg<uint16_t> prof(Ni+No,mesh.getNbVertices());
        prof.fill(0);

        for(int ptIdx = 0; ptIdx < mesh.getNbVertices(); ++ptIdx) {
            glm::vec3 currentPoint = mesh.getWorldVertice(ptIdx);
            glm::vec3 currentNormal = mesh.getVerticeNormal(ptIdx);

            float direction = -1.;
            for(int step = 0; step < Ni; ++step) {
                glm::vec3 newPoint = currentPoint + direction * float(step) * l * currentNormal;
                prof(Ni-step-1, ptIdx) = img.getValueFromWorldPoint(newPoint, InterpolationMethod::Linear);
            }

            direction = 1.;
            for(int step = 0; step < No; ++step) {
                glm::vec3 newPoint = currentPoint + direction * float(step) * l * currentNormal;
                prof(Ni+step, ptIdx) = img.getValueFromWorldPoint(newPoint, InterpolationMethod::Linear);
            }
        }

        //prof.display();
        prof.save_bmp("nextProfil.bmp");
        return prof;
    }

    // 0 = SSD
    CImg<float> computeDistance(const CImg<uint16_t>& sourceProf, const CImg<uint16_t>& targetProf, int metric) {

        unsigned int nbpoints=sourceProf.height();
        unsigned int N=sourceProf.width();
        unsigned int S=(targetProf.width()-sourceProf.width())/2;

        CImg<float> dist(2*S,nbpoints);
        dist.fill(0);

        if(metric==0) {
            for(int ptIdx = 0; ptIdx < nbpoints; ++ptIdx)
                for(int offset = 0; offset < 2*S; ++offset)
                    for(int step = 0; step < N; ++step)
                        dist(offset, ptIdx) += std::pow(float(sourceProf(step, ptIdx)) - float(targetProf(step+offset, ptIdx)), 2);
        } else {
            for(int ptIdx = 0; ptIdx < nbpoints; ++ptIdx) {
                bool meanToZero = false;
                double srcMean = sourceProf.get_row(ptIdx).mean();
                if(srcMean == 0) meanToZero = true;

                for(int offset = 0; offset < 2*S; ++offset) {
                    double trgMean = targetProf.get_row(ptIdx).get_columns(offset, offset+N).mean();
                    if(trgMean == 0) meanToZero = true;

                    if(meanToZero) {
                        dist(offset, ptIdx) = 0;
                    } else {
                        double subMult      = 0;
                        double subSquareSrc = 0;
                        double subSquareTrg = 0;
                        for(int step = 0; step < N; ++step) {
                            subMult += (double(sourceProf(step, ptIdx)) - srcMean) * (double(targetProf(offset+step, ptIdx)) - trgMean);
                            subSquareSrc += std::pow(double(sourceProf(step, ptIdx)) - srcMean, 2);
                            subSquareTrg += std::pow(double(targetProf(offset+step, ptIdx)) - trgMean, 2);
                        }

                        dist(offset, ptIdx) = subMult / std::sqrt(subSquareSrc * subSquareTrg);
                    }
                }
            }
        }
        //dist.display();
        dist.save_bmp("dist.bmp");
        return dist;
    }

    //void Registration(glm::mat3& A,  glm::vec3& t,const ICPMesh& mesh);
    void Registration(float A[3][3],  float t[3], const ICPMesh& mesh);

    void iteration(bool print=false) {
        this->surface->computeNormals();
        CImg<uint16_t> targetProf = computeProfiles(*this->surface, *this->target, Ni+S, No+S, l, print);
        CImg<float> dist = computeDistance(this->sourceProf,targetProf,this->metric);
        computeCorrespondences(*this->surface,dist,l,this->metric);
        Registration(this->A, this->t, *this->surface);

        glm::mat4 transformation(1.f);
        for(int i = 0; i < 3; ++i) {
            transformation[3][i] = this->t[i];
            for(int j = 0; j < 3; ++j)
                transformation[i][j] = this->A[i][j];
        }
        this->surface->setTransformation(transformation * this->surface->originalTransformation);

        nbIt += 1;
    }
};

#endif	  // VIEWER_INCLUDE_SCENE_HPP_
