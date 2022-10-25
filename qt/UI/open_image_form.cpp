
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
        bool useVoxelSize = this->getVoxelSize() != glm::vec3(1., 1., 1.);
        int minVoxeSize = std::floor(std::min(this->getVoxelSize().x, std::min(this->getVoxelSize().y, this->getVoxelSize().z)));
        bool useSubsample = minVoxeSize >= 2.;
        //bool useSubsample = this->getSubsample() != 1.;
        if(useVoxelSize) {
            //if(!useTetMesh) {
            //    scene->updateTextureCoordinates(this->getGridName());
            //    scene->updateTetmeshAllGrids(true);// Update all informations, in particular texture coordinates because the Tetmesh is smaller
            //}
            if(useTetMesh)
            {
                QMessageBox msgBox;
                msgBox.setText("Warning: scale offset between the image and the tetrahedral mesh.");
                msgBox.setInformativeText(std::string(std::string("You choose a subsample of [") + std::to_string(this->getSubsample()) + std::string("]. However, the tetrahedral mesh you select seems to be at a different scale. Do you want the software to load the mesh at the correct scale ?")).c_str());
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);
                int ret = msgBox.exec();
                if(ret == QMessageBox::Yes) {
                    scene->getBaseMesh(this->getGridName())->scale(this->getVoxelSize());
                    //if(useSubsample)
                    //    scene->updateTextureCoordinates(this->getGridName());
                    scene->updateTetmeshAllGrids(true);// Update all informations, in particular texture coordinates because the Tetmesh is smaller
                    scene->updateSceneCenter();
                }
                if(useCage) {
                    QMessageBox msgBox;
                    msgBox.setText("Warning: scale offset between the image and the cage.");
                    msgBox.setInformativeText("Do you want the software to load the cage at the correct scale ?");
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msgBox.setDefaultButton(QMessageBox::Yes);
                    int ret = msgBox.exec();
                    if(ret == QMessageBox::Yes) {
                        scene->setBindMeshToCageMove(this->getGridName() + "_cage", false);
                        scene->getBaseMesh(this->getGridName() + "_cage")->scale(this->getVoxelSize());
                        scene->setBindMeshToCageMove(this->getGridName() + "_cage", true);
                    }
                }
            }
        }
        // WARNING this section is if you want to still load the grid with voxel at 1x1x1
        //if(this->useTetMesh && useVoxelSize) {
        //    {
        //        QMessageBox msgBox;
        //        msgBox.setText("Warning: different voxel size.");
        //        msgBox.setInformativeText(std::string(std::string("You choose a voxel size of [") + std::to_string(this->getVoxelSize().x) + ", " + std::to_string(this->getVoxelSize().y) + ", " + std::to_string(this->getVoxelSize().z) + std::string("]. Do you want the software to load the mesh with a corresponding scale ?")).c_str());
        //        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        //        msgBox.setDefaultButton(QMessageBox::Yes);
        //        int ret = msgBox.exec();
        //        if(ret == QMessageBox::Yes) {
        //            scene->getBaseMesh(this->getGridName())->scale(glm::vec3(float(this->getVoxelSize().x), float(this->getVoxelSize().y), float(this->getVoxelSize().z)));
        //            //scene->updateTextureCoordinates(this->getGridName());
        //            scene->updateTetmeshAllGrids(true);// Update all informations, in particular texture coordinates because the Tetmesh is smaller
        //            scene->updateSceneCenter();
        //        }
        //    }
        //    {
        //        if(useCage) {
        //            QMessageBox msgBox;
        //            msgBox.setText("Warning: different voxel size.");
        //            msgBox.setInformativeText("Do you want the software to load the cage with the voxel size scale ?");
        //            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        //            msgBox.setDefaultButton(QMessageBox::Yes);
        //            int ret = msgBox.exec();
        //            if(ret == QMessageBox::Yes) {
        //                scene->setBindMeshToCageMove(this->getGridName() + "_cage", false);
        //                scene->getBaseMesh(this->getGridName() + "_cage")->scale(glm::vec3(float(this->getVoxelSize().x), float(this->getVoxelSize().y), float(this->getVoxelSize().z)));
        //                scene->setBindMeshToCageMove(this->getGridName() + "_cage", true);
        //            }
        //        }
        //    }
        //}
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
