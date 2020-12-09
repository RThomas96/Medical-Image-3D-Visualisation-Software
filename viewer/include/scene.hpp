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
#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>
// STD headers :
#include <vector>
#include <mutex>

class ControlPanel; // Forward declaration

enum DrawMode { Solid, SolidAndWireframe, Wireframe };
enum planes { x = 1, y = 2, z = 3 };

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
		void recompileShaders(void);

		/// @brief For the dual-viewer : draw in real space
		void drawGridOnly(GLfloat mvMat[], GLfloat pMat[]);
		/// @brief draw the planes, in the real space
		void drawPlanes(GLfloat mvMat[], GLfloat pMat[]);

		/// @brief For the dual-viewer : draw in grid space
		void drawWithPlanes(GLfloat mvMat[], GLfloat pMat[]);
		/// @b Draw a given plane 'view' (single plane on the framebuffer).
		void drawPlaneView(glm::vec2 fbDims, planes _plane);

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
		bool isInitialized; ///< tracks if the scene was initialized or not
		void printVAOStateNext() { this->showVAOstate = true; }
		glm::vec3 getPlanePositions(void) { return this->planePosition; } ///< Get the cutting planes' positions
		uint getMinTexValue(void) const { return this->minTexVal; }
		uint getMaxTexValue(void) const { return this->maxTexVal; }

		void slotTogglePolygonMode(bool show);
		void slotToggleShowTextureCube(bool show);
		void slotSetPlaneDepthX(float newXCoord);
		void slotSetPlaneDepthY(float newYCoord);
		void slotSetPlaneDepthZ(float newZCoord);
		void slotSetMinTexValue(uchar val);
		void slotSetMaxTexValue(uchar val);
		void slotSetPlanePositionX(float coord);
		void slotSetPlanePositionY(float coord);
		void slotSetPlanePositionZ(float coord);
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
		void prepPlaneUniforms(GLfloat *mvMat, GLfloat *pMat, planes _plane);
		/// @brief prep the plane uniforms to draw in space
		void prepPlane_SingleUniforms(planes _plane, glm::vec2 fbDims, const std::shared_ptr<DiscreteGrid> _grid);
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
	protected:
		void generateGrid();

		ControlPanel* controlPanel; ///< pointer to the control panel
		std::shared_ptr<InputGrid> texStorage; ///< textureLoader and 'manager'
		std::shared_ptr<OutputGrid> voxelGrid; ///< Voxel grid to fill upon keypress
		std::shared_ptr<TetMesh> mesh; ///< creates a mesh around the queried point
		GridControl* gridControl;
		/*
		GridDetailedView* detailsView;
		GridView* listViewInput;
		GridView* listViewOutput;
		*/

		std::size_t gridWidth; ///< grid size
		std::size_t gridHeight; ///< grid size
		std::size_t gridDepth; ///< grid size

		std::size_t renderSize;
		bool inputGridVisible; ///< does the user want to show the input grid ?
		bool outputGridVisible; ///< does the user want to show the output grid ?
		bool colorOrTexture; ///< do we use the RGB2HSV function or the color scale ?
		uchar minTexVal;
		uchar maxTexVal;

		glm::vec3 planePosition;
		glm::vec3 planeDepths;
		glm::vec3 sceneBBPosition;
		glm::vec3 sceneBBDiag;
		DrawMode drawMode;
		bool showVAOstate;

		GLuint vboVertPosHandle;
		GLuint vboVertNormHandle;
		GLuint vboVertTexHandle;
		GLuint vboElementHandle;
		GLuint vboPlaneElementHandle;
		GLuint vaoHandle;
		GLuint programHandle;
		GLuint planeProgramHandle;
		GLuint planeViewerProgramHandle;

		GLuint textureHandle; ///< handle for glTexImage3D
		GLuint voxelGridTexHandle; ///< handle for the voxel grid's data
		GLuint colorScaleHandle; ///< handle for the uploaded color scale
};

inline int __GetOpenGLError ( char* szFile, int iLine );

#if not defined( NDEBUG )
	#define GetOpenGLError() __GetOpenGLError( ( char* )__FILE__, ( int )__LINE__ )
#else
	#define GetOpenGLError()
#endif

#endif // VIEWER_INCLUDE_SCENE_HPP_

// vim : ts=8
