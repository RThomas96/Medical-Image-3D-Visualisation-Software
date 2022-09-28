#ifndef SAVE_IMAGE_FORM
#define SAVE_IMAGE_FORM

#include "form.hpp"

class SaveImageForm : Form {
    Q_OBJECT

public:

    SaveImageForm(Scene * scene, QWidget *parent = nullptr):Form(parent){init(scene);connect(scene);}

public slots:

    void init(Scene * scene) {
        this->add(WidgetType::GRID_CHOOSE, "Grid", "Grid: ");
        this->setObjectTypeToChoose("Grid", ObjectToChoose::GRID);

        this->addWithLabel(WidgetType::CHECK_BOX, "Colormap", "Use colormap: ");
        this->addWithLabel(WidgetType::CHECK_BOX, "Resolution", "Export at full resolution: ");

        this->add(WidgetType::TIFF_SAVE, "Export image", "Export");
    }

    void show() {
        Form::show();
    }

    void update(Scene * scene);
    void connect(Scene * scene);
};


#endif
