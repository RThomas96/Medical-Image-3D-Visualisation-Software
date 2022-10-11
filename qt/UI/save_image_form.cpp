
#include "save_image_form.hpp"
#include "../scene.hpp"
#include "qobject.h"
#include<QDoubleSpinBox>
#include "../scene.hpp"

void SaveImageForm::update(Scene * scene) {
    Form::update(scene);
    this->updateSpinBoxes(scene);
}

void SaveImageForm::updateSpinBoxes(Scene * scene) {
    int gridIdx = scene->getGridIdx(this->objectChoosers["Grid"]->currentText().toStdString());
    if(gridIdx == -1)
        return;
    auto bbox = scene->grids[gridIdx]->getBoundingBox();
    this->blockSignalsInGroup("GroupBBMin", true);
    this->blockSignalsInGroup("GroupBBMax", true);
    this->doubleSpinBoxes["BBMinX"]->setValue(bbox.first.x);
    this->doubleSpinBoxes["BBMinY"]->setValue(bbox.first.y);
    this->doubleSpinBoxes["BBMinZ"]->setValue(bbox.first.z);
    this->doubleSpinBoxes["BBMaxX"]->setValue(bbox.second.x);
    this->doubleSpinBoxes["BBMaxY"]->setValue(bbox.second.y);
    this->doubleSpinBoxes["BBMaxZ"]->setValue(bbox.second.z);
    this->blockSignalsInGroup("GroupBBMin", false);
    this->blockSignalsInGroup("GroupBBMax", false);

    this->updateBoxToDisplay(scene);
}

void SaveImageForm::updateBoxToDisplay(Scene * scene) {
    //std::string gridName = this->objectChoosers["Grid"]->currentText().toStdString();
    ////scene->slotSetPlaneDisplacement(gridName, Scene::CuttingPlaneDirection::X, this->doubleSpinBoxes["BBMinX"]->value());
    ////scene->slotSetPlaneDisplacement(gridName, Scene::CuttingPlaneDirection::Y, this->doubleSpinBoxes["BBMinY"]->value());
    ////scene->slotSetPlaneDisplacement(gridName, Scene::CuttingPlaneDirection::Z, this->doubleSpinBoxes["BBMinZ"]->value());
    scene->clearBoxes();
    glm::vec3 bbMin(this->doubleSpinBoxes["BBMinX"]->value(), this->doubleSpinBoxes["BBMinY"]->value(), this->doubleSpinBoxes["BBMinZ"]->value());
    glm::vec3 bbMax(this->doubleSpinBoxes["BBMaxX"]->value(), this->doubleSpinBoxes["BBMaxY"]->value(), this->doubleSpinBoxes["BBMaxZ"]->value());
    if(!this->isHidden())
        scene->addBox(Scene::Box(bbMin, bbMax));
}

void SaveImageForm::connect(Scene * scene) {
    QObject::connect(this->fileChoosers["Export image"], &FileChooser::fileSelected, [this, scene](){
        ResolutionMode resolution = ResolutionMode::SAMPLER_RESOLUTION;
        if(this->checkBoxes["Resolution"]->isChecked())
            resolution = ResolutionMode::FULL_RESOLUTION;
        bool useColorMap = this->checkBoxes["Colormap"]->isChecked();
        glm::vec3 bbMin(this->doubleSpinBoxes["BBMinX"]->value(), this->doubleSpinBoxes["BBMinY"]->value(), this->doubleSpinBoxes["BBMinZ"]->value());
        glm::vec3 bbMax(this->doubleSpinBoxes["BBMaxX"]->value(), this->doubleSpinBoxes["BBMaxY"]->value(), this->doubleSpinBoxes["BBMaxZ"]->value());
        scene->writeDeformedImage(this->fileChoosers["Export image"]->filename.toStdString(), this->objectChoosers["Grid"]->currentText().toStdString(), bbMin, bbMax, useColorMap, resolution);
        this->hide();
    });
    QObject::connect(this->objectChoosers["Grid"], &ObjectChooser::currentTextChanged, [this, scene](){this->updateSpinBoxes(scene);});

    QObject::connect(this, &Form::widgetModified, [this, scene](const QString &id){
        if(id == "BBMinX" || id == "BBMinY" || id == "BBMinZ" || id == "BBMaxX" || id == "BBMaxY" || id == "BBMaxZ")
            this->updateBoxToDisplay(scene);
    });
}

void SaveImageForm::show() {
    Form::show();
    this->updateBoxToDisplay(scene);
}

void SaveImageForm::hide() {
    Form::hide();
    scene->clearBoxes();
}
