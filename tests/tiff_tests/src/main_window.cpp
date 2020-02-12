#include <QMenuBar>
#include <iostream>

#include "../include/main_window.hpp"
#include "../include/tiff_inspector.hpp"

main_window::main_window() {
	// OPTIONNAL : Add a menu bar with commonly accessed functions
	this->setParent(nullptr);
	this->setCentralWidget(new tiff_inspector(this));
}

QSize main_window::sizeHint() const {
	QSize s(800, 400);
	return s;
}

void main_window::on_add_new() {
	std::clog << "Added new widget" << std::endl;
}
