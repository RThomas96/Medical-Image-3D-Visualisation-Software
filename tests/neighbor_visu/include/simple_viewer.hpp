#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_SIMPLE_VIEWER_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_SIMPLE_VIEWER_HPP_

#include <QWidget>
#include <QGLViewer/qglviewer.h>

#include <glm/glm.hpp>

class SimpleViewer : public QGLViewer {
		Q_OBJECT

		/// @brief Redefinition of the default glm::vec3 type to follow the qreal type
		typedef glm::vec<3, qreal, glm::defaultp> vec3;
		/// @brief Redefinition of the default glm::vec4 type to follow the qreal type
		typedef glm::vec<4, qreal, glm::defaultp> vec4;

		/// @brief Redefinition of the default glm::mat3 type to follow the qreal type
		typedef glm::mat<3, 3, qreal, glm::defaultp> mat3;
		/// @brief Redefinition of the default glm::mat4 type to follow the qreal type
		typedef glm::mat<4, 4, qreal, glm::defaultp> mat4;

	protected:
		/// @brief Positions of vertices
		vec3* vertex_pos;

		/// @brief Normals of vertices
		vec3* vertex_nor;

		/// @brief Pointers to the coordinates of the texture coordinates needed for the vertex.
		qreal**vertex_tex;

		/// @brief Min texture coordinates for cube
		vec3 tex_coords_min;

		/// @brief Max texture coordinates for cube
		vec3 tex_coords_max;

		/// @brief Number of images loaded.
		size_t nb_images_loaded;

		/// @brief ID for the 3D texture to apply
		GLuint texture_id;

		/// @brief Pointer to a stack loader for image retrieval
		bulk_texture_loader* stack_loader;

		/// @brief Checks if we need to undistort the stack of images or not.
		bool undistort;

		/// @brief Transformation matrix defined by Gille and Chalard in their report on the field.
		mat3 transformation_matrix;
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_SIMPLE_VIEWER_HPP_
