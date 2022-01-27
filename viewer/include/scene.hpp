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
#include "../../qt/include/manipulator.hpp"
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

// New structure
#include "../../qt/include/GLmanipulator.hpp"

#include <thread>
#include "../../image/utils/include/threaded_task.hpp"

// Tinytiff
#include <tinytiffreader.h>
#include <tinytiffwriter.h>

#include "../../grid/include/grid.hpp"

/// @defgroup graphpipe Graphics pipeline
/// @brief This group contains all classes closely or loosely related to the graphics pipeline.
/// @details There are very few classes in this group, but that's only because Scene is a god-object. Some attempt was
/// made to move much of the code for displaying a grid over to the GridViewer class, altough that has not been tested.
/// @warning Spaghetti code ahead.

class ControlPanel;	   // Forward declaration
namespace UITool {
	namespace GL {
		class MeshManipulator;	  // Forward declaration
	}
}	 // namespace UITool

namespace UITool {
		class MeshManipulator;	  // Forward declaration
}	 // namespace UITool

/// @brief Simple enum to keep track of the different viewing primitives for the program.
enum DrawMode { Solid,
	Volumetric,
	VolumetricBoxed };
/// @brief The RGB mode chosen by the user
enum RGBMode { None = 0,
	RedOnly			= 1,
	GreenOnly		= 2,
	RedAndGreen		= 3,
	HandEColouring	= 4 };
/// @brief Simple enum to keep track of which color function to apply to the viewers.
enum ColorFunction { SingleChannel,
	HistologyHandE,
	HSV2RGB,
	ColorMagnitude };
/// @brief Simple enum to define which plane we are drawing
enum planes { x = 1,
	y			= 2,
	z			= 3 };
/// @brief Simple enum to keep track of a plane's orientation.
enum planeHeading { North = 0,
	East				  = 1,
	South				  = 2,
	West				  = 3,
	Up					  = North,
	Right				  = East,
	Down				  = South,
	Left				  = West };

// Usefull struct to choose which data of the tetmesh to send to the GPU
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
    void sendTetmeshToGPU(int gridIdx, const InfoToSend infoToSend);
	void generateColorScales();
    /*************************************************/

    /* Others */
    void slotApplyDeformation(glm::vec3 oldPosition, glm::vec3 newPosition);
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

    /* Attributes */

    //TODO: to remove
    int temp_ratio;
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
    GridGL * initial;
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
	RGBMode rgbMode;
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
    bool slotGetPositionFromRay(const glm::vec3& origin, const glm::vec3& direction, glm::vec3& res);

	void showVisuBoxController(VisuBoxController* _controller);
	void removeVisuBoxController();

	void prepareManipulators();
    // Set all manipulators positions to the points of the tetmesh
    void updateManipulatorPositions();

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

	void setRGBMode(RGBMode _mode);
	
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

// All these indirections are important because for most of them they interacts with various components of the scene
// And it allow more flexibility as the scene control ALL the informations to transit from class to class
public slots:
    // MeshManipulator slots
    void createNewMeshManipulator(int i);
	void toggleWireframe();
    void toggleManipulatorActivation();
    void setManipulatorRadius(float radius);

    // MeshDeformator slots
    void setNormalDeformationMethod();
    void setWeightedDeformationMethod(float radius);

/*************/
/* Temporary */
/*************/
public:
    std::string filename = "";
};

/// @brief Type-safe conversion of enum values to unsigned ints.
inline unsigned int planeHeadingToIndex(planeHeading _heading);

/// @brief Helper struct to store the indices of vertices that make a face, and compare two faces to one another.
struct Face
{
public:
	inline Face(unsigned int v0, unsigned int v1, unsigned int v2) {
		if (v1 < v0)
			std::swap(v0, v1);
		if (v2 < v1)
			std::swap(v1, v2);
		if (v1 < v0)
			std::swap(v0, v1);
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
	}
	inline Face(const Face& f) {
		v[0] = f.v[0];
		v[1] = f.v[1];
		v[2] = f.v[2];
	}
	inline virtual ~Face() {}
	inline Face& operator=(const Face& f) {
		v[0] = f.v[0];
		v[1] = f.v[1];
		v[2] = f.v[2];
		return (*this);
	}
	inline bool operator==(const Face& f) { return (v[0] == f.v[0] && v[1] == f.v[1] && v[2] == f.v[2]); }
	inline bool operator<(const Face& f) const { return (v[0] < f.v[0] || (v[0] == f.v[0] && v[1] < f.v[1]) || (v[0] == f.v[0] && v[1] == f.v[1] && v[2] < f.v[2])); }
	inline bool contains(unsigned int i) const { return (v[0] == i || v[1] == i || v[2] == i); }
	inline unsigned int getVertex(unsigned int i) const { return v[i]; }
	unsigned int v[3];
};

#endif	  // VIEWER_INCLUDE_SCENE_HPP_

// vim : ts=8
