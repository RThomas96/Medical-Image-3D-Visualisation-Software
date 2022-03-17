
#include "applyCageWidget.hpp";
#include "../viewers/include/scene.hpp"

ApplyCageWidget::ApplyCageWidget(Scene* scene, QWidget* parent) {

    this->selectedFile = new QLabel();
    this->selectedFile->setText(QString("no file selected"));
    //this->selectedFile->setReadOnly(true);
    //QObject::connect(this->chooseFileName, &QLineEdit::textChanged, this, [this]() {this->adjustSize();});

    this->comboApply = new QComboBox();

    this->chooseFileName = new QPushButton("Choose");
    QObject::connect(this->chooseFileName, &QPushButton::released, this, [this, scene]() {this->selectFile();});

    saveCageButton = new QPushButton("Apply");
    QObject::connect(this->saveCageButton, &QPushButton::released, this, [this, scene]() {this->applyCage(scene);});

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

    fileLayout->addWidget(line);
    fileLayout->addLayout(selectLayout);
    fileLayout->addWidget(this->comboApply);

    mainLayout->addLayout(fileLayout);

    mainLayout->addWidget(saveCageButton);

    this->setLayout(this->mainLayout);
}

void ApplyCageWidget::selectFile() {
    QString file = QFileDialog::getOpenFileName(this, tr("Select the cage to apply"), QDir::currentPath(), tr("OFF Files (*.off)"));
    this->selectedFile->setText(file);
}

void ApplyCageWidget::applyCage(Scene * scene) {
    scene->applyCage(this->comboApply->currentText().toStdString(), this->selectedFile->text().toStdString());
}

void ApplyCageWidget::setPotentialCageToApply(const QStringList& namesOfPotentialCagesToApply) {
    this->comboApply->clear();
    this->comboApply->insertItems(0, namesOfPotentialCagesToApply);
}
