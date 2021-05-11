#include "../include/transform_stack.hpp"

TransformStack::TransformStack(void) {
	this->head_transform = std::make_shared<DefaultTransform>();
	this->transform_stack_depth = 1;
	this->precomputedTransform = this->head_transform;
}

TransformStack& TransformStack::pushTransform(ITransform::Ptr _transform) {
	// set the incoming's next pointer to the current head, and then set the head to the next pointer :
	_transform->setNextTransform(this->head_transform);
	this->head_transform = _transform;
	// increment stack depth, and
	this->transform_stack_depth++;
	this->update_precomputed_matrix();
}

ITransform::Ptr TransformStack::popTransform() {
	// if head was null, create a new default transform :
	if (this->head_transform == nullptr) {
		this->head_transform = std::make_shared<DefaultTransform>();
		this->transform_stack_depth = 1;
	}

	// get head transform :
	ITransform::Ptr last_head = this->head_transform;
	ITransform::Ptr next_head = this->head_transform->nextTransform();

	// If head has a next transform, set it as head and return it :
	if (next_head != nullptr) {
		this->head_transform = next_head;
		this->transform_stack_depth--;
	} else {
		// we don't know if the head transform was a default one, create a new default one
		this->head_transform = std::make_shared<DefaultTransform>();
		this->transform_stack_depth = 1;
		if (last_head->transformType() != TransformType::Default) {
			last_head->setNextTransform(this->head_transform);
		}
	}

	this->update_precomputed_matrix();

	return last_head;
}

glm::vec4 TransformStack::to_image(glm::vec4 world_space_location) const {
	return this->precomputedTransform->to_image(world_space_location);
}

glm::vec4 TransformStack::to_world(glm::vec4 image_space_location) const {
	return this->precomputedTransform->to_world(image_space_location);
}

void TransformStack::update_precomputed_matrix() {
	// build a stack of the known matrices :
	ITransform::Ptr iterator = this->head_transform;
	// stack of matrices :
	std::vector<glm::mat4> matrices;

	while (iterator != nullptr) {
		switch (iterator->transformType()) {
			case TransformType::TRS : {
				std::shared_ptr<TRSTransform> trs = std::dynamic_pointer_cast<TRSTransform>(iterator);
				// We never know, might happen sometimes :/
				if (trs != nullptr) { matrices.push_back(trs->matrix()); }
			}
			break;

			case TransformType::Affine_Matrix: {
				std::shared_ptr<MatrixTransform> m = std::dynamic_pointer_cast<MatrixTransform>(iterator);
				// We never know, might happen sometimes :/
				if (m != nullptr) { matrices.push_back(m->matrix()); }
			}
			break;

			case TransformType::Default:
				matrices.push_back(glm::identity<glm::mat4>());
				[[fallthrough]];
			case TransformType::Unknown:
			break;
		}
		// advance in the list
		iterator = iterator->nextTransform();
	}

	glm::mat4 resultMatrix = glm::identity<glm::mat4>();

	std::for_each(matrices.crend(), matrices.crbegin(), [&resultMatrix](const glm::mat4 curMatrix) {
		resultMatrix = resultMatrix * curMatrix;
	});
	this->precomputedTransform = std::make_shared<MatrixTransform>(resultMatrix);

	return;
}
