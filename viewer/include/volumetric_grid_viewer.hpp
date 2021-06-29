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
class VolumetricGridViewer : public QObject {
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
		/// @todo Re-enable this ctor once the class is more fleshed out.
		VolumetricGridViewer(void) = delete;

		/// @b Default ctor for the grid viewer. Tries to clear the
		~VolumetricGridViewer(void);

		/// @b Initializes all the data that's possible on the host side, before the
		void initializeData(void);

		/// @b Initializes the GL data that must be used to show the grid.
		void initializeGLData(Scene* _scene_to_modify);

		/// @b Draw the grid, using the right underlying method call to draw it in the right mode.
		/// @details Under the hood, binds the uniforms for the current program, and
		/// @warning Assumes a GL context is current, valid, and bound.
		void draw(Scene* _scene_functions);

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
		void draw_solid(void);

		/// @b Draw the grid, in volumetric mode.
		void draw_volumetric(void);

		/// @b Draw the grid, in boxed volumetric mode.
		void draw_volumetric_boxed();

		/// @b Bind the uniform buffer that the grid requires.
		void bindUniformBuffer(void);

		/// @b Binds the textures of the grid to the current GL program / draw call.
		/// @details Will only bind the necessary textures. If drawing in solid mode, binds only the grid texture
		/// alongside the color channels. If drawing in volumetric (boxed) mode, will also bind the volumetric mesh
		/// textures in order to draw them properly.
		void bindTextures();

		/// @b Update the main channel data within the UBO data on the GL side.
		void updateMainChannel_UBO();

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
};

#endif // VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_
