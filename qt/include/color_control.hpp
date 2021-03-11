#ifndef VISUALIZATION_COLOR_CONTROL_HPP
#define VISUALIZATION_COLOR_CONTROL_HPP

#include "../../viewer/include/scene.hpp"
#include "../../grid/include/discrete_grid.hpp"
#include "./neighbor_visu_main_widget.hpp"
#include "./scene_control.hpp"

#include <QSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QPixmap>
#include <QColor>

class ColorBoundControl : public QWidget {
	Q_OBJECT
	public:
		ColorBoundControl(Scene* _sc, ControlPanel* _cp, MainWidget* main, QWidget* parent = nullptr);
		~ColorBoundControl(void);
	public slots:
		void setMinColorBound(int val);
		void setMaxColorBound(int val);
		void setColor0(void);
		void setColor1(void);
	protected:
		QSpinBox* spinbox_min;
		QSpinBox* spinbox_max;
		QLabel* label_min;
		QLabel* label_max;
		QGridLayout* grid;
		Scene* _scene;
		ControlPanel* _controlpanel;
		MainWidget* _main;
		DiscreteGrid::data_t _min;
		DiscreteGrid::data_t _max;

		// For color segment-based colouring :
		QLabel* label_color0;
		QLabel* label_color1;
		QPushButton* button_baseColor0;
		QPushButton* button_baseColor1;
		QPixmap* pixmap_baseColor0;
		QPixmap* pixmap_baseColor1;
		QColor color_0;
		QColor color_1;
};

#endif // VISUALIZATION_COLOR_CONTROL_HPP
