/**********************************************************************
 * FILE : qt/src/main_window.cpp
 * AUTH : Thibault de Villèle
 * DATE : 06/02/2020
 * DESC : The implementation file for the main_window class
 **********************************************************************/

#include "../include/main_window.hpp"

#include <QMenuBar>
#include <QGLViewer/qglviewer.h>

main_window::main_window() {
	// OPTIONNAL : Add a menu bar with commonly accessed functions
	this->setParent(nullptr);
}
