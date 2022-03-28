#ifndef QT_INCLUDE_DEFORMATION_WIDGET_HPP_
#define QT_INCLUDE_DEFORMATION_WIDGET_HPP_

//#include "../../grid/include/discrete_grid.hpp"
//#include "../../grid/include/discrete_grid_reader.hpp"
//#include "../../grid/include/input_discrete_grid.hpp"
#include "../../macros.hpp"
#include "../../qt/viewers/include/neighbor_visu_viewer.hpp"
#include "../../qt/viewers/include/scene.hpp"
#include "./include/scene_control.hpp"

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
    void updateScene(Scene * scene, UITool::MeshManipulatorType meshTool, int moveMethod, bool activeMeshChanged = false);

    UITool::MeshManipulatorType currentMeshTool = UITool::MeshManipulatorType::DIRECT;
    int currentMoveMethod = 0;
    int gridToDraw = -1;
    bool registrationInitialize = false;

    std::vector<std::string> meshNames;
    std::vector<std::pair<bool, bool>> gridOrCage;

public slots:
    void addNewMesh(const std::string& name, bool grid, bool cage) {
        this->meshNames.push_back(name);
        this->gridOrCage.push_back(std::pair<bool, bool>(grid, cage));
        this->combo_mesh->insertItem(this->combo_mesh->count(), QString(this->meshNames.back().c_str()));
        if(!this->gridOrCage.back().first)
            this->combo_mesh_register->insertItem(this->combo_mesh_register->count(), QString(this->meshNames.back().c_str()));
    }

protected:
    QVBoxLayout* mainLayout;

	QVBoxLayout*             layout_mesh;
	QGroupBox*                group_mesh;
	QRadioButton*      radio_mesh_grid_1; 
	QRadioButton*      radio_mesh_grid_2; 

    QComboBox* combo_mesh;

	QVBoxLayout*          layout_selector;
	QGroupBox*             group_selector;
	QRadioButton*   radio_selector_direct; 
	QRadioButton*     radio_selector_free; 
	QRadioButton* radio_selector_position; 
    QPushButton*             bindMove;
	QRadioButton*     radio_selector_comp; 
    QComboBox*        combo_mesh_register;
    QPushButton*  selection_mode_register;
    QPushButton*                 validate;
    QPushButton*                 undo;
    QPushButton*                 clear;
    QPushButton*                    apply;
	QRadioButton* radio_selector_ARAP; 
	QRadioButton* radio_selector_slice; 
	QHBoxLayout*          layout_slice;
    QPushButton*                    slice_X;
    QPushButton*                    slice_Y;
    QPushButton*                    slice_Z;
    QPushButton*                    slice_handle;
    QPushButton*                    slice_clear;

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
