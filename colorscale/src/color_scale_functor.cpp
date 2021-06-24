#include "../include/color_scale_functor.hpp"

namespace Color {

ColorScaleFunctor::ColorScaleFunctor(ColorScaleFunctor::functor_type&& _functor) :
		ColorScaleBase(), sampling_functor(std::forward<functor_type&>(_functor)) {
	this->name_user = "functor_scale";
	this->cs_type = ColorScale_t::Functor;
	QObject::connect(this, &ColorScaleFunctor::functorChanged, this, &ColorScaleFunctor::dataChanged);
}

ColorScaleFunctor::~ColorScaleFunctor() {
	QObject::disconnect(this, &ColorScaleFunctor::functorChanged, this, &ColorScaleFunctor::dataChanged);
}

glm::vec3 ColorScaleFunctor::sample(float sample_factor) const { return this->sampling_functor(sample_factor); }

std::vector<glm::vec3> ColorScaleFunctor::getColorScale() const { return this->getColorScale(this->sample_count); }

std::vector<glm::vec3> ColorScaleFunctor::getColorScale(std::size_t number_of_steps) const {
	std::vector<glm::vec3> color_data(number_of_steps);
	float max_f = static_cast<float>(number_of_steps-1);
	for (std::size_t i = 0; i < number_of_steps; ++i) { color_data[i] = this->sample(static_cast<float>(i)/max_f); }
	return color_data;
}

}
