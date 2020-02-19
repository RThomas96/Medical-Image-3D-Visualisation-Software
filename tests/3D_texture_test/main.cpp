/**********************************************************************
 * FILE : tests/3D_texture_test/main.cpp
 * AUTH : Thibault de Villèle
 * DATE : 12/02/2020
 * DESC : The main file of the testing for OpenGL's 3D textures,
 *	  launching the Qt application.
 **********************************************************************/

#include <iostream>
#include <QApplication>
#include <QCommandLineParser>

#include "include/container_widget.hpp"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	QCoreApplication::setApplicationName("Simple 3D Texture Viewer [TESTING]");
	QCoreApplication::setOrganizationName("LIRMM");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);

	container_widget container;
	container.resize(container.sizeHint());
	container.show();

	return app.exec();
}
