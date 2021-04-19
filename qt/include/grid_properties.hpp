#ifndef VISUALIZATION_QT_INCLUDE_GRID_PROPERTIES_HPP_
#define VISUALIZATION_QT_INCLUDE_GRID_PROPERTIES_HPP_

#include "../../grid/include/discrete_grid.hpp"
#include "../../viewer/include/scene.hpp"
#include "../../viewer/include/viewer_structs.hpp"

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

class GridPropertiesPanel : public QWidget {
	public:
		GridPropertiesPanel(void);
		~GridPropertiesPanel(void);
	protected:
		GridGLView::Ptr controlledGrid;
		QLabel* label_textureMin;
		QLabel* label_textureMax;
		QLabel* label_textureValue;
		QLabel* label_colorMin;
		QLabel* label_colorMax;
};

#endif // VISUALIZATION_QT_INCLUDE_GRID_PROPERTIES_HPP_
