
#include "save_image_form.hpp"
#include "../scene.hpp"

void SaveImageForm::update(Scene * scene) {
    Form::update(scene);
}

void SaveImageForm::connect(Scene * scene) {
    QObject::connect(this->fileChoosers["Export image"], &FileChooser::fileSelected, [this, scene](){
            ResolutionMode resolution = ResolutionMode::SAMPLER_RESOLUTION;
            if(this->checkBoxes["Resolution"]->isChecked())
                resolution = ResolutionMode::FULL_RESOLUTION;
            bool useColorMap = this->checkBoxes["Colormap"]->isChecked();
            scene->writeDeformedImage(this->fileChoosers["Export image"]->filename.toStdString(), this->objectChoosers["Grid"]->currentText().toStdString(), useColorMap, resolution);
            this->hide();
            });
}
