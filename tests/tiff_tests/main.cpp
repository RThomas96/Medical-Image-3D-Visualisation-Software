/**********************************************************************
 * FILE : tests/tiff_tests/main.cpp
 * AUTH : Thibault de Villèle
 * DATE : 10/02/2020
 * DESC : The main file of the TinyTIFF library test, launching the Qt
 *	  application.
 **********************************************************************/

#include <iostream>
#include <QApplication>
#include <QCommandLineParser>

#include "qt/include/main_window.hpp"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	QCoreApplication::setApplicationName("Medical Image Visualizer");
	QCoreApplication::setOrganizationName("LIRMM");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);

	main_window mainwindow;
	mainwindow.resize(mainwindow.sizeHint());
	mainwindow.show();

	return app.exec();
}
