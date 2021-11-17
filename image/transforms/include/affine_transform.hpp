#ifndef VISUALISATION_IMAGE_TRANSFORM_INCLUDE_AFFINE_TRANSFORM_HPP_
#define VISUALISATION_IMAGE_TRANSFORM_INCLUDE_AFFINE_TRANSFORM_HPP_

#include "transform_interface.hpp"

#include <glm/ext/matrix_transform.hpp>

/// @brief An overload of ITransform, representing an immutable matrix transform.
/// @details An inverse matrix is precomputed for quicker image-to-world space conversions.
class MatrixTransform : public ITransform {
public:
	typedef std::shared_ptr<MatrixTransform> Ptr;

protected:
	/// @brief Protected ctor, giving this object an identity matrix as both forward and inverse transforms.
	explicit MatrixTransform(void);

public:
	/// @brief Default constructor for the matrix transform, taking a mat4 as an argument.
	MatrixTransform(glm::mat4 _mat);

	/// @brief Supplementary constructor for the matrix transform, taking a mat3 as an argument.
	MatrixTransform(glm::mat3 _mat);

	/// @brief Default dtor for the matrix transforms.
	virtual ~MatrixTransform(void) = default;

	/// @brief Allow to get the matrix representation for this transform.
	glm::mat4 matrix() const;

	/// @brief Transform a point from image-space into world-space.
	glm::vec4 to_image(glm::vec4 world_space_location) const override;

	/// @brief Transform a point from world-space into image-space.
	glm::vec4 to_world(glm::vec4 image_space_location) const override;

protected:
	/// @brief The internal matrix representing the transform.
	glm::mat4 m_matrix;

	/// @brief Pre-computed inverse matrix, for quicker to_world computation.
	glm::mat4 inverse_matrix;
};

/// @brief Overload of the matrix transform, to represent an identity transformation.
/// @note Has all the same features as MatrixTransform, but represents an identity transformation.
class DefaultTransform : public MatrixTransform {
public:
	typedef std::shared_ptr<DefaultTransform> Ptr;
	/// @brief Default and only ctor, assigning an identity matrix to this transform.
	DefaultTransform(void) :
		MatrixTransform() {}

	/// @brief Default dtor
	virtual ~DefaultTransform(void) = default;

	/// @brief Transform a point from image-space into world-space.
	glm::vec4 to_image(glm::vec4 world_space_location) const override;

	/// @brief Transform a point from world-space into image-space.
	glm::vec4 to_world(glm::vec4 image_space_location) const override;
};

#endif	  // VISUALISATION_IMAGE_TRANSFORM_INCLUDE_AFFINE_TRANSFORM_HPP_
