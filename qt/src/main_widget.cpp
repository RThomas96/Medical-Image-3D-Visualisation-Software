#include "../include/main_widget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>

main_widget::main_widget(QWidget* parent) : QWidget(parent) {
	this->tex_viewer = new texture_viewer();
	this->tex_sliders = new texture_sliders(this->tex_viewer);
	this->tex_controller = new texture_controller(this->tex_viewer);

	QHBoxLayout* bottom_half = new QHBoxLayout();
	bottom_half->addWidget(this->tex_sliders);
	bottom_half->addWidget(this->tex_controller);

	QVBoxLayout* main_layout = new QVBoxLayout(this);
	main_layout->addWidget(this->tex_viewer);
	main_layout->addLayout(bottom_half);
}
