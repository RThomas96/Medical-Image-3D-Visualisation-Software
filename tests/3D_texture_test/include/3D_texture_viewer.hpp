#ifndef TESTS_3D_TEXTURE_TEST_3D_TEXTURE_VIEWER_HPP
#define TESTS_3D_TEXTURE_TEST_3D_TEXTURE_VIEWER_HPP

#include <QGLViewer/qglviewer.h>

#include "../include/image_stack_loader.hpp"

class simple_3D_texture_viewer : public QGLViewer {
	public:
		virtual void init() override;
		void load_textures();
		virtual void draw() override;
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
		std::vector<uchar> texture_data;
		/**
		 * @brief ID for the 3D texture to apply
		 */
		GLuint texture_id;
		/**
		 * @brief Pointer to a stack loader for image retrieval
		 */
		image_stack_loader* stack_loader;
	private:
		/**
		 * @brief Setup the cube attributes
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
};

#endif // TESTS_3D_TEXTURE_TEST_3D_TEXTURE_VIEWER_HPP
