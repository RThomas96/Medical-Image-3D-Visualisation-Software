
#include "chooser.hpp"
#include "../scene.hpp"

#include <QFileDialog>

void ObjectChooser::fillChoices(const ObjectToChoose& objectToChoose, Scene * scene) {
    this->clear();
    this->objectToChoose = objectToChoose;
    std::vector<std::string> all;
    if(objectToChoose == ObjectToChoose::ALL)
        all = scene->getAllBaseMeshesName();

    if(objectToChoose == ObjectToChoose::GRID)
        all = scene->getAllGridsName();

    this->blockSignals(true);
    for(std::string name : all) {
        this->addItem(QString(name.c_str()));
    }
    this->blockSignals(false);
}

void ObjectChooser::fillChoices(Scene * scene) {
    this->fillChoices(this->objectToChoose, scene);
}

//////////////

void FileChooser::init(FileChooserType type, FileChooserFormat format) {
    this->type = type;
    this->format = format;
    QObject::connect(this, &QPushButton::clicked, [this](){this->click();});
    this->resetValues();
}

void FileChooser::resetValues() {
    this->filename.clear();
}

void FileChooser::setType(FileChooserType type) {
    this->type = type;
}

void FileChooser::setFormat(FileChooserFormat format) {
    this->format = format;
}

void FileChooser::click() {
    QString filename;
    switch(this->type) {
        case FileChooserType::SELECT:
            if(this->format == FileChooserFormat::TIFF)
                filename = QFileDialog::getOpenFileName(nullptr, "Open TIFF images", QDir::currentPath(), "TIFF files (*.tiff *.tif)", 0, QFileDialog::DontUseNativeDialog);
            else if(this->format == FileChooserFormat::MESH)
                filename = QFileDialog::getOpenFileName(nullptr, "Open mesh file", QDir::currentPath(), "MESH files (*.mesh)", 0, QFileDialog::DontUseNativeDialog);
            else
                filename = QFileDialog::getOpenFileName(nullptr, "Open mesh file", QDir::currentPath(), "OFF files (*.off)", 0, QFileDialog::DontUseNativeDialog);
            break;

        case FileChooserType::SAVE:
            if(this->format == FileChooserFormat::TIFF)
                filename = QFileDialog::getSaveFileName(nullptr, "Select the image to save", QDir::currentPath(), tr("TIFF Files (*.tiff)"), 0, QFileDialog::DontUseNativeDialog);
            else if(this->format == FileChooserFormat::MESH)
                filename = QFileDialog::getSaveFileName(nullptr, "Select the mesh to save", QDir::currentPath(), tr("MESH Files (*.mesh)"), 0, QFileDialog::DontUseNativeDialog);
            else if(this->format == FileChooserFormat::PATH)
                filename = QFileDialog::getExistingDirectory(nullptr, "Select the directory to save", QDir::currentPath(), QFileDialog::DontUseNativeDialog);
            else
                filename = QFileDialog::getSaveFileName(nullptr, "Select the mesh to save", QDir::currentPath(), tr("OFF Files (*.off)"), 0, QFileDialog::DontUseNativeDialog);
            break;
    }
    if(!filename.isEmpty()) {
        this->filename = filename;
        Q_EMIT fileSelected();
    }
}

void FileChooser::setManually(std::string filename) {
    if(!filename.empty()) {
        this->filename = QString(filename.c_str());
        Q_EMIT fileSelected();
    }
}

void FileChooser::setManually(QString filename) {
    this->setManually(filename.toStdString());
}

///////

void FileName::init(FileChooser * fileChooser) {
    this->resetValues();
    if(fileChooser)
        QObject::connect(fileChooser, &FileChooser::fileSelected, [this, fileChooser](){this->setText(fileChooser->filename);});
}

void FileName::resetValues() {
    this->setText("[Select a file]");
}
