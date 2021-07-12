#ifndef TRANSFORM_STACK_HPP
#define TRANSFORM_STACK_HPP

#include "./transform_interface.hpp"
#include "./affine_transform.hpp"
#include "./trs_transform.hpp"

/// @brief This class represents a stack of transforms.
/// @details In addition, it also computes a pre-computed matrix transform. This will likely change in the future, once
/// we migrate away from supporting _only_ matrix transforms. Thus, the precomputed matrix represents the whole
/// transformation stack for now. It is updated every time a matrix is pushed to/popped from the stack.
class TransformStack {
	public:
		/// @brief Pointer type to the transform stack.
		using Ptr = std::shared_ptr<TransformStack>;

	public:
		/// @brief Default ctor for a transform stack, only initializes a default transform.
		TransformStack(void);
		/// @brief Default dtor, removing a reference to the head, which should remove any dangling references
		~TransformStack(void) = default;

		/// @brief Add a transform to the stack; recomputing the cached compoited transform.
		TransformStack& pushTransform(ITransform::Ptr _transform);

		/// @brief Remove the top transform from the stack,
		ITransform::Ptr popTransform(void);

		/// @brief Swap the two transforms in the stack, and update the cached composition of matrices.
		TransformStack& swap(ITransform::Ptr lhs, ITransform::Ptr rhs);

		/// @brief Transform a point from world space, to image space using the precomputed transforms.
		glm::vec4 to_image(glm::vec4 world_space_location) const;

		/// @brief Transform a point from image space, to world space using the precomputed transforms.
		glm::vec4 to_world(glm::vec4 image_space_location) const;

		/// @brief Get the precomputed matrix
		MatrixTransform::Ptr getPrecomputedMatrix();

	protected:
		/// @brief Update the precomputed matrix transform in this class.
		void update_precomputed_matrix();

	protected:
		/// @brief Current top transform of the stack.
		ITransform::Ptr head_transform;

		/// @brief Quick access to the number of transforms in the stack
		std::size_t transform_stack_depth;

		/// @brief The result of the pre-computation of the available transforms.
		MatrixTransform::Ptr precomputedTransform;
};

#endif // TRANSFORM_STACK_HPP
