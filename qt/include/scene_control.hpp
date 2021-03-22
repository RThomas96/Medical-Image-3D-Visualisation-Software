#ifndef QT_INCLUDE_SCENE_CONTROL_HPP_
#define QT_INCLUDE_SCENE_CONTROL_HPP_

#include "../../macros.hpp"
#include "../../features.hpp"
#include "../../grid/include/discrete_grid.hpp"

#include <QLabel>
#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include <iostream>

#define COLOR_CONTROL

class Scene; // forward declaration
class Viewer; // forward declaration

class ControlPanel : public QWidget {
		Q_OBJECT
	public:
		ControlPanel(Scene* const scene, Viewer* lv, QWidget* parent = nullptr);
		~ControlPanel();
	protected:
		void initSignals(void);
		void updateViewers(void);
	public slots:
		void activatePanels(bool activeStatus = true);
		void updateLabels();
		void updateValues(void);
		void updateMinValue(int val);
		void updateMaxValue(int val);
		void updateMinValueAlternate(int val);
		void updateMaxValueAlternate(int val);
	private:
		Scene* const sceneToControl;

		QSlider* minValueTexture_top; ///< Slider to determine the min value in the texture which constitutes viable information
		QSlider* maxValueTexture_top; ///< Slider to determine the max value in the texture which constitutes viable information
		QSlider* minValueTexture_bottom; ///< Slider to determine the min value in the texture which constitutes viable information
		QSlider* maxValueTexture_bottom; ///< Slider to determine the max value in the texture which constitutes viable information

		QLabel* label_top_tex_min_min;	///< Label for the min value of the texture slider (left)
		QLabel* label_top_tex_min_max;	///< Label for the max value of the texture slider (left)
		QLabel* label_top_tex_max_min;	///< Label for the min value of the texture slider (right)
		QLabel* label_top_tex_max_max;	///< Label for the max value of the texture slider (right)

		QLabel* label_top_tex_min_header;
		QLabel* label_top_tex_max_header;
		QLabel* label_top_tex_min_value;
		QLabel* label_top_tex_max_value;

		QLabel* label_bottom_tex_min_min;
		QLabel* label_bottom_tex_min_max;
		QLabel* label_bottom_tex_max_min;
		QLabel* label_bottom_tex_max_max;

		QLabel* label_bottom_tex_min_header;
		QLabel* label_bottom_tex_max_header;
		QLabel* label_bottom_tex_min_value;
		QLabel* label_bottom_tex_max_value;

		QWidget* controlContainer;
		Viewer* const viewer;
		DiscreteGrid::data_t min, max;
		DiscreteGrid::data_t minAlternate, maxAlternate;

	public slots:
		void setMinTexVal(int val);
		void setMaxTexVal(int val);
		void setMinTexValBottom(int val);
		void setMaxTexValBottom(int val);
		void setClipDistance(double val);
};

#endif // QT_INCLUDE_SCENE_CONTROL_HPP_
