#include "../include/trs_transform.hpp"

#include <glm/ext/matrix_transform.hpp>	   // glm::translate, glm::rotate, glm::scale and glm::identity

TRSTransform::TRSTransform(void) :
	ITransform(TransformType::TRS) {
	this->m_scaling		= glm::vec3(1.f, 1.f, 1.f);
	this->m_rotation	= glm::vec3(.0f);
	this->m_translation = glm::vec3(.0f);
	this->update_cached_matrix();
}

glm::mat4 TRSTransform::matrix() const {
	return this->cached_matrix;
}

glm::vec3 TRSTransform::scaling() const {
	return this->m_scaling;
}

glm::vec3 TRSTransform::rotation() const {
	return this->m_rotation;
}

glm::vec3 TRSTransform::translation() const {
	return this->m_translation;
}

TRSTransform& TRSTransform::setScaling(glm::vec3 _new_scale) {
	this->m_scaling = _new_scale;
	this->update_cached_matrix();
	return *this;
}

TRSTransform& TRSTransform::setRotation(glm::vec3 _new_rotation) {
	this->m_rotation = _new_rotation;
	this->update_cached_matrix();
	return *this;
}

TRSTransform& TRSTransform::setTranslation(glm::vec3 _new_trans) {
	this->m_translation = _new_trans;
	this->update_cached_matrix();
	return *this;
}

void TRSTransform::update_cached_matrix() {
	// first, translation :
	glm::mat4 target_matrix = glm::translate(target_matrix, this->m_translation);
	// rotation around each axis :
	target_matrix = glm::rotate(target_matrix, this->m_rotation.z, glm::vec3(0.f, 0.f, 1.f));	 // z
	target_matrix = glm::rotate(target_matrix, this->m_rotation.y, glm::vec3(0.f, 1.f, 0.f));	 // y
	target_matrix = glm::rotate(target_matrix, this->m_rotation.x, glm::vec3(1.f, 0.f, 0.f));	 // x
	// finally, scaling :
	this->cached_matrix = glm::scale(target_matrix, this->m_scaling);
	// precompute inverse :
	this->cached_inverse_matrix = glm::inverse(this->cached_matrix);

	return;
}
