#ifndef VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_
#define VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_

#include "../../image/api/include/backend.hpp"
#include "../../image/api/include/grid.hpp"

#include "./viewer_structs.hpp"

#include <QOpenGLFunctions_3_2_Core>
#include <memory>
#include <vector>

class Scene; // fwd-declaration

/// @b This class is a proxy object, enabling to display a grid in real-time.
class GridViewer : public QObject {
		// Define this class to be signal-able via Qt's slot-signal system.
		Q_OBJECT;
	public:
		/// @b The different available visualization modes
		enum VisualizationMode {
			Solid,			///< Solid viewing, project the texture onto a cube.
			Volumetric,		///< Volumetric visualization, occluded only by the cutting planes.
			VolumetricBoxed	///< Volumetric visualisation, occluded outside of the min and max coordinates.
		};
		Q_ENUM(VisualizationMode);

		Q_PROPERTY(bool is_grid_hidden READ hidden WRITE hide NOTIFY visibilityChanged);
		Q_PROPERTY(VisualizationMode vis_mode READ viewMode WRITE setViewMode NOTIFY viewModeChanged);

	public:
		/// @b The default ctor for the class. Queues up the initialization of the GL for the next screen refresh.
		GridViewer(Image::Grid::Ptr& _grid_to_show);

		/// @b Default ctor for the grid viewer. Tries to clear the
		~GridViewer(void);

		/// @b Initializes all the data that's possible on the host side, before the
		void initializeData(void);

		/// @b Initializes the GL data that must be used to show the grid.
		void initializeGLData(Scene* _scene_to_modify);

		/// @b Draw the grid, using the right underlying method call to draw it in the right mode.
		/// @details Under the hood, binds the uniforms for the current program, and binds the UBO.
		/// @warning Assumes a GL context is current, valid, and bound.
		void draw3D(Scene* _scene_functions, GLfloat* view_matrix, GLfloat* projection_matrix);

		/// @b Returns true if the grid is hidden, otherwise returns false.
		bool hidden() const noexcept;

		/// @b Return the current view mode of this grid.
		VisualizationMode viewMode() const noexcept;

	public slots:
		/// @b Hide or show the grid. By default (with no argument), hides the grid.
		void hide(bool should_hide = true);
		/// @b Set a new visualization mode for the current grid.
		void setViewMode(VisualizationMode _new_vis_mode);

	signals:
		/// @b Signal fired before the data initialization on the host (CPU) side.
		void beforeCPUInit();
		/// @b Signal fired after the data initialization on the host (CPU) side.
		void afterCPUInit();

		/// @b Signal fired before the data initialization on the device (GPU) side.
		void beforeGLInit();
		/// @b Signal fired after the data initialization on the device (GPU) side.
		void afterGLInit();

		/// @b Used to signify the grid has been hidden by a certain
		void visibilityChanged(bool is_hidden);

		/// @b  USed to signal whenever the visualization mode changes.
		void viewModeChanged(VisualizationMode _new_mode);

	protected:
		/// @b Draw the grid, in solid mode.
		void draw_solid(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix);

		/// @b Draw the grid, in volumetric mode.
		void draw_volumetric(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix);

		/// @b Draw the grid, in boxed volumetric mode.
		void draw_volumetric_boxed(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix);

		/// @b Bind the uniform buffer that the grid requires.
		void bindUniformBuffer(Scene* _scene, GLuint _program_handle, const char* uniform_buffer_name);

		/// @b Binds the textures of the grid to the current GL program / draw call.
		void bindTextures_Solid(Scene* _scene);
		/// @b Binds the textures of the grid to the current GL program / draw call.
		void bindTextures_Volumetric(Scene* _scene);
		/// @b Binds the textures of the grid to the current GL program / draw call.
		void bindTextures_VolumetricBoxed(Scene* _scene);

		/// @b Binds the solid viewing program uniforms to the current GL context.
		void bindUniforms_Solid(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix);
		/// @b Binds the volumetric viewing program uniforms to the current GL context.
		void bindUniforms_Volumetric(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix);
		/// @b Binds the volumetric boxed viewing program uniforms to the current GL context.
		void bindUniforms_VolumetricBoxed(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix);

		/// @b Update the main channel data within the UBO data on the GL side.
		/// @warning Assumes the GL context used in the scene is bound and made current.
		void updateMainChannel_UBO(Scene* _scene);

		/// @b Generates the mesh data, for the initialization functions.
		/// @details It only generates the mesh data on the host (CPU) side. The function tasked with uploading the
		/// data to the device (GPU) side is integrated in the initializeGLData() function.
		void generateMeshData();

	protected:
		/// @b Is this grid supposed to be shown or not ?
		bool is_grid_hidden;
		/// @b The current visualization mode of the grid (solid, volumetric ?)
		VisualizationMode vis_mode;
		/// @b The grid we are displaying
		Image::Grid::Ptr source_grid;
		/// @b The texture handle for the
		GLuint grid_texture;
		/// @b Texture handles for the volumetric mesh
		VolMesh texture_handles;
		/// @b The volumetric mesh that is actually shown
		VolMeshData volumetric_mesh;
		/// @b The uniform buffer handle for the color channel data
		GLuint ubo_handle;
		/// @b The index of the channel considered to be the 'main' channel (either R, G, or B).
		std::uint8_t main_channel_index;
		/// @b The color channel attributes for the R, G, and B channels of the uploaded texture.
		std::array<colorChannelAttributes_GL, 3> colorChannelAttributes;
		/// @b The epsilon to add to the volumetric viewer in order not to cut tetrahedrons too early.
		glm::vec3 volumetric_epsilon;
};

#endif // VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_
