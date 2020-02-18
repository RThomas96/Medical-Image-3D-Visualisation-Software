#ifndef TESTS_3D_TEXTURE_TEST_INCLUDE_SLIDER_WIDGET_HPP_
#define TESTS_3D_TEXTURE_TEST_INCLUDE_SLIDER_WIDGET_HPP_

#include <QWidget>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "./3D_texture_viewer.hpp"

class slider_widget : public QWidget {
		Q_OBJECT
	private:
		/**
		 * @brief Slider for the minimum X tex value
		 */
		QSlider* min_x_slider;
		/**
		 * @brief Slider for the minimum Y tex value
		 */
		QSlider* min_y_slider;
		/**
		 * @brief Slider for the minimum Z tex value
		 */
		QSlider* min_z_slider;
		/**
		 * @brief Slider for the maximum X tex value
		 */
		QSlider* max_x_slider;
		/**
		 * @brief Slider for the maximum Y tex value
		 */
		QSlider* max_y_slider;
		/**
		 * @brief Slider for the maximum Z tex value
		 */
		QSlider* max_z_slider;
	public:
		/**
		 * @brief Default constructor for the slider widget.
		 * @param viewer the viewer to hold onto (to change texture values)
		 */
		slider_widget(simple_3D_texture_viewer* viewer);
	private:
		/**
		 * @brief Set the minimum value of the given slider to the given value
		 * @param slider_to_change The slider to change
		 * @param new_min_value The new min value to apply
		 */
		void set_min_value(QSlider* slider_to_change, int new_min_value);
		/**
		 * @brief Set the maximum value of the given slider to the given value
		 * @param slider_to_change The slider to change
		 * @param new_min_value The new min value to apply
		 */
		void set_max_value(QSlider* slider_to_change, int new_max_value);
	signals:
		/**
		 * @brief Sets the max X coordinate for the texture.
		 * @param x the max coordinate for the texture
		 */
		void set_Max_X_Texture(double x);
		/**
		 * @brief Sets the max Y coordinate for the texture.
		 * @param y the max coordinate for the texture
		 */
		void set_Max_Y_Texture(double y);
		/**
		 * @brief Sets the max Z coordinate for the texture.
		 * @param z the max coordinate for the texture
		 */
		void set_Max_Z_Texture(double z);
		/**
		 * @brief Sets the min X coordinate for the texture.
		 * @param x the max coordinate for the texture
		 */
		void set_Min_X_Texture(double x);
		/**
		 * @brief Sets the min Y coordinate for the texture.
		 * @param y the max coordinate for the texture
		 */
		void set_Min_Y_Texture(double y);
		/**
		 * @brief Sets the min Z coordinate for the texture.
		 * @param z the max coordinate for the texture
		 */
		void set_Min_Z_Texture(double z);
	protected slots:
		/**
		 * @brief Triggered when min x value changed
		 * @param x the new slider value
		 */
		void min_x_value_changed(int x);
		/**
		 * @brief Triggered when min y value changed
		 * @param y the new slider value
		 */
		void min_y_value_changed(int y);
		/**
		 * @brief Triggered when min z value changed
		 * @param z the new slider value
		 */
		void min_z_value_changed(int z);
		/**
		 * @brief Triggered when max x value changed
		 * @param x the new slider value
		 */
		void max_x_value_changed(int x);
		/**
		 * @brief Triggered when max y value changed
		 * @param y the new slider value
		 */
		void max_y_value_changed(int y);
		/**
		 * @brief Triggered when max z value changed
		 * @param z the new slider value
		 */
		void max_z_value_changed(int z);
};

#endif // TESTS_3D_TEXTURE_TEST_INCLUDE_SLIDER_WIDGET_HPP_
