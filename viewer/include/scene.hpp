#ifndef VIEWER_INCLUDE_SCENE_HPP_
#define VIEWER_INCLUDE_SCENE_HPP_

// Program-wide features and macros
#include "../../macros.hpp"
#include "../../features.hpp"
// Scene control panel :
#include "../../qt/include/scene_control.hpp"
// Discrete grid and grid generation :
#include "../../grid/include/tetmesh.hpp"
#include "../../grid/include/discrete_grid.hpp"
#include "../../grid/include/input_discrete_grid.hpp"
#include "../../grid/include/output_discrete_grid.hpp"
// UI elements :
#include "../../qt/include/grid_control.hpp"
#include "../../qt/include/grid_detailed_view.hpp"
#include "../../qt/include/grid_list_view.hpp"
#include "../../qt/include/visu_box_controller.hpp"
#include "../../qt/include/opengl_debug_log.hpp"
// Helper structs and functions :
#include "./viewer_structs.hpp"
// New grid API :
#include "../../image/api/include/grid.hpp"
#include "../../image/tiff/include/tiff_backend.hpp"
// Qt headers :
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLDebugLogger>
#include <QStatusBar>
#include <QProgressBar>
// libQGLViewer :
#include <QGLViewer/qglviewer.h>
// glm include :
#include <glm/glm.hpp>
// STD headers :
#include <vector>
#include <mutex>

#define PLANE_POS_FLOOR

class ControlPanel; // Forward declaration
class VolumetricGridViewer; /// fwd-decl

/// @b Simple enum to keep track of the different viewing primitives for the program.
enum DrawMode { Solid, Volumetric, VolumetricBoxed };
/// @b The RGB mode chosen by the user
enum RGBMode { None = 0, RedOnly = 1, GreenOnly = 2, RedAndGreen = 3, HandEColouring = 4};
/// @b Simple enum to keep track of which color function to apply to the viewers.
enum ColorFunction { SingleChannel, HistologyHandE, HSV2RGB, ColorMagnitude };
/// @b Simple enum to define which plane we are drawing
enum planes { x = 1, y = 2, z = 3 };
/// @b Simple enum to keep track of a plane's orientation.
enum planeHeading { North = 0, East = 1, South = 2, West = 3, Up = North, Right = East, Down = South, Left = West };

class Scene : public QOpenGLFunctions_3_2_Core {
		typedef glm::vec<4, unsigned int, glm::defaultp> uvec4;
		typedef glm::uvec3 uvec3;
	public:
		Scene(); ///< default constructor
		~Scene(void); ///< default destructor

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
		/// @b Remove the grid controller pointer from the scene class.
		void removeController();
		/// @brief reload the default shader files
		void recompileShaders(bool verbose = true);

		/// @b Loads a designated ROI in high-resolution
		void loadGridROI(void);
		/// @b Adds a grid to the list of grids present and to be drawn, and generates the data structure to visualize it.
		void addGrid(const std::shared_ptr<DiscreteGrid> _grid, std::string meshPath);
		/// @b Adds a grid to the list of grids present and to be drawn, which is composed of two separate grids' data.
		void addTwoGrids(const std::shared_ptr<DiscreteGrid> _gridR, const std::shared_ptr<DiscreteGrid> _gridG, std::string meshPath);

		/// @b Add grids, with the new API (more streamlined)
		void newAPI_addGrid(Image::Grid::Ptr gridLoaded);

		/// @b Computes the planes positions based on their parameters.
		glm::vec3 computePlanePositions();
		/// @b Returns the plane directions for outside use
		glm::vec3 getPlaneDirections() const;
		/// @b Get the lights, from outside.
		const std::vector<glm::vec3>& getLights() const;

		/// @b return the ids of the color scales, for now
		/// @note will switch over to the color scale manager once this is over.
		glm::tvec4<GLuint> draft_getGeneratedColorScales();

		/// @b Draw the 3D view of the scene.
		void draw3DView(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, bool showTexOnPlane = true);

		/// @b Draw a given plane 'view' (single plane on the framebuffer).
		void drawPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading, float zoomRatio, glm::vec2 offset);

		/// @b Create a texture suited for framebuffer rendering, by passing the dimensions of it to the function.
		GLuint updateFBOOutputs(glm::ivec2 dimensions, GLuint fb_handle, GLuint old_texture = 0);

		/// @b Read the specified texture 'tex_handl', at coordinates 'image_coordinates' and return the RGB[A] value
		glm::vec4 readFramebufferContents(GLuint fb_handle, glm::ivec2 image_coordinates);

		/// @b Returns the current scene boundaries.
		glm::vec3 getSceneBoundaries() const;

		/// @b Set the draw mode for the 3D view of the scene.
		void setDrawMode(DrawMode _mode);

		/// @b Launches a save dialog, to generate a grid.
		void launchSaveDialog();
		/// @b Adds a 'dummy' grid, to draw its bounding box only.
		void addDummyGrid(std::shared_ptr<DiscreteGrid>& _grid);

		/// @b Deletes a grid from the array of grids to show
		void deleteGrid(const std::shared_ptr<DiscreteGrid>& _grid);

		/// @brief Prints info about the VAO on next refresh
		void printVAOStateNext() { this->showVAOstate = true; }

		/// @brief Get the minimum texture value to represent
		DiscreteGrid::data_t getMinTexValue(void) const { return this->textureBounds0.x; }
		/// @brief Get the maximum texture value to represent
		DiscreteGrid::data_t getMaxTexValue(void) const { return this->textureBounds0.y; }
		/// @brief Get the minimum texture value to represent
		DiscreteGrid::data_t getMinTexValueAlternate(void) const { return this->textureBounds1.x; }
		/// @brief Get the maximum texture value to represent
		DiscreteGrid::data_t getMaxTexValueAlternate(void) const { return this->textureBounds1.y; }

		/// @brief Get the minimum color value, for the color scale resizing.
		uint getMinColorValue(void) const { return this->colorBounds0.x; }
		/// @brief Get the maximum color value, for the color scale resizing.
		uint getMaxColorValue(void) const { return this->colorBounds0.y; }
		/// @brief Get the minimum color value, for the color scale resizing.
		uint getMinColorValueAlternate(void) const { return this->colorBounds1.x; }
		/// @brief Get the maximum color value, for the color scale resizing.
		uint getMaxColorValueAlternate(void) const { return this->colorBounds1.y; }

		/// @brief Returns the current visu box
		DiscreteGrid::bbox_t getVisuBox(void);
		std::pair<glm::uvec3, glm::uvec3> getVisuBoxCoordinates(void);
		/// @brief Sets the visu box
		void setVisuBox(DiscreteGrid::bbox_t box);
		void setVisuBoxMinCoord(glm::uvec3 coor_min);
		void setVisuBoxMaxCoord(glm::uvec3 coor_max);
		/// @brief Resets the visu box
		void resetVisuBox();

		/// @brief Get the scene radius at this time
		float getSceneRadius();
		/// @brief Get the scene center at this time
		glm::vec3 getSceneCenter();
		/// @brief Returns the current scene bounding box.
		DiscreteGrid::bbox_t getSceneBoundingBox() const;

		/// @brief Upload a 3D texture with the given parameters.
		GLuint newAPI_uploadTexture3D(const GLuint handle, const TextureUpload& tex, std::size_t s, std::vector<std::uint16_t> & data);
		GLuint newAPI_uploadTexture3D_allocateonly(const TextureUpload& tex);
		/// @brief Upload a 3D texture with the given parameters.
		GLuint uploadTexture3D_iterative(const TextureUpload& tex, const std::shared_ptr<DiscreteGrid>&, const std::shared_ptr<DiscreteGrid>&);
		/// @brief Tests the texture upload capabilities of OpenGL
		GLuint testTextureUpload(GLuint nbTex, DiscreteGrid::sizevec3 dims);
		/// @brief Inserts a debug message from OpenGL directly to stderr
		void openGLDebugLogger_inserter(const QOpenGLDebugMessage m);

		/// @b Changes the texture coloration mode to the desired setting
		void setColorFunction_r(ColorFunction _c);
		void setColorFunction_g(ColorFunction _c);

		/// @b Changes the RGB mode of the scene.
		void setRGBMode(RGBMode _mode);

		/// @b Set the color of the beginning of the color segment for the segmented color scale
		void setColor0(qreal r, qreal g, qreal b);
		/// @b Set the color of the beginning of the color segment for the segmented color scale
		void setColor1(qreal r, qreal g, qreal b);
		/// @b Set the color of the beginning of the color segment for the segmented color scale
		void setColor0Alternate(qreal r, qreal g, qreal b);
		/// @b Set the color of the beginning of the color segment for the segmented color scale
		void setColor1Alternate(qreal r, qreal g, qreal b);

		/// @b Create a uniform buffer of size `size_bytes` and fill it with null data.
		GLuint createUniformBuffer(std::size_t size_bytes, GLenum draw_mode);
		/// @b Overwrite data at position 'begin_bytes' with 'size_bytes' of 'data'.
		void setUniformBufferData(GLuint uniform_buffer, std::size_t begin_bytes, std::size_t size_bytes, GLvoid* data);

		/// @brief Set X's plane displacement within the bounding box to be `scalar`
		void slotSetPlaneDisplacementX(float scalar);
		/// @brief Set Y's plane displacement within the bounding box to be `scalar`
		void slotSetPlaneDisplacementY(float scalar);
		/// @brief Set Z's plane displacement within the bounding box to be `scalar`
		void slotSetPlaneDisplacementZ(float scalar);

		/// @brief Set minimum texture intensity.
		void slotSetMinTexValue(DiscreteGrid::data_t val);
		/// @brief Set maximum texture intensity.
		void slotSetMaxTexValue(DiscreteGrid::data_t val);
		/// @brief Set minimum texture intensity.
		void slotSetMinTexValueAlternate(DiscreteGrid::data_t val);
		/// @brief Set maximum texture intensity.
		void slotSetMaxTexValueAlternate(DiscreteGrid::data_t val);
		/// @brief Set minimum color intensity.
		void slotSetMinColorValue(DiscreteGrid::data_t val);
		/// @brief Set maximum color intensity.
		void slotSetMaxColorValue(DiscreteGrid::data_t val);
		/// @brief Set minimum color intensity.
		void slotSetMinColorValueAlternate(DiscreteGrid::data_t val);
		/// @brief Set maximum color intensity.
		void slotSetMaxColorValueAlternate(DiscreteGrid::data_t val);

		/// @brief set the clip plane distance from camera to be `val`
		void slotSetClipDistance(double val) { this->clipDistanceFromCamera = static_cast<float>(val); return; }

		/// @brief Get clipping distance from camera
		float getClipDistance(void) { return this->clipDistanceFromCamera; }

		/// @b Toggles the visibility of the plane in argument
		void togglePlaneVisibility(planes _plane);
		/// @b Signals all planes they need to be to be [in]visible.
		void toggleAllPlaneVisibilities(void);

		/// @b Sets the new 'heading' of the plane (to rotate the planar viewers in the framebuffer)
		void setPlaneHeading(planes _plane, planeHeading _heading);

		/// @b Toggles visibility of plane X.
		void slotTogglePlaneDirectionX();
		/// @b Toggles visibility of plane Y.
		void slotTogglePlaneDirectionY();
		/// @b Toggles visibility of plane Z.
		void slotTogglePlaneDirectionZ();
		/// @b Signals all planes they need to be inverted.
		void toggleAllPlaneDirections();

		std::vector<std::shared_ptr<DiscreteGrid>> getInputGrids(void) const;
		std::size_t getInputGridCount(void) const;

		QOpenGLContext* get_context() const { return this->context; }

		/// @b computes the transformation matrix of the input grid
		glm::mat4 computeTransformationMatrix(const std::shared_ptr<DiscreteGrid>& _grid) const;

		/// @b Checks if the scene is already initialized.
		bool isSceneInitialized(void) const { return this->isInitialized; }

		/// @b Returns the volumetric GL program handle.
		GLuint getVolumetricProgram(void);
		/// @b Returns the handle for the solid viewing program
		GLuint getSolidProgram(void);
		/// @b Returns the handle for the 3D plane program
		GLuint get3DPlaneProgram(void);
		/// @b Returns the handle for the texture explorer program
		GLuint getTextureExplorerProgram(void);

		/// @b Binds a given program instance.
		void useProgram(GLuint _program_handle);

		/// @b Find the given uniform name in the given program.
		GLint findUniform(GLuint _program_handle, const char* _uniform_name);

	private :
		/// @b compile the given shader at 'path' as a shader of type 'shaType'
		GLuint compileShader(const std::string& path, const GLenum shaType, bool verbose = false);
		/// @b Create and link a program, with the given (valid) shader IDs.
		GLuint compileProgram(const GLuint vSha = 0, const GLuint gSha = 0, const GLuint fSha = 0, bool verbose = false);
		/// @b Compile the given shaders, and return the ID of the program generated. On any error, returns 0.
		GLuint compileShaders(std::string vPath, std::string gPath, std::string fPath, bool verbose = false);

		/// @b Creates all the VAO/VBO handles
		void createBuffers();
		/// @b Generate the unit cube used to draw the grid in non-volumetric mode, as well as the plane positions and bounding box buffers.
		void generateSceneData();
		/// @b Generates the vertices, normals, and tex coordinates for a basic unit cube
		void generateTexCube(Mesh& _mesh);
		/// @b Generates the plane's vertices array indexes
		void generatePlanesArray(Mesh& _mesh);
		/// @b setup the buffers' data
		void setupVBOData(const Mesh& _mesh);
		/// @b setup the vao binding setup
		void setupVAOPointers();

		/// @b Immediate deletion of the grid index to delete
		void deleteGridNow();

		/// @b Sets up the OpenGL debug log.
		void setupGLOutput();
		/// @b Print the OpenGL message to std::cerr, if no OpenGLDebugLogMessages are enabled.
		void printOpenGLMessage(const QOpenGLDebugMessage& message);

		/// Updates the progress bar added to the main statusbar
		void updateProgressBar();

		/// @b Test function to print all the new uniforms in the
		void newSHADERS_print_all_uniforms(GLuint program);

		/// @b Create the color scales used for the program.
		void newSHADERS_generateColorScales(void);

		/// @b Update the user-defined color scales
		void newSHADERS_updateUserColorScales();
		/// @b Signals to update user-defined color scales whenever is next appropriate.
		void signal_updateUserColorScales();

		void newSHADERS_updateUBOData();

		/// @b preps uniforms for a grid
		void prepareUniforms_3DSolid(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, const GridGLView::Ptr& grid);
		void newAPI_prepareUniforms_3DSolid(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, const NewAPI_GridGLView::Ptr& grid);
		/// @b preps uniforms for a given plane
		void prepareUniforms_3DPlane(GLfloat *mvMat, GLfloat *pMat, planes _plane, const GridGLView::Ptr& grid, bool showTexOnPlane = true);
		void newAPI_prepareUniforms_3DPlane(GLfloat *mvMat, GLfloat *pMat, planes _plane, const NewAPI_GridGLView::Ptr& grid, bool showTexOnPlane = true);
		/// @brief prep the plane uniforms to draw in space
		void prepareUniforms_PlaneViewer(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, glm::vec2 offset, const GridGLView::Ptr& _grid);
		void newAPI_prepareUniforms_PlaneViewer(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, glm::vec2 offset, const NewAPI_GridGLView::Ptr& _grid);
		/// @brief Prepare the uniforms for volumetric drawing
		void prepareUniforms_Volumetric(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, const GridGLView::Ptr& _grid);
		void newAPI_prepareUniforms_Volumetric(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, const NewAPI_GridGLView::Ptr& _grid);

		/// @brief draw the planes, in the real space
		void drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane = true);
		/// @brief draw the planes, in the real space using the new api
		void newAPI_drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane = true);
		/// @b draws a grid, slightly more generic than drawVoxelGrid()
		void drawGrid(GLfloat mvMat[], GLfloat pMat[], glm::mat4 baseMatrix, const GridGLView::Ptr& grid);
		void newAPI_drawGrid(GLfloat mvMat[], GLfloat pMat[], glm::mat4 baseMatrix, const NewAPI_GridGLView::Ptr& grid);
		/// @brief Draws the 3D texture with a volumetric-like visualization method
		void drawVolumetric(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const GridGLView::Ptr& grid);
		void newAPI_drawVolumetric(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const NewAPI_GridGLView::Ptr& grid);

		/// @b Prints grid info.
		void printGridInfo(const std::shared_ptr<DiscreteGrid>& grid);

		/// @b Generate a scale of colors for the program.
		void generateColorScale();
		/// @b Uploads the color scale to OpenGL
		void uploadColorScale();
		/// @b Returns an unsigned int (suitable for uniforms) from a color function
		uint colorFunctionToUniform(ColorFunction _c);

		/// @b Prints the accessible uniforms and attributes of the given program.
		void printProgramUniforms(const GLuint _pid);

		/// @b Updates the visibility array to only show values between the min and max tex values
		void updateVis();

		/// @b Updates the color and visibility ranges for all visible grids.
		void updateCVR();

		/// @b Creates the VBO/VAO handles for bounding boxes
		void createBoundingBoxBuffers();
		/// @b Orders the VAO pointers for the bounding box
		void setupVAOBoundingBox();
		/// @b Draw a bounding box
		void drawBoundingBox(const DiscreteGrid::bbox_t& _box, glm::vec3 color, GLfloat* vMat, GLfloat* pMat);
		/// @b Update the scene's bounding box with the currently drawn grids.
		void updateBoundingBox(void);
		void updateVisuBoxCoordinates(void);

		void initialize_limits(void);

		/*************************************/
		/*************************************/
		/****** TEXTURE3D VISUALIZATION ******/
		/*************************************/
		/*************************************/
		void tex3D_buildMesh(GridGLView::Ptr& grid, const std::string path = "");
		void newAPI_tex3D_buildMesh(NewAPI_GridGLView::Ptr& grid, const std::string path = "");
		void tex3D_buildVisTexture(VolMesh& volMesh);
		void tex3D_buildBuffers(VolMesh& volMesh);
		void tex3D_bindVAO();
		void tex3D_loadMESHFile(const std::string name, const GridGLView::Ptr& grid, VolMeshData& _mesh);
		void newAPI_tex3D_loadMESHFile(const std::string name, const NewAPI_GridGLView::Ptr& grid, VolMeshData& _mesh);
		void tex3D_generateMESH(GridGLView::Ptr& grid, VolMeshData& _mesh);
		void newAPI_tex3D_generateMESH(NewAPI_GridGLView::Ptr& grid, VolMeshData& _mesh);
	protected:
		bool isInitialized;						///< tracks if the scene was initialized or not
		bool showVAOstate;						///< Do we need to print the VAO/program state on next draw ?
		bool shouldUpdateVis;					///< Should we update visibility on next draw ?
		bool shouldDeleteGrid;					///< Should we delete a grid on next draw ?
		std::vector<std::size_t> delGrid;		///< Grids to delete at next refresh

		// Grids :
		std::vector<GridGLView::Ptr> grids;		///< Grids to display in the different views.
		std::vector<NewAPI_GridGLView::Ptr> newGrids;	///< Grids, but with the new API.

		// OpenGL-related stuff :
		QOpenGLContext* context;				///< The context with which the scene has been created with
		OpenGLDebugLog* glOutput;				///< Output of the GL log.
		QOpenGLDebugLogger* debugLog;			///< The debug log reading messages from the GL_KHR_debug extension

		// Widgets that may interact with the scene :
		GridControl* gridControl;				///< The controller for the grid 'save' feature (generation)
		ControlPanel* controlPanel;				///< pointer to the control panel
		QStatusBar* programStatusBar;			///< Status bar to show some info about the program.
		VisuBoxController* visuBoxController;	///< The controller for the visualization box

		// Render parameters :
		std::size_t renderSize;					///< Number of primitives to render for the solid view mode.
		GridGLView::data_2 textureBounds0;
		GridGLView::data_2 textureBounds1;
		GridGLView::data_2 colorBounds0;
		GridGLView::data_2 colorBounds1;
		std::array<glm::vec3,8> lightPositions;	///< Scene lights (positionned at the corners of the scene BB)

		glm::bvec3 planeVisibility;				///< Should we show each plane (X, Y, Z)
		glm::vec3 planeDirection;				///< Cutting plane directions (-1 or 1 on each axis)
		glm::vec3 planeDisplacement;			///< %age of the scene bounding box to place the planes
		DiscreteGrid::bbox_t sceneBB;			///< Outer BB of the scene
		DiscreteGrid::bbox_t sceneDataBB;		///< Outer BB of the scene's data
		float clipDistanceFromCamera;			/// Distance from the camera to its clip plane
		glm::uvec3 visuMin;						///< The min image coordinate to display on the visu box mode
		glm::uvec3 visuMax;						///< The max image coordinate to display on the visu box mode
		DiscreteGrid::bbox_t visuBox;			///< Used to restrict the view to a box with its coordinates
		DrawMode drawMode;						///< Current 3D draw mode
		RGBMode rgbMode;						///< Current RGB mode
		ColorFunction channels_r;				///< Channel(s) to display on the viewers
		GLuint selectedChannel_r;				///< The currently selected channel for greyscale mode.
		ColorFunction channels_g;				///< Channel(s) to display on the viewers
		GLuint selectedChannel_g;				///< The currently selected channel for greyscale mode.

		glm::vec3 color0;			///< The color segment when approaching 0
		glm::vec3 color1;			///< The color segment when approaching 1
		glm::vec3 color0_second;	///< The color segment when approaching 0
		glm::vec3 color1_second;	///< The color segment when approaching 1

		// VAO handles :
		GLuint vaoHandle;
		GLuint vaoHandle_VolumetricBuffers;
		GLuint vaoHandle_boundingBox;
		// VBO handles :
		GLuint vboHandle_VertPos;
		GLuint vboHandle_VertNorm;
		GLuint vboHandle_VertTex;
		GLuint vboHandle_Element;
		GLuint vboHandle_PlaneElement;
		GLuint vboHandle_SinglePlaneElement;
		GLuint vboHandle_boundingBoxVertices;
		GLuint vboHandle_boundingBoxIndices;

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
		GLuint texHandle_ColorScaleGrid;			///< handle for the uploaded color scale
		GLuint texHandle_ColorScaleGridAlternate;	///< handle for the uploaded color scale
		VolMesh volumetricMesh;
		GLuint vboHandle_Texture3D_VertPos;
		GLuint vboHandle_Texture3D_VertNorm;
		GLuint vboHandle_Texture3D_VertTex;
		GLuint vboHandle_Texture3D_VertIdx;
		float* visibleDomains;					///< Array deciding which values are visible
		float* visibleDomainsAlternate;			///< Array deciding which values are visible

		GLuint texHandle_colorScale_greyscale;
		GLuint texHandle_colorScale_hsv2rgb;
		GLuint texHandle_colorScale_user0;
		GLuint texHandle_colorScale_user1;

		/// @b Generate the default color scales' data, and upload them.
		void generateColorScales();

		bool shouldUpdateUserColorScales; ///< Should we update the color scale data next drawcall ?
		bool shouldUpdateUBOData; ///< Should we update all UBO data next drawcall ?

		/********************************************/
		/* Threaded loading of high-resolution grid */
		/********************************************/
		std::mutex mutexout;
		std::mutex mutexadd;
		std::vector<std::shared_ptr<DiscreteGrid>> gridsToAdd;
		std::vector<IO::ThreadedTask::Ptr> tasks;
		std::vector<std::shared_ptr<std::thread>> runningThreads;
		QTimer* timer_refreshProgress;
		QProgressBar* pb_loadProgress;
		bool isFinishedLoading;
		void replaceGridsWithHighRes();

		/// @b Prints all uniforms contained in the compiled program.
		/// @details  Simple print debug for a shader's uniforms, notably to see which uniforms are available after
		/// the GLSL compiler's optimization. Also prints all subparts of uniforms (array indices for arrays, and
		/// fields for user-defined structs). Does _not_ print the attributes.
		void printAllUniforms(GLuint _shader_program);

		/********************************************/
		/*    Keeps track of the limits of the GL   */
		/********************************************/
		GLint gl_limit_max_texture_size;
};

/// @brief Type-safe conversion of enum values to unsigned ints.
inline unsigned int planeHeadingToIndex(planeHeading _heading);

/// @brief Helper struct to store the indices of vertices that make a face, and compare two faces to one another.
struct Face {
	public:
		inline Face ( unsigned int v0, unsigned int v1, unsigned int v2) {
			if (v1 < v0) std::swap(v0,v1);
			if (v2 < v1) std::swap(v1,v2);
			if (v1 < v0) std::swap(v0,v1);
			v[0] = v0; v[1] = v1; v[2] = v2;
		}
		inline Face (const Face & f) { v[0] = f.v[0]; v[1] = f.v[1]; v[2] = f.v[2]; }
		inline virtual ~Face () {}
		inline Face & operator= (const Face & f) { v[0] = f.v[0]; v[1] = f.v[1]; v[2] = f.v[2]; return (*this); }
		inline bool operator== (const Face & f) { return (v[0] == f.v[0] && v[1] == f.v[1] && v[2] == f.v[2]); }
		inline bool operator< (const Face & f) const { return (v[0] < f.v[0] || (v[0] == f.v[0] && v[1] < f.v[1]) || (v[0] == f.v[0] && v[1] == f.v[1] && v[2] < f.v[2])); }
		inline bool contains (unsigned int i) const { return (v[0] == i || v[1] == i || v[2] == i); }
		inline unsigned int getVertex (unsigned int i) const { return v[i]; }
		unsigned int v[3];
};

#endif // VIEWER_INCLUDE_SCENE_HPP_

// vim : ts=8
