#ifndef VISUALIZATION_COLOR_SCALE_INCLUDE_COLOR_SCALE_FUNCTOR_HPP_
#define VISUALIZATION_COLOR_SCALE_INCLUDE_COLOR_SCALE_FUNCTOR_HPP_

#include "./color_scale_base.hpp"

#include <functional>

namespace Color {

	/// @ingroup colorscales
	/// @brief The ColorScaleFunctor class defines a color scale which sampling equation is a `std::function<>` object.
	/// @details This specialization of the ColorScaleBase class allows to have lambda functions (any callable object)
	/// define a particular color scale. This is especially useful in the HSV-to-RGB color scale, where the function
	/// was already in the codebase, and only needed to be ported over to this class.
	/// @note Not used yet in the visualization pipeline.
	class ColorScaleFunctor : public ColorScaleBase {
		public:
			/// @brief Functor type accepted by the color scale
			typedef std::function<glm::vec3(float)> functor_type;

			Q_OBJECT;
			Q_PROPERTY(functor_type sampling_functor READ getFunctor WRITE setFunctor NOTIFY functorChanged);

		public:
			/// @brief Redefinition of the pointer type for this class.
			typedef std::shared_ptr<ColorScaleFunctor> Ptr;

			/// @brief Default ctor for the function, accepts a functor and initializes the sampler based on it.
			ColorScaleFunctor(functor_type&& _functor);

			/// @brief Dtor for the color scale
			~ColorScaleFunctor(void);

			/// @brief Returns the functor type used in the class instance.
			functor_type getFunctor(void) const;

			/// @brief Sets a new sampling functor
			void setFunctor(functor_type&& _func);

			/// @brief Samples the color scale at a certain point between O (min) and 1 (max).
			virtual glm::vec3 sample(float sample_factor) const override;

			/// @brief Allows to sample the whole color scale, using the internal number of samples.
			virtual std::vector<glm::vec3> getColorScale() const override;

			/// @brief Allows to sample the whole color scale, using a user-defined number of samples.
			virtual std::vector<glm::vec3> getColorScale(std::size_t number_of_steps) const override;

		signals:
			/// @brief Signals the functor in the color scale has changed, and it's time to update it.
			void functorChanged();

		protected:
			std::function<glm::vec3(float)> sampling_functor;
	};
}

#endif // VISUALIZATION_COLOR_SCALE_INCLUDE_COLOR_SCALE_FUNCTOR_HPP_
