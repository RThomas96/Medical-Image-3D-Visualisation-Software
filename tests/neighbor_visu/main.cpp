/**********************************************************************
 * FILE : main.cpp
 * AUTH : Thibault de Villèle
 * DATE : 06/02/2020
 * DESC : The main file of the project, launching the Qt application
 **********************************************************************/

#include <iostream>
#include <QApplication>

#include "include/main_widget.hpp"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	QCoreApplication::setApplicationName("Medical Image Visualizer");
	QCoreApplication::setOrganizationName("LIRMM");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);

	MainWidget mainwidget;
	mainwidget.show();

	return app.exec();
}
