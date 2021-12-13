#include "../include/affine_transform.hpp"

MatrixTransform::MatrixTransform(void) :
	ITransform(TransformType::Default) {
	this->m_matrix		 = glm::identity<glm::mat4>();
	this->inverse_matrix = this->m_matrix;
}

MatrixTransform::MatrixTransform(glm::mat4 _mat) :
	ITransform(TransformType::Affine_Matrix), m_matrix(_mat) {
	this->inverse_matrix = glm::inverse(this->m_matrix);
}

MatrixTransform::MatrixTransform(glm::mat3 _mat) :
	ITransform(TransformType::Affine_Matrix) {
	this->m_matrix = glm::mat4(_mat);
}

glm::mat4 MatrixTransform::matrix() const {
	return this->m_matrix;
}

glm::vec4 MatrixTransform::to_image(glm::vec4 world_space_location) const {
	return this->m_matrix * world_space_location;
}

glm::vec4 MatrixTransform::to_world(glm::vec4 image_space_location) const {
	return this->inverse_matrix * image_space_location;
}

glm::vec4 DefaultTransform::to_image(glm::vec4 world_space_location) const {
	return world_space_location;
}
glm::vec4 DefaultTransform::to_world(glm::vec4 image_space_location) const {
	return image_space_location;
}
