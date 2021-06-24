#ifndef VISUALIZATION_COLORSCALE_INCLUDE_GRADIENT_COLOR_SCALE_HPP_
#define VISUALIZATION_COLORSCALE_INCLUDE_GRADIENT_COLOR_SCALE_HPP_

#include "./color_scale_base.hpp"

#include <glm/glm.hpp>

#include <QColor>
#include <QObject>

namespace Color {

	/// @b A simple gradient is defined as a color scale with only two points for the gradient.
	class SimpleGradientColorScale : public ColorScaleBase {

			Q_OBJECT;
			Q_PROPERTY(glm::vec3 min_color READ getMinColor WRITE setMinColor NOTIFY minColorChanged)
			Q_PROPERTY(glm::vec3 max_color READ getMaxColor WRITE setMaxColor NOTIFY maxColorChanged)

		public:
			/// @b Redefinition of the pointer type for this class.
			typedef std::shared_ptr<SimpleGradientColorScale> Ptr;

			/// @b Ctor for a simple 2-point gradient scale. Initializes the min and max points.
			SimpleGradientColorScale(glm::vec3 min, glm::vec3 max);

			/// @b Default dtor for a simple gradient color scale.
			~SimpleGradientColorScale(void);

			/// @b Sets the minimum color of the color scale
			void setMinColor(glm::vec3 _min);

			/// @b Sets the maximum color of the color scale
			void setMaxColor(glm::vec3 _max);

			/// @b Returns the minimum color of the color scale
			glm::vec3 getMinColor() const;

			/// @b Returns the maximum color of the color scale
			glm::vec3 getMaxColor() const;

			/// @b Samples the color scale at a certain point between O (min) and 1 (max).
			virtual glm::vec3 sample(float sample_point) const override;

			/// @b Allows to sample the whole color scale, using the internal number of samples.
			virtual std::vector<glm::vec3> getColorScale() const override;

			/// @b Allows to sample the whole color scale, using a user-defined number of samples.
			virtual std::vector<glm::vec3> getColorScale(std::size_t number_of_steps) const override;

		signals:
			/// @b Signal fired when the min color is changed
			void minColorChanged();
			/// @b Signal fired when the max color is changed
			void maxColorChanged();

		protected:
			/// @b The beginning color, returned when sampling at 0.
			glm::vec3 min_color;

			/// @b The ending color, returned when sampling at 1.
			glm::vec3 max_color;
	};

}

#endif // VISUALIZATION_COLORSCALE_INCLUDE_GRADIENT_COLOR_SCALE_HPP_
