
#include "saveMeshWidget.hpp";
#include "../viewers/include/scene.hpp"

SaveMeshWidget::SaveMeshWidget(Scene* scene, QWidget* parent) {

    this->selectedFile = new QLabel();
    this->selectedFile->setText(QString("no file selected"));
    //this->selectedFile->setReadOnly(true);
    //QObject::connect(this->chooseFileName, &QLineEdit::textChanged, this, [this]() {this->adjustSize();});

    this->comboSave = new QComboBox();

    this->chooseFileName = new QPushButton("Choose");
    QObject::connect(this->chooseFileName, &QPushButton::released, this, [this, scene]() {this->selectFile();});

    saveMeshButton = new QPushButton("Save");
    QObject::connect(this->saveMeshButton, &QPushButton::released, this, [this, scene]() {this->saveMesh(scene);});

    this->line = new QFrame;
    this->line->setFrameShape(QFrame::HLine);
    this->line->setFrameShadow(QFrame::Sunken);

    // Layout
    mainLayout = new QVBoxLayout();

    fileLayout = new QVBoxLayout();
    //QHBoxLayout * selectLayout = 

    selectLayout = new QHBoxLayout();
    selectLayout->addWidget(this->chooseFileName);
    selectLayout->addWidget(this->selectedFile); 

    //fileLayout->addLayout(nameLayout);
    fileLayout->addWidget(line);
    fileLayout->addLayout(selectLayout);
    fileLayout->addWidget(this->comboSave);

    mainLayout->addLayout(fileLayout);

    mainLayout->addWidget(saveMeshButton);

    this->setLayout(this->mainLayout);
}

void SaveMeshWidget::selectFile() {
    QString file = QFileDialog::getSaveFileName(this, tr("Select the mesh to save"), QDir::currentPath(), tr("OFF Files (*.off)"));
    this->selectedFile->setText(file);
}

void SaveMeshWidget::saveMesh(Scene * scene) {
    scene->saveMesh(this->comboSave->currentText().toStdString(), this->selectedFile->text().toStdString());
}

void SaveMeshWidget::setPotentialMeshToSave(const QStringList& namesOfPotentialMeshesToSave) {
    this->comboSave->clear();
    this->comboSave->insertItems(0, namesOfPotentialMeshesToSave);
}
