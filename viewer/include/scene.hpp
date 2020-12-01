#ifndef VIEWER_INCLUDE_SCENE_HPP_
#define VIEWER_INCLUDE_SCENE_HPP_

//#include "gl/GLHandler/include/ShaderObject.hpp"
//#include "gl/GLHandler/include/ProgramObject.hpp"
//#include "gl/GLHandler/include/VAOObject.hpp"

#include "../../macros.hpp"
#include "../../features.hpp"

#include "../../grid/include/tetmesh.hpp"
#include "../../grid/include/discrete_grid.hpp"
#include "../../grid/include/input_discrete_grid.hpp"
#include "../../grid/include/output_discrete_grid.hpp"

#include "../../qt/include/grid_control.hpp"
#include "../../qt/include/grid_detailed_view.hpp"
#include "../../qt/include/grid_list_view.hpp"

#include <QOpenGLFunctions_4_0_Core>
#include <QGLViewer/qglviewer.h>
#include <glm/glm.hpp>

#include <vector>
#include <mutex>

class ControlPanel; // Forward declaration

enum DrawMode { Solid, SolidAndWireframe, Wireframe };
enum planes { x, y, z };

class Scene : public QOpenGLFunctions_4_0_Core {
		typedef glm::vec<4, unsigned int, glm::defaultp> uvec4;
		typedef glm::uvec3 uvec3;
	public:
		Scene(GridControl* const gc); ///< default constructor
		~Scene(void); ///< default destructor

		/// @brief initialize the variables of the scene
		void initGl(QOpenGLContext* context, std::size_t _x = 1, std::size_t _y = 1, std::size_t _z = 1);

		/// @brief set the control panel responsible for controlling the scene
		void setControlPanel(ControlPanel* cp) { this->controlPanel = cp; }

		/// @brief compile the given shaders
		void compileShaders(std::string vPath, std::string gPath, std::string fPath);
		/// @brief reload the default shader files
		void recompileShaders(void);

		/// @brief For the dual-viewer : draw in real space
		void drawRealSpace(GLfloat mvMat[], GLfloat pMat[]);
		/// @brief For the dual-viewer : draw in grid space
		void drawInitialSpace(GLfloat mvMat[], GLfloat pMat[]);

		/// @brief Draws the X plane.
		void drawPlaneX();
		/// @brief Draws the Y plane.
		void drawPlaneY();
		/// @brief Draws the Z plane.
		void drawPlaneZ();
		/// @brief draw the planes, in the
		void drawPlanes(void);

		/// @brief load the 3D texture to opengl
		void loadImage();
		/// @brief load the voxel grid generated
		void loadVoxelGrid();

		/// @brief fill the grid using trilinear interpolation
		void fillTrilinear();
		/// @brief fill the grid using nearest neighbor interpolation
		void fillNearestNeighbor();

		/// @brief show the tex cube or not
		void toggleTexCubeVisibility(bool visibility) { this->showTextureCube = visibility; }
		/// @brief show the tex cube or not
		void toggleTexCubeVisibility() { this->toggleTexCubeVisibility(!this->showTextureCube); }

		void toggleColorOrTexture(bool _cOT) { this->colorOrTexture = _cOT; }
		void toggleColorOrTexture() { this->toggleColorOrTexture(!this->colorOrTexture); }

		glm::vec3 getSceneBoundaries(void) const;
		glm::vec3 getTexCubeBoundaries(bool realSpace) const;

		void setDrawModeSolid() { this->drawMode = DrawMode::Solid; }
		void setDrawModeSolidAndWireframe() { this->drawMode = DrawMode::SolidAndWireframe; }
		void setDrawModeWireframe() { this->drawMode = DrawMode::Wireframe; }

		void cleanup(void); ///< cleanup function for vbo and other parts
		bool isInitialized; ///< tracks if the scene was initialized or not

		void slotTogglePolygonMode(bool show);
		void slotToggleShowTextureCube(bool show);
		void slotSetTextureXCoord(uint newXCoord);
		void slotSetTextureYCoord(uint newYCoord);
		void slotSetTextureZCoord(uint newZCoord);
		void slotSetMinTexValue(uchar val);
		void slotSetMaxTexValue(uchar val);
		void slotSetCutPlaneX_Min(float coord);
		void slotSetCutPlaneY_Min(float coord);
		void slotSetCutPlaneZ_Min(float coord);
		void slotSetCutPlaneX_Max(float coord);
		void slotSetCutPlaneY_Max(float coord);
		void slotSetCutPlaneZ_Max(float coord);
	private :
		void generateTexCube(std::vector<glm::vec4>& vertPos, std::vector<glm::vec4>& vertNorm, std::vector<unsigned int>& vertIdx);
		void generatePlanesArray(std::vector<unsigned int>& idx);
		glm::mat4 computeTransformationMatrix() const;
		GLuint compileShader(const std::string& path, const GLenum shaType);
		GLuint compileProgram(const GLuint vSha = 0, const GLuint gSha = 0, const GLuint fSha = 0);
		/// @b preps uniforms for a grid
		void prepGridUniforms(GLfloat* mvMat, GLfloat* pMat, glm::vec4 lightPos, glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid>& grid);
		/// @b draws a grid, slightly more generic than drawVoxelGrid()
		void drawGrid_Generic(GLfloat mvMat[], GLfloat pMat[], glm::mat4 baseMatrix, GLuint texHandle, const std::shared_ptr<DiscreteGrid>& grid);
		/// @b preps uniforms for a given plane
		void prepSinglePlaneUniforms(planes _plane);
		/// @b draws a given plane
		void drawPlane_single(planes _plane);
		/// @brief prep the plane uniforms to draw in space
		void prepPlaneSpaceUniforms(planes _plane, GLfloat* mvMat, GLfloat* pMat, glm::vec4);
		/// @b Prints grid info.
		void printGridInfo(const std::shared_ptr<DiscreteGrid>& grid);
		/// @b Generate a scale of colors for the program.
		std::vector<float> generateColorScale(std::size_t minVal, std::size_t maxVal); ///< Generate a color scale for the data
		void uploadColorScale(const std::vector<float>& colorScale); ///< Uploads the color scale to OpenGL
		/// @b setup the buffers' data
		void setupVBOData(const std::vector<glm::vec4>& vertPos, const std::vector<glm::vec4>& vertNorm, const std::vector<unsigned int>& vertIdx, const std::vector<unsigned int>& vertIdx_plane);
		/// @b setup the vao binding setup
		void setupVAOPointers();
	protected:
		void generateGrid(std::size_t _x, std::size_t _y, std::size_t _z);

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
		std::size_t neighborWidth; ///< neighborhood size
		std::size_t neighborHeight; ///< neighborhood size
		std::size_t neighborDepth; ///< neighborhood size

		std::size_t renderSize;
		bool showTextureCube; ///< does the user want to show the texture cube ?
		bool colorOrTexture; ///< do we use the RGB2HSV function or the color scale ?
		uchar minTexVal;
		uchar maxTexVal;

		glm::vec3 cutPlaneMin;
		glm::vec3 cutPlaneMax;
		uvec3 neighborPos;
		DrawMode drawMode;

		// Uniform locations :
		GLint colorScaleLocation;

		GLuint vboVertPosHandle;
		GLuint vboVertNormHandle;
		GLuint vboElementHandle;
		GLuint vboPlaneElementHandle;
		GLuint vaoHandle;
		GLuint programHandle;
		GLuint planeProgramHandle;

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
