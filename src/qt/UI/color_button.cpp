#include "color_button.hpp"
#include <QColorDialog>

ColorButton::ColorButton(QColor _color, QWidget* parent) :
	QWidget(parent) {
	this->color	 = _color;
	this->button = new QPushButton;
	this->pixmap = new QPixmap(30, 30);
	this->icon	 = nullptr;
	this->setColor(this->color);
	this->button->setFixedSize(this->button->sizeHint());

	this->layout = new QVBoxLayout();
	this->layout->addWidget(this->button);
	this->setLayout(this->layout);

	QObject::connect(this->button, &QPushButton::clicked, this, [this](void) -> void {
		QColor c = QColorDialog::getColor(this->color, this, "Pick a color", QColorDialog::ColorDialogOption::DontUseNativeDialog);
		if (c.isValid() == false) {
			return;
		}
		this->setColor(c);
		return;
	});
}

ColorButton::~ColorButton() {
	delete this->icon;
	delete this->pixmap;
	delete this->button;
}

void ColorButton::setColor(QColor _color) {
	this->color = _color;
	this->pixmap->fill(this->color);
	if (this->icon != nullptr) {
		delete this->icon;
	}
	this->icon = new QIcon(*this->pixmap);
	this->button->setIcon(*this->icon);
	/*
	QPalette button_palette;
	button_palette.setColor(QPalette::Button, this->color);
	this->button->setPalette(button_palette);
	*/
	emit this->colorChanged(this->color);
}

QColor ColorButton::getColor() const {
	return this->color;
}

