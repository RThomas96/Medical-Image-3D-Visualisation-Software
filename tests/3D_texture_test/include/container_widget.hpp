#ifndef TESTS_3D_TEXTURE_TEST_INCLUDE_CONTAINER_WIDGET_HPP_
#define TESTS_3D_TEXTURE_TEST_INCLUDE_CONTAINER_WIDGET_HPP_

#include <iostream>

#include <QWidget>
#include <QVBoxLayout>

#include "./3D_texture_viewer.hpp"
#include "./slider_widget.hpp"

class container_widget : public QWidget {
	public:
		container_widget() {
			this->viewer = new simple_3D_texture_viewer();
			this->sliders = new slider_widget(this->viewer);
			std::cerr << "Created widgets at adresses " << this->viewer << " and " << this->sliders << std::endl;
			std::cerr.flush();

			this->vbox = new QVBoxLayout();
			this->vbox->addWidget(this->viewer);
			this->vbox->addWidget(this->sliders);

			this->setLayout(this->vbox);
		}
	private:
		simple_3D_texture_viewer* viewer;
		slider_widget* sliders;
		QVBoxLayout* vbox;
};

#endif // TESTS_3D_TEXTURE_TEST_INCLUDE_CONTAINER_WIDGET_HPP_
