#include "../include/texture_control.hpp"

#include <QLabel>
#include <QVBoxLayout>

texture_controller::texture_controller(const texture_viewer* viewer) {
	this->load_tex_button = new QPushButton("Load", nullptr);
	this->free_tex_button = new QPushButton("Free", nullptr);
	this->undistort_button = new QPushButton("Undistort", nullptr);

	QLabel* title_label = new QLabel("Texture control");
	QVBoxLayout* main_layout = new QVBoxLayout(this);
	main_layout->addWidget(title_label, 0, Qt::AlignCenter);
	main_layout->addWidget(this->load_tex_button, 0, Qt::AlignCenter);
	main_layout->addWidget(this->free_tex_button, 0, Qt::AlignCenter);
	main_layout->addWidget(this->undistort_button, 0, Qt::AlignCenter);

	connect(this->load_tex_button, &QPushButton::clicked, viewer, &texture_viewer::request_texture_load);
	connect(this->free_tex_button, &QPushButton::clicked, viewer, &texture_viewer::request_texture_deletion);
	connect(this->undistort_button, &QPushButton::clicked, viewer, &texture_viewer::toggle_distortion);

	this->setMaximumWidth(250);
	this->setMaximumHeight(125);
}
