#include "./main_widget.hpp"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
	QSurfaceFormat fmt;
	fmt.setOption(QSurfaceFormat::DebugContext); // adds GL_KHR_debug extension to the OpenGL context creation
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setVersion(4, 5);
	QSurfaceFormat::setDefaultFormat(fmt);

	QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

	QApplication application(argc, argv);

	MainWidget widg;
	widg.show();

	return application.exec();
}
