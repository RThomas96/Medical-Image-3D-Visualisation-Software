/**********************************************************************
 * FILE : qt/src/main_window.cpp
 * AUTH : Thibault de Villèle
 * DATE : 06/02/2020
 * DESC : The implementation file for the main_window class
 **********************************************************************/

#include <QMenuBar>

#include "../include/main_window.hpp"
#include "../../viewer/include/simple_viewer.hpp"

main_window::main_window() {
	// OPTIONNAL : Add a menu bar with commonly accessed functions
	this->setParent(nullptr);
	this->setCentralWidget(new simple_viewer());
}

void main_window::on_add_new() {
	std::clog << "Added new widget" << std::endl;
}
