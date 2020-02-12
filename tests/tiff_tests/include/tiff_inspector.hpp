#ifndef TESTS_TIFF_TESTS_INCLUDE_TIFF_INSPECTOR_HPP_
#define TESTS_TIFF_TESTS_INCLUDE_TIFF_INSPECTOR_HPP_

#include <QLabel>
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QPlainTextEdit>

#include "tiff_displayer.hpp"

class tiff_inspector : public QWidget {
		Q_OBJECT
	public:
		tiff_inspector(QWidget* parent);
	protected:
		/**
		 * @brief Updates image labels after the user clicked on 'Set image'
		 */
		void update_labels_for_image_data();
	protected slots:
		/**
		 * @brief Triggered when the user clicked on 'Set image'
		 */
		void on_set_image_clicked();
	private:
		/**
		 * @brief Button to open a new image
		 */
		QPushButton* open_image_button;
		/**
		 * @brief File name of the currently opened image
		 */
		QString file_name;
		/**
		 * @brief Label to display the image name
		 */
		QLabel* image_name_label;
		/**
		 * @brief Label to display file size (in bytes/kbytes/mbytes)
		 */
		QLabel* image_size_label;
		/**
		 * @brief Label to display file dimensions (pixels)
		 */
		QLabel* image_dims_label;
		/**
		 * @brief Label to display the time it took to read the image.
		 */
		QLabel* image_time_label;
		/**
		 * @brief Label to display more file info
		 */
		QPlainTextEdit* image_info_label;
		/**
		 * @brief Display area for TIFF files
		 */
		tiff_display* tiff_displayer;
};

#endif // TESTS_TIFF_TESTS_INCLUDE_TIFF_INSPECTOR_HPP_
