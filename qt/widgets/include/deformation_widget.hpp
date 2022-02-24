#ifndef QT_INCLUDE_DEFORMATION_WIDGET_HPP_
#define QT_INCLUDE_DEFORMATION_WIDGET_HPP_

//#include "../../grid/include/discrete_grid.hpp"
//#include "../../grid/include/discrete_grid_reader.hpp"
//#include "../../grid/include/input_discrete_grid.hpp"
#include "../../macros.hpp"
#include "../../qt/viewers/include/neighbor_visu_viewer.hpp"
#include "../../qt/viewers/include/scene.hpp"
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

class GridDeformationWidget : public QWidget {
	Q_OBJECT
public:
	GridDeformationWidget(Scene* scene, QWidget* parent = nullptr);
	~GridDeformationWidget(void);
	void setupLayouts();
	void setupSignals(Scene* scene);

    bool useSurface = false;;

protected:
    QVBoxLayout* mainLayout;

	QHBoxLayout*             layout_mesh;
	QGroupBox*                group_mesh;
	QRadioButton*      radio_mesh_grid_1; 
	QRadioButton*      radio_mesh_grid_2; 
	QRadioButton*     radio_mesh_surface; 

	QHBoxLayout*          layout_selector;
	QGroupBox*             group_selector;
	QRadioButton*   radio_selector_direct; 
	QRadioButton*     radio_selector_free; 
	QRadioButton* radio_selector_position; 

	QHBoxLayout*              layout_move;
	QGroupBox*                 group_move;
	QRadioButton*       radio_move_normal; 
	QRadioButton*     radio_move_weighted; 
	QRadioButton*     radio_move_ARAP; 

	QLabel*           label_radius_selection;
	QDoubleSpinBox* spinbox_radius_selection;
	QDoubleSpinBox* spinbox_l_selection;
	QDoubleSpinBox* spinbox_N_selection;
	QDoubleSpinBox* spinbox_S_selection;

	QLabel*           label_radius_sphere;
	QDoubleSpinBox* spinbox_radius_sphere;

	QLabel*               label_wireframe;
	QCheckBox*         checkbox_wireframe;

    QPushButton*             debug_button;
    QPushButton*                 debug_it;
    QPushButton*                 debug_init;
};

#endif	  // QT_INCLUDE_DEFORMATION_WIDGET_HPP_
