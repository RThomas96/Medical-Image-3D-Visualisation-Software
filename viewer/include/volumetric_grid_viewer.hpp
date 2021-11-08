#ifndef VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_
#define VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_

#include "../../image/api/include/backend.hpp"
#include "../../image/api/include/grid.hpp"

#include "./viewer_structs.hpp"

#include <QOpenGLFunctions_3_2_Core>
#include <memory>
#include <vector>

class Scene; // fwd-declaration

/// @ingroup graphpipe
/// @brief This class is a proxy object, enabling to display a grid in real-time.
/// @warning Not yet tested.
class GridViewer : public QObject {
		// Define this class to be signal-able via Qt's slot-signal system.
		Q_OBJECT;
	public:
		/// @brief The different available visualization modes
		enum VisualizationMode {
			Solid,			///< Solid viewing, project the texture onto a cube.
			Volumetric,		///< Volumetric visualization, occluded only by the cutting planes.
			VolumetricBoxed	///< Volumetric visualisation, occluded outside of the min and max coordinates.
		};
		Q_ENUM(VisualizationMode);

		Q_PROPERTY(bool is_grid_hidden READ hidden WRITE hide NOTIFY visibilityChanged);
		Q_PROPERTY(VisualizationMode vis_mode READ viewMode WRITE setViewMode NOTIFY viewModeChanged);
		Q_PROPERTY(std::uint8_t main_channel_index WRITE setMainChannel NOTIFY mainChannelChanged)

	public:
		/// @brief The default ctor for the class. Queues up the initialization of the GL for the next screen refresh.
		GridViewer(Image::Grid::Ptr& _grid_to_show);

		/// @brief Default ctor for the grid viewer. Tries to clear the
		~GridViewer(void);

		/// @brief Initializes all the data that's possible on the host side, before the
		void initializeData(void);

		/// @brief Initializes the GL data that must be used to show the grid.
		void initializeGLData(Scene* _scene_to_modify);

		/// @brief Draw the grid, using the right underlying method call to draw it in the right mode.
		/// @details Under the hood, binds the uniforms for the current program, and binds the UBO.
		/// @warning Assumes a GL context is current, valid, and bound.
		void draw3D(Scene* _scene_functions, GLfloat* view_matrix, GLfloat* projection_matrix, glm::vec3 cam_position);

		/// @brief Returns true if the grid is hidden, otherwise returns false.
		bool hidden() const noexcept;

		/// @brief Return the current view mode of this grid.
		VisualizationMode viewMode() const noexcept;

	public slots:
		/// @brief Hide or show the grid. By default (with no argument), hides the grid.
		void hide(bool should_hide = true);
		/// @brief Set a new visualization mode for the current grid.
		void setViewMode(VisualizationMode _new_vis_mode);
		/// @brief Sets the main channel to be the i-th channel from the loaded data.
		/// @warning Should be 0, 1, or 2 (red, green, blue respectively). Anything else will be ignored.
		void setMainChannel(std::uint8_t _new_main_channel);

		/// @brief Sets the minimum displayed value of the 'c'-th channel to 'n'
		void setChannelMinimumVisible(std::uint8_t c, colorChannelAttributes_GL::bound_t::value_type n);
		/// @brief Sets the maximum displayed value of the 'c'-th channel to 'n'
		void setChannelMaximumVisible(std::uint8_t c, colorChannelAttributes_GL::bound_t::value_type n);
		/// @brief Sets the minimum value of the 'c'-th color scale range to 'n'
		void setChannelMinimumColorRange(std::uint8_t c, colorChannelAttributes_GL::bound_t::value_type n);
		/// @brief Sets the maximum value of the 'c'-th color scale range to 'n'
		void setChannelMaximumColorRange(std::uint8_t c, colorChannelAttributes_GL::bound_t::value_type n);

	signals:
		/// @brief Signal fired before the data initialization on the host (CPU) side.
		void beforeCPUInit();
		/// @brief Signal fired after the data initialization on the host (CPU) side.
		void afterCPUInit();

		/// @brief Signal fired before the data initialization on the device (GPU) side.
		void beforeGLInit();
		/// @brief Signal fired after the data initialization on the device (GPU) side.
		void afterGLInit();

		/// @brief Used to signify the grid has been hidden by a certain
		void visibilityChanged(bool is_hidden);

		/// @brief  Used to signal whenever the visualization mode changes.
		void viewModeChanged(VisualizationMode _new_mode);

		/// @brief Used to signal whenever the main channel index changes.
		/// @note Is useful to know when to update the scene UBO.
		void mainChannelChanged(std::uint8_t _new_channel);

	protected:
		/// @brief Draw the grid, in solid mode.
		void draw_solid(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix);

		/// @brief Draw the grid, in volumetric mode.
		void draw_volumetric(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix, glm::vec3 cam_pos);

		/// @brief Bind the uniform buffer that the grid requires.
		void bindUniformBuffer(Scene* _scene, const char* uniform_buffer_name);

		/// @brief Binds the textures of the grid to the current GL program / draw call.
		void bindTextures_Solid(Scene* _scene);
		/// @brief Binds the textures of the grid to the current GL program / draw call.
		void bindTextures_Volumetric(Scene* _scene);

		/// @brief Binds the solid viewing program uniforms to the current GL context.
		void bindUniforms_Solid(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix);
		/// @brief Binds the volumetric viewing program uniforms to the current GL context.
		void bindUniforms_Volumetric(Scene* _scene, GLfloat* view_matrix, GLfloat* projection_matrix, glm::vec3 campos);

		/// @brief Update the main channel data within the UBO data on the GL side.
		/// @warning Assumes the GL context used in the scene is bound and made current.
		void updateMainChannel_UBO(Scene* _scene);

		/// @brief Generates the mesh data, for the initialization functions.
		/// @details It only generates the mesh data on the host (CPU) side. The function tasked with uploading the
		/// data to the device (GPU) side is integrated in the initializeGLData() function.
		void generateMeshData();

		/// @brief Generates _and_ uploads the visibility texture all at once.
		/// @note For performance reasons, should not be called everytime a bound value changes but on the next draw (to
		/// basically 'wait for the event loop to finish'. If multiple values change between two frames, we want to call
		/// this function only once.
		/// @warning Should not be called when a GL context is not bound.
		void generateAndUploadVisibilityTexture(Scene* _scene);

	protected:
		/// @brief Is this grid supposed to be shown or not ?
		bool is_grid_hidden;
		/// @brief The current visualization mode of the grid (solid, volumetric ?)
		VisualizationMode vis_mode;
		/// @brief The grid we are displaying
		Image::Grid::Ptr source_grid;
		/// @brief The texture handle for the grid.
		GLuint grid_texture;
		/// @brief Texture handles for the volumetric mesh
		VolMesh texture_handles;
		/// @brief The volumetric mesh that is actually shown
		VolMeshData volumetric_mesh;
		/// @brief The uniform buffer handle for the color channel data
		GLuint ubo_handle;
		/// @brief The index of the channel considered to be the 'main' channel (either R, G, or B).
		std::uint8_t main_channel_index;
		/// @brief The color channel attributes for the R, G, and B channels of the uploaded texture.
		std::array<colorChannelAttributes_GL, 3> colorChannelAttributes;
		/// @brief The epsilon to add to the volumetric viewer in order not to cut tetrahedrons too early.
		glm::vec3 volumetric_epsilon;

		/// @brief Handle for the visibility texture for all values of the image.
		GLuint visibility_texture;

		/// @brief Currently in-use program handle.
		/// @details Updated every drawcall. Ideally, we would have a nicer way to 'require' the draw mode/program of
		/// the grid viewer, but for now this'll do.
		GLuint program_handle;
};

#endif // VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_