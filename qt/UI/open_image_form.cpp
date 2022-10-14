
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
            scene->openGrid(this->getName(), this->getImgFilenames(), this->getSubsample(), this->getTetmeshFilename());
        } else {
            scene->openGrid(this->getName(), this->getImgFilenames(), this->getSubsample(), this->getSizeVoxel(), this->getSizeTetmesh());
        }
        bool useCage = !this->fileChoosers["Cage choose"]->filename.isEmpty();
        if(useCage) {
            scene->openCage(this->getName() + "_cage", this->fileChoosers["Cage choose"]->filename.toStdString(), this->getName(), this->checkBoxes["mvc"]->isChecked());
        }
        if(!scene->checkTransferMeshValidity(this->getName())) {
            {
                QMessageBox msgBox;
                msgBox.setText("Warning: scale offset between the image and the tetrahedral mesh.");
                msgBox.setInformativeText(std::string(std::string("You choose a subsample of [") + std::to_string(this->getSubsample()) + std::string("]. However, the tetrahedral mesh you select seems to be at a different scale. Do you want the software to load the mesh at the correct scale ?")).c_str());
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);
                int ret = msgBox.exec();
                if(ret == QMessageBox::Yes) {
                    scene->getBaseMesh(this->getName())->scale(glm::vec3(1./float(this->getSubsample()), 1./float(this->getSubsample()), 1./float(this->getSubsample())));
                    scene->updateTextureCoordinates(this->getName());
                    scene->updateTetmeshAllGrids(true);// Update all informations, in particular texture coordinates because the Tetmesh is smaller
                    scene->updateSceneCenter();
                }
            }
            {
                if(useCage) {
                    QMessageBox msgBox;
                    msgBox.setText("Warning: scale offset between the image and the cage.");
                    msgBox.setInformativeText("Do you want the software to load the cage at the correct scale ?");
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msgBox.setDefaultButton(QMessageBox::Yes);
                    int ret = msgBox.exec();
                    if(ret == QMessageBox::Yes) {
                        scene->setBindMeshToCageMove(this->getName() + "_cage", false);
                        scene->getBaseMesh(this->getName() + "_cage")->scale(glm::vec3(1./float(this->getSubsample()), 1./float(this->getSubsample()), 1./float(this->getSubsample())));
                        scene->setBindMeshToCageMove(this->getName() + "_cage", true);
                    }
                }
            }
        }
        this->hide();
        Q_EMIT loaded();
    });

    QObject::connect(this->buttons["Mouse brain atlas"], &QPushButton::clicked, [this, scene](){
        this->labels["Name"]->setText("Mouse brain atlas");
        this->checkBoxes["Segmented"]->setChecked(true);
        this->fileChoosers["Image choose"]->setManually(std::string("../data/atlas.tiff"));
        this->sections["Mesh"].first->setChecked(true);
        this->fileChoosers["Mesh choose"]->setManually(std::string("../data/atlas-transfert.mesh"));
        this->sections["Cage"].first->setChecked(true);
        this->fileChoosers["Cage choose"]->setManually(std::string("../data/atlas-cage.off"));
    });
}
