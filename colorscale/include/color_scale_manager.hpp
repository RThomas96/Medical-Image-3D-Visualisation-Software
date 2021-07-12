#ifndef VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_MANAGER_HPP_
#define VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_MANAGER_HPP_

#include "./color_scale_base.hpp"
#include "./gradient_color_scale.hpp"
#include "./color_scale_functor.hpp"

#include <QObject>
#include <QWidget>

namespace Color {

	/// @ingroup colorscales
	/// @brief The ColorScaleManager class is a super-object managing the color scales available in the program.
	/// @details Ideally, this class will be able not only to manage color scales (add them, modify them, delete them)
	/// but should also serve as a point of entry for any widget that may display the available color scales. It should
	/// also be useful to make color scales resident in GPU memory, and allow them to be bound to the current draw call.
	/// @note Not used yet in the visualization pipeline.
	class ColorScaleManager : public QObject {
			Q_OBJECT;

		public:
			/// @brief The iterator type for the class, in order to iterate on known color scales.
			typedef std::vector<ColorScaleBase::Ptr>::iterator iterator;
			/// @brief The const iterator type for the class, in order to iterate on known color scales.
			typedef std::vector<ColorScaleBase::Ptr>::const_iterator const_iterator;
			/// @brief The reverse iterator type for the class, in order to iterate backward on known color scales.
			typedef std::vector<ColorScaleBase::Ptr>::reverse_iterator reverse_iterator;
			/// @brief The const reverse iterator type for the class, in order to iterate backward on known color scales.
			typedef std::vector<ColorScaleBase::Ptr>::const_reverse_iterator const_reverse_iterator;

		public:
			/// @brief Default ctor for the manager. Builds the default color scales.
			ColorScaleManager(void);

			/// @brief Default dtor for the class. Removes thecolor scales from the program.
			~ColorScaleManager(void);

			/// @brief Add a color scale to the manager
			void addColorScale(ColorScaleBase::Ptr _to_add);

			/// @brief Returns the color scale at index 'i' within the vector.S
			ColorScaleBase::Ptr getColorScale(int i) const;

			/// @brief Remove the given color scale from the vector.
			void removeColorScale(ColorScaleBase::Ptr to_remove);

			/// @brief Add a gradient color scale to the manager.
			SimpleGradientColorScale::Ptr addGradientScale(glm::vec3 _min, glm::vec3 _max);

			/// @brief Add a functor to the available color scales.
			ColorScaleFunctor::Ptr addFunctorScale(ColorScaleFunctor::functor_type&& _functor);

			/// @brief Return the default greyscale color scale.
			SimpleGradientColorScale::Ptr getDefaultColorScale_greyscale();

			/// @brief Return the default greyscale color scale.
			ColorScaleFunctor::Ptr getDefaultColorScale_hsv2rgb();

			/// @brief Returns the beginning of the stored color scales in this program.
			iterator begin() { return this->color_scale_user.begin(); }
			/// @brief Returns the end of the stored color scales in this program.
			iterator end() { return this->color_scale_user.end(); }

			/// @brief Returns the read-only beginning of the stored color scales in this program.
			const_iterator cbegin() { return this->color_scale_user.cbegin(); }
			/// @brief Returns the read-only end of the stored color scales in this program.
			const_iterator cend() { return this->color_scale_user.cend(); }

			/// @brief Returns the reverse beginning of the stored color scales in this program.
			reverse_iterator rbegin() { return this->color_scale_user.rbegin(); }
			/// @brief Returns the reverse end of the stored color scales in this program.
			reverse_iterator rend() { return this->color_scale_user.rend(); }

			/// @brief Returns the read-only reverse beginning of the stored color scales in this program.
			const_reverse_iterator crbegin() { return this->color_scale_user.crbegin(); }
			/// @brief Returns the read-only reverse end of the stored color scales in this program.
			const_reverse_iterator crend() { return this->color_scale_user.crend(); }

		signals:
			/// @brief Signal fired whenever a color scale is added to the available color scales.
			/// @details The color scale in argument is the color scale added to the program.
			void addedColorScale(ColorScaleBase::Ptr _new_color);

			/// @brief Signal fired whenever a color scale is removed to the available color scales.
			/// @details The pointer in argument is the color scale removed.
			void removedColorScale(ColorScaleBase::Ptr _to_remove);

		protected:
			/// @brief Intialize the default color scales with their data
			void init_default_color_scales();

		protected:
			/// @brief The default color scale. Simple greyscale coloring of the grid.
			SimpleGradientColorScale::Ptr greyscale;
			/// @brief The HSV to RGB color scale, useful for highly contrasted visualization.
			ColorScaleFunctor::Ptr hsv2rgb;
			/// @brief All the other available color scales.
			std::vector<ColorScaleBase::Ptr> color_scale_user;
	};

}

#endif // VISUALIZATION_COLORSCALE_INCLUDE_COLOR_SCALE_MANAGER_HPP_
