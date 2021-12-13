#ifndef VISUALIZATION_COLORSCALE_INCLUDE_GRADIENT_COLOR_SCALE_HPP_
#define VISUALIZATION_COLORSCALE_INCLUDE_GRADIENT_COLOR_SCALE_HPP_

#include "./color_scale_base.hpp"

#include <glm/glm.hpp>

#include <QColor>
#include <QObject>

namespace Color {

	/// @ingroup colorscales
	/// @brief The SimpleGradientColorScale is defined as a color scale with only two points for the gradient.
	/// @details As such, the color scale will effectively be a simple linear interpolation of the two colors over the
	/// range [0, 1].
	/// @note Not used yet in the visualization pipeline.
	class SimpleGradientColorScale : public ColorScaleBase {
		Q_OBJECT;
		Q_PROPERTY(glm::vec3 min_color READ getMinColor WRITE setMinColor NOTIFY minColorChanged)
		Q_PROPERTY(glm::vec3 max_color READ getMaxColor WRITE setMaxColor NOTIFY maxColorChanged)

	public:
		/// @brief Redefinition of the pointer type for this class.
		typedef std::shared_ptr<SimpleGradientColorScale> Ptr;

		/// @brief Ctor for a simple 2-point gradient scale. Initializes the min and max points.
		SimpleGradientColorScale(glm::vec3 min, glm::vec3 max);

		/// @brief Default dtor for a simple gradient color scale.
		~SimpleGradientColorScale(void);

		/// @brief Sets the minimum color of the color scale
		void setMinColor(glm::vec3 _min);

		/// @brief Sets the maximum color of the color scale
		void setMaxColor(glm::vec3 _max);

		/// @brief Returns the minimum color of the color scale
		glm::vec3 getMinColor() const;

		/// @brief Returns the maximum color of the color scale
		glm::vec3 getMaxColor() const;

		/// @brief Samples the color scale at a certain point between O (min) and 1 (max).
		virtual glm::vec3 sample(float sample_point) const override;

		/// @brief Allows to sample the whole color scale, using the internal number of samples.
		virtual std::vector<glm::vec3> getColorScale() const override;

		/// @brief Allows to sample the whole color scale, using a user-defined number of samples.
		virtual std::vector<glm::vec3> getColorScale(std::size_t number_of_steps) const override;

	signals:
		/// @brief Signal fired when the min color is changed
		void minColorChanged();
		/// @brief Signal fired when the max color is changed
		void maxColorChanged();

	protected:
		/// @brief The beginning color, returned when sampling at 0.
		glm::vec3 min_color;

		/// @brief The ending color, returned when sampling at 1.
		glm::vec3 max_color;
	};

}	 // namespace Color

#endif	  // VISUALIZATION_COLORSCALE_INCLUDE_GRADIENT_COLOR_SCALE_HPP_
