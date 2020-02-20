#include "../include/texture_sliders.hpp"

texture_sliders::texture_sliders(texture_viewer* viewer) {
	// Dirty hack to not let the widget take the whole
	// fucking screen. Fuck automatic size constraints.
	this->setMaximumHeight(125);

	/**
	 * Create the sliders (horizontally)
	 */
	this->min_x_slider = new QSlider(Qt::Horizontal);
	this->min_y_slider = new QSlider(Qt::Horizontal);
	this->min_z_slider = new QSlider(Qt::Horizontal);
	this->max_x_slider = new QSlider(Qt::Horizontal);
	this->max_y_slider = new QSlider(Qt::Horizontal);
	this->max_z_slider = new QSlider(Qt::Horizontal);

	/**
	 * Set their range, value, tick position and intervals
	 */
	this->min_x_slider->setRange(-1, 999);
	this->min_x_slider->setValue(0);
	this->min_x_slider->setTickPosition(QSlider::TicksBelow);
	this->min_x_slider->setTickInterval(100);
	this->min_y_slider->setRange(-1, 999);
	this->min_y_slider->setValue(0);
	this->min_y_slider->setTickPosition(QSlider::TicksBelow);
	this->min_y_slider->setTickInterval(100);
	this->min_z_slider->setRange(-1, 999);
	this->min_z_slider->setValue(0);
	this->min_z_slider->setTickPosition(QSlider::TicksBelow);
	this->min_z_slider->setTickInterval(100);

	this->max_x_slider->setRange(0, 1000);
	this->max_x_slider->setValue(999);
	this->max_x_slider->setTickPosition(QSlider::TicksBelow);
	this->max_x_slider->setTickInterval(100);
	this->max_y_slider->setRange(0, 1000);
	this->max_y_slider->setValue(999);
	this->max_y_slider->setTickPosition(QSlider::TicksBelow);
	this->max_y_slider->setTickInterval(100);
	this->max_z_slider->setRange(0, 1000);
	this->max_z_slider->setValue(999);
	this->max_z_slider->setTickPosition(QSlider::TicksBelow);
	this->max_z_slider->setTickInterval(100);

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
	 *     - each column will have a title label, as well as a pair of {label, slider} to represent the value slider
	 */
	// left sliders box (for min tex coordinates)
	QVBoxLayout* min_sliders = new QVBoxLayout();
	min_sliders->addWidget(min_label, 0, Qt::AlignCenter | Qt::AlignTop);
	{
		// X slider box :
		QHBoxLayout* min_x_box = new QHBoxLayout();
		min_x_box->addWidget(x_label, 0, Qt::AlignRight);
		min_x_box->addWidget(this->min_x_slider);
		min_sliders->addLayout(min_x_box);
		// Y slider box :
		QHBoxLayout* min_y_box = new QHBoxLayout();
		min_y_box->addWidget(y_label, 0, Qt::AlignRight);
		min_y_box->addWidget(this->min_y_slider);
		min_sliders->addLayout(min_y_box);
		// Z slider box :
		QHBoxLayout* min_z_box = new QHBoxLayout();
		min_z_box->addWidget(z_label, 0, Qt::AlignRight);
		min_z_box->addWidget(this->min_z_slider);
		min_sliders->addLayout(min_z_box);
	}
	// right sliders box (for max tex coordinates)
	QVBoxLayout* max_sliders = new QVBoxLayout();
	max_sliders->addWidget(max_label, 0, Qt::AlignCenter | Qt::AlignTop);
	{
		// X slider box :
		QHBoxLayout* max_x_box = new QHBoxLayout();
		max_x_box->addWidget(x_label_bis, 0, Qt::AlignRight);
		max_x_box->addWidget(this->max_x_slider);
		max_sliders->addLayout(max_x_box);
		// Y slider box :
		QHBoxLayout* max_y_box = new QHBoxLayout();
		max_y_box->addWidget(y_label_bis, 0, Qt::AlignRight);
		max_y_box->addWidget(this->max_y_slider);
		max_sliders->addLayout(max_y_box);
		// Z slider box :
		QHBoxLayout* max_z_box = new QHBoxLayout();
		max_z_box->addWidget(z_label_bis, 0, Qt::AlignRight);
		max_z_box->addWidget(this->max_z_slider);
		max_sliders->addLayout(max_z_box);
	}
	/**
	 * Add them all into a main layout, to group them together
	 */
	QHBoxLayout* main_layout = new QHBoxLayout();
	main_layout->addLayout(min_sliders);
	main_layout->addLayout(max_sliders);
	this->setLayout(main_layout);

	/**
	 * Connect signals to the passthrough they're assigned to
	 */
	QObject::connect(this->min_x_slider, &QSlider::valueChanged, this, &texture_sliders::min_x_value_changed);
	QObject::connect(this->min_y_slider, &QSlider::valueChanged, this, &texture_sliders::min_y_value_changed);
	QObject::connect(this->min_z_slider, &QSlider::valueChanged, this, &texture_sliders::min_z_value_changed);
	QObject::connect(this->max_x_slider, &QSlider::valueChanged, this, &texture_sliders::max_x_value_changed);
	QObject::connect(this->max_y_slider, &QSlider::valueChanged, this, &texture_sliders::max_y_value_changed);
	QObject::connect(this->max_z_slider, &QSlider::valueChanged, this, &texture_sliders::max_z_value_changed);

	/**
	 * Connect passthroughs to the viewer slots :
	 */
	QObject::connect(this, &texture_sliders::set_Min_X_Texture, viewer, &texture_viewer::set_min_X_tex_value);
	QObject::connect(this, &texture_sliders::set_Min_Y_Texture, viewer, &texture_viewer::set_min_Y_tex_value);
	QObject::connect(this, &texture_sliders::set_Min_Z_Texture, viewer, &texture_viewer::set_min_Z_tex_value);
	QObject::connect(this, &texture_sliders::set_Max_X_Texture, viewer, &texture_viewer::set_max_X_tex_value);
	QObject::connect(this, &texture_sliders::set_Max_Y_Texture, viewer, &texture_viewer::set_max_Y_tex_value);
	QObject::connect(this, &texture_sliders::set_Max_Z_Texture, viewer, &texture_viewer::set_max_Z_tex_value);
}

void texture_sliders::set_min_value(QSlider *slider_to_change, int new_min_value) {
	/**
	 * In our case, we don't want to block signals : otherwise
	 * the max value will be set, but not updated in the viewer.
	 *
	 * We need to set the value of the slider, and let it emit
	 * the signal to the viewer (risking a recursive call of
	 * XX_XX_value_changed(), better than nothing)
	 */
	if (slider_to_change->value() < new_min_value) {
		slider_to_change->setValue(new_min_value);
	}
}

void texture_sliders::set_max_value(QSlider *slider_to_change, int new_max_value) {
	/**
	 * In our case, we don't want to block signals : otherwise
	 * the max value will be set, but not updated in the viewer.
	 *
	 * We need to set the value of the slider, and let it emit
	 * the signal to the viewer (risking a recursive call of
	 * XX_XX_value_changed(), better than nothing)
	 */
	if (slider_to_change->value() > new_max_value) {
		slider_to_change->setValue(new_max_value);
	}
}

void texture_sliders::min_x_value_changed(int x) {
	this->set_min_value(this->max_x_slider, x);
	emit this->set_Min_X_Texture(static_cast<double>(x) / 1000);
}

void texture_sliders::min_y_value_changed(int y) {
	this->set_min_value(this->max_y_slider, y);
	emit this->set_Min_Y_Texture(static_cast<double>(y) / 1000);
}

void texture_sliders::min_z_value_changed(int z) {
	this->set_min_value(this->max_z_slider, z);
	emit this->set_Min_Z_Texture(static_cast<double>(z) / 1000);
}

void texture_sliders::max_x_value_changed(int x) {
	this->set_max_value(this->min_x_slider, x);
	emit this->set_Max_X_Texture(static_cast<double>(x) / 1000);
}

void texture_sliders::max_y_value_changed(int y) {
	this->set_max_value(this->min_y_slider, y);
	emit this->set_Max_Y_Texture(static_cast<double>(y) / 1000);
}

void texture_sliders::max_z_value_changed(int z) {
	this->set_max_value(this->min_z_slider, z);
	emit this->set_Max_Z_Texture(static_cast<double>(z) / 1000);
}
