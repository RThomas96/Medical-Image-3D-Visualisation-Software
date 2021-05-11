#ifndef VISUALISATION_IMAGE_TRANSFORM_INCLUDE_AFFINE_TRANSFORM_HPP_
#define VISUALISATION_IMAGE_TRANSFORM_INCLUDE_AFFINE_TRANSFORM_HPP_

#include "transform_interface.hpp"

#include <glm/ext/matrix_transform.hpp>

/// @b An overload of ITransform, representing an immutable matrix transform.
/// @details An inverse matrix is precomputed for quicker image-to-world space conversions.
class MatrixTransform : public ITransform {
	protected:
		/// @b Protected ctor, giving this object an identity matrix as both forward and inverse transforms.
		explicit MatrixTransform(void);

	public:
		/// @b Default constructor for the matrix transform, taking a mat4 as an argument.
		MatrixTransform(glm::mat4 _mat);

		/// @b Supplementary constructor for the matrix transform, taking a mat3 as an argument.
		MatrixTransform(glm::mat3 _mat);

		/// @b Default dtor for the matrix transforms.
		virtual ~MatrixTransform(void) = default;

		/// @b Allow to get the matrix representation for this transform.
		glm::mat4 matrix() const;

		/// @b Transform a point from image-space into world-space.
		glm::vec4 to_image(glm::vec4 world_space_location) const override;

		/// @b Transform a point from world-space into image-space.
		glm::vec4 to_world(glm::vec4 image_space_location) const override;

	protected:
		/// @b The internal matrix representing the transform.
		glm::mat4 m_matrix;

		/// @b Pre-computed inverse matrix, for quicker to_world computation.
		glm::mat4 inverse_matrix;
};

/// @b Overload of the matrix transform, to represent an identity transformation.
/// @note Has all the same features as MatrixTransform, but represents an identity transformation.
class DefaultTransform : public MatrixTransform {
	public:
		/// @b Default and only ctor, assigning an identity matrix to this transform.
		DefaultTransform(void) : MatrixTransform() {}

		/// @b Default dtor
		virtual ~DefaultTransform(void) = default;

		/// @b Transform a point from image-space into world-space.
		glm::vec4 to_image(glm::vec4 world_space_location) const override;

		/// @b Transform a point from world-space into image-space.
		glm::vec4 to_world(glm::vec4 image_space_location) const override;
};

#endif // VISUALISATION_IMAGE_TRANSFORM_INCLUDE_AFFINE_TRANSFORM_HPP_
