#ifndef OPEN_IMAGE_FORM
#define OPEN_IMAGE_FORM

#include<iostream>
#include<QButtonGroup>
#include<QFileInfo>

#include "glm/glm.hpp"
#include "form.hpp"


class OpenImageForm : public Form {
    Q_OBJECT

public:

    bool useTetMesh;
    OpenImageForm(Scene * scene, QWidget *parent = nullptr):Form(parent){init();connect(scene);}

public slots:

    void init() {
        this->useTetMesh = false;
        //this->addFileChooser("Save image", FileChooserType::SAVE);


        this->addWithLabel(WidgetType::H_GROUP, "GroupPreset", "Presets");
        this->addAllNextWidgetsToGroup("GroupPreset");
        this->add(WidgetType::BUTTON, "Mouse brain atlas");
        //this->add(WidgetType::BUTTON, "Mouse brain autofluo");
        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::LINE_EDIT, "Name", "Name: ");

        this->add(WidgetType::SECTION, "Image");
        this->addAllNextWidgetsToSection("Image");

        /***/

        this->addWithLabel(WidgetType::SPIN_BOX, "Subsample", "Subsample: ");

        /***/

        this->add(WidgetType::SECTION_CHECKABLE, "Image subregion");
        this->addAllNextWidgetsToSection("Image subregion");

        this->addWithLabel(WidgetType::H_GROUP, "GroupBBMin", "BBox min");
        this->addAllNextWidgetsToGroup("GroupBBMin");

        this->add(WidgetType::SPIN_BOX, "BBMinX");
        this->add(WidgetType::SPIN_BOX, "BBMinY");
        this->add(WidgetType::SPIN_BOX, "BBMinZ");

        this->addAllNextWidgetsToDefaultGroup();
        this->addAllNextWidgetsToSection("Image subregion");

        this->addWithLabel(WidgetType::H_GROUP, "GroupBBMax", "BBox max");
        this->addAllNextWidgetsToGroup("GroupBBMax");

        this->add(WidgetType::SPIN_BOX, "BBMaxX");
        this->add(WidgetType::SPIN_BOX, "BBMaxY");
        this->add(WidgetType::SPIN_BOX, "BBMaxZ");

        /***/

        this->addAllNextWidgetsToSection("Image");

        this->addWithLabel(WidgetType::CHECK_BOX, "Segmented", "Segmented: ");

        this->addWithLabel(WidgetType::H_GROUP, "GroupPosition", "Position: ");
        this->addAllNextWidgetsToGroup("GroupPosition");

        this->add(WidgetType::SPIN_BOX_DOUBLE, "PositionX");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "PositionY");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "PositionZ");

        this->addAllNextWidgetsToSection("Image");

        this->addWithLabel(WidgetType::H_GROUP, "GroupVoxelSize", "Voxel size: ");
        this->addAllNextWidgetsToGroup("GroupVoxelSize");

        this->add(WidgetType::SPIN_BOX_DOUBLE, "SizeVoxelX");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "SizeVoxelY");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "SizeVoxelZ");

        this->addAllNextWidgetsToSection("Image");

        this->add(WidgetType::FILENAME, "Image filename");
        this->add(WidgetType::TIFF_CHOOSE, "Image choose", "Select image file");
        this->linkFileNameToFileChooser("Image filename", "Image choose");

        /***/

        this->addAllNextWidgetsToDefaultGroup();
        this->addAllNextWidgetsToDefaultSection();

        this->add(WidgetType::SECTION_CHECKABLE, "Mesh");
        this->addAllNextWidgetsToSection("Mesh");

        //this->add(WidgetType::SECTION, "Tetrahedral mesh size");
        //this->addAllNextWidgetsToSection("Tetrahedral mesh size");

        //this->addWithLabel(WidgetType::H_GROUP, "GroupNbTet", "Nb tetrahedra");
        //this->addAllNextWidgetsToGroup("GroupNbTet");

        //this->add(WidgetType::SPIN_BOX, "NbTetX");
        //this->add(WidgetType::SPIN_BOX, "NbTetY");
        //this->add(WidgetType::SPIN_BOX, "NbTetZ");

        this->addAllNextWidgetsToSection("Mesh");

        this->addAllNextWidgetsToSection("Mesh");

        this->add(WidgetType::FILENAME, "Mesh filename");
        this->add(WidgetType::MESH_CHOOSE, "Mesh choose", "Select mesh file");
        this->linkFileNameToFileChooser("Mesh filename", "Mesh choose");

        /***/

        this->addAllNextWidgetsToDefaultGroup();
        this->addAllNextWidgetsToDefaultSection();

        this->add(WidgetType::SECTION_CHECKABLE, "Cage");
        this->addAllNextWidgetsToSection("Cage");

        this->addWithLabel(WidgetType::H_GROUP, "GroupCageType", "Type:");
        //this->addAllNextWidgetsToGroup("GroupCageType");

        //this->addWithLabel(WidgetType::CHECK_BOX, "MVC", "MVC");
        //this->addWithLabel(WidgetType::CHECK_BOX, "Green", "Green");

        //this->addAllNextWidgetsToSection("Cage");

        QLabel * mvcLabel = new QLabel("MVC");
        QCheckBox * mvc = new QCheckBox();
        mvc->setChecked(true);
        QLabel * greenLabel = new QLabel("Green");
        QCheckBox * green = new QCheckBox();

        this->checkBoxes["mvc"] = mvc;
        this->checkBoxes["green"] = green;

        QHBoxLayout * mvcLayout = new QHBoxLayout();
        mvcLayout->setAlignment(Qt::AlignHCenter);
        QHBoxLayout * greenLayout = new QHBoxLayout();
        greenLayout->setAlignment(Qt::AlignHCenter);

        mvcLayout->addWidget(mvcLabel);
        mvcLayout->addWidget(mvc);

        greenLayout->addWidget(greenLabel);
        greenLayout->addWidget(green);

        QButtonGroup * group = new QButtonGroup();
        group->addButton(mvc);
        group->addButton(green);

        this->groups["GroupCageType"]->addLayout(mvcLayout);
        this->groups["GroupCageType"]->addLayout(greenLayout);

        this->add(WidgetType::FILENAME, "Cage filename");
        this->add(WidgetType::OFF_CHOOSE, "Cage choose", "Select cage file");
        this->linkFileNameToFileChooser("Cage filename", "Cage choose");

        /***/

        this->addAllNextWidgetsToDefaultGroup();
        this->addAllNextWidgetsToDefaultSection();

        this->add(WidgetType::BUTTON, "Load");

        this->resetValues();

        //this->sections["Image subsample"].first->hide();
        this->sections["Image subregion"].first->hide();
        //this->labels["GroupVoxelSize"]->hide();
        //this->doubleSpinBoxes["SizeVoxelX"]->hide();
        //this->doubleSpinBoxes["SizeVoxelY"]->hide();
        //this->doubleSpinBoxes["SizeVoxelZ"]->hide();
    }

    void resetValues() {
        this->useTetMesh = false;
        this->lineEdits["Name"]->clear();

        this->fileNames["Image filename"]->resetValues();
        this->fileChoosers["Image choose"]->resetValues();

        this->fileNames["Mesh filename"]->resetValues();
        this->fileChoosers["Mesh choose"]->resetValues();

        this->fileNames["Cage filename"]->resetValues();
        this->fileChoosers["Cage choose"]->resetValues();

        this->spinBoxes["Subsample"]->setValue(1);
        this->spinBoxes["Subsample"]->setMinimum(1);

        //this->spinBoxes["NbTetX"]->setValue(5);
        //this->spinBoxes["NbTetX"]->setMinimum(1);
        //this->spinBoxes["NbTetY"]->setValue(5);
        //this->spinBoxes["NbTetY"]->setMinimum(1);
        //this->spinBoxes["NbTetZ"]->setValue(5);
        //this->spinBoxes["NbTetZ"]->setMinimum(1);

        this->doubleSpinBoxes["SizeVoxelX"]->setValue(1);
        this->doubleSpinBoxes["SizeVoxelX"]->setMinimum(0);
        this->doubleSpinBoxes["SizeVoxelY"]->setValue(1);
        this->doubleSpinBoxes["SizeVoxelY"]->setMinimum(0);
        this->doubleSpinBoxes["SizeVoxelZ"]->setValue(1);
        this->doubleSpinBoxes["SizeVoxelZ"]->setMinimum(0);

        this->sections["Mesh"].first->hide();
        this->sections["Cage"].first->hide();

        this->sections["Mesh"].first->setChecked(false);
        this->sections["Cage"].first->setChecked(false);

        this->sections["Image subregion"].first->setEnabled(false);

        this->buttons["Load"]->setEnabled(false);
    }

    void update(Scene * scene);

    void show() {
        this->resetValues();
        Form::show();
    }

    std::string getFromGridName() {
        return this->objectChoosers["From"]->currentText().toStdString();
    }

    std::string getToGridName() {
        return this->objectChoosers["To"]->currentText().toStdString();
    }

    std::string getGridName() {
        return this->lineEdits["Name"]->text().toStdString();
    }

    std::vector<std::string> getImgFilenames() {
        return std::vector<std::string>{this->fileChoosers["Image choose"]->filename.toStdString()};
    }

    std::string getTetmeshFilename() {
        return this->fileChoosers["Mesh choose"]->filename.toStdString();
    }

    int getSubsample() {
        return this->spinBoxes["Subsample"]->value();
    }

    glm::vec3 getVoxelSize() {
        return glm::vec3(this->doubleSpinBoxes["SizeVoxelX"]->value(),
                         this->doubleSpinBoxes["SizeVoxelY"]->value(),
                         this->doubleSpinBoxes["SizeVoxelZ"]->value());
    }

    glm::vec3 getSizeTetmesh() {
        return glm::vec3(5., 5., 5.);
        //return glm::vec3(this->spinBoxes["NbTetX"]->value(),
        //                 this->spinBoxes["NbTetY"]->value(),
        //                 this->spinBoxes["NbTetZ"]->value());
    }

    void prefillFields(const std::vector<std::string>& files) {
        std::cout << "***" << std::endl;
        std::cout << "Parse image to fill informations" << std::endl;
        std::cout << "Disabled for now" << std::endl;
        //SimpleImage image(files);
        //this->doubleSpinBoxes["SizeVoxelX"]->setValue(image.voxelSize.x);
        //this->doubleSpinBoxes["SizeVoxelY"]->setValue(image.voxelSize.y);
        //this->doubleSpinBoxes["SizeVoxelZ"]->setValue(image.voxelSize.z);
        this->lineEdits["Name"]->setText(QFileInfo(QString(files[0].c_str())).baseName());
        std::cout << "***" << std::endl;
    }

    void connect(Scene * scene);
public:
signals:
    void loaded();
};


#endif
