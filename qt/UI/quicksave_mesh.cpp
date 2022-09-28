#include "quicksave_mesh.hpp"
#include "../scene.hpp"

#include <QMessageBox>

void QuickSaveMesh::save() {
    if(filePath.isEmpty()) {
        this->fileChooser->click();
        filePath = this->fileChooser->filename;
        if(filePath.isEmpty())
            return;
    }

    bool saved = scene->saveActiveMesh(filePath.toStdString());
    if(!saved)
        QMessageBox::critical(fileChooser, "Warning", "Selected object is not a mesh, can't save.");
}
