#ifndef QT_INCLUDE_MAIN_WIDGET_HPP_
#define QT_INCLUDE_MAIN_WIDGET_HPP_

#include <QWidget>

#include "../../viewer/include/texture_viewer.hpp"
#include "./texture_sliders.hpp"
#include "./texture_control.hpp"

class main_widget : public QWidget {
	public:
		/**
		 * @brief The default constructor of the main widget
		 */
		explicit main_widget(QWidget* parent = nullptr);
	protected:
		/**
		 * @brief Texture viewer embedded within this widget.
		 * @note Will always be extended to the full widget width.
		 */
		texture_viewer* tex_viewer;
		/**
		 * @brief Sliders for the current OpenGL context
		 */
		texture_sliders* tex_sliders;
		/**
		 * @brief Controller for the textures of the current opengl context.
		 */
		texture_controller* tex_controller;
};

#endif // QT_INCLUDE_MAIN_WIDGET_HPP_
