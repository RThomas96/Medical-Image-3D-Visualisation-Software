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
// Discrete grid and grid generation :
#include "../../grid/include/discrete_grid.hpp"
#include "../../grid/include/input_discrete_grid.hpp"
#include "../../grid/include/output_discrete_grid.hpp"
#include "../../grid/include/tetmesh.hpp"
// Mesh operations :
#include "../../meshes/operations/arap/Manipulator.h"
#include "../../meshes/operations/arap/RectangleSelection.h"
#include "../../meshes/operations/arap/PCATools.h"
#include "../../meshes/operations/arap/mesh_manip_interface.h"
// UI elements :
#include "../../qt/include/grid_control.hpp"
#include "../../qt/include/grid_detailed_view.hpp"
#include "../../qt/include/grid_list_view.hpp"
#include "../../qt/include/manipulator.hpp"
#include "../../qt/include/opengl_debug_log.hpp"
#include "../../qt/include/visu_box_controller.hpp"
// Helper structs and functions :
#include "./viewer_structs.hpp"
// New grid API :
#include "../../image/tiff/include/tiff_reader.hpp"
#include "../../new_grid/include/grid.hpp"
// Qt headers :
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLFunctions_4_0_Core>
#include <QProgressBar>
#include <QStatusBar>
// libQGLViewer :
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/camera.h>
// glm include :
#include <glm/glm.hpp>
// STD headers :
#include <mutex>
#include <vector>

// New structure
#include "../../qt/include/GLmanipulator.hpp"

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
class Scene : public QOpenGLFunctions_3_2_Core {
	/// @brief typedef to omit glm:: from a 'uvec3'
	typedef glm::uvec3 uvec3;

public:
	Scene();	///< default constructor
	~Scene(void);	 ///< default destructor

	/// @brief initialize the variables of the scene
	void initGl(QOpenGLContext* context);

	/// @brief Upload a 1D texture with the given parameters.
	GLuint uploadTexture1D(const TextureUpload& tex);
	/// @brief Upload a 2D texture with the given parameters.
	GLuint uploadTexture2D(const TextureUpload& tex);
	/// @brief Upload a 3D texture with the given parameters.
	GLuint uploadTexture3D(const TextureUpload& tex);

	/// @brief Show the Visualization box controller
	void showVisuBoxController(VisuBoxController* _controller);
	/// @brief Remove the visu box controller if it closes
	void removeVisuBoxController();

	/// @brief Adds a widget to which redirect to OpenGL output.
	void addOpenGLOutput(OpenGLDebugLog* _gldl);
	/// @brief Add a status bar to the program
	void addStatusBar(QStatusBar* _s);

	/// @brief set the control panel responsible for controlling the scene
	void setControlPanel(ControlPanel* cp) { this->controlPanel = cp; }
	/// @brief Remove the grid controller pointer from the scene class.
	void removeController();
	/// @brief reload the default shader files
	void recompileShaders(bool verbose = true);

	/// @brief Loads a designated ROI in high-resolution
	// DEPRECATED
	void loadGridROI(void);

	/// @brief Add grids, with the new API (more streamlined)
	void newAPI_addGrid(Image::Grid::Ptr gridLoaded);

	/// @brief Computes the planes positions based on their parameters.
	glm::vec3 computePlanePositions();

	/// @brief Will attempt to do a grid save, to test the new writer backend.
	void draft_tryAndSaveFirstGrid(void);

	/// @brief Draw the 3D view of the scene.
	void draw3DView(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, bool showTexOnPlane);

	/// @brief Draw a given plane 'view' (single plane on the framebuffer).
	void drawPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading, float zoomRatio, glm::vec2 offset);

	void getTetraMeshPoints(std::vector<glm::vec3>& points);

	/// @brief Draws a set of spheres at the positions given in argument
	/// @param mvMat The view matrix from the camera
	/// @param pMat The projection matrix for the camera
	/// @param camPos The camera position in world space
	/// @param positions The sphere positions to draw
	/// @param radius The sphere's radius when rendering it, in world-space units
	void drawPointSpheres_quick(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const std::vector<glm::vec3>& positions, float radius);
	/// @brief Draws a set of colored spheres at the positions given in argument
	/// @param mvMat The view matrix from the camera
	/// @param pMat The projection matrix for the camera
	/// @param camPos The camera position in world space
	/// @param positions The sphere positions to draw
	/// @param radius The sphere's radius when rendering it, in world-space units
	/// @param color The color to set the spheres to
	void drawColoredPointSpheres_quick(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const std::vector<glm::vec3>& positions, float radius, glm::vec4 color);
	/// @brief Draws a set of colored spheres at the positions given in argument
	/// @param mvMat The view matrix from the camera
	/// @param pMat The projection matrix for the camera
	/// @param camPos The camera position in world space
	/// @param positions The sphere positions to draw
	/// @param highlighted_sphere The index of the sphere to 'highlight' (aka set to a different color set by highlight_color)
	/// @param radius The sphere's radius when rendering it, in world-space units
	/// @param color The color to set the spheres to
	/// @param highlight_color The color to set the highlighted sphere to
	void drawColoredPointSpheres_highlighted_quick(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const std::vector<glm::vec3>& positions, std::size_t highlighted_sphere, float radius, glm::vec4 color, glm::vec4 highlight_color);

	/// @brief Create a texture suited for framebuffer rendering, by passing the dimensions of it to the function.
	GLuint updateFBOOutputs(glm::ivec2 dimensions, GLuint fb_handle, GLuint old_texture = 0);

	/// @brief Read the specified texture 'tex_handl', at coordinates 'image_coordinates' and return the RGB[A] value
	glm::vec4 readFramebufferContents(GLuint fb_handle, glm::ivec2 image_coordinates);

	/// @brief Returns the current scene boundaries.
	glm::vec3 getSceneBoundaries() const;

	/// @brief Set the draw mode for the 3D view of the scene.
	void setDrawMode(DrawMode _mode);

	/// @brief Launches a save dialog, to generate a grid.
	void launchSaveDialog();

	/// @brief Launches the grid deformation
	void launchDeformation(int tetIdx, glm::vec3 point);

	/// @brief Return the position of the tetrahedra point index
	glm::vec3 getVertexPosition(int index);

	/// @brief Prints info about the VAO on next refresh
	void printVAOStateNext() { this->showVAOstate = true; }

	/// @brief Get the minimum value that the grid at gridIndex can contain
	double getMinNumericLimit(size_t gridIndex) const { return Image::getMinNumericLimit(newGrids[gridIndex]->grid->getInternalDataType()); }
	/// @brief Get the maximum value that the grid at gridIndex can contain
	double getMaxNumericLimit(size_t gridIndex) const { return Image::getMaxNumericLimit(newGrids[gridIndex]->grid->getInternalDataType()); }

	/// @brief Get the minimum texture value to represent
	double getMinTexValue(void) const { return static_cast<double>(this->textureBounds0.x); }
	/// @brief Get the maximum texture value to represent
	double getMaxTexValue(void) const { return static_cast<double>(this->textureBounds0.y); }
	/// @brief Get the minimum texture value to represent
	double getMinTexValueAlternate(void) const { return static_cast<double>(this->textureBounds1.x); }
	/// @brief Get the maximum texture value to represent
	double getMaxTexValueAlternate(void) const { return static_cast<double>(this->textureBounds1.y); }

	/// These functions are used by the sliders to control what image values to display
	/// @brief Set minimum texture intensity.
	void slotSetMinTexValue(double val);
	/// @brief Set maximum texture intensity.
	void slotSetMaxTexValue(double val);
	/// @brief Set minimum texture intensity.
	void slotSetMinTexValueAlternate(double val);
	/// @brief Set maximum texture intensity.
	void slotSetMaxTexValueAlternate(double val);
	/// @brief Set minimum color intensity.
	void slotSetMinColorValue(double val);
	/// @brief Set maximum color intensity.
	void slotSetMaxColorValue(double val);
	/// @brief Set minimum color intensity.
	void slotSetMinColorValueAlternate(double val);
	/// @brief Set maximum color intensity.
	void slotSetMaxColorValueAlternate(double val);

	/// @brief Get the minimum color value, for the color scale resizing.
	uint getMinColorValue(void) const { return this->colorBounds0.x; }
	/// @brief Get the maximum color value, for the color scale resizing.
	uint getMaxColorValue(void) const { return this->colorBounds0.y; }
	/// @brief Get the minimum color value, for the color scale resizing.
	uint getMinColorValueAlternate(void) const { return this->colorBounds1.x; }
	/// @brief Get the maximum color value, for the color scale resizing.
	uint getMaxColorValueAlternate(void) const { return this->colorBounds1.y; }

	/// @brief Returns the image coordinates of the visu box.
	std::pair<glm::uvec3, glm::uvec3> getVisuBoxCoordinates(void);
	/// @brief Sets the min coordinate of the visu box
	void setVisuBoxMinCoord(glm::uvec3 coor_min);
	/// @brief Sets the max coordinate of the visu box
	void setVisuBoxMaxCoord(glm::uvec3 coor_max);
	/// @brief Resets the visu box
	void resetVisuBox();

	/// @brief Get the scene radius at this time
	float getSceneRadius();
	/// @brief Get the scene center at this time
	glm::vec3 getSceneCenter();
	/// @brief Returns the bounding box of the scene.
	Image::bbox_t getSceneBoundingBox() const;

	/// @brief Upload a 3D texture with the given parameters.
	GLuint newAPI_uploadTexture3D(const GLuint handle, const TextureUpload& tex, std::size_t s, std::vector<std::uint16_t>& data);
	/// @brief Allocate a texture conformant with the given settings, but don't fill it with data yet.
	GLuint newAPI_uploadTexture3D_allocateonly(const TextureUpload& tex);

	// ******************* //
	//                     //
	// ARAP Functions !!!! //
	//                     //
	// ******************* //

	/// @brief Loads a mesh (OFF) and uploads it to the GL.
	void loadMesh();
	/// @brief Loads a curve (OBJ) and uploads it to the GL.
	void loadCurve();
	/// @brief Sets the corresponding viewer so that we can access it inside the class.
	void setViewer(Viewer* parent) { this->viewer = parent; }

	/// @brief Updates the mesh _and_ the curve, if both are loaded
	void updateMeshAndCurve();
	/// @brief Updates the mesh _and_ the curve, if both are loaded, but don't apply the transformation to them
	void updateMeshAndCurve_No_Image_Resizing();
	/// @brief Updates the positions in the mesh manip interface
	/// @note This really ought to be done automatically, by putting the mesh inside the manip interface instead of copying vertices everytime ...
	void updateMeshInterface();

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
	bool dummy_check_point_in_mesh_bb(glm::vec3 query, std::size_t& mesh_index);
	/// @brief Adds to the 'i-th' mesh a constraint at vertex 'n'
	/// @note THIS IS A WIP/DRAFT FUNCTION, NOT DESIGNED FOR PRODUCTION RELEASE
	void dummy_add_arap_constraint_mesh(std::size_t drawable, std::size_t vtx_idx);
	/// @brief Prints the ARAP constraints as they currently are set in the program.
	/// @note THIS IS A WIP/DRAFT FUNCTION, NOT DESIGNED FOR PRODUCTION RELEASE
	void dummy_print_arap_constraints();
	/// @brief Applies the mesh alignment before the ARAP solver
	void dummy_apply_alignment_before_arap();
	/// @brief Save the mesh to a file
	void dummy_save_mesh_to_file();
	/// @brief Save the curve to a file
	void dummy_save_curve_to_file();
	/// @brief Loads the constraints for the currently loaded mesh from a file
	void dummy_loadConstraintsFromFile();
	/// @brief Returns the positions of the mesh constraints, when loaded in.
	std::vector<glm::vec3> dummy_get_loaded_constraint_positions() const;
	/// @brief From the loaded mesh/curve combo, load another curve and match their lengths.
	void dummy_resize_curve_to_match_other_curve();
	/// @brief Scales the mesh to fit the BB of the selected points
	void dummy_scale_mesh_to_cp_bb();

	/// @brief Resets the scene data in order for the ARAP controller to perform its duties.
	void arap_reset_scene_data();
	/// @brief Loads the mesh data from the ARAP controller.
	void arap_load_mesh_data(Mesh::Ptr& mesh_to_upload);
	/// @brief Loads the curve data from the ARAP controller.
	void arap_load_curve_data(Curve::Ptr& curve_to_upload);
	/// @brief Loads the image data from the ARAP controller.
	void arap_load_image_data(Image::Grid::Ptr& grid_to_upload);

	DrawableMesh::Ptr& arap_get_mesh_drawable();
	DrawableCurve::Ptr& arap_get_curve_drawable();

	void arap_delete_grid_data();
	void arap_delete_mesh_drawable();
	void arap_delete_curve_drawable();

	/*
	std::shared_ptr<MMInterface<glm::vec3>> getMeshInterface() const { return this->mesh_interface; }
	std::shared_ptr<SimpleManipulator> getManipulator() const { return this->arapManipulator; }
	std::shared_ptr<RectangleSelection> getRectangleSelection() const { return this->rectangleSelection; }

	const Mesh::Ptr& getMesh() const { return this->mesh; }
	const Curve::Ptr& getCurve() const { return this->curve; }
	*/
	const DrawableMesh::Ptr getDrawableMesh() const { return this->mesh_draw; }
	const DrawableCurve::Ptr getDrawableCurve() const { return this->curve_draw; }

	/// @brief Changes the texture coloration mode to the desired setting
	void setColorFunction_r(ColorFunction _c);
	void setColorFunction_g(ColorFunction _c);

	/// @brief Changes the RGB mode of the scene.
	void setRGBMode(RGBMode _mode);

	/// @brief Set the color of the beginning of the color segment for the segmented color scale
	void setColor0(qreal r, qreal g, qreal b);
	/// @brief Set the color of the beginning of the color segment for the segmented color scale
	void setColor1(qreal r, qreal g, qreal b);
	/// @brief Set the color of the beginning of the color segment for the segmented color scale
	void setColor0Alternate(qreal r, qreal g, qreal b);
	/// @brief Set the color of the beginning of the color segment for the segmented color scale
	void setColor1Alternate(qreal r, qreal g, qreal b);

	/// @brief Create a uniform buffer of size `size_bytes` and fill it with null data.
	GLuint createUniformBuffer(std::size_t size_bytes, GLenum draw_mode);
	/// @brief Overwrite data at position 'begin_bytes' with 'size_bytes' of 'data'.
	void setUniformBufferData(GLuint uniform_buffer, std::size_t begin_bytes, std::size_t size_bytes, GLvoid* data);

	/// @brief Set X's plane displacement within the bounding box to be `scalar`
	void slotSetPlaneDisplacementX(float scalar);
	/// @brief Set Y's plane displacement within the bounding box to be `scalar`
	void slotSetPlaneDisplacementY(float scalar);
	/// @brief Set Z's plane displacement within the bounding box to be `scalar`
	void slotSetPlaneDisplacementZ(float scalar);

	/// @brief Toggles the visibility of the plane in argument
	void togglePlaneVisibility(planes _plane);
	/// @brief Signals all planes they need to be to be [in]visible.
	void toggleAllPlaneVisibilities(void);

	/// @brief Sets the new 'heading' of the plane (to rotate the planar viewers in the framebuffer)
	void setPlaneHeading(planes _plane, planeHeading _heading);

	/// @brief Toggles visibility of plane X.
	void slotTogglePlaneDirectionX();
	/// @brief Toggles visibility of plane Y.
	void slotTogglePlaneDirectionY();
	/// @brief Toggles visibility of plane Z.
	void slotTogglePlaneDirectionZ();
	/// @brief Signals all planes they need to be inverted.
	void toggleAllPlaneDirections();

	/// @brief Position a qglviewer::Frame at the given position, in 3D space.
	void setPositionResponse(glm::vec4 _resp);
	/// @brief Draw a set of arrows at the position designated by setPositionResponse()
	void drawPositionResponse(float radius, bool drawOnTop = false);
	/// @brief Reset the axis positions.
	void resetPositionResponse(void);

	/// @brief Applies a user-defined function on grids, with the constraint it must be const.
	/// @warning Only applies the lambda on the new Image::Grid interface, and only when it is wrapped in the
	/// NewAPI_GridGLView helper class !!!
	void lambdaOnGrids(std::function<void(const NewAPI_GridGLView::Ptr&)>& callable) const {
		std::for_each(this->newGrids.cbegin(), this->newGrids.cend(), callable);
	}

	/// @brief Returns the context, for external use.
	QOpenGLContext* get_context() const { return this->context; }

	/// @brief Checks if the scene is already initialized.
	bool isSceneInitialized(void) const { return this->isInitialized; }

	/// @brief Update the scene's bounding box with the currently drawn grids.
	void updateBoundingBox(void);

	void bindMeshManipulator(UITool::MeshManipulator* meshManipulator);
	void toggleManipulatorDisplay();
	void toggleWireframe();
	void prepareManipulators();

private:
	/// @brief Compile the given shaders, and return the ID of the program generated. On any error, returns 0.
	GLuint compileShaders(std::string vPath, std::string gPath, std::string fPath, bool verbose = false);

	/// @brief Creates all the VAO/VBO handles
	void createBuffers();
	/// @brief Generate the unit cube used to draw the grid in non-volumetric mode, as well as the plane positions and bounding box buffers.
	void generateSceneData();
	/// @brief Generates the vertices, normals, and tex coordinates for a basic unit cube
	void generateTexCube(SimpleVolMesh& _mesh);
	/// @brief Generates the plane's vertices array indexes
	void generatePlanesArray(SimpleVolMesh& _mesh);
	/// @brief setup the buffers' data
	void setupVBOData(const SimpleVolMesh& _mesh);
	/// @brief setup the vao binding setup
	void setupVAOPointers();
	/// @brief Generates the data necessary to draw a sphere.
	void generateSphereData();

	/// @brief Immediate deletion of the grid index to delete
	void deleteGridNow();

	/// @brief Sets up the OpenGL debug log.
	void setupGLOutput();
	/// @brief Print the OpenGL message to std::cerr, if no OpenGLDebugLogMessages are enabled.
	void printOpenGLMessage(const QOpenGLDebugMessage& message);

	/// @brief Updates the progress bar added to the main statusbar
	void updateProgressBar();

	/// @brief Resize the mesh for the loaded image, or the other way around.
	void resizeMeshForGrid();

	/// @brief Test function to print all the new uniforms in the
	void newSHADERS_print_all_uniforms(GLuint program);

	/// @brief Create the color scales used for the program.
	void newSHADERS_generateColorScales(void);

	/// @brief Update the user-defined color scales
	void newSHADERS_updateUserColorScales();
	/// @brief Signals to update user-defined color scales whenever is next appropriate.
	void signal_updateUserColorScales();

	/// @brief Upload the data necessary to make the UBO work for a grid.
	void newSHADERS_updateUBOData();

	/// @brief preps uniforms for a grid [[NEW API]]
	void newAPI_prepareUniforms_3DSolid(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, const NewAPI_GridGLView::Ptr& grid);
	/// @brief preps uniforms for a given plane [[NEW API]]
	void newAPI_prepareUniforms_3DPlane(GLfloat* mvMat, GLfloat* pMat, planes _plane, const NewAPI_GridGLView::Ptr& grid, bool showTexOnPlane = true);
	/// @brief prep the plane uniforms to draw in space [[NEW API]]
	void newAPI_prepareUniforms_PlaneViewer(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, glm::vec2 offset, const NewAPI_GridGLView::Ptr& _grid);
	/// @brief Prepare the uniforms for volumetric drawing [[NEW API]]
	void newAPI_prepareUniforms_Volumetric(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, const NewAPI_GridGLView::Ptr& _grid);

	/// @brief draw the planes, in the real space using the new api
	void newAPI_drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane = true);
	/// @brief draws a grid, slightly more generic than drawVoxelGrid() [[NEW API]]
	void newAPI_drawGrid(GLfloat mvMat[], GLfloat pMat[], glm::mat4 baseMatrix, const NewAPI_GridGLView::Ptr& grid);
	/// @brief Draws the 3D texture with a volumetric-like visualization method [[NEW API]]
	void newAPI_drawVolumetric(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const NewAPI_GridGLView::Ptr& grid);

	/// @brief Returns an unsigned int (suitable for uniforms) from a color function
	uint colorFunctionToUniform(ColorFunction _c);

	/// @brief Updates the visibility array to only show values between the min and max tex values
	void updateVis();

	/// @brief Updates the color and visibility ranges for all visible grids.
	void updateCVR();

	/// @brief Creates the VBO/VAO handles for bounding boxes
	void createBoundingBoxBuffers();
	/// @brief Orders the VAO pointers for the bounding box
	void setupVAOBoundingBox();
	/// @brief Draw a bounding box
	void drawBoundingBox(const Image::bbox_t& _box, glm::vec3 color, GLfloat* vMat, GLfloat* pMat);
	void updateVisuBoxCoordinates(void);

	/// @brief Stub function to initialize some system-level limits. Currently only fetches max texture size.
	void initialize_limits(void);

	/*************************************/
	/*************************************/
	/****** TEXTURE3D VISUALIZATION ******/
	/*************************************/
	/*************************************/
	/// @brief Build a tetrahedral mesh for a loaded grid. [[NEW API]]
	void newAPI_tex3D_buildMesh(NewAPI_GridGLView::Ptr& grid, const std::string path = "");
	/// @brief Build the visibility texture for a grid.
	void tex3D_buildVisTexture(VolMesh& volMesh);
	/// @brief Build the draw buffers for a grid.
	void tex3D_buildBuffers(VolMesh& volMesh);
	/// @brief Bind the VAO created for the volumetric drawing method.
	void tex3D_bindVAO();
	/// @brief Load a .mesh file for the tetrahedral mesh, instead of generating it. [[NEW API]]
	void newAPI_tex3D_loadMESHFile(const std::string name, const NewAPI_GridGLView::Ptr& grid, VolMeshData& _mesh);
	/// @brief Generate a surrounding tetrahedral mesh for the loaded grid. [[NEW API]]
	void newAPI_tex3D_generateMESH(NewAPI_GridGLView::Ptr& grid, VolMeshData& _mesh);

private:
	UITool::GL::MeshManipulator* glMeshManipulator;

	bool isInitialized;	   ///< tracks if the scene was initialized or not
	bool showVAOstate;	  ///< Do we need to print the VAO/program state on next draw ?
	bool shouldUpdateVis;	 ///< Should we update visibility on next draw ?
	bool shouldDeleteGrid;	  ///< Should we delete a grid on next draw ?
	std::vector<std::size_t> delGrid;	 ///< Grids to delete at next refresh

	// Grids :
	// std::vector<GridGLView::Ptr> grids;	   ///< Grids to display in the different views.
	std::vector<NewAPI_GridGLView::Ptr> newGrids;	 ///< Grids, but with the new API.

	qglviewer::Frame* posFrame;
	glm::vec4 posRequest;

	// OpenGL-related stuff :
	QOpenGLContext* context;	///< The context with which the scene has been created with
	OpenGLDebugLog* glOutput;	 ///< Output of the GL log.
	QOpenGLDebugLogger* debugLog;	 ///< The debug log reading messages from the GL_KHR_debug extension

	// Widgets that may interact with the scene :
	GridControl* gridControl;	 ///< The controller for the grid 'save' feature (generation)
	ControlPanel* controlPanel;	   ///< pointer to the control panel
	QStatusBar* programStatusBar;	 ///< Status bar to show some info about the program.
	VisuBoxController* visuBoxController;	 ///< The controller for the visualization box

	// Render parameters :
	std::size_t renderSize;	   ///< Number of primitives to render for the solid view mode.
	NewAPI_GridGLView::data_2 textureBounds0;
	NewAPI_GridGLView::data_2 textureBounds1;
	NewAPI_GridGLView::data_2 colorBounds0;
	NewAPI_GridGLView::data_2 colorBounds1;
	std::array<glm::vec3, 8> lightPositions;	///< Scene lights (positionned at the corners of the scene BB)

	glm::bvec3 planeVisibility;	   ///< Should we show each plane (X, Y, Z)
	glm::vec3 planeDirection;	 ///< Cutting plane directions (-1 or 1 on each axis)
	glm::vec3 planeDisplacement;	///< %age of the scene bounding box to place the planes
	Image::bbox_t sceneBB;	  ///< Outer BB of the scene
	Image::bbox_t sceneDataBB;	  ///< Outer BB of the scene's data
	float clipDistanceFromCamera;	 /// Distance from the camera to its clip plane
	glm::uvec3 visuMin;	   ///< The min image coordinate to display on the visu box mode
	glm::uvec3 visuMax;	   ///< The max image coordinate to display on the visu box mode
	Image::bbox_t visuBox;	  ///< Used to restrict the view to a box with its coordinates
	DrawMode drawMode;	  ///< Current 3D draw mode
	RGBMode rgbMode;	///< Current RGB mode
	ColorFunction channels_r;	 ///< Channel(s) to display on the viewers
	GLuint selectedChannel_r;	 ///< The currently selected channel for greyscale mode.
	ColorFunction channels_g;	 ///< Channel(s) to display on the viewers
	GLuint selectedChannel_g;	 ///< The currently selected channel for greyscale mode.

	glm::vec3 color0;	 ///< The color segment when approaching 0
	glm::vec3 color1;	 ///< The color segment when approaching 1
	glm::vec3 color0_second;	///< The color segment when approaching 0
	glm::vec3 color1_second;	///< The color segment when approaching 1

	// VAO handles :
	GLuint vaoHandle;
	GLuint vaoHandle_VolumetricBuffers;
	GLuint vaoHandle_boundingBox;
	// VBO handles :
	GLuint vboHandle_VertPos;	 ///< The VBO for the vertex positions.
	GLuint vboHandle_VertNorm;	  ///< The VBO for the vertex normals.
	GLuint vboHandle_VertTex;	 ///< The VBO for the texture data.
	GLuint vboHandle_Element;	 ///< The vertex indices necessary to draw the tetrahedral mesh.
	GLuint vboHandle_PlaneElement;	  ///< The vertex indices necessary to draw the planes (a collection of the 3 main planes X/Y/Z).
	GLuint vboHandle_SinglePlaneElement;	///< The vertex indices necessary to draw a single plane.
	GLuint vboHandle_boundingBoxVertices;
	GLuint vboHandle_boundingBoxIndices;

	///
	/// Necessary for the spheres :
	///
	GLuint vaoHandle_spheres;
	GLuint vboHandle_spherePositions;
	GLuint vboHandle_sphereNormals;
	GLuint vboHandle_sphereIndices;
	GLuint programHandle_sphere;
	GLuint sphere_size_to_draw;

	//
	// Stubs for ARAP manipulation :
	//
#ifdef ENABLE_SCENE_DEFORMATION
	Mesh::Ptr mesh;	   ///< The loaded mesh, if any
	Curve::Ptr curve;	 ///< The loaded curve, if any
#endif
	DrawableMesh::Ptr mesh_draw;
	DrawableCurve::Ptr curve_draw;
#ifdef ENABLE_SCENE_DEFORMATION
	std::vector<std::pair<std::size_t, std::size_t>> mesh_idx_constraints;	  ///< The mesh vertices considered constraints. Pair = <mesh_idx , vertex_idx>
	std::vector<glm::vec3> image_constraints;	 ///< The positions of those constraints explained above
	std::shared_ptr<MMInterface<glm::vec3>> mesh_interface;
	std::shared_ptr<SimpleManipulator> arapManipulator;
	std::shared_ptr<RectangleSelection> rectangleSelection;

	std::string arap_mesh_file_path;
	std::string arap_mesh_file_name;
	std::string arap_mesh_file_constraints;
#endif

	Viewer* viewer;
	/// @brief A pointer to the 3D viewer's camera.
	qglviewer::Camera* camera;

	/// @brief A compiler for the shaders
	std::unique_ptr<ShaderCompiler> shaderCompiler;

	// Program handles :
	GLuint programHandle_projectedTex;
	GLuint programHandle_Plane3D;
	GLuint programHandle_PlaneViewer;
	GLuint programHandle_VolumetricViewer;
	GLuint programHandle_BoundingBox;

	/*************************************/
	/*************************************/
	/****** TEXTURE3D VISUALIZATION ******/
	/*************************************/
	/*************************************/
	GLuint texHandle_ColorScaleGrid;	///< handle for the uploaded color scale
	GLuint texHandle_ColorScaleGridAlternate;	 ///< handle for the uploaded color scale
	VolMesh volumetricMesh;
	GLuint vboHandle_Texture3D_VertPos;
	GLuint vboHandle_Texture3D_VertNorm;
	GLuint vboHandle_Texture3D_VertTex;
	GLuint vboHandle_Texture3D_VertIdx;
	// float* visibleDomains;	  ///< Array deciding which values are visible
	// float* visibleDomainsAlternate;	   ///< Array deciding which values are visible

	GLuint texHandle_colorScale_greyscale;
	GLuint texHandle_colorScale_hsv2rgb;
	GLuint texHandle_colorScale_user0;
	GLuint texHandle_colorScale_user1;

	/// @brief Generate the default color scales' data, and upload them.
	void generateColorScales();

	bool shouldUpdateUserColorScales;	 ///< Should we update the color scale data next drawcall ?
	bool shouldUpdateUBOData;	 ///< Should we update all UBO data next drawcall ?

	/********************************************/
	/* Threaded loading of high-resolution grid */
	/********************************************/
	std::mutex mutexout;
	std::vector<IO::ThreadedTask::Ptr> tasks;
	std::vector<std::shared_ptr<std::thread>> runningThreads;
	QTimer* timer_refreshProgress;
	QProgressBar* pb_loadProgress;
	bool isFinishedLoading;

	/// @brief Prints all uniforms contained in the compiled program.
	/// @details  Simple print debug for a shader's uniforms, notably to see which uniforms are available after
	/// the GLSL compiler's optimization. Also prints all subparts of uniforms (array indices for arrays, and
	/// fields for user-defined structs). Does _not_ print the attributes.
	void printAllUniforms(GLuint _shader_program);

	/********************************************/
	/*    Keeps track of the limits of the GL   */
	/********************************************/
	GLint gl_limit_max_texture_size;

public:
	SceneGL sceneGL;
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
