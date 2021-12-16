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
public slots:
	void toggleDeformationButtons(bool is_enabled);
protected:
	Viewer* viewer;

	QPushButton* button_selection;
	QPushButton* button_update;
	QPushButton* button_alignARAP;
	QPushButton* button_launchARAP;
	QPushButton* button_vaoState;
	QPushButton* button_select_all;
	QPushButton* button_unselect_all;
	QPushButton* button_reset_arap;
	QPushButton* button_enable_def;
	QPushButton* button_save_mesh;
	QPushButton* button_save_curve;
};


#endif // VISUALISATION_QT_INCLUDE_VIEWER_HELPER_HPP_
