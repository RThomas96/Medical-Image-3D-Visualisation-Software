#ifndef TRANSFORM_STACK_HPP
#define TRANSFORM_STACK_HPP

#include "./transform_interface.hpp"
#include "./affine_transform.hpp"
#include "./trs_transform.hpp"

/// @b This class represents a stack of transforms. It computes a
class TransformStack {
	public:
		/// @b Pointer type to the transform stack.
		using Ptr = std::shared_ptr<TransformStack>;
	public:
		/// @b Default ctor for a transform stack, only initializes a default transform.
		TransformStack(void);
		/// @b Default dtor, removing a reference to the head, which should remove any dangling references
		~TransformStack(void) = default;

		/// @b Add a transform to the stack; recomputing the cached compoited transform.
		TransformStack& pushTransform(ITransform::Ptr _transform);

		/// @b Remove the top transform from the stack,
		ITransform::Ptr popTransform(void);

		/// @b Swap the two transforms in the stack, and update the cached composition of matrices.
		TransformStack& swap(ITransform::Ptr lhs, ITransform::Ptr rhs);

		/// @b Transform a point from world space, to image space using the precomputed transforms.
		glm::vec4 to_image(glm::vec4 world_space_location) const;

		/// @b Transform a point from image space, to world space using the precomputed transforms.
		glm::vec4 to_world(glm::vec4 image_space_location) const;

	protected:
		/// @b Update the precomputed matrix transform in this class.
		void update_precomputed_matrix();

	protected:
		/// @b Current top transform of the stack.
		ITransform::Ptr head_transform;

		/// @b Quick access to the number of transforms in the stack
		std::size_t transform_stack_depth;

		/// @b The result of the pre-computation of the available transforms.
		MatrixTransform::Ptr precomputedTransform;
};

#endif // TRANSFORM_STACK_HPP
