
#include "openMeshWidget.hpp";
#include "../viewers/include/scene.hpp"

OpenMeshWidget::OpenMeshWidget(Scene* scene, QWidget* parent) {

    this->labelName = new QLabel("Name: ");
    this->groupName = new QGroupBox("");
    this->groupName->setCheckable(false);
    this->name = new QLineEdit();
    this->name->setText(QString("MyMesh"));

    this->selectedFile = new QLabel();
    this->selectedFile->setText(QString("no file selected"));
    //this->selectedFile->setReadOnly(true);
    //QObject::connect(this->chooseFileName, &QLineEdit::textChanged, this, [this]() {this->adjustSize();});

    this->chooseFileName = new QPushButton("Choose");
    QObject::connect(this->chooseFileName, &QPushButton::released, this, [this, scene]() {this->selectFile();});

    this->cageGroup = new QGroupBox("Cage");
    this->cageGroup->setCheckable(true);
    this->cageGroup->setChecked(false);
    this->radioMVC = new QRadioButton("MVC");
    this->radioMVC->setChecked(false);
    this->radioGreen = new QRadioButton("Green");
    this->radioGreen->setChecked(true);
    this->comboCages = new QComboBox();

    loadMesh = new QPushButton("Load");
    QObject::connect(this->loadMesh, &QPushButton::released, this, [this, scene]() {this->loadMeshToScene(scene);});

    this->line = new QFrame;
    this->line->setFrameShape(QFrame::HLine);
    this->line->setFrameShadow(QFrame::Sunken);

    // Layout
    mainLayout = new QVBoxLayout();

    fileLayout = new QVBoxLayout();
    //QHBoxLayout * selectLayout = 
    nameLayout = new QHBoxLayout();
    nameLayout->addWidget(this->labelName);
    nameLayout->addWidget(this->name);

    selectLayout = new QHBoxLayout();
    selectLayout->addWidget(this->chooseFileName);
    selectLayout->addWidget(this->selectedFile); 

    fileLayout->addLayout(nameLayout);
    fileLayout->addWidget(line);
    fileLayout->addLayout(selectLayout);

    groupName->setLayout(fileLayout);
    groupName->setStyleSheet("QGroupBox{padding-top:15px; margin-top:-15px}");

    mainLayout->addWidget(groupName);

    cageLayout = new QVBoxLayout();
    radioLayout = new QHBoxLayout();
    radioLayout->addWidget(this->radioMVC);
    radioLayout->addWidget(this->radioGreen);
    cageLayout->addLayout(radioLayout);
    cageLayout->addWidget(this->comboCages);
    cageGroup->setLayout(cageLayout);


    mainLayout->addWidget(cageGroup);
    mainLayout->addWidget(loadMesh);

    this->setLayout(this->mainLayout);
}

void OpenMeshWidget::setPotentialCages(const QStringList& namesOfPotentialCages) {
    this->comboCages->clear();
    this->comboCages->insertItems(0, namesOfPotentialCages);
}

void OpenMeshWidget::selectFile() {
    QString file = QFileDialog::getOpenFileName(this, tr("Select the mesh to load"), QDir::currentPath(), tr("OFF Files (*.off)"), 0, QFileDialog::DontUseNativeDialog);
    this->selectedFile->setText(file);

    QFileInfo info(file);
    if(QString::compare(this->name->text(), QString("MyMesh"), Qt::CaseInsensitive) == 0)
        this->name->setText(info.baseName());
}

void OpenMeshWidget::loadMeshToScene(Scene * scene) {
    if(this->cageGroup->isChecked()) {
        scene->openCage(this->name->text().toStdString(), this->selectedFile->text().toStdString(), this->comboCages->currentText().toStdString(), this->radioMVC->isChecked());
    } else {
        scene->openMesh(this->name->text().toStdString(), this->selectedFile->text().toStdString());
    }
    Q_EMIT loaded();
    this->hide();
}
