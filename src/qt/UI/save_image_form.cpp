
#include "save_image_form.hpp"
#include "../scene.hpp"
#include "qobject.h"
#include<QDoubleSpinBox>
#include "../scene.hpp"

void SaveImageForm::update(Scene * scene) {
    Form::update(scene);
    this->initSpinBoxes(scene);
}

void SaveImageForm::initSpinBoxes(Scene * scene) {
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
    this->updateSpinBoxes(scene);
}

glm::vec3 SaveImageForm::getDimension() {
    glm::vec3 bbMin(this->doubleSpinBoxes["BBMinX"]->value(), this->doubleSpinBoxes["BBMinY"]->value(), this->doubleSpinBoxes["BBMinZ"]->value());
    glm::vec3 bbMax(this->doubleSpinBoxes["BBMaxX"]->value(), this->doubleSpinBoxes["BBMaxY"]->value(), this->doubleSpinBoxes["BBMaxZ"]->value());
    return bbMax - bbMin;
}

void SaveImageForm::updateImageSize() {
    glm::vec3 dimension = this->getDimension();
    glm::vec3 voxelSize(this->doubleSpinBoxes["VoxelSizeX"]->value(), this->doubleSpinBoxes["VoxelSizeY"]->value(), this->doubleSpinBoxes["VoxelSizeZ"]->value());
    this->blockSignalsInGroup("GroupImageSize", true);
    this->doubleSpinBoxes["ImageSizeX"]->setValue(std::ceil(dimension.x/voxelSize.x));
    this->doubleSpinBoxes["ImageSizeY"]->setValue(std::ceil(dimension.y/voxelSize.y));
    this->doubleSpinBoxes["ImageSizeZ"]->setValue(std::ceil(dimension.z/voxelSize.z));
    this->blockSignalsInGroup("GroupImageSize", false);
}

void SaveImageForm::updateVoxelSize() {
    glm::vec3 dimension = this->getDimension();
    glm::vec3 imageSize(this->doubleSpinBoxes["ImageSizeX"]->value(), this->doubleSpinBoxes["ImageSizeY"]->value(), this->doubleSpinBoxes["ImageSizeZ"]->value());
    this->blockSignalsInGroup("GroupVoxelSize", true);
    this->doubleSpinBoxes["VoxelSizeX"]->setValue(dimension.x/imageSize.x);
    this->doubleSpinBoxes["VoxelSizeY"]->setValue(dimension.y/imageSize.y);
    this->doubleSpinBoxes["VoxelSizeZ"]->setValue(dimension.z/imageSize.z);
    this->blockSignalsInGroup("GroupVoxelSize", false);
}

void SaveImageForm::updateSpinBoxes(Scene * scene) {
    if(this->checkBoxes["Resolution"]->isChecked()) {
        glm::vec3 voxelSize = scene->grids[scene->getGridIdx(this->objectChoosers["Grid"]->currentText().toStdString())]->getVoxelSize();
        this->doubleSpinBoxes["VoxelSizeX"]->setValue(voxelSize.x);
        this->doubleSpinBoxes["VoxelSizeY"]->setValue(voxelSize.y);
        this->doubleSpinBoxes["VoxelSizeZ"]->setValue(voxelSize.z);
        this->doubleSpinBoxes["VoxelSizeX"]->setEnabled(false);
        this->doubleSpinBoxes["VoxelSizeY"]->setEnabled(false);
        this->doubleSpinBoxes["VoxelSizeZ"]->setEnabled(false);

        this->updateImageSize();
        this->doubleSpinBoxes["ImageSizeX"]->setEnabled(false);
        this->doubleSpinBoxes["ImageSizeY"]->setEnabled(false);
        this->doubleSpinBoxes["ImageSizeZ"]->setEnabled(false);
    } else {
        this->doubleSpinBoxes["VoxelSizeX"]->setValue(1);
        this->doubleSpinBoxes["VoxelSizeY"]->setValue(1);
        this->doubleSpinBoxes["VoxelSizeZ"]->setValue(1);
        this->doubleSpinBoxes["VoxelSizeX"]->setEnabled(true);
        this->doubleSpinBoxes["VoxelSizeY"]->setEnabled(true);
        this->doubleSpinBoxes["VoxelSizeZ"]->setEnabled(true);

        this->updateImageSize();
        this->doubleSpinBoxes["ImageSizeX"]->setEnabled(true);
        this->doubleSpinBoxes["ImageSizeY"]->setEnabled(true);
        this->doubleSpinBoxes["ImageSizeZ"]->setEnabled(true);
    }
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
        Grid * grid = scene->grids[scene->getGridIdx(this->objectChoosers["Grid"]->currentText().toStdString())];
        glm::vec3 voxelSize(this->doubleSpinBoxes["VoxelSizeX"]->value(), this->doubleSpinBoxes["VoxelSizeY"]->value(), this->doubleSpinBoxes["VoxelSizeZ"]->value());
        bool useColorMap = this->checkBoxes["Colormap"]->isChecked();
        glm::vec3 bbMin(this->doubleSpinBoxes["BBMinX"]->value(), this->doubleSpinBoxes["BBMinY"]->value(), this->doubleSpinBoxes["BBMinZ"]->value());
        glm::vec3 bbMax(this->doubleSpinBoxes["BBMaxX"]->value(), this->doubleSpinBoxes["BBMaxY"]->value(), this->doubleSpinBoxes["BBMaxZ"]->value());

        scene->writeDeformedImage(this->fileChoosers["Export image"]->filename.toStdString(), this->objectChoosers["Grid"]->currentText().toStdString(), bbMin, bbMax, useColorMap, voxelSize);
        this->hide();
    });

    QObject::connect(this->objectChoosers["Grid"], &ObjectChooser::currentTextChanged, [this, scene](){this->initSpinBoxes(scene);});

    QObject::connect(this, &Form::widgetModified, [this, scene](const QString &id){
        std::cout << id.toStdString() << std::endl;
        if(id == "BBMinX" || id == "BBMinY" || id == "BBMinZ" || id == "BBMaxX" || id == "BBMaxY" || id == "BBMaxZ") {
            this->updateBoxToDisplay(scene);
            this->updateImageSize();
            //this->updateVoxelSize();
        }

        if(id == "Resolution")
            this->updateSpinBoxes(scene);

        if(id == "Reset")
            this->initSpinBoxes(scene);

        if(id == "VoxelSizeX" || id == "VoxelSizeY" || id == "VoxelSizeZ")
            this->updateImageSize();

        if(id == "ImageSizeX" || id == "ImageSizeY" || id == "ImageSizeZ")
            this->updateVoxelSize();
    });
}

void SaveImageForm::show() {
    Form::show();
    this->updateBoxToDisplay(scene);
    //this->scene->setOrthographicCamera();
}

void SaveImageForm::hide() {
    scene->clearBoxes();
    //this->scene->setPerspectiveCamera();
    Form::hide();
}

void SaveImageForm::closeEvent(QCloseEvent *bar) {
    this->hide();
}
