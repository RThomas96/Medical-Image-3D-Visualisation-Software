#ifndef VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_MANAGER_HPP_
#define VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_MANAGER_HPP_

#include "./color_scale_base.hpp"
#include "./gradient_color_scale.hpp"
#include "./color_scale_functor.hpp"

#include <QObject>
#include <QWidget>

namespace Color {

	class ColorScaleManager : public QObject {
			Q_OBJECT;
		public:
			ColorScaleManager(void);
			~ColorScaleManager(void);

			/// @b Add a color scale to the manager
			void addColorScale(ColorScaleBase::Ptr _to_add);

			/// @b Returns the color scale at index 'i' within the vector.S
			ColorScaleBase::Ptr getColorScale(int i) const;

			/// @b Remove the given color scale from the vector.
			void removeColorScale(ColorScaleBase::Ptr to_remove);

			/// @b Add a gradient color scale to the manager.
			SimpleGradientColorScale::Ptr addGradientScale(glm::vec3 _min, glm::vec3 _max);

			/// @b Add a functor to the available color scales.
			ColorScaleFunctor::Ptr addFunctorScale(ColorScaleFunctor::functor_type&& _functor);

			/// @b Return the default greyscale color scale.
			SimpleGradientColorScale::Ptr getDefaultColorScale_greyscale();

			/// @b Return the default greyscale color scale.
			ColorScaleFunctor::Ptr getDefaultColorScale_hsv2rgb();

		signals:
			void addedColorScale(ColorScaleBase::Ptr _new_color);
			void removedColorScale(ColorScaleBase::Ptr _to_remove);

		protected:
			/// @b Intialize the default color scales with their data
			void init_default_color_scales();

		protected:
			SimpleGradientColorScale::Ptr greyscale;
			ColorScaleFunctor::Ptr hsv2rgb;
			std::vector<ColorScaleBase::Ptr> color_scale_user;
	};

}

#endif // VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_MANAGER_HPP_
