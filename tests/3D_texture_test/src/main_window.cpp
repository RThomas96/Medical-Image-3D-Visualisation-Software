#include <QMenuBar>
#include <QVBoxLayout>
#include <iostream>

#include "../include/main_window.hpp"

main_window::main_window() {
	// OPTIONNAL : Add a menu bar with commonly accessed functions
	this->setParent(nullptr);
	this->widget = new container_widget();
	this->setCentralWidget(this->widget);
}

QSize main_window::sizeHint() const {
	QSize s(800, 500);
	return s;
}

void main_window::on_add_new() {
	std::clog << "Added new widget" << std::endl;
}
