#ifndef VISUALIZATION_QT_INCLUDE_DOUBLE_SLIDER_HPP_
#define VISUALIZATION_QT_INCLUDE_DOUBLE_SLIDER_HPP_

#include "./range_slider.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QWidget>

/// @ingroup qtwidgets
/// @brief Was supposed to provide a range slider, allowing to specify min and max values over a predefined range.
/// @warning Does not work, and is not used in the code.
class DoubleSlider : public QWidget {
	Q_OBJECT
public:
	DoubleSlider(QWidget* parent = nullptr);
	virtual ~DoubleSlider(void) = default;

public:
	void disable(bool _dis);
public slots:
	void setRange(int min, int max);
	void setMin(int min);
	void setMax(int max);
	void setMinValue(int min);
	void setMaxValue(int max);
signals:
	void minChanged(int val);
	void maxChanged(int val);

protected:
	void changeMin(int val);
	void changeMax(int val);
	void updateLabels();
	void dumpSliderState();

protected:
	QLabel* label_header_min;
	QLabel* label_header_max;
	QLabel* label_min_header_current;
	QLabel* label_value_min;
	QLabel* label_value_max;
	QLabel* label_min_value_current;
	RangeSlider* value_slider;
	QGridLayout* layout_grid;
};

#endif	  // VISUALIZATION_QT_INCLUDE_DOUBLE_SLIDER_HPP_
