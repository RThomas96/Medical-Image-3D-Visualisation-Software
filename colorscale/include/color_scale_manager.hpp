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
			/// @b The iterator type for the class, in order to iterate on known color scales.
			typedef std::vector<ColorScaleBase::Ptr>::iterator iterator;
			/// @b The const iterator type for the class, in order to iterate on known color scales.
			typedef std::vector<ColorScaleBase::Ptr>::const_iterator const_iterator;
			/// @b The reverse iterator type for the class, in order to iterate backward on known color scales.
			typedef std::vector<ColorScaleBase::Ptr>::reverse_iterator reverse_iterator;
			/// @b The const reverse iterator type for the class, in order to iterate backward on known color scales.
			typedef std::vector<ColorScaleBase::Ptr>::const_reverse_iterator const_reverse_iterator;

		public:
			/// @b Default ctor for the manager. Builds the default color scales.
			ColorScaleManager(void);

			/// @b Default dtor for the class. Removes thecolor scales from the program.
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

			/// @b Returns the beginning of the stored color scales in this program.
			iterator begin() { return this->color_scale_user.begin(); }
			/// @b Returns the end of the stored color scales in this program.
			iterator end() { return this->color_scale_user.end(); }

			/// @b Returns the read-only beginning of the stored color scales in this program.
			const_iterator cbegin() { return this->color_scale_user.cbegin(); }
			/// @b Returns the read-only end of the stored color scales in this program.
			const_iterator cend() { return this->color_scale_user.cend(); }

			/// @b Returns the reverse beginning of the stored color scales in this program.
			reverse_iterator rbegin() { return this->color_scale_user.rbegin(); }
			/// @b Returns the reverse end of the stored color scales in this program.
			reverse_iterator rend() { return this->color_scale_user.rend(); }

			/// @b Returns the read-only reverse beginning of the stored color scales in this program.
			const_reverse_iterator crbegin() { return this->color_scale_user.crbegin(); }
			/// @b Returns the read-only reverse end of the stored color scales in this program.
			const_reverse_iterator crend() { return this->color_scale_user.crend(); }

		signals:
			/// @b Signal fired whenever a color scale is added to the available color scales.
			/// @details The color scale in argument is the color scale added to the program.
			void addedColorScale(ColorScaleBase::Ptr _new_color);

			/// @b Signal fired whenever a color scale is removed to the available color scales.
			/// @details The pointer in argument is the color scale removed.
			void removedColorScale(ColorScaleBase::Ptr _to_remove);

		protected:
			/// @b Intialize the default color scales with their data
			void init_default_color_scales();

		protected:
			/// @b The default color scale. Simple greyscale coloring of the grid.
			SimpleGradientColorScale::Ptr greyscale;
			/// @b The HSV to RGB color scale, useful for highly contrasted visualization.
			ColorScaleFunctor::Ptr hsv2rgb;
			/// @b All the other available color scales.
			std::vector<ColorScaleBase::Ptr> color_scale_user;
	};

}

#endif // VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_MANAGER_HPP_
