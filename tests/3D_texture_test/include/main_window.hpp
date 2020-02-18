#ifndef TESTS_3D_TEXTURE_TEST_INCLUDE_MAIN_WINDOW_HPP_
#define TESTS_3D_TEXTURE_TEST_INCLUDE_MAIN_WINDOW_HPP_

#include <QMainWindow>

#include "../include/3D_texture_viewer.hpp"
#include "../include/slider_widget.hpp"
#include "../include/container_widget.hpp"

class main_window : public QMainWindow {
		Q_OBJECT
	public:
		/**
		 * @brief Default constructor.
		 */
		main_window();
		/**
		 * @brief Define a size hint for the window
		 * @return The expected size of the window
		 */
		virtual QSize sizeHint() const override;
	private slots:
		/**
		 * @brief Slot called when a QWidget is added as a child to this main window.
		 */
		void on_add_new();
	private:
		container_widget* widget;
};

#endif // TESTS_3D_TEXTURE_TEST_INCLUDE_MAIN_WINDOW_HPP_
