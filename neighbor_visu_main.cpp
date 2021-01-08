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

#include <dlfcn.h>

int main(int argc, char* argv[]) {
/*	void* lib = NULL;
	if ((lib = dlopen("librenderdoc.so", RTLD_NOW))) {
		pRENDERDOC_GetAPI RENDERDOCGETAPI = (pRENDERDOC_GetAPI)dlsym(lib, "RENDERDOC_GetAPI");
		int ret = RENDERDOCGETAPI(eRENDERDOC_API_Version_1_4_1, (void **)&rdocAPI);
		assert(ret==1);
		std::cerr << "LOADED RENDERDOC" << '\n';
		if (rdocAPI == 0) { std::cerr << "ERROR LOADING RENDERDOC" << '\n'; }
	} else {
		std::cerr << "NOT LOADING RENDERDOC" << '\n';
		fprintf(stderr, "dlopen failed: %s\n", dlerror());
	}*/

	QCoreApplication::setApplicationName("Medical Image Visualizer");
	QCoreApplication::setOrganizationName("LIRMM");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);
	// Share OpenGL context when possible :
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

	// So the edges don't look jagged
/*
	QSurfaceFormat fmt;
	fmt.setSamples(5);
	QSurfaceFormat::setDefaultFormat(fmt);
*/

	QApplication app(argc, argv);

	MainWidget mainwidget;
	mainwidget.show();

	return app.exec();
}
