#ifndef COLOR_BUTTON
#define COLOR_BUTTON

#include<QColor>
#include<QPushButton>
#include<QVBoxLayout>

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
    QPushButton* button;

protected:
	QPixmap* pixmap;
	QIcon* icon;
	QColor color;
	QVBoxLayout* layout;
};

#endif
