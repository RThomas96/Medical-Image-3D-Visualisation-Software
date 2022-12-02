/**********************************************************************
 * FILE : main.cpp
 * AUTH : Thibault de Villèle
 * DATE : 06/02/2020
 * DESC : The main file of the project, launching the Qt application
 **********************************************************************/

#include <QApplication>
#include <iostream>

#include "src/qt/main_widget.hpp"

/*! \mainpage Developper guide
 *
 * \section sec_intro Introduction
 *
 * Welcome to the developper guide.
 * This guide is aim to provide quick overview of where the code for each feature is located.
 * Some of the classe's code is not documented, this guide allows to summarize which files are involved for each feature.
 *
 * The documentation is organised into categories:
 * - %Image reading and cache management: \ref img
 * - %Mesh structure, reading, writing, generation, grid: \ref geometry
 * - %Cage system: \ref deformation
 * - %Move tool, Direct tool, ARAP tool, selection etc: \ref tools
 * - 3D rendering: \ref gl
 * - %User interface: \ref ui
 *
 */

int main(int argc, char* argv[]) {
	QSurfaceFormat fmt;
	fmt.setOption(QSurfaceFormat::DebugContext);	// adds GL_KHR_debug extension to the OpenGL context creation
	//fmt.setSamples(4); // enables multi-sampling
	QSurfaceFormat::setDefaultFormat(fmt);

	QCoreApplication::setApplicationName("Medical Image Visualizer");
	QCoreApplication::setOrganizationName("LIRMM");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);
	// Share OpenGL context when possible :
	//QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

	QApplication app(argc, argv);

	MainWidget mainwidget;
	mainwidget.show();
    mainwidget.initialize();

	return app.exec();
}
