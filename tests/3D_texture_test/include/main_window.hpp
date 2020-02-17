#ifndef TESTS_3D_TEXTURE_TEST_INCLUDE_MAIN_WINDOW_HPP_
#define TESTS_3D_TEXTURE_TEST_INCLUDE_MAIN_WINDOW_HPP_

#include <QMainWindow>

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
};

#endif // TESTS_3D_TEXTURE_TEST_INCLUDE_MAIN_WINDOW_HPP_
