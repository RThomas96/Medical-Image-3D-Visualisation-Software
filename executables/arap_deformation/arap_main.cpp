/**********************************************************************
* FILE : main.cpp
* AUTH : Thibault de Villèle
* DATE : 20/12/2021
* DESC : The main file of the project, launching the Qt application
**********************************************************************/

#include <QApplication>
#include <iostream>

#include "./features.hpp"
#include "./qt/include/arap_main_widget.hpp"

#if (defined(_WIN32) || defined(_WIN64))
extern "C"
{
__declspec(dllexport) unsigned long NvOptimusEnablement		   = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

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

	ARAPMainWidget mainwidget;
	mainwidget.show();

   return app.exec();
}
