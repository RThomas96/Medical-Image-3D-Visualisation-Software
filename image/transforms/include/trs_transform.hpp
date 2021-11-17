#ifndef VISUALISATION_IMAGE_TRANSFORM_INCLUDE_TRS_TRANSFORM_HPP_
#define VISUALISATION_IMAGE_TRANSFORM_INCLUDE_TRS_TRANSFORM_HPP_

#include "transform_interface.hpp"

/// @ingroup newgrid
/// @brief The TRSTransform class represents a translation/rotation/scale matrix.
/// @details This kind of transform is quite popular in 3D software, and I thought it would be good to have a similar
/// feature. Allows to provide magnitudes for 3 vectors (translation, rotation, scale) independently.
/// @warning Not yet tested.
class TRSTransform : public ITransform {
public:
	typedef std::shared_ptr<TRSTransform> Ptr;

public:
	TRSTransform(void);

	/// @brief Default dtor for the TRS transforms.
	virtual ~TRSTransform(void) = default;

	/// @brief Returns the cached, computed matrix of the TRS transform.
	glm::mat4 matrix() const;

	/// @brief Returns the scale parameter of the TRS transform.
	glm::vec3 scaling() const;

	/// @brief Returns the rotation parameter of the TRS transform.
	glm::vec3 rotation() const;

	/// @brief Returns the translation parameter of the TRS transform.
	glm::vec3 translation() const;

	/// @brief Sets the scaling parameter of the TRS transforms, and updates the cached matrix.
	TRSTransform& setScaling(glm::vec3 _new_scale);

	/// @brief Sets the rotation parameter of the TRS transforms, and updates the cached matrix.
	TRSTransform& setRotation(glm::vec3 _new_rot);

	/// @brief Sets the translation parameter of the TRS transforms, and updates the cached matrix.
	TRSTransform& setTranslation(glm::vec3 _new_trans);

protected:
	/// @brief Updates the cached representation of the TRS matrix, for when the computation
	void update_cached_matrix(void);

protected:
	/// @brief The scale of the transform on each axis.
	glm::vec3 m_scaling;

	/// @brief The rotation of the transform on each axis.
	glm::vec3 m_rotation;

	/// @brief The translation of the transform along each axis.
	glm::vec3 m_translation;

	/// @brief The matrix representing the current translation, scaling and rotation of the transform.
	glm::mat4 cached_matrix;

	/// @brief The inverse of cached_matrix, for quicker computations.
	glm::mat4 cached_inverse_matrix;
};

#endif	  // VISUALISATION_IMAGE_TRANSFORM_INCLUDE_TRS_TRANSFORM_HPP_
