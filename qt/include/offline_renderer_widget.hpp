#ifndef QT_INCLUDE_OFFLINE_RENDERER_WIDGET_HPP_
#define QT_INCLUDE_OFFLINE_RENDERER_WIDGET_HPP_

#include "../../image/include/image_storage.hpp"

#include <QWidget>
#include <QPushButton>
#include <QComboBox>

#include <memory>
#include <vector>

class OfflineWidget : public QWidget {
	public:
		OfflineWidget(void);
		~OfflineWidget(void);
	protected:
		QPushButton* button_loadImages; ///< A button to load images
		QPushButton* button_populateGrid; ///< A button to populate the grid
		QPushButton* button_saveGrid; ///< A button to save the generated grid to a file
		QComboBox* dropdown_interpolationMethodPicker; ///< Allows to choose an interpolation method
};

#endif // QT_INCLUDE_OFFLINE_RENDERER_WIDGET_HPP_
