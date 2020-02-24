#ifndef VIEWER_INCLUDE_TEXTURE_VIEWER_HPP_
#define VIEWER_INCLUDE_TEXTURE_VIEWER_HPP_

#include <QGLViewer/qglviewer.h>

#include "../../image/include/bulk_texture_loader.hpp"

class texture_viewer : public QGLViewer {

		friend class slider_widget; // for easy access to private values

		Q_OBJECT

	protected:
		/**
		 * @brief Positions of veritces
		 */
		qreal* vertex_pos;
		/**
		 * @brief Normals of vertices
		 */
		qreal* vertex_nor;
		/**
		 * @brief Pointers to the coordinates of the texture coordinates needed for the vertex.
		 */
		qreal**vertex_tex;
		/**
		 * @brief Min texture coordinates for cube
		 */
		qreal* tex_coords_min;
		/**
		 * @brief Max texture coordinates for cube
		 */
		qreal* tex_coords_max;
		/**
		 * @brief ID for the 3D texture to apply
		 */
		GLuint texture_id;
		/**
		 * @brief Pointer to a stack loader for image retrieval
		 */
		bulk_texture_loader* stack_loader;
	public:
		/**
		 * @brief Initializes the needed OpenGL state, and some variables
		 */
		virtual void init() override;
		/**
		 * @brief Load the texture, using a stack loader.
		 */
		void load_textures();
		/**
		 * @brief Draw the cube, and the texure on it.
		 */
		virtual void draw() override;
	public slots:
		/**
		 * @brief Sets the minimum X value for texture coordinates
		 * @param x the value of x for tex_coords_min
		 */
		void set_min_X_tex_value(double x);
		/**
		 * @brief Sets the minimum Y value for texture coordinates
		 * @param y the value of y for tex_coords_min
		 */
		void set_min_Y_tex_value(double y);
		/**
		 * @brief Sets the minimum Z value for texture coordinates
		 * @param z the value of z for tex_coords_min
		 */
		void set_min_Z_tex_value(double z);
		/**
		 * @brief Sets the maximum X value for texture coordinates
		 * @param x the value of x for tex_coords_max
		 */
		void set_max_X_tex_value(double x);
		/**
		 * @brief Sets the maximum Y value for texture coordinates
		 * @param y the value of y for tex_coords_max
		 */
		void set_max_Y_tex_value(double y);
		/**
		 * @brief Sets the maximum Z value for texture coordinates
		 * @param z the value of z for tex_coords_max
		 */
		void set_max_Z_tex_value(double z);
		/**
		 * @brief Request for a texture load from an external signal ( a button for example )
		 */
		void request_texture_load();
		/**
		 * @brief Request current texture freeing from an external signal ( a button for example )
		 */
		void request_texture_deletion();
	private:
		void print_opengl_info() const;
		/**
		 * @brief Setup the cube attributes (vertices, normals and tex coordinates)
		 */
		void setup_cube_attribs();
		/**
		 * @brief Draw a face, calling appropriate OpenGL gl_XXXXXX_3d functions.
		 * @param v0 The first vertex of the face
		 * @param v1 The second vertex of the face
		 * @param v2 The last vertex of the face
		 *
		 * @note The indices must be given in a counter-clockwise pattern.
		 * @warning The function does not check if the vertices are in the good order. Get that checked out yourself.
		 */
		void draw_face_gl(std::size_t v0, std::size_t v1, std::size_t v2);
		/**
		 * @brief Sets the X coordinate of the vertices in the back face to be the same as the texture
		 */
		void set_back_x_plane_coordinates();
		/**
		 * @brief Sets the Y coordinate of the vertices in the front face to be the same as the texture
		 */
		void set_front_x_plane_coordinates();
		/**
		 * @brief Sets the Y coordinate of the vertices in the back face to be the same as the texture
		 */
		void set_back_y_plane_coordinates();
		/**
		 * @brief Sets the Y coordinate of the vertices in the front face to be the same as the texture
		 */
		void set_front_y_plane_coordinates();
		/**
		 * @brief Sets the Z coordinate of the vertices in the back face to be the same as the texture
		 */
		void set_back_z_plane_coordinates();
		/**
		 * @brief Sets the Y coordinate of the vertices in the front face to be the same as the texture
		 */
		void set_front_z_plane_coordinates();
		/**
		 * @brief Set the cube to doubled unit dimensions ([-1;1] instead of [0,1])
		 */
		void set_to_unit_cube();
		/**
		 * @brief Sets the cube to the real (but scaled) voxel dimensions, to get a better view of the stack of images.
		 * @note Supposes the doubled unit square ([-1;1] on each side) is image-sized (2048 * 2048 pixels at 0.39µm * 0.39µm per pixel).
		 * @param nb_images_loaded The number of images loaded.
		 */
		void set_real_voxel_dimensions(size_t nb_images_loaded);
};

#endif // VIEWER_INCLUDE_TEXTURE_VIEWER_HPP_
