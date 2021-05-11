#ifndef VISUALISATION_IMAGE_TRANSFORM_INCLUDE_TRS_TRANSFORM_HPP_
#define VISUALISATION_IMAGE_TRANSFORM_INCLUDE_TRS_TRANSFORM_HPP_

#include "transform_interface.hpp"

class TRSTransform : public ITransform {
	public:
		TRSTransform(void);

		/// @b Default dtor for the TRS transforms.
		virtual ~TRSTransform(void) = default;

		/// @b Returns the cached, computed matrix of the TRS transform.
		glm::mat4 matrix() const;

		/// @b Returns the scale parameter of the TRS transform.
		glm::vec3 scaling() const;

		/// @b Returns the rotation parameter of the TRS transform.
		glm::vec3 rotation() const;

		/// @b Returns the translation parameter of the TRS transform.
		glm::vec3 translation() const;

		/// @b Sets the scaling parameter of the TRS transforms, and updates the cached matrix.
		TRSTransform& setScaling(glm::vec3 _new_scale);

		/// @b Sets the rotation parameter of the TRS transforms, and updates the cached matrix.
		TRSTransform& setRotation(glm::vec3 _new_rot);

		/// @b Sets the translation parameter of the TRS transforms, and updates the cached matrix.
		TRSTransform& setTranslation(glm::vec3 _new_trans);

	protected:
		/// @b Updates the cached representation of the TRS matrix, for when the computation
		void update_cached_matrix(void);

	protected:
		/// @b The scale of the transform on each axis.
		glm::vec3 m_scaling;

		/// @b The rotation of the transform on each axis.
		glm::vec3 m_rotation;

		/// @b The translation of the transform along each axis.
		glm::vec3 m_translation;

		/// @b The matrix representing the current translation, scaling and rotation of the transform.
		glm::mat4 cached_matrix;

		/// @b The inverse of cached_matrix, for quicker computations.
		glm::mat4 cached_inverse_matrix;
};

#endif // VISUALISATION_IMAGE_TRANSFORM_INCLUDE_TRS_TRANSFORM_HPP_
