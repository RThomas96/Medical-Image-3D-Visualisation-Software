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
// Qt headers :
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>
// STD headers :
#include <vector>
#include <mutex>

class ControlPanel; // Forward declaration

enum DrawMode { Solid, SolidAndWireframe, Wireframe };
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

		/// @brief For the dual-viewer : draw in real space
		void drawGridOnly(GLfloat mvMat[], GLfloat pMat[]);
		/// @brief draw the planes, in the real space
		void drawPlanes(GLfloat mvMat[], GLfloat pMat[], bool showTexOnPlane = true);

		/// @brief For the dual-viewer : draw in grid space
		void drawWithPlanes(GLfloat mvMat[], GLfloat pMat[]);
		/// @b Draw a given plane 'view' (single plane on the framebuffer).
		void drawPlaneView(glm::vec2 fbDims, planes _plane, planeHeading _heading);

		/// @brief Draws the 3D texture with a volumetric-like visualization method
		void drawVolumetric(GLfloat mvMat[], GLfloat pMat[], glm::vec3 camPos);

		/// @brief load the 3D texture to opengl
		void loadImage();
		/// @brief load the voxel grid generated
		void loadVoxelGrid();

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

		glm::vec3 getSceneBoundaries(bool realSpace = true) const;

		void setDrawModeSolid() { this->drawMode = DrawMode::Solid; }
		void setDrawModeSolidAndWireframe() { this->drawMode = DrawMode::SolidAndWireframe; }
		void setDrawModeWireframe() { this->drawMode = DrawMode::Wireframe; }

		void cleanup(void); ///< cleanup function for vbo and other parts
		void printVAOStateNext() { this->showVAOstate = true; } ///< prints info about the VAO on next refresh
		glm::vec3 getPlanePositions(void) { return this->planePosition; } ///< Get the cutting planes' positions
		uint getMinTexValue(void) const { return this->minTexVal; }
		uint getMaxTexValue(void) const { return this->maxTexVal; }

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
	private :
		/// @b compile the given shader at 'path' as a shader of type 'shaType'
		GLuint compileShader(const std::string& path, const GLenum shaType, bool verbose = false);
		/// @b Create and link a program, with the given (valid) shader IDs.
		GLuint compileProgram(const GLuint vSha = 0, const GLuint gSha = 0, const GLuint fSha = 0, bool verbose = false);
		/// @b Compile the given shaders, and return the ID of the program generated. On any error, returns 0.
		GLuint compileShaders(std::string vPath, std::string gPath, std::string fPath, bool verbose = false);
		/// @b Generates the vertices, normals, and tex coordinates for a basic unit cube
		void generateTexCube(std::vector<glm::vec4>& vertPos, std::vector<glm::vec4>& vertNorm, std::vector<glm::vec3>& vertTex, std::vector<unsigned int>& vertIdx);
		/// @b Generates the plane's vertices array indexes
		void generatePlanesArray(std::vector<unsigned int>& idx);
		/// @b computes the transformation matrix of the input grid
		glm::mat4 computeTransformationMatrix() const;
		/// @b preps uniforms for a grid
		void prepGridUniforms(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid>& grid);
		/// @b draws a grid, slightly more generic than drawVoxelGrid()
		void drawGrid_Generic(GLfloat mvMat[], GLfloat pMat[], glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid>& grid);
		/// @b preps uniforms for a given plane
		void prepPlaneUniforms(GLfloat *mvMat, GLfloat *pMat, planes _plane, bool showTexOnPlane = true);
		/// @brief prep the plane uniforms to draw in space
		void prepPlane_SingleUniforms(planes _plane, planeHeading _heading, glm::vec2 fbDims, const std::shared_ptr<DiscreteGrid> _grid);
		/// @b Prints grid info.
		void printGridInfo(const std::shared_ptr<DiscreteGrid>& grid);
		/// @b Generate a scale of colors for the program.
		std::vector<float> generateColorScale(std::size_t minVal, std::size_t maxVal);
		/// @b Uploads the color scale to OpenGL
		void uploadColorScale(const std::vector<float>& colorScale);
		/// @b setup the buffers' data
		void setupVBOData(const std::vector<glm::vec4>& vertPos, const std::vector<glm::vec4>& vertNorm, const std::vector<glm::vec3>& vertTex, const std::vector<unsigned int>& vertIdx, const std::vector<unsigned int>& vertIdx_plane);
		/// @b setup the vao binding setup
		void setupVAOPointers();
		/// @b bind the textures to use them later
		void bindTextures();
		/// @b Prints the accessible uniforms and attributes of the given program.
		void printProgramUniforms(const GLuint _pid);
		/// @b Updates the visibility array to only show values between the min and max tex values
		void updateVis();

		/// @b Creates the VBO/VAO handles for bounding boxes
		void createBoundingBoxBuffers();
		/// @b Updates the VBO/VAO buffers of bounding boxes with new vertices
		void updateBoundingBoxPositions(const DiscreteGrid::bbox_t& _box);
		/// @b Draw a bounding box
		void drawBoundingBox(const DiscreteGrid::bbox_t& _box, GLfloat* vMat, GLfloat* pMat);

		/*************************************/
		/*************************************/
		/****** TEXTURE3D VISUALIZATION ******/
		/*************************************/
		/*************************************/
		void tex3D_buildTexture();
		void tex3D_buildMesh();
		void tex3D_buildVisTexture();
		void tex3D_buildBuffers();
		void tex3D_bindVAO();
		void tex3D_loadMESHFile(const std::string name, std::vector<glm::vec4>& vert, std::vector<glm::vec3>& texCoords, std::vector<std::array<std::size_t, 4>>& tet);
	public:
		bool isInitialized; ///< tracks if the scene was initialized or not (query-able from anywhere)
	protected:
		void generateGrid();

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

		std::size_t gridWidth; ///< grid size
		std::size_t gridHeight; ///< grid size
		std::size_t gridDepth; ///< grid size

		std::size_t renderSize;
		bool inputGridVisible; ///< does the user want to show the input grid ?
		bool outputGridVisible; ///< does the user want to show the output grid ?
		bool colorOrTexture; ///< do we use the RGB2HSV function or the color scale ?
		uchar minTexVal;
		uchar maxTexVal;
		uchar minColorVal;
		uchar maxColorVal;

		std::array<glm::vec3, 8> lightPositions; ///< Scene lights (positionned at every corner of the scene BB)

		glm::vec3 planePosition;
		glm::vec3 planeDirection;
		glm::vec3 planeDisplacement;
		DiscreteGrid::bbox_t sceneBB;
		glm::vec3 sceneBBPosition;
		glm::vec3 sceneBBDiag;
		float clipDistanceFromCamera;
		DrawMode drawMode;
		bool showVAOstate;

		// VBO/VAO handles :
		GLuint vboHandle_VertPos;
		GLuint vboHandle_VertNorm;
		GLuint vboHandle_VertTex;
		GLuint vboHandle_Element;
		GLuint vboHandle_PlaneElement;
		GLuint vboHandle_boundingBoxVertices;
		GLuint vboHandle_boundingBoxIndices;
		GLuint vaoHandle;
		GLuint vaoHandle_VolumetricBuffers;
		GLuint vaoHandle_boundingBox;

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
		GLuint texHandle_InputGrid;			///< handle for glTexImage3D
		GLuint texHandle_OutputGrid;			///< handle for the voxel grid's data
		GLuint texHandle_ColorScaleGrid;		///< handle for the uploaded color scale
		GLuint texHandle_tetrahedraNeighborhood;	///< handle for tetrhedra neighbors' texture
		GLuint texHandle_tetrahedraFaceNormals;		///< handle for the per-face normals of each tetrahedra
		GLuint texHandle_tetrahedraVertexPositions;	///< vertex positions for the tetrahedra
		GLuint texHandle_tetrahedraVertexTexCoords;	///< vertex positions for the tetrahedra
		GLuint texHandle_visibilityMap;			///< texture for visibility
		GLuint vboHandle_Texture3D_VertPos;
		GLuint vboHandle_Texture3D_VertNorm;
		GLuint vboHandle_Texture3D_VertTex;
		GLuint vboHandle_Texture3D_VertIdx;
		unsigned int* visibleDomains;			///< Array deciding which values are visible
		GLsizei tetCount;
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
