#ifndef QT_INCLUDE_TEXTURE_CONTROL_HPP_
#define QT_INCLUDE_TEXTURE_CONTROL_HPP_

#include <QWidget>
#include <QPushButton>

#include "../../viewer/include/texture_viewer.hpp"

class texture_controller : public QWidget {
		Q_OBJECT
	public:
		texture_controller(const texture_viewer* viewer);
	private:
		/**
		 * @brief Button to request texture loading
		 */
		QPushButton* load_tex_button;
		/**
		 * @brief Button to request texture deletion
		 */
		QPushButton* free_tex_button;
};

#endif // QT_INCLUDE_TEXTURE_CONTROL_HPP_
