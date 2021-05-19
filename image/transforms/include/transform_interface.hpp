#ifndef VISUALISATION_IMAGE_TRANSFORM_INCLUDE_TRANSFORM_INTERFACE_HPP_
#define VISUALISATION_IMAGE_TRANSFORM_INCLUDE_TRANSFORM_INTERFACE_HPP_

#include <glm/glm.hpp>

#include <vector>
#include <memory>

enum TransformType {
	/// @b Invalid type, reserved for un-initialized transforms.
	Unknown,
	/// @b Default transform type, representing an identity matrix.
	Default,
	/// @b Transform-Rotate-Scale transforms, which are the easiest to manipulate in a GUI.
	TRS,
	/// @b Affine matrices, defined elsewhere (user, or a file)
	Affine_Matrix,
};

/// @b Interface to all transform types, defining the few functions common to all transform types.
class ITransform {

	public:
		/// @b Shared pointer type for the ITransform class
		using Ptr = std::shared_ptr<ITransform>;

	protected:
		/// @b Default ctor for the ITransform class.
		ITransform(TransformType _type = Unknown) : m_transformType(_type), m_nextTransform(nullptr) {}

	public:
		/// @b Default dtor for the ITransform class.
		virtual ~ITransform(void) { this->m_nextTransform.reset(); }

		/// @b Returns the transformation's type, for when the use of a dynamic_pointer_cast<> is necessary.
		TransformType transformType(void) const { return this->m_transformType; }

		/// @b Get the next transform in the chain, if applicable. Otherwise, returns nullptr.
		ITransform::Ptr nextTransform(void) const { return ITransform::Ptr(this->m_nextTransform); }

		/// @b Sets the next transform to the given transform pointer.
		virtual ITransform& setNextTransform(ITransform::Ptr _next) { this->m_nextTransform = _next; return *this; }

		/// @b Transform a world-space location to the image space.
		virtual glm::vec4 to_image(glm::vec4 world_space_location) const = 0;

		/// @b Transform a image-space location to the world space.
		virtual glm::vec4 to_world(glm::vec4 grid_space_position) const	= 0;

	protected:
		/// @b The transform type for this transform.
		/// @n Used for dynamic_pointer_casts<> to the underlying type. Should never be set explicitely anywhere, except
		/// in the constructor of the derived types from this class.
		TransformType m_transformType;

		/// @b The next transform in the transform chain.
		ITransform::Ptr m_nextTransform;

};

#endif // VISUALISATION_IMAGE_TRANSFORM_INCLUDE_TRANSFORM_INTERFACE_HPP_
