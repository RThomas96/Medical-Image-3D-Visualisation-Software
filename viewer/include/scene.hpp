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
// Qt headers :
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLDebugLogger>
#include <QStatusBar>
// libQGLViewer :
#include <QGLViewer/qglviewer.h>
// glm include :
#include <glm/glm.hpp>
// STD headers :
#include <vector>
#include <mutex>

#define PLANE_POS_FLOOR

class ControlPanel; // Forward declaration

enum DrawMode { Solid, Volumetric, VolumetricBoxed };
enum planes { x = 1, y = 2, z = 3 };
enum planeHeading { North = 0, East = 1, South = 2, West = 3, Up = North, Right = East, Down = South, Left = West };

class Scene : public QOpenGLFunctions_4_0_Core {
		typedef glm::vec<4, unsigned int, glm::defaultp> uvec4;
		typedef glm::uvec3 uvec3;
	public:
		Scene(); ///< default constructor
		~Scene(void); ///< default destructor

		/// @brief initialize the variables of the scene
		void initGl(QOpenGLContext* context);

		/// @brief Show the Visualization box controller
		void showVisuBoxController();
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

		/// @b Adds a grid to the list of grids present and to be drawn, and generates the data structure to visualize it.
		void addGrid(const std::shared_ptr<InputGrid> _grid, std::string meshPath);
		/// @b Adds a grid to the list of grids present and to be drawn, which is composed of two separate grids' data.
		void addTwoGrids(const std::shared_ptr<InputGrid> _gridR, const std::shared_ptr<InputGrid> _gridG, std::string meshPath);

		/// @b Draw the 3D view of the scene.
		void draw3DView(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, bool showTexOnPlane = true);

		/// @b Draw a given plane 'view' (single plane on the framebuffer).
		void drawPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading, float zoomRatio, glm::vec2 offset);

		/// @b Returns the current scene boundaries.
		glm::vec3 getSceneBoundaries() const;

		/// @b Set the draw mode for the 3D view of the scene.
		void setDrawMode(DrawMode _mode);

		/// @b Launches a save dialog, to generate a grid.
		void launchSaveDialog();

		/// @b Deletes a grid from the array of grids to show
		void deleteGrid(const std::shared_ptr<DiscreteGrid>& _grid);

		/// @brief Prints info about the VAO on next refresh
		void printVAOStateNext() { this->showVAOstate = true; }

		/// @brief Get the minimum texture value to represent
		DiscreteGrid::data_t getMinTexValue(void) const { return this->minTexVal; }
		/// @brief Get the maximum texture value to represent
		DiscreteGrid::data_t getMaxTexValue(void) const { return this->maxTexVal; }

		/// @brief Get the minimum color value, for the color scale resizing.
		uint getMinColorValue(void) const { return this->minColorVal; }
		/// @brief Get the maximum color value, for the color scale resizing.
		uint getMaxColorValue(void) const { return this->maxColorVal; }

		/// @brief Returns the current visu box
		DiscreteGrid::bbox_t getVisuBox(void) { return this->visuBox; }
		/// @brief Sets the visu box
		void setVisuBox(DiscreteGrid::bbox_t box);
		/// @brief Resets the visu box
		void resetVisuBox();

		/// @brief Get the scene radius at this time
		float getSceneRadius();
		/// @brief Get the scene center at this time
		glm::vec3 getSceneCenter();

		/// @brief Upload a 1D texture with the given parameters.
		GLuint uploadTexture1D(const TextureUpload& tex);
		/// @brief Upload a 2D texture with the given parameters.
		GLuint uploadTexture2D(const TextureUpload& tex);
		/// @brief Upload a 3D texture with the given parameters.
		GLuint uploadTexture3D(const TextureUpload& tex);

		/// @brief (Obsolete) Function to write the generated output grid as a DIM/IMA file
		void writeGridDIM(const std::string name);

		/// @brief Draft function to write the loaded input grid as a DIM/IMA file
		void draft_writeRawGridPortion(DiscreteGrid::sizevec3 begin, DiscreteGrid::sizevec3 size, std::string name, const std::shared_ptr<DiscreteGrid>& _grid);

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
		/// @brief Set minimum color intensity.
		void slotSetMinColorValue(DiscreteGrid::data_t val);
		/// @brief Set maximum color intensity.
		void slotSetMaxColorValue(DiscreteGrid::data_t val);
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

		/// @b computes the transformation matrix of the input grid
		glm::mat4 computeTransformationMatrix(const std::shared_ptr<DiscreteGrid>& _grid) const;
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

		/// @b Sets up the OpenGL debug log.
		void setupGLOutput();
		/// @b Print the OpenGL message to std::cerr, if no OpenGLDebugLogMessages are enabled.
		void printOpenGLMessage(const QOpenGLDebugMessage& message);

		/// @b preps uniforms for a grid
		void prepGridUniforms(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, const GridGLView& grid);
		/// @b preps uniforms for a given plane
		void prepPlaneUniforms(GLfloat *mvMat, GLfloat *pMat, planes _plane, const GridGLView& grid, bool showTexOnPlane = true);
		/// @brief prep the plane uniforms to draw in space
		void prepPlane_SingleUniforms(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, glm::vec2 offset, const GridGLView& _grid);

		/// @brief draw the planes, in the real space
		void drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane = true);
		/// @b draws a grid, slightly more generic than drawVoxelGrid()
		void drawGrid(GLfloat mvMat[], GLfloat pMat[], glm::mat4 baseMatrix, const GridGLView& grid);
		/// @brief Draws the 3D texture with a volumetric-like visualization method
		void drawVolumetric(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const GridGLView& grid);

		/// @b Prints grid info.
		void printGridInfo(const std::shared_ptr<DiscreteGrid>& grid);

		/// @b Generate a scale of colors for the program.
		void generateColorScale();
		/// @b Uploads the color scale to OpenGL
		void uploadColorScale();

		/// @b Prints the accessible uniforms and attributes of the given program.
		void printProgramUniforms(const GLuint _pid);

		/// @b Updates the visibility array to only show values between the min and max tex values
		void updateVis();

		/// @b Creates the VBO/VAO handles for bounding boxes
		void createBoundingBoxBuffers();
		/// @b Orders the VAO pointers for the bounding box
		void setupVAOBoundingBox();
		/// @b Draw a bounding box
		void drawBoundingBox(const DiscreteGrid::bbox_t& _box, glm::vec3 color, GLfloat* vMat, GLfloat* pMat);
		/// @b Update the scene's bounding box with the currently drawn grids.
		void updateBoundingBox(void);

		/*************************************/
		/*************************************/
		/****** TEXTURE3D VISUALIZATION ******/
		/*************************************/
		/*************************************/
		void tex3D_buildTexture();
		void tex3D_buildMesh(GridGLView& grid, const std::string path = "");
		void tex3D_buildVisTexture(VolMesh& volMesh);
		void tex3D_buildBuffers(VolMesh& volMesh);
		void tex3D_bindVAO();
		void tex3D_loadMESHFile(const std::string name, const GridGLView& grid, VolMeshData& _mesh);
		void tex3D_generateMESH(const GridGLView& grid, VolMeshData& _mesh);
	protected:
		bool isInitialized;	///< tracks if the scene was initialized or not
		bool inputGridVisible;	///< does the user want to show the input grid ?
		bool outputGridVisible;	///< does the user want to show the output grid ?
		bool colorOrTexture;	///< do we use the RGB2HSV function or the color scale ?
		bool showVAOstate;	///< Do we need to print the VAO/program state on next draw ?

		std::vector<GridGLView> grids;		///< Grids to display in the different views.

		QOpenGLContext* context;		///< The context with which the scene has been created with
		QOpenGLDebugLogger* debugLog;		///< The debug log reading messages from the GL_KHR_debug extension
		ControlPanel* controlPanel;		///< pointer to the control panel
		VisuBoxController* visuBoxController;	///< The controller for the visualization box
		std::shared_ptr<OutputGrid> outputGrid; ///< output grid
		GridControl* gridControl;		///< The controller for the grid 'save' feature (generation)
		OpenGLDebugLog* glOutput;		///< Output of the GL log.
		QStatusBar* programStatusBar;		///< Status bar to show some info about the program.

		DiscreteGrid::data_t minTexVal;			///< The minimum texture intensity to display
		DiscreteGrid::data_t maxTexVal;			///< The maximum texture intensity to display
		DiscreteGrid::data_t minColorVal;			///< The minimum color intensity to use for the color computation
		DiscreteGrid::data_t maxColorVal;			///< The maximum color intensity to use for the color computation
		std::size_t renderSize;			///< Number of primitives to render for the solid view mode.

		std::array<glm::vec3, 8> lightPositions; ///< Scene lights (positionned at the corners of the scene BB)

		glm::vec<3, bool, glm::defaultp> planeVisibility; ///< Should we show each plane (X, Y, Z)
		glm::vec3 planeDirection;		///< Cutting plane directions (-1 or 1 on each axis)
		glm::vec3 planeDisplacement;		///< %age of the scene bounding box to place the planes
		DiscreteGrid::bbox_t sceneBB;		///< Outer BB of the scene
		float clipDistanceFromCamera;		/// Distance from the camera to its clip plane
		DrawMode drawMode;			///< Current 3D draw mode
		DiscreteGrid::bbox_t visuBox;		///< Used to restrict the view to a box with its coordinates

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
		GLuint texHandle_ColorScaleGrid;		///< handle for the uploaded color scale
		bool shouldUpdateVis;
		VolMesh volumetricMesh;
		GLuint vboHandle_Texture3D_VertPos;
		GLuint vboHandle_Texture3D_VertNorm;
		GLuint vboHandle_Texture3D_VertTex;
		GLuint vboHandle_Texture3D_VertIdx;
		float* visibleDomains;			///< Array deciding which values are visible

};

inline int __GetOpenGLError ( char* szFile, int iLine );

inline unsigned int planeHeadingToIndex(planeHeading _heading);

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

#if not defined( NDEBUG )
	#define GetOpenGLError() __GetOpenGLError( ( char* )__FILE__, ( int )__LINE__ )
#else
	#define GetOpenGLError()
#endif

#endif // VIEWER_INCLUDE_SCENE_HPP_

// vim : ts=8
