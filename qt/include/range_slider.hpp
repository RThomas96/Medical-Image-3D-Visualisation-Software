#ifndef VISUALIZATION_QT_INCLUDE_RANGE_SLIDER_HPP_
#define VISUALIZATION_QT_INCLUDE_RANGE_SLIDER_HPP_

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QGridLayout>

class RangeSlider : public QWidget {
		Q_OBJECT
	public:
		RangeSlider(QWidget* parent = nullptr);
		virtual ~RangeSlider(void) = default;
	public:
		virtual void setDisabled(bool _dis) override;
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
		void updateLabels(int unused_val);
	protected:
		QLabel* label_min_header_min;
		QLabel* label_min_header_max;
		QLabel* label_min_header_current;
		QLabel* label_min_value_min;
		QLabel* label_min_value_max;
		QLabel* label_min_value_current;
		QLabel* label_max_header_min;
		QLabel* label_max_header_max;
		QLabel* label_max_header_current;
		QLabel* label_max_value_min;
		QLabel* label_max_value_max;
		QLabel* label_max_value_current;
		QSlider* slider_min;
		QSlider* slider_max;
		QGridLayout* layout_grid;
};

#endif // VISUALIZATION_QT_INCLUDE_RANGE_SLIDER_HPP_
