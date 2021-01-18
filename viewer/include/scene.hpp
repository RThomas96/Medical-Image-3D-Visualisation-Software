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
// Helper structs and functions :
#include "./viewer_structs.hpp"
// Qt headers :
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLDebugLogger>
#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>
// STD headers :
#include <vector>
#include <mutex>

#define PLANE_POS_FLOOR

class ControlPanel; // Forward declaration

enum DrawMode { Solid, Volumetric };
enum planes { x = 1, y = 2, z = 3 };
enum planeHeading { North = 0, East = 1, South = 2, West = 3, Up = North, Right = East, Down = South, Left = West };

class Scene : public QOpenGLFunctions_4_0_Core {
		typedef glm::vec<4, unsigned int, glm::defaultp> uvec4;
		typedef glm::uvec3 uvec3;
	public:
		Scene(GridControl* const gc); ///< default constructor
		~Scene(void); ///< default destructor

		/// @brief initialize the variables of the scene
		void initGl(QOpenGLContext* context);

		/// @brief set the control panel responsible for controlling the scene
		void setControlPanel(ControlPanel* cp) { this->controlPanel = cp; }
		/// @brief reload the default shader files
		void recompileShaders(bool verbose = true);

		/// @b Adds a grid to the list of grids present and to be drawn, and generates the data structure to visualize it.
		void addGrid(const std::shared_ptr<InputGrid> _grid, std::string meshPath);

		/// @brief draw the planes, in the real space
		void drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane = true);

		/// @b Draw the 3D view of the scene.
		void draw3DView(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, bool showTexOnPlane = true);

		/// @brief Draws the scene in world-space, along with planes.
		void drawWithPlanes(GLfloat mvMat[], GLfloat pMat[]);
		/// @b Draw a given plane 'view' (single plane on the framebuffer).
		void drawPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading, float zoomRatio, GLfloat* vMat, GLfloat* pMat);

		/// @brief Draws the 3D texture with a volumetric-like visualization method
		void drawVolumetric(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos, const GridGLView& grid);

		/// @brief fill the grid using trilinear interpolation
		void fillTrilinear();
		/// @brief fill the grid using nearest neighbor interpolation
		void fillNearestNeighbor();

		/// @brief show the tex cube or not
		void toggleInputGridVisible(bool visibility) { this->inputGridVisible = visibility; }
		/// @brief show the tex cube or not
		void toggleInputGridVisible() { this->toggleInputGridVisible(!this->inputGridVisible); }
		/// @brief show the tex cube or not
		void toggleOutputGridVisible(bool visibility) { this->outputGridVisible = visibility; }
		/// @brief show the tex cube or not
		void toggleOutputGridVisible() { this->outputGridVisible = not this->outputGridVisible; }

		void toggleColorOrTexture(bool _cOT) { this->colorOrTexture = _cOT; }
		void toggleColorOrTexture() { this->toggleColorOrTexture(!this->colorOrTexture); }

		glm::vec3 getSceneBoundaries() const;

		/// @b Set the draw mode for the 3D view of the scene.
		void setDrawMode(DrawMode _mode);

		void cleanup(void); ///< cleanup function for vbo and other parts
		void printVAOStateNext() { this->showVAOstate = true; } ///< prints info about the VAO on next refresh
		glm::vec3 getPlanePositions(void) { return this->planePosition; } ///< Get the cutting planes' positions
		uint getMinTexValue(void) const { return this->minTexVal; }
		uint getMaxTexValue(void) const { return this->maxTexVal; }
		uint getMinColorValue(void) const { return this->minColorVal; }
		uint getMaxColorValue(void) const { return this->maxColorVal; }

		GLuint uploadTexture1D(const TextureUpload& tex);
		GLuint uploadTexture2D(const TextureUpload& tex);
		GLuint uploadTexture3D(const TextureUpload& tex);

		void writeGridDIM(const std::string name);

		void draft_writeRawGridPortion(DiscreteGrid::sizevec3 begin, DiscreteGrid::sizevec3 size, std::string name);

		void slotTogglePolygonMode(bool show);
		void slotToggleShowTextureCube(bool show);
		void slotSetPlaneDisplacementX(float scalar);
		void slotSetPlaneDisplacementY(float scalar);
		void slotSetPlaneDisplacementZ(float scalar);
		void slotTogglePlaneDirectionX();
		void slotTogglePlaneDirectionY();
		void slotTogglePlaneDirectionZ();
		void slotSetMinTexValue(uchar val);
		void slotSetMaxTexValue(uchar val);
		void slotSetMinColorValue(uchar val);
		void slotSetMaxColorValue(uchar val);
		void slotSetPlanePositionX(float coord);
		void slotSetPlanePositionY(float coord);
		void slotSetPlanePositionZ(float coord);
		void slotSetClipDistance(double val) { this->clipDistanceFromCamera = static_cast<float>(val); return; }
		float getClipDistance(void) { return this->clipDistanceFromCamera; }
		/// @b Toggles the visibility of the plane in argument
		void togglePlaneVisibility(planes _plane);
		/// @b Sets the new 'heading' of the plane (to rotate the planar viewers in the framebuffer)
		void setPlaneHeading(planes _plane, planeHeading _heading);
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

		/// @b Print the OpenGL message to std::cerr
		void printOpenGLMessage(const QOpenGLDebugMessage& message);

		/// @b computes the transformation matrix of the input grid
		glm::mat4 computeTransformationMatrix(const std::shared_ptr<DiscreteGrid>& _grid) const;

		/// @b preps uniforms for a grid
		void prepGridUniforms(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid>& grid);
		/// @b draws a grid, slightly more generic than drawVoxelGrid()
		void drawGrid(GLfloat mvMat[], GLfloat pMat[], glm::mat4 baseMatrix, const GridGLView& grid);
		/// @b preps uniforms for a given plane
		void prepPlaneUniforms(GLfloat *mvMat, GLfloat *pMat, planes _plane, const GridGLView& grid, bool showTexOnPlane = true);
		/// @brief prep the plane uniforms to draw in space
		void prepPlane_SingleUniforms(planes _plane, planeHeading _heading, glm::vec2 fbDims, float zoomRatio, const GridGLView& _grid);

		/// @b Prints grid info.
		void printGridInfo(const std::shared_ptr<DiscreteGrid>& grid);

		/// @b Generate a scale of colors for the program.
		std::vector<float> generateColorScale(std::size_t minVal, std::size_t maxVal);
		/// @b Uploads the color scale to OpenGL
		void uploadColorScale(const std::vector<float>& colorScale);

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
	protected:
		bool isInitialized;	///< tracks if the scene was initialized or not
		bool inputGridVisible;	///< does the user want to show the input grid ?
		bool outputGridVisible;	///< does the user want to show the output grid ?
		bool colorOrTexture;	///< do we use the RGB2HSV function or the color scale ?
		bool showVAOstate;	///< Do we need to print the VAO/program state on next draw ?

		std::vector<GridGLView> grids;

		QOpenGLContext* context; ///< The context with which the scene has been created with
		QOpenGLDebugLogger* debugLog; ///< The debug log reading messages from the GL_KHR_debug extension
		ControlPanel* controlPanel; ///< pointer to the control panel
		#ifdef LOAD_RED_AND_BLUE_IMAGE_STACKS
		std::shared_ptr<InputGrid> inputGrid_Blue; ///< input grid (blue channel)
		std::shared_ptr<InputGrid> inputGrid_Red; ///< input grid (red channel)
		#else
		std::shared_ptr<InputGrid> inputGrid; ///< input grid
		#endif
		std::shared_ptr<OutputGrid> outputGrid; ///< output grid
		std::shared_ptr<TetMesh> mesh; ///< creates a mesh around the queried point
		GridControl* gridControl;

		uchar minTexVal;
		uchar maxTexVal;
		uchar minColorVal;
		uchar maxColorVal;
		std::size_t renderSize;

		std::array<glm::vec3, 8> lightPositions; ///< Scene lights (positionned at the corners of the scene BB)

		glm::vec3 planePosition;
		glm::vec3 planeDirection;
		glm::vec3 planeDisplacement;
		DiscreteGrid::bbox_t sceneBB;
		float clipDistanceFromCamera;
		DrawMode drawMode;

		glm::vec<3, bool, glm::defaultp> planeVisibility; ///< Should we show each plane (X, Y, Z)

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
		VolMesh volumetricMesh;
		GLuint vboHandle_Texture3D_VertPos;
		GLuint vboHandle_Texture3D_VertNorm;
		GLuint vboHandle_Texture3D_VertTex;
		GLuint vboHandle_Texture3D_VertIdx;
		unsigned int* visibleDomains;			///< Array deciding which values are visible

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
