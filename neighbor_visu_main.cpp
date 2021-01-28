/**********************************************************************
 * FILE : main.cpp
 * AUTH : Thibault de Villèle
 * DATE : 06/02/2020
 * DESC : The main file of the project, launching the Qt application
 **********************************************************************/

#include <iostream>
#include <QApplication>

#include "./qt/include/neighbor_visu_main_widget.hpp"
#include "./features.hpp"

#if (defined (_WIN32) || defined (_WIN64))
extern "C"
{
  __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main(int argc, char* argv[]) {

	QCoreApplication::setApplicationName("Medical Image Visualizer");
	QCoreApplication::setOrganizationName("LIRMM");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);
	// Share OpenGL context when possible :
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

	QSurfaceFormat fmt;
	fmt.setOption(QSurfaceFormat::DebugContext);
	QSurfaceFormat::setDefaultFormat(fmt);

	QApplication app(argc, argv);

	MainWidget mainwidget;
	mainwidget.show();

	return app.exec();
}
