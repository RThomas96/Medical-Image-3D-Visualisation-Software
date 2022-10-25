
#include <QMessageBox>
#include "open_image_form.hpp"
#include "../scene.hpp"

void OpenImageForm::update(Scene * scene) {
    Form::update(scene);
}

void OpenImageForm::connect(Scene * scene) {
    QObject::connect(this->fileChoosers["Image choose"], &FileChooser::fileSelected, [this](){
            this->buttons["Load"]->setEnabled(true);
            //this->sections["Image subregion"].first->setEnabled(true);// This functionnality isn't available yet
            this->sections["Mesh"].first->show();
            this->sections["Cage"].first->show();

            this->sections["Mesh"].first->setChecked(false);
            this->sections["Cage"].first->setChecked(false);

            this->prefillFields({this->fileChoosers["Image choose"]->filename.toStdString()});
            });

    QObject::connect(this->fileChoosers["Mesh choose"], &FileChooser::fileSelected, [this](){
            this->useTetMesh = true;
            });

    QObject::connect(this->buttons["Load"], &QPushButton::clicked, [this, scene](){
        if(this->useTetMesh) {
            scene->openGrid(this->getGridName(), this->getImgFilenames(), this->getSubsample(), this->getVoxelSize(), this->getTetmeshFilename());
        } else {
            scene->openGrid(this->getGridName(), this->getImgFilenames(), this->getSubsample(), this->getVoxelSize(), this->getSizeTetmesh());
        }
        bool useCage = !this->fileChoosers["Cage choose"]->filename.isEmpty();
        if(useCage) {
            scene->openCage(this->getGridName() + "_cage", this->fileChoosers["Cage choose"]->filename.toStdString(), this->getGridName(), this->checkBoxes["mvc"]->isChecked());
        }
        this->hide();
        Q_EMIT loaded();
    });

    QObject::connect(this->buttons["Mouse brain atlas"], &QPushButton::clicked, [this, scene](){
        this->checkBoxes["Segmented"]->setChecked(true);
        this->fileChoosers["Image choose"]->setManually(std::string("../data/atlas.tiff"));
        this->sections["Mesh"].first->setChecked(true);
        this->fileChoosers["Mesh choose"]->setManually(std::string("../data/atlas-transfert.mesh"));
        this->sections["Cage"].first->setChecked(true);
        this->fileChoosers["Cage choose"]->setManually(std::string("../data/atlas-cage.off"));
        this->lineEdits["Name"]->setText("Mouse brain atlas");
        if(scene->isGrid("Mouse brain atlas"))
            this->lineEdits["Name"]->setText("Mouse brain atlas2");
    });

    QObject::connect(this->buttons["IRM"], &QPushButton::clicked, [this, scene](){
        this->checkBoxes["Segmented"]->setChecked(false);
        this->fileChoosers["Image choose"]->setManually(std::string("../data/irm.tif"));
        this->sections["Mesh"].first->setChecked(false);
        this->sections["Cage"].first->setChecked(false);
        this->doubleSpinBoxes["SizeVoxelX"]->setValue(3.9);
        this->doubleSpinBoxes["SizeVoxelY"]->setValue(3.9);
        this->doubleSpinBoxes["SizeVoxelZ"]->setValue(50);
        this->lineEdits["Name"]->setText("irm");
        if(scene->isGrid("irm"))
            this->lineEdits["Name"]->setText("irm2");
    });
}
