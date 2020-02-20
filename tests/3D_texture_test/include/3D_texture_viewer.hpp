#ifndef TESTS_3D_TEXTURE_TEST_3D_TEXTURE_VIEWER_HPP
#define TESTS_3D_TEXTURE_TEST_3D_TEXTURE_VIEWER_HPP

#include <QGLViewer/qglviewer.h>

#include "../include/image_stack_loader.hpp"

class simple_3D_texture_viewer : public QGLViewer {

		friend class slider_widget; // for easy access to private values

		Q_OBJECT

	protected:
		/**
		 * @brief Number of vertices to draw
		 */
		std::size_t vertex_count;
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
		 * @brief Raw data from image stack
		 */
		unsigned char* texture_data;
		/**
		 * @brief ID for the 3D texture to apply
		 */
		GLuint texture_id;
		/**
		 * @brief Pointer to a stack loader for image retrieval
		 */
		image_stack_loader* stack_loader;
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
	private:
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
};

#endif // TESTS_3D_TEXTURE_TEST_3D_TEXTURE_VIEWER_HPP
