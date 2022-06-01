#ifndef QT_INCLUDE_SCENE_CONTROL_HPP_
#define QT_INCLUDE_SCENE_CONTROL_HPP_

#include "../../features.hpp"
//#include "../../grid/include/discrete_grid.hpp"
#include "../../macros.hpp"

#include "./double_slider.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QWidget>

#include <iostream>

#define COLOR_CONTROL

class Scene;	// forward declaration
class Viewer;	 // forward declaration

/// @ingroup qtwidgets
/// @brief Provides a simple button with a pixmap as a label, allowing to color the button.
class ColorButton : public QWidget {
	Q_OBJECT
public:
	ColorButton(QColor _color, QWidget* parent = nullptr);
	virtual ~ColorButton(void);
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

/// @ingroup qtwidgets
/// @brief Provides a small window where one can provide a minimum and maximum value to normalize a color scale over.
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

/// @ingroup qtwidgets
/// @brief The ControlPanel class represents the widget present at the bottom of the program.
/// @details It allows to set the visible ranges of each color channel in the program, as well as allowing to select a
/// color scale, and min and max values to normalize the color scale over.
/// @note Even though this code was supposed to be for DiscreteGrid, all its signals and parameters are actually
/// controlled by the Scene class. Can be used with the new Grid interface.
class ControlPanel : public QWidget {
	Q_OBJECT
public:
	ControlPanel(Scene* const scene, Viewer* lv, QWidget* parent = nullptr);
	virtual ~ControlPanel();

	/// @brief Set min and max values of sliders according to the grids datatype numerical limit
	/// TODO: change this function to set sliders according to min/max values in image
	void setSlidersToNumericalLimits(void);

protected:
	void initSignals(void);
	void updateViewers(void);
public slots:
	void updateLabels();
	void updateValues(void);
	void updateMinValue(double val);
	void updateMaxValue(double val);
	void updateMinValueAlternate(double val);
	void updateMaxValueAlternate(double val);
	void updateRGBMode(void);
	void updateChannelRed(int value);
	void updateChannelGreen(int value);
	void launchRedColorBounds(void);
	void launchGreenColorBounds(void);

private:
	Scene* const sceneToControl;

	QGroupBox* groupbox_red;
	QGroupBox* groupbox_green;

	DoubleSlider* rangeslider_red;
	DoubleSlider* rangeslider_green;

	QHBoxLayout* layout_widgets_red;
	QHBoxLayout* layout_widgets_green;

	ColorButton* colorbutton_red_min;
	ColorButton* colorbutton_red_max;
	ColorButton* colorbutton_green_min;
	ColorButton* colorbutton_green_max;

	QPushButton* button_red_colorbounds;
	ColorBoundsControl* cb_red_bounds;

	QPushButton* button_green_colorbounds;
	ColorBoundsControl* cb_green_bounds;

	QComboBox* red_coloration;
	QComboBox* green_coloration;

	Viewer* const viewer;
	double min, max;
	double minAlternate, maxAlternate;

public slots:
	void setMinTexVal(double val);
	void setMaxTexVal(double val);
	void setMinTexValAlternate(double val);
	void setMaxTexValAlternate(double val);
};

#endif	  // QT_INCLUDE_SCENE_CONTROL_HPP_
