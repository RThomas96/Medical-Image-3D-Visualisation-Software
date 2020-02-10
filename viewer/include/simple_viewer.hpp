#ifndef SIMPLE_VIEWER_HPP
#define SIMPLE_VIEWER_HPP

#include <QGLViewer/qglviewer.h>

class simple_viewer : public QGLViewer {
	public:
		virtual void draw();
		virtual void init();
		virtual QString helpString() const;
};

#endif // SIMPLE_VIEWER_HPP
