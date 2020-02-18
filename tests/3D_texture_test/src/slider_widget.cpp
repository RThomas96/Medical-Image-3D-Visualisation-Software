#include "../include/slider_widget.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

slider_widget::slider_widget(simple_3D_texture_viewer* viewer) {
	/**
	 * Create the sliders
	 */
	this->min_x_slider = new QSlider(Qt::Horizontal);
	this->min_y_slider = new QSlider(Qt::Horizontal);
	this->min_z_slider = new QSlider(Qt::Horizontal);
	this->max_x_slider = new QSlider(Qt::Horizontal);
	this->max_y_slider = new QSlider(Qt::Horizontal);
	this->max_z_slider = new QSlider(Qt::Horizontal);

	this->min_x_slider->setRange(-1, 1000);
	this->min_x_slider->setValue(0);
	this->min_y_slider->setRange(-1, 1000);
	this->min_y_slider->setValue(0);
	this->min_z_slider->setRange(-1, 1000);
	this->min_z_slider->setValue(0);

	this->max_x_slider->setRange(-1, 1000);
	this->max_x_slider->setValue(999);
	this->max_y_slider->setRange(-1, 1000);
	this->max_y_slider->setValue(999);
	this->max_z_slider->setRange(-1, 1000);
	this->max_z_slider->setValue(999);
	/**
	 * Create labels for them :
	 */
	QLabel* min_label = new QLabel("Minimum values for texture coordinates");
	QLabel* max_label = new QLabel("Maximum values for texture coordinates");
	QLabel* x_label = new QLabel("X :");
	QLabel* y_label = new QLabel("Y :");
	QLabel* z_label = new QLabel("Z :");
	QLabel* x_label_bis = new QLabel("X :");
	QLabel* y_label_bis = new QLabel("Y :");
	QLabel* z_label_bis = new QLabel("Z :");
	/**
	 * Assign them into a layout, meaning :
	 *     - the layout will contain 2 columns
	 *     - each column will have a title, as well as a pair of {label, slider} to represent the value slider
	 */
	min_label->setAutoFillBackground(true);
	QPalette pal (min_label->palette());
	pal.setColor(QPalette::Background, QColor("#ff00ff"));
	QPalette pal2(pal);
	min_label->setPalette(pal);
	min_label->setMargin(0);
	min_label->setContentsMargins(0,0,0,0);
	max_label->setMargin(0);
	max_label->setContentsMargins(0,0,0,0);
	// left sliders box (for min tex coordinates)
	QVBoxLayout* min_sliders = new QVBoxLayout();
	min_sliders->addWidget(min_label, 0, Qt::AlignCenter);
	{
		// X slider box :
		QHBoxLayout* min_x_box = new QHBoxLayout();
		min_x_box->addWidget(x_label, 0, Qt::AlignRight);
		min_x_box->addWidget(this->min_x_slider);
		min_sliders->addLayout(min_x_box);
	}
	{
		// Y slider box :
		QHBoxLayout* min_y_box = new QHBoxLayout();
		min_y_box->addWidget(y_label, 0, Qt::AlignRight);
		min_y_box->addWidget(this->min_y_slider);
		min_sliders->addLayout(min_y_box);
	}
	{
		// Z slider box :
		QHBoxLayout* min_z_box = new QHBoxLayout();
		min_z_box->addWidget(z_label, 0, Qt::AlignRight);
		min_z_box->addWidget(this->min_z_slider);
		min_sliders->addLayout(min_z_box);
	}
	// right sliders box (for max tex coordinates)
	QVBoxLayout* max_sliders = new QVBoxLayout();
	max_sliders->addWidget(max_label, 0, Qt::AlignCenter);
	{
		// X slider box :
		QHBoxLayout* max_x_box = new QHBoxLayout();
		max_x_box->addWidget(x_label_bis, 0, Qt::AlignRight);
		max_x_box->addWidget(this->max_x_slider);
		max_sliders->addLayout(max_x_box);
	}
	{
		// Y slider box :
		QHBoxLayout* max_y_box = new QHBoxLayout();
		max_y_box->addWidget(y_label_bis, 0, Qt::AlignRight);
		max_y_box->addWidget(this->max_y_slider);
		max_sliders->addLayout(max_y_box);
	}
	{
		// Z slider box :
		QHBoxLayout* max_z_box = new QHBoxLayout();
		max_z_box->addWidget(z_label_bis, 0, Qt::AlignRight);
		max_z_box->addWidget(this->max_z_slider);
		max_sliders->addLayout(max_z_box);
	}
	QHBoxLayout* main_layout = new QHBoxLayout();
	main_layout->addLayout(min_sliders);
	main_layout->addLayout(max_sliders);
	this->setLayout(main_layout);

	/**
	 * Connect signals to the passthrough they're assigned to
	 */
	connect(this->min_x_slider, &QSlider::valueChanged, this, &slider_widget::min_x_value_changed);
	connect(this->min_y_slider, &QSlider::valueChanged, this, &slider_widget::min_y_value_changed);
	connect(this->min_z_slider, &QSlider::valueChanged, this, &slider_widget::min_z_value_changed);
	connect(this->max_x_slider, &QSlider::valueChanged, this, &slider_widget::max_x_value_changed);
	connect(this->max_y_slider, &QSlider::valueChanged, this, &slider_widget::max_y_value_changed);
	connect(this->max_z_slider, &QSlider::valueChanged, this, &slider_widget::max_z_value_changed);

	/**
	 * Connect passthroughs to the viewer slots :
	 */
	connect(this, &slider_widget::set_Min_X_Texture, viewer, &simple_3D_texture_viewer::set_min_X_tex_value);
	connect(this, &slider_widget::set_Min_Y_Texture, viewer, &simple_3D_texture_viewer::set_min_Y_tex_value);
	connect(this, &slider_widget::set_Min_Z_Texture, viewer, &simple_3D_texture_viewer::set_min_Z_tex_value);
	connect(this, &slider_widget::set_Max_X_Texture, viewer, &simple_3D_texture_viewer::set_max_X_tex_value);
	connect(this, &slider_widget::set_Max_Y_Texture, viewer, &simple_3D_texture_viewer::set_max_Y_tex_value);
	connect(this, &slider_widget::set_Max_Z_Texture, viewer, &simple_3D_texture_viewer::set_max_Z_tex_value);
}

void slider_widget::set_min_value(QSlider *slider_to_change, int new_min_value) {
	this->blockSignals(true);
	slider_to_change->blockSignals(true);
	int old_max = slider_to_change->maximum();
	if (slider_to_change->value() < new_min_value) {
		slider_to_change->setValue(new_min_value);
	}
	slider_to_change->setRange(new_min_value, old_max);
	slider_to_change->blockSignals(false);
	this->blockSignals(false);
}

void slider_widget::set_max_value(QSlider *slider_to_change, int new_max_value) {
	this->blockSignals(true);
	slider_to_change->blockSignals(true);
	int old_min = slider_to_change->minimum();
	if (slider_to_change->value() > new_max_value) {
		slider_to_change->setValue(new_max_value);
	}
	slider_to_change->setRange(old_min, new_max_value);
	slider_to_change->blockSignals(false);
	this->blockSignals(false);
}

void slider_widget::min_x_value_changed(int x) {
	double d_x = static_cast<double>(x) / 1000;
	this->set_min_value(this->max_x_slider, x);
	emit this->set_Min_X_Texture(d_x);
}

void slider_widget::min_y_value_changed(int y) {
	double d_y = static_cast<double>(y) / 1000;
	this->set_min_value(this->max_y_slider, y);
	emit this->set_Min_Y_Texture(d_y);
}

void slider_widget::min_z_value_changed(int z) {
	double d_z = static_cast<double>(z) / 1000;
	this->set_min_value(this->max_z_slider, z);
	emit this->set_Min_Z_Texture(d_z);
}

void slider_widget::max_x_value_changed(int x) {
	double d_x = static_cast<double>(x) / 1000;
	this->set_max_value(this->min_x_slider, x);
	emit this->set_Max_X_Texture(d_x);
}

void slider_widget::max_y_value_changed(int y) {
	double d_y = static_cast<double>(y) / 1000;
	this->set_max_value(this->min_y_slider, y);
	emit this->set_Max_Y_Texture(d_y);
}

void slider_widget::max_z_value_changed(int z) {
	double d_z = static_cast<double>(z) / 1000;
	this->set_max_value(this->min_z_slider, z);
	emit this->set_Max_Z_Texture(d_z);
}
