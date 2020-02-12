/**********************************************************************
 * FILE : qt/include/main_window.hpp
 * AUTH : Thibault de Villèle
 * DATE : 06/02/2020
 * DESC : The file for the main_window class, serving as a base for the
 *	  Qt application.
 **********************************************************************/

#ifndef VISUALISATION_QT_INCLUDE_MAIN_WINDOW_HPP
#define VISUALISATION_QT_INCLUDE_MAIN_WINDOW_HPP

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

#endif // VISUALISATION_QT_INCLUDE_MAIN_WINDOW_HPP
