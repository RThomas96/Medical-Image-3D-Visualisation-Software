#ifndef SAVE_IMAGE_FORM
#define SAVE_IMAGE_FORM

#include "form.hpp"
#include "glm/glm.hpp"

class SaveImageForm : Form {
    Q_OBJECT

public:

    SaveImageForm(Scene * scene, QWidget *parent = nullptr):Form(parent){init(scene);connect(scene);}

public slots:

    void init(Scene * scene) {
        this->add(WidgetType::H_GROUP, "GroupHeader");
        this->addAllNextWidgetsToGroup("GroupHeader");

        this->add(WidgetType::GRID_CHOOSE, "Grid", "Grid: ");
        this->setObjectTypeToChoose("Grid", ObjectToChoose::GRID);

        this->add(WidgetType::BUTTON, "Reset");

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::CHECK_BOX, "Colormap", "Use colormap: ");
        this->addWithLabel(WidgetType::CHECK_BOX, "Resolution", "Export at full resolution: ");

        this->addWithLabel(WidgetType::H_GROUP, "GroupBBMin", "BBox min");
        this->addAllNextWidgetsToGroup("GroupBBMin");

        this->add(WidgetType::SPIN_BOX_DOUBLE, "BBMinX");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "BBMinY");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "BBMinZ");

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::H_GROUP, "GroupBBMax", "BBox max");
        this->addAllNextWidgetsToGroup("GroupBBMax");

        this->add(WidgetType::SPIN_BOX_DOUBLE, "BBMaxX");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "BBMaxY");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "BBMaxZ");

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::H_GROUP, "GroupVoxelSize", "Voxel size");
        this->addAllNextWidgetsToGroup("GroupVoxelSize");

        this->add(WidgetType::SPIN_BOX_DOUBLE, "VoxelSizeX");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "VoxelSizeY");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "VoxelSizeZ");

        this->addAllNextWidgetsToDefaultGroup();

        this->add(WidgetType::TIFF_SAVE, "Export image", "Export");
        this->scene = scene;
    }

    void show();
    void hide();

    void closeEvent(QCloseEvent *bar);

    void update(Scene * scene);
    void connect(Scene * scene);

protected:
    Scene * scene;

    void initSpinBoxes(Scene * scene);
    void updateSpinBoxes(Scene * scene);

    void updateBoxToDisplay(Scene * scene);
};


#endif
