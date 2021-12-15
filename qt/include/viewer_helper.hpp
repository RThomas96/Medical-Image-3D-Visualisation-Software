#ifndef VISUALISATION_QT_INCLUDE_VIEWER_HELPER_HPP_
#define VISUALISATION_QT_INCLUDE_VIEWER_HELPER_HPP_

#include "viewer/include/neighbor_visu_viewer.hpp"
#include <QDockWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class ViewerHelper : public QWidget {
	Q_OBJECT
public:
	ViewerHelper(Viewer* _v, QWidget* parent = nullptr);
	virtual ~ViewerHelper() = default;
	void init();
	void initSignals();
protected:
	Viewer* viewer;
};


#endif // VISUALISATION_QT_INCLUDE_VIEWER_HELPER_HPP_
