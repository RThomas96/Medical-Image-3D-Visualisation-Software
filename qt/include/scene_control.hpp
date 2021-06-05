#ifndef QT_INCLUDE_SCENE_CONTROL_HPP_
#define QT_INCLUDE_SCENE_CONTROL_HPP_

#include "../../macros.hpp"
#include "../../features.hpp"
#include "../../grid/include/discrete_grid.hpp"

#include "./double_slider.hpp"

#include <QLabel>
#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QGridLayout>

#include <iostream>

#define COLOR_CONTROL

class Scene; // forward declaration
class Viewer; // forward declaration

class ColorButton : public QWidget {
	Q_OBJECT
	public:
		ColorButton(QColor _color, QWidget* parent = nullptr);
		virtual ~ColorButton(void) = default;
	signals:
		void colorChanged(QColor color);
	public slots:
		void setColor(QColor _color);
	public:
		QColor getColor(void) const;
	protected:
		QPushButton* button;
		QPixmap* pixmap;
		QIcon* icon;
		QColor color;
		QVBoxLayout* layout;
};

class ColorBoundsControl : public QWidget {
	Q_OBJECT
	public:
		ColorBoundsControl(Scene* scene, bool _primary, QWidget* parent = nullptr);
		virtual ~ColorBoundsControl(void);
	signals:
		void minChanged(int val);
		void maxChanged(int val);
	public:
		int getMin(void) const;
		int getMax(void) const;
	protected:
		void getCurrentValues();
	protected:
		bool _primary;
		Scene* scene;
		QSpinBox* sb_min;
		QSpinBox* sb_max;
		QGridLayout* layout;
};

class ControlPanel : public QWidget {
		Q_OBJECT
	public:
		ControlPanel(Scene* const scene, Viewer* lv, QWidget* parent = nullptr);
		~ControlPanel();
	protected:
		void initSignals(void);
		void updateViewers(void);
	public slots:
		void updateLabels();
		void updateValues(void);
		void updateMinValue(int val);
		void updateMaxValue(int val);
		void updateMinValueAlternate(int val);
		void updateMaxValueAlternate(int val);
		void updateRGBMode(void);
		void updateChannelRed(int value);
		void updateChannelGreen(int value);
		void launchRedColorBounds(void);
		void launchGreenColorBounds(void);
	private:
		/// @b The scene to control !
		Scene* const sceneToControl;

		/// @b Box regrouping the controls of the red channel
		QGroupBox* groupbox_red;
		/// @b Box regrouping the controls of the green channel
		QGroupBox* groupbox_green;

		/// @b Range controller for the red texture bounds
		DoubleSlider* rangeslider_red;
		/// @b Range controller for the green texture bounds
		DoubleSlider* rangeslider_green;

		/// @b The layout of the red groupbox
		QHBoxLayout* layout_widgets_red;
		/// @b The layout of the green groupbox
		QHBoxLayout* layout_widgets_green;

		/// @b Minimum color of the color segment for the red channel
		ColorButton* colorbutton_red_min;
		/// @b Maximum color of the color segment for the red channel
		ColorButton* colorbutton_red_max;
		/// @b Minimum color of the color segment for the green channel
		ColorButton* colorbutton_green_min;
		/// @b Minimum color of the color segment for the green channel
		ColorButton* colorbutton_green_max;

		/// @b Button to launch a dialog to change the color bounds or the red channel
		QPushButton* button_red_colorbounds;
		ColorBoundsControl* cb_red_bounds;
		/// @b Button to launch a dialog to change the color bounds or the green channel
		QPushButton* button_green_colorbounds;
		ColorBoundsControl* cb_green_bounds;

		/// @b Picker for the coloration method of the red channel
		QComboBox* red_coloration;
		/// @b Picker for the coloration method of the green channel
		QComboBox* green_coloration;

		/// @b The viewer to update on value changes
		Viewer* const viewer;
		/// @b Texture bounds for red channel
		DiscreteGrid::data_t min, max;
		/// @b Texture bounds for green channel
		DiscreteGrid::data_t minAlternate, maxAlternate;

	public slots:
		void setMinTexVal(int val);
		void setMaxTexVal(int val);
		void setMinTexValBottom(int val);
		void setMaxTexValBottom(int val);
		void setClipDistance(double val);
};

#endif // QT_INCLUDE_SCENE_CONTROL_HPP_
