#ifndef QT_INCLUDE_LOADER_WIDGET_HPP_
#define QT_INCLUDE_LOADER_WIDGET_HPP_

//#include "../../grid/include/discrete_grid.hpp"
//#include "../../grid/include/discrete_grid_reader.hpp"
//#include "../../grid/include/input_discrete_grid.hpp"
#include "../../macros.hpp"
#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "../../viewer/include/scene.hpp"
#include "./scene_control.hpp"

#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWidget>

/// @ingroup qtwidgets
/// @brief The GridLoaderWidget is the class representing the widget to load the grids.
/// @details This is the widget that launches when selecting 'File' > 'Open Images' or pressing Ctrl+'O'. It is
/// responsible to let the user select their own files to load into the program.
/// @note Has implementations for loading both old-style DiscreteGrid grids, and the new Grid implementation.
class GridLoaderWidget : public QWidget {
	Q_OBJECT
public:
	GridLoaderWidget(Scene* _scene, Viewer* _viewer, ControlPanel* _cp, QWidget* parent = nullptr);
	~GridLoaderWidget(void);
	void setupLayouts();
	void setupSignals();

public slots:
	void loadNewGridAPI();
	void loadGrid();
	void loadGrid_newAPI();

protected:
	QDir basePath;
	Scene* scene;
	Viewer* viewer;
	ControlPanel* _cp;

	const SimpleGrid * _testing_grid;

	QLabel* label_header;

	QPushButton* button_loadGrids;

    QVBoxLayout* mainLayout;

	QHBoxLayout*     layout_subsample;
	QGroupBox*        group_subsample;
	QLabel*           label_subsample;
	QSpinBox* spinbox_subsample;

	QGridLayout*     layout_bbox;
	QGroupBox*        group_bbox;
	QLabel*           label_bbox;
	QLabel*           label_bboxMin;
	QDoubleSpinBox* spinbox_bboxMin_x;
	QDoubleSpinBox* spinbox_bboxMin_y;
	QDoubleSpinBox* spinbox_bboxMin_z;
	QLabel*           label_bboxMax;
	QDoubleSpinBox* spinbox_bboxMax_x;
	QDoubleSpinBox* spinbox_bboxMax_y;
	QDoubleSpinBox* spinbox_bboxMax_z;

	QProgressBar* progress_load;

	QPushButton* button_loadNewGridAPI;
};

#endif	  // QT_INCLUDE_LOADER_WIDGET_HPP_
