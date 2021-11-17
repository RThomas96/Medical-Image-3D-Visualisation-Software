#ifndef VISUALIZATION_QT_INCLUDE_RANGE_SLIDER_HPP_
#define VISUALIZATION_QT_INCLUDE_RANGE_SLIDER_HPP_

#include <QSlider>

class QStylePainter;
class RangeSliderPrivate;

/// @ingroup qtwidgets
/// @brief Provides a range slider, allowing to specify min and max values over a wider range.
/// @note This class is lifted directly from Kitware's ctkWidgets. Get them on GitHub.
/// @warning Should probably read up on how I can legally use this code...
class RangeSlider : public QSlider {
	Q_OBJECT
	Q_PROPERTY(int minimumValue READ minimumValue WRITE setMinimumValue)
	Q_PROPERTY(int maximumValue READ maximumValue WRITE setMaximumValue)
	Q_PROPERTY(int minimumPosition READ minimumPosition WRITE setMinimumPosition)
	Q_PROPERTY(int maximumPosition READ maximumPosition WRITE setMaximumPosition)
	Q_PROPERTY(bool symmetricMoves READ symmetricMoves WRITE setSymmetricMoves)
	Q_PROPERTY(QString handleToolTip READ handleToolTip WRITE setHandleToolTip)

public:
	// Superclass typedef
	typedef QSlider Superclass;
	/// Constructor, builds a RangeSlider that ranges from 0 to 100 and has
	/// a lower and upper values of 0 and 100 respectively, other properties
	/// are set the QSlider default properties.
	explicit RangeSlider(Qt::Orientation o, QWidget* par = 0);
	explicit RangeSlider(QWidget* par = 0);
	virtual ~RangeSlider();

	///
	/// This property holds the slider's current minimum value.
	/// The slider silently forces minimumValue to be within the legal range:
	/// minimum() <= minimumValue() <= maximumValue() <= maximum().
	/// Changing the minimumValue also changes the minimumPosition.
	int minimumValue() const;

	///
	/// This property holds the slider's current maximum value.
	/// The slider forces the maximum value to be within the legal range:
	/// The slider silently forces maximumValue to be within the legal range:
	/// Changing the maximumValue also changes the maximumPosition.
	int maximumValue() const;

	///
	/// This property holds the current slider minimum position.
	/// If tracking is enabled (the default), this is identical to minimumValue.
	int minimumPosition() const;
	void setMinimumPosition(int min);

	///
	/// This property holds the current slider maximum position.
	/// If tracking is enabled (the default), this is identical to maximumValue.
	int maximumPosition() const;
	void setMaximumPosition(int max);

	///
	/// Utility function that set the minimum position and
	/// maximum position at once.
	void setPositions(int min, int max);

	///
	/// When symmetricMoves is true, moving a handle will move the other handle
	/// symmetrically, otherwise the handles are independent. False by default
	bool symmetricMoves() const;
	void setSymmetricMoves(bool symmetry);

	///
	/// Controls the text to display for the handle tooltip. It is in addition
	/// to the widget tooltip.
	/// "%1" is replaced by the current value of the slider.
	/// Empty string (by default) means no tooltip.
	QString handleToolTip() const;
	void setHandleToolTip(const QString& toolTip);

	/// Returns true if the minimum value handle is down, false if it is up.
	/// \sa isMaximumSliderDown()
	bool isMinimumSliderDown() const;
	/// Returns true if the maximum value handle is down, false if it is up.
	/// \sa isMinimumSliderDown()
	bool isMaximumSliderDown() const;

Q_SIGNALS:
	///
	/// This signal is emitted when the slider minimum value has changed,
	/// with the new slider value as argument.
	void minimumValueChanged(int min);
	///
	/// This signal is emitted when the slider maximum value has changed,
	/// with the new slider value as argument.
	void maximumValueChanged(int max);
	///
	/// Utility signal that is fired when minimum or maximum values have changed.
	void valuesChanged(int min, int max);

	///
	/// This signal is emitted when sliderDown is true and the slider moves.
	/// This usually happens when the user is dragging the minimum slider.
	/// The value is the new slider minimum position.
	/// This signal is emitted even when tracking is turned off.
	void minimumPositionChanged(int min);

	///
	/// This signal is emitted when sliderDown is true and the slider moves.
	/// This usually happens when the user is dragging the maximum slider.
	/// The value is the new slider maximum position.
	/// This signal is emitted even when tracking is turned off.
	void maximumPositionChanged(int max);

	///
	/// Utility signal that is fired when minimum or maximum positions
	/// have changed.
	void positionsChanged(int min, int max);

public Q_SLOTS:
	///
	/// This property holds the slider's current minimum value.
	/// The slider silently forces min to be within the legal range:
	/// minimum() <= min <= maximumValue() <= maximum().
	/// Note: Changing the minimumValue also changes the minimumPosition.
	/// \sa stMaximumValue, setValues, setMinimum, setMaximum, setRange
	void setMinimumValue(int min);

	///
	/// This property holds the slider's current maximum value.
	/// The slider silently forces max to be within the legal range:
	/// minimum() <= minimumValue() <= max <= maximum().
	/// Note: Changing the maximumValue also changes the maximumPosition.
	/// \sa stMinimumValue, setValues, setMinimum, setMaximum, setRange
	void setMaximumValue(int max);

	///
	/// Utility function that set the minimum value and maximum value at once.
	/// The slider silently forces min and max to be within the legal range:
	/// minimum() <= min <= max <= maximum().
	/// Note: Changing the minimumValue and maximumValue also changes the
	/// minimumPosition and maximumPosition.
	/// \sa setMinimumValue, setMaximumValue, setMinimum, setMaximum, setRange
	void setValues(int min, int max);

protected Q_SLOTS:
	void onRangeChanged(int minimum, int maximum);

protected:
	RangeSlider(RangeSliderPrivate* impl, Qt::Orientation o, QWidget* par = 0);
	RangeSlider(RangeSliderPrivate* impl, QWidget* par = 0);

	// Description:
	// Standard Qt UI events
	virtual void mousePressEvent(QMouseEvent* ev);
	virtual void mouseMoveEvent(QMouseEvent* ev);
	virtual void mouseReleaseEvent(QMouseEvent* ev);

	// Description:
	// Rendering is done here.
	virtual void paintEvent(QPaintEvent* ev);
	virtual void initMinimumSliderStyleOption(QStyleOptionSlider* option) const;
	virtual void initMaximumSliderStyleOption(QStyleOptionSlider* option) const;

	// Description:
	// Reimplemented for the tooltips
	virtual bool event(QEvent* event);

protected:
	QScopedPointer<RangeSliderPrivate> d_ptr;

private:
	Q_DECLARE_PRIVATE(RangeSlider)
	Q_DISABLE_COPY(RangeSlider)
};

#endif	  // VISUALIZATION_QT_INCLUDE_RANGE_SLIDER_HPP_
