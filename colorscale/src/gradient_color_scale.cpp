#include "../include/gradient_color_scale.hpp"

namespace Color {

SimpleGradientColorScale::SimpleGradientColorScale(glm::vec3 _min, glm::vec3 _max) :
		ColorScaleBase(), min_color(_min), max_color(_max) {
	// Set a default name quietly, without firing the signal :
	this->name_user = "user_gradient";
	this->cs_type = ColorScale_t::SimpleGradient;
}

glm::vec3 SimpleGradientColorScale::sample(float sample_point) const {
	return glm::mix(this->min_color, this->max_color, sample_point);
}

std::vector<glm::vec3> SimpleGradientColorScale::getColorScale() const{
	return this->getColorScale(this->sample_count);
}

std::vector<glm::vec3> SimpleGradientColorScale::getColorScale(std::size_t number_of_steps) const {
	// Create vector and variables for the loop :
	std::vector<glm::vec3> color_data(number_of_steps);

	float steps = static_cast<float>(number_of_steps - 1u);
	for (std::size_t i = 0; i < number_of_steps; ++i) { color_data[i] = this->sample(static_cast<float>(i) / steps); }

	return color_data;
}

}
