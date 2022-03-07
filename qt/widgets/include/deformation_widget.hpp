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
    void updateScene(Scene * scene, int meshTool, int moveMethod);

    bool useSurface = false;
    int meshManipulatorType = 0;
    int moveMethod = 0;
    int gridToDraw = 0;

protected:
    QVBoxLayout* mainLayout;

	QVBoxLayout*             layout_mesh;
	QGroupBox*                group_mesh;
	QRadioButton*      radio_mesh_grid_1; 
	QRadioButton*      radio_mesh_grid_2; 
	QRadioButton*     radio_mesh_surface; 

	QVBoxLayout*          layout_selector;
	QGroupBox*             group_selector;
	QRadioButton*   radio_selector_direct; 
	QRadioButton*     radio_selector_free; 
	QRadioButton* radio_selector_position; 
	QRadioButton* radio_selector_comp; 
	QRadioButton* radio_selector_ARAP; 

	QVBoxLayout*              layout_move;
	QGroupBox*                 group_move;
	QRadioButton*       radio_move_normal; 
	QRadioButton*     radio_move_weighted; 
	QRadioButton*     radio_move_ARAP; 

	QLabel*           label_radius_selection;
	QDoubleSpinBox* spinbox_radius_selection;
	QDoubleSpinBox* spinbox_l_selection;
	QDoubleSpinBox* spinbox_N_selection;
	QDoubleSpinBox* spinbox_S_selection;

	QVBoxLayout*              layout_visu;
	QGroupBox*                 group_visu;
	QLabel*           label_radius_sphere;
	QDoubleSpinBox* spinbox_radius_sphere;
	QCheckBox*         checkbox_wireframe;

    QPushButton*             handleMode;
    QPushButton*                 debug_it;
    QPushButton*                 debug_init;
};

#endif	  // QT_INCLUDE_DEFORMATION_WIDGET_HPP_
