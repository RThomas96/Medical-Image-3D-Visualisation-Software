#ifndef QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
#define QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_

#include <iomanip>
#include <sstream>
#include "../../qt/viewers/include/neighbor_visu_viewer.hpp"
#include "../../qt/viewers/include/planar_viewer.hpp"
#include "../../qt/viewers/include/scene.hpp"
//#include "./grid_control.hpp"
#include "./loader_widget.hpp"
#include "../deformation_widget.hpp"
#include "../openMeshWidget.hpp"
#include "../saveMeshWidget.hpp"
#include "../applyCageWidget.hpp"
#include "../CutPlaneGroupBox.h"
#include "./opengl_debug_log.hpp"
#include "./scene_control.hpp"
#include "./user_settings_widget.hpp"
#include "glm/fwd.hpp"
#include "qboxlayout.h"
#include "qbuttongroup.h"
#include "qjsonarray.h"
#include "qnamespace.h"
#include "qobjectdefs.h"
#include "qt/viewers/include/viewer_structs.hpp"
#include <random>

#include <map>

#include <QMessageBox>
#include <QPainter>
#include <QGLViewer/qglviewer.h>
#include <QMainWindow>
#include <QWidget>
#include <QToolBar>
#include <QFrame>
#include <QSizePolicy>
#include <QTabWidget>
#include <QShortcut>
#include <QMenuBar>
#include <QToolButton>
#include <QLineEdit>
#include <QFormLayout>
#include <QTextEdit>
#include <QTextBlock>
#include <QMessageBox>
#include <QImage>
#include <QSlider>
#include <QSignalMapper>
#include <QSplitter>
#include <QScrollArea>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <string>
#include <vector>

class ColorBoundWidget;

enum ObjectToChoose {
    ALL,
    GRID
};

class ObjectChooser : public QComboBox {
    Q_OBJECT

public:
    ObjectToChoose objectToChoose;

    ObjectChooser(QWidget *parent = nullptr):QComboBox(parent){}

    void fillChoices(const ObjectToChoose& objectToChoose, Scene * scene) {
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

    void fillChoices(Scene * scene) {
        this->fillChoices(this->objectToChoose, scene);
    }
};

enum FileChooserType {
    SELECT,
    SAVE
};

enum class FileChooserFormat {
    TIFF,
    MESH,
    PATH,
    OFF
};

class FileChooser : public QPushButton {
    Q_OBJECT

public:
    FileChooserFormat format;
    FileChooserType type;
	QString filename;
    FileChooser(QString name, FileChooserType type = FileChooserType::SELECT, FileChooserFormat format = FileChooserFormat::TIFF, QWidget *parent = nullptr):QPushButton(name, parent){init(type, format);}

public slots:

    void init(FileChooserType type, FileChooserFormat format) {
        this->type = type;
        this->format = format;
        QObject::connect(this, &QPushButton::clicked, [this](){this->click();});
        this->resetValues();
    }

    void resetValues() {
        this->filename.clear();
    }

    void setType(FileChooserType type) {
        this->type = type;
    }

    void setFormat(FileChooserFormat format) {
        this->format = format;
    }

    void click() {
        QString filename;
        switch(this->type) {
            case FileChooserType::SELECT:
                if(this->format == FileChooserFormat::TIFF)
	                filename = QFileDialog::getOpenFileName(nullptr, "Open TIFF images", QDir::currentPath(), "TIFF files (*.tiff *.tif)", 0, QFileDialog::DontUseNativeDialog);
                else if(this->format == FileChooserFormat::MESH)
	                filename = QFileDialog::getOpenFileName(nullptr, "Open mesh file", QDir::currentPath(), "MESH files (*.mesh)", 0, QFileDialog::DontUseNativeDialog);
                else
                    filename = QFileDialog::getOpenFileName(nullptr, "Open mesh file", QDir::currentPath(), "MESH files (*.off)", 0, QFileDialog::DontUseNativeDialog);
                break;

            case FileChooserType::SAVE:
                if(this->format == FileChooserFormat::TIFF)
	                filename = QFileDialog::getSaveFileName(nullptr, "Select the image to save", QDir::currentPath(), tr("TIFF Files (*.tiff)"), 0, QFileDialog::DontUseNativeDialog);
                else if(this->format == FileChooserFormat::MESH)
                    filename = QFileDialog::getSaveFileName(nullptr, "Select the mesh to save", QDir::currentPath(), tr("OFF Files (*.off)"), 0, QFileDialog::DontUseNativeDialog);
                else if(this->format == FileChooserFormat::PATH)
                    filename = QFileDialog::getExistingDirectory(nullptr, "Select the directory to save", QDir::currentPath(), QFileDialog::DontUseNativeDialog);
                else
                    filename = QFileDialog::getSaveFileName(nullptr, "Select the mesh to save", QDir::currentPath(), tr("MESH Files (*.mesh)"), 0, QFileDialog::DontUseNativeDialog);
                break;
        }
        if(!filename.isEmpty()) {
            this->filename = filename;
            Q_EMIT fileSelected();
        }
    }

signals:
    void fileSelected();
};

//class FormSection : public QGroupBox {
//    Q_OBJECT
//
//public:
//
//    Form * form;
//
//    FormSection(QWidget *parent = nullptr):QGroupBox(parent){init();}
//
//public slots:
//
//    void init() {
//        this->form = new form();
//    }
//};

class FileName : public QLabel {
    Q_OBJECT

public:
    FileName(QWidget *parent = nullptr):QLabel(parent){init(nullptr);}

public slots:

    void init(FileChooser * fileChooser) {
        this->resetValues();
        if(fileChooser)
            QObject::connect(fileChooser, &FileChooser::fileSelected, [this, fileChooser](){this->setText(fileChooser->filename);});
    }

    void resetValues() {
        this->setText("[Select a file]");
    }
};

enum WidgetType{
    LABEL,
    LINE_EDIT,
    TEXT_EDIT,
    BUTTON,
    BUTTON_CHECKABLE,
    BUTTON_CHECKABLE_AUTOEXCLUSIVE,
    SPIN_BOX,
    SPIN_BOX_DOUBLE,
    MESH_SAVE,
    OFF_SAVE,
    TIFF_SAVE,
    PATH_SAVE,
    MESH_CHOOSE,
    OFF_CHOOSE,
    TIFF_CHOOSE,
    FILENAME,
    GRID_CHOOSE,
    SECTION,
    SECTION_CHECKABLE,
    H_GROUP,
    V_GROUP,
    COMBO_BOX,
    CHECK_BOX,
    SLIDER
};

class Form : public QWidget {
    Q_OBJECT

public:
    bool insertNextWidgetInSection;
    QString sectionToInsertIn;

    bool insertNextWidgetInGroup;
    QString groupToInsertIn;

    QFormLayout * layout;

    QStringList names;

    std::map<QString, QLabel*> labels;
    std::map<QString, QLineEdit*> lineEdits;
    std::map<QString, ObjectChooser*> objectChoosers;
    std::map<QString, QTextEdit*> textEdits;
    std::map<QString, QPushButton*> buttons;
    std::map<QString, QSpinBox*> spinBoxes;
    std::map<QString, QDoubleSpinBox*> doubleSpinBoxes;
    std::map<QString, FileName*> fileNames;
    std::map<QString, FileChooser*> fileChoosers;
    std::map<QString, std::pair<QGroupBox*, QFormLayout*>> sections;
    std::map<QString, QComboBox*> comboBoxes;
    std::map<QString, QCheckBox*> checkBoxes;
    std::map<QString, QSlider*> sliders;

    std::map<QString, QBoxLayout*> groups;

    Form(QWidget *parent = nullptr):QWidget(parent){init();}

signals:
    void widgetModified(const QString &id);

public slots:

    void init() {
        this->setWindowFlags(Qt::WindowStaysOnTopHint);
        this->layout = new QFormLayout();
        this->setLayout(this->layout);

        insertNextWidgetInSection = false;
        sectionToInsertIn = QString("");

        insertNextWidgetInGroup = false;
        groupToInsertIn = QString("");
    }

    void blockSignalsInGroup(const QString& group, bool blockSignals) {
        QLayout * layout = groups[group];
        for (int i = 0; i < layout->count(); ++i) {
            layout->itemAt(i)->widget()->blockSignals(blockSignals);
        }
    }

    void addGroup(QLayout * group) {
        if(insertNextWidgetInSection) {
            this->addGroupToSection(this->sectionToInsertIn, group);
        } else if(insertNextWidgetInGroup) {
            this->addGroupToGroup(this->groupToInsertIn, group);
        } else {
            this->layout->addRow(group);
        }
    }

    void addGroup(QWidget * label, QLayout * group) {
        if(insertNextWidgetInSection) {
            this->addGroupToSection(this->sectionToInsertIn, label, group);
        } else if(insertNextWidgetInGroup) {
            this->addGroupToGroup(this->groupToInsertIn, label, group);
        } else {
            this->layout->addRow(label, group);
        }
    }

    void addWidget(QWidget * widget1, QWidget * widget2 = nullptr) {
        if(widget2) {
            if(insertNextWidgetInSection) {
                this->addWidgetToSection(this->sectionToInsertIn, widget1, widget2);
            } else if(insertNextWidgetInGroup) {
                this->addWidgetToGroup(this->groupToInsertIn, widget1, widget2);
            } else {
                this->layout->addRow(widget1, widget2);
            }
        } else {
            if(insertNextWidgetInSection) {
                this->addWidgetToSection(this->sectionToInsertIn, widget1);
            } else if(insertNextWidgetInGroup) {
                this->addWidgetToGroup(this->groupToInsertIn, widget1);
            } else {
                this->layout->addRow(widget1);
            }
        }
    }

    void addAllNextWidgetsToSection(const QString& name) {
        this->insertNextWidgetInSection = true;
        this->insertNextWidgetInGroup = false;
        this->sectionToInsertIn = name;
    }

    void addAllNextWidgetsToDefaultSection() {
        this->insertNextWidgetInSection = false;
    }

    void addAllNextWidgetsToGroup(const QString& name) {
        this->insertNextWidgetInGroup = true;
        this->insertNextWidgetInSection = false;
        this->groupToInsertIn = name;
    }

    void addAllNextWidgetsToDefaultGroup() {
        this->insertNextWidgetInGroup = false;
    }

    void setSectionCheckable(const QString& name, bool checkable) {
        this->sections[name].first->setCheckable(checkable);
    }
    
    void setTextEditEditable(const QString& id, bool editable) {
        textEdits[id]->setReadOnly(!editable);
    }

    void linkFileNameToFileChooser(const QString& filenameId, const QString& fileChooserId) {
        this->fileNames[filenameId]->init(this->fileChoosers[fileChooserId]);
    }

    void setObjectTypeToChoose(const QString& id, ObjectToChoose objectToChoose) {
        objectChoosers[id]->objectToChoose = objectToChoose;
    }

    void setFileChooserType(const QString& id, FileChooserType type) {
        fileChoosers[id]->setType(type);
    }

    void setComboChoices(const QString& id, const std::vector<QString>& choices) {
        this->comboBoxes[id]->blockSignals(true);
        for(auto choice : choices) {
            this->comboBoxes[id]->addItem(choice);
        }
        this->comboBoxes[id]->blockSignals(false);
    }

    void setComboChoices(const QString& id, const std::vector<std::string>& choices) {

        this->comboBoxes[id]->blockSignals(true);
        for(auto choice : choices) {
            this->comboBoxes[id]->addItem(QString(choice.c_str()));
        }
        this->comboBoxes[id]->blockSignals(false);
    }

    void addWithLabel(const WidgetType& type, const QString& id, const QString& label) {
        this->add(type, id, id, label);
    }

    void add(const WidgetType& type, const QString& id) {
        this->add(type, id, id, "");
    }

    void add(const WidgetType& type, const QString& id, const QString& name) {
        this->add(type, id, name, "");
    }

    void add(const WidgetType& type, const QString& id, const QString& name, const QString& label) {
        names += id;
        QWidget * newWidget = nullptr;
        QLayout * newGroup = nullptr;
        switch(type) {
            case WidgetType::LINE_EDIT:
                lineEdits[id] = new QLineEdit();
                newWidget = lineEdits[id]; 
                break;
            case WidgetType::TEXT_EDIT:
                textEdits[id] = new QTextEdit();
                newWidget = textEdits[id];
                break;
            case WidgetType::GRID_CHOOSE:
                objectChoosers[id] = new ObjectChooser();
                newWidget = objectChoosers[id];
                objectChoosers[id]->objectToChoose = ObjectToChoose::GRID;
                connect(objectChoosers[id], QOverload<int>::of(&QComboBox::currentIndexChanged), [this, id] { widgetModified(id); });
                break;
            case WidgetType::SPIN_BOX:
                spinBoxes[id] = new QSpinBox();
                spinBoxes[id]->setRange(-10000000, 10000000);
                newWidget = spinBoxes[id];
                connect(spinBoxes[id], QOverload<int>::of(&QSpinBox::valueChanged), [this, id] { widgetModified(id); });
                break;
            case WidgetType::SPIN_BOX_DOUBLE:
                doubleSpinBoxes[id] = new QDoubleSpinBox();
                doubleSpinBoxes[id]->setRange(-10000000, 10000000);
                doubleSpinBoxes[id]->setDecimals(6);
                newWidget = doubleSpinBoxes[id];
                connect(doubleSpinBoxes[id], QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, id] { widgetModified(id); });
                break;
            case WidgetType::LABEL:
                labels[id] = new QLabel(name);
                newWidget = labels[id];
                break;
            case WidgetType::SECTION_CHECKABLE:
                sections[id] = std::make_pair(new QGroupBox(name), new QFormLayout());
                sections[id].first->setLayout(this->sections[id].second);
                sections[id].first->setCheckable(true);
                newWidget = sections[name].first;
                break;
            case WidgetType::SECTION:
                sections[id] = std::make_pair(new QGroupBox(name), new QFormLayout());
                sections[id].first->setLayout(this->sections[id].second);
                sections[id].first->setCheckable(false);
                newWidget = sections[name].first;
                break;
            case WidgetType::BUTTON:
                buttons[id] = new QPushButton(name);
                newWidget = buttons[id];
                connect(buttons[id], &QPushButton::clicked, [this, id] { widgetModified(id); });
                break;
            case WidgetType::BUTTON_CHECKABLE:
                buttons[id] = new QPushButton(name);
                buttons[id]->setCheckable(true);
                newWidget = buttons[id];
                connect(buttons[id], &QPushButton::clicked, [this, id] { widgetModified(id); });
                break;
            case WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE:
                buttons[id] = new QPushButton(name);
                buttons[id]->setCheckable(true);
                buttons[id]->setAutoExclusive(true);
                newWidget = buttons[id];
                connect(buttons[id], &QPushButton::clicked, [this, id] { widgetModified(id); });
                break;
            case WidgetType::FILENAME:
                fileNames[id] = new FileName();
                newWidget = fileNames[id];
                break;
            case WidgetType::MESH_CHOOSE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SELECT, FileChooserFormat::MESH);
                newWidget = fileChoosers[id];
                break;
            case WidgetType::OFF_CHOOSE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SELECT, FileChooserFormat::OFF);
                newWidget = fileChoosers[id];
                break;
            case WidgetType::TIFF_CHOOSE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SELECT, FileChooserFormat::TIFF);
                newWidget = fileChoosers[id];
                break;
            case WidgetType::MESH_SAVE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SAVE, FileChooserFormat::MESH);
                newWidget = fileChoosers[id];
                break;
            case WidgetType::OFF_SAVE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SAVE, FileChooserFormat::OFF);
                newWidget = fileChoosers[id];
                break;
            case WidgetType::TIFF_SAVE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SAVE, FileChooserFormat::TIFF);
                newWidget = fileChoosers[id];
                break;
            case WidgetType::PATH_SAVE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SAVE, FileChooserFormat::PATH);
                newWidget = fileChoosers[id];
                break;
            case WidgetType::H_GROUP:
                groups[id] = new QHBoxLayout();
                newGroup = groups[id];
                break;
            case WidgetType::V_GROUP:
                groups[id] = new QVBoxLayout();
                newGroup = groups[id];
                break;
            case WidgetType::COMBO_BOX:
                comboBoxes[id] = new QComboBox();
                newWidget = comboBoxes[id];
                connect(comboBoxes[id], QOverload<int>::of(&QComboBox::currentIndexChanged), [this, id] { widgetModified(id); });
                break;
            case WidgetType::CHECK_BOX:
                checkBoxes[id] = new QCheckBox();
                newWidget = checkBoxes[id];
                connect(checkBoxes[id], &QCheckBox::clicked, [this, id] { widgetModified(id); });
                break;
            case WidgetType::SLIDER:
                sliders[id] = new QSlider();
                sliders[id]->setOrientation(Qt::Orientation::Horizontal);
                newWidget = sliders[id];
                connect(sliders[id], &QSlider::valueChanged, [this, id] { widgetModified(id); });
                break;
        };
        if(newWidget) {
            if(label != "") {
                labels[id] = new QLabel(label);
                this->addWidget(labels[id], newWidget);
            } else {
                this->addWidget(newWidget);
            }
        } else if(newGroup) {
            if(label != "") {
                labels[id] = new QLabel(label);
                this->addGroup(labels[id], newGroup);
            } else {
                this->addGroup(newGroup);
            }
        }
    }

    void update(Scene * scene) {
        for(auto& chooser : objectChoosers) {
            chooser.second->fillChoices(scene);
        }
    }

private:
    void addWidgetToSection(const QString& name, QWidget * widget1, QWidget * widget2 = nullptr) {
        if(widget2)
            this->sections[name].second->addRow(widget1, widget2);
        else
            this->sections[name].second->addRow(widget1);
    }

    void addWidgetToGroup(const QString& name, QWidget * widget1, QWidget * widget2 = nullptr) {
            this->groups[name]->addWidget(widget1);
            if(widget2)
                this->groups[name]->addWidget(widget2);
    }

    void addGroupToSection(const QString& name, QWidget * label, QLayout * group) {
            this->sections[name].second->addRow(label, group);
    }

    void addGroupToSection(const QString& name, QLayout * group) {
            this->sections[name].second->addRow(group);
    }

    void addGroupToGroup(const QString& name, QWidget * label, QLayout * group) {
            this->groups[name]->addLayout(group);
    }

    void addGroupToGroup(const QString& name, QLayout * group) {
            this->groups[name]->addLayout(group);
    }
};

class DeformationForm : Form {
    Q_OBJECT

public:

    std::vector<glm::vec3> origins;
    std::vector<glm::vec3> results;
    DeformationForm(Scene * scene, QWidget *parent = nullptr):Form(parent){init();connect(scene);}

public slots:

    void init() {
        this->add(WidgetType::GRID_CHOOSE, "From");
        this->setObjectTypeToChoose("From", ObjectToChoose::GRID);

        this->add(WidgetType::GRID_CHOOSE, "To");
        this->setObjectTypeToChoose("To", ObjectToChoose::GRID);

        this->add(WidgetType::TEXT_EDIT, "PtToDeform", "Points to transform");
        this->setTextEditEditable("PtToDeform", true);

        this->add(WidgetType::TEXT_EDIT, "Results", "Results");
        this->setTextEditEditable("Results", true);

        this->add(WidgetType::BUTTON, "Deform", "Transform");
        this->add(WidgetType::BUTTON, "Preview");

        this->add(WidgetType::TIFF_CHOOSE, "Save image");
        this->setFileChooserType("Save image", FileChooserType::SAVE);
    }

    void update(Scene * scene) {
        Form::update(scene);
    }

    void show() {
        Form::show();
    }

    void extractPointsFromText(std::vector<glm::vec3>& points) {
        QTextDocument * doc = this->textEdits["PtToDeform"]->document();
        int nbLine = doc->lineCount();
        for(int i = 0; i < nbLine; ++i) {
            QString line = doc->findBlockByLineNumber(i).text();
            line.replace(".", ",");
            QStringList toRemove{"[", "]", "(", ")"};
            for(const QString& stringToRemove : toRemove) {
                line.remove(stringToRemove);
            }
            QStringList separators{";", " ", "; "};
            for(const QString& separator : separators) {
                QStringList parsed = line.split(separator);
                if(parsed.length() == 3) {
                    glm::vec3 point;
                    point.x = std::atof(parsed[0].toStdString().c_str());
                    point.y = std::atof(parsed[1].toStdString().c_str());
                    point.z = std::atof(parsed[2].toStdString().c_str());
                    points.push_back(point);
                    break;
                }
            }
        }
    }

    void convertPoints(Scene * scene) {
        this->results.clear();
        this->origins.clear();
        this->extractPointsFromText(origins);
        QString result;
        for(auto& pt : this->origins) {
            glm::vec3 newPt = scene->getTransformedPoint(pt, this->objectChoosers["From"]->currentText().toStdString(), this->objectChoosers["To"]->currentText().toStdString());
            QString line;
            line += "[ ";
            line += std::to_string(newPt.x).c_str();
            line += " ";
            line += std::to_string(newPt.y).c_str();
            line += " ";
            line += std::to_string(newPt.z).c_str();
            line += " ]\n";
            result += line;
            this->results.push_back(newPt);
        }
        this->textEdits["Results"]->clear();
        this->textEdits["Results"]->setPlainText(result);
    }

    std::string getFromGridName() {
        return this->objectChoosers["From"]->currentText().toStdString();
    }

    std::string getToGridName() {
        return this->objectChoosers["To"]->currentText().toStdString();
    }

    void connect(Scene * scene) {
        QObject::connect(this->buttons["Deform"], &QPushButton::clicked, [this, scene](){this->convertPoints(scene);});
        QObject::connect(this->buttons["Preview"], &QPushButton::clicked, [this, scene](){
                scene->writeImageWithPoints("previewFrom.tiff", this->getFromGridName(), this->origins);
                scene->writeImageWithPoints("previewTo.tiff", this->getToGridName(), this->results);
        });
        QObject::connect(this->fileChoosers["Save image"], &FileChooser::fileSelected, [this, scene](){
            scene->writeDeformation(this->fileChoosers["Save image"]->filename.toStdString(), this->getFromGridName(), this->getToGridName());
        });
    }
};

class Raw3DImage {

public:
    int max;

    QImage::Format format;
    glm::ivec3 imgSize;
    std::vector<std::vector<uint16_t>> data;
    GridGLView * grid;

private:
    std::vector<bool> upToDate;
    std::vector<QImage> images;

public:
    Raw3DImage(const glm::ivec3 imgSize, QImage::Format format) {
        this->imgSize = imgSize;
        max = 0;
        this->format = format;
        for(int k = 0; k < imgSize[2]; ++k) {
            this->data.push_back(std::vector<uint16_t>(this->imgSize.x*this->imgSize.y, 0));
            this->images.push_back(QImage(this->imgSize.x, this->imgSize.y, format));
            this->images.back().fill(QColor(0, 0, 0));

            this->upToDate.push_back(false);
        }
    }

    void setSlice(const int& idx, const std::vector<uint16_t>& data) {
        this->data[idx] = data;
        this->updateMaxValue();
        this->upToDate[idx] = false;
    }

    void setImage(const std::vector<std::vector<uint16_t>>& data) {
        this->data = data;
        this->updateMaxValue();
        this->images.clear();
        this->upToDate.clear();
        for(int k = 0; k < imgSize[2]; ++k) {
            this->images.push_back(QImage(this->imgSize.x, this->imgSize.y, format));
            this->upToDate.push_back(false);
        }
    }

    QImage& getImage(const int& imageIdx) {
        if(!this->upToDate[imageIdx]) {
            this->convertDataToImg(imageIdx);
            this->upToDate[imageIdx] = true;
        }
        return images[imageIdx];
    }

private:
    void updateMaxValue() {
        max = 0;
        for(int i = 0; i < this->data.size(); ++i) {
            uint16_t currentMax = *max_element(data[i].begin(), data[i].end());
            if(currentMax > max)
                max = currentMax;
        }
    }

    void convertDataToImg(const int& imageIdx) {
        this->updateMaxValue();
        for(int i = 0; i < imgSize[0]; ++i) {
            for(int j = 0; j < imgSize[1]; ++j) {
                uint16_t value = data[imageIdx][i + j * imgSize[0]];
                int color = int((float(value) / float(max))*255.);
                QColor qColor;
                if(color < 0 || color > 255)
                    qColor = QColor(0, 0, 0);
                else
                    qColor = QColor(color, color, color);
                this->images[imageIdx].setPixelColor(i, j, qColor);
            }
        }
    }
};

class Image2DViewer : public QWidget {
    Q_OBJECT;

public:
    Scene * scene;
    bool activated;

    // Interactions variables
    bool inMoveMode;
    QPoint movementOrigin;
    int zoomSpeed;
    float zoom;
    int minimumSize;
    bool mirrorX;
    bool mirrorY;

    // Data
    QImage imageData;// Image data to be painted
    QSize dataImageSize;// Original image size
    QSize targetImageSize;// Optimal image size according to voxel size, etc

    QPoint paintedImageOrigin;
    QSize paintedImageSize;

    //Display variables
    QLabel * display;
    QHBoxLayout * layout;
    QImage::Format format;

    Image2DViewer(QImage::Format format, QWidget *parent = nullptr): QWidget(parent), format(format){init();}

    void init() {
        this->mirrorX = false;
        this->mirrorY = false;
        this->zoomSpeed = 30;
        this->zoom = 1;
        this->imageData = QImage(10., 10., format);
        this->imageData.fill(QColor(0., 0., 0.));
        this->targetImageSize = this->imageData.size();
        this->paintedImageOrigin = QPoint(0, 0);
        this->paintedImageSize = this->imageData.size();

        this->setMouseTracking(true);
        this->activated = false;
        this->layout = new QHBoxLayout();
        this->setLayout(this->layout);
        this->display = new QLabel();
        this->layout->addWidget(this->display, 1);
        this->layout->setContentsMargins(0, 0, 0, 0);
        this->display->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        this->minimumSize = 15;
    }

    void toggleShow() {
       if(this->isVisible()) {
           this->hide();
       } else {
           this->show();
       }
    }

    void show() {
        if(this->size().width() < minimumSize || this->size().height() < minimumSize) {
            this->activated = false;
        } else {
            this->activated = true;
            this->fitToWindow();
            this->draw();
        }
        QWidget::show();
    }

    void hide() {
       QWidget::hide();
       this->activated = false;
    }

    void clearColor() {
        this->imageData.fill(QColor(0, 0, 0));
    }

    void setImageSize(const QSize& dataImageSize, const QSize& targetImageSize, bool mirrorX, bool mirrorY) {
        this->mirrorX = mirrorX;
        this->mirrorY = mirrorY;
        this->dataImageSize = dataImageSize;
        this->targetImageSize = targetImageSize;
        this->zoomSpeed = std::max(this->targetImageSize.width(), this->targetImageSize.height())/10;

        this->paintedImageOrigin = QPoint(0, 0);
        this->paintedImageSize = targetImageSize;
        this->fitToWindow();
    }

    void updateImageData(const QImage& image) {
        if(image.size() == this->dataImageSize) {
            this->imageData = image;
        } else {
            //std::cout << "WARNING: trying to add an image in a 2D viewer with incorrect size ! Size expected: [" << this->originalImageSize <<"], size received: [" << image.size() << "]" << std::endl;
            std::cout << "WARNING: trying to add an image in a 2D viewer with incorrect size !" << std::endl;
        }
    }

    void draw() {
        if(!activated)
            return;
        QPixmap finalScreen(this->size().width(), this->size().height());
        finalScreen.fill(Qt::black);
        QPainter painter(&finalScreen);
        QRect target(paintedImageOrigin.x(), paintedImageOrigin.y(), this->paintedImageSize.width(), this->paintedImageSize.height());
        //painter.drawPixmap(target, QPixmap::fromImage(this->imageData.scaled(this->paintedImageSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
        painter.drawPixmap(target, QPixmap::fromImage(this->imageData.scaled(this->paintedImageSize, Qt::IgnoreAspectRatio, Qt::FastTransformation).mirrored(mirrorX, mirrorY)));
        this->display->setPixmap(finalScreen);
    }

    void resizeEvent(QResizeEvent *) {
        if(this->size().width() < minimumSize || this->size().height() < minimumSize) {
            this->activated = false;
        } else {
            this->activated = true;
            this->fitToWindow();
            this->draw();
        }
    }

    void fitToWindow() {
        if(!activated)
            return;
        float dx = std::max(float(minimumSize), float(this->size().width()))/float(this->targetImageSize.width());
        float dy = std::max(float(minimumSize), float(this->size().height()))/float(this->targetImageSize.height());
        float df = std::min(dx, dy);
        this->paintedImageSize = QSize(std::floor(float(this->targetImageSize.width()) * df), std::floor(float(this->targetImageSize.height()) * df));
        this->paintedImageOrigin.rx() = std::max(0., std::floor(float(this->size().width() - this->paintedImageSize.width())/2.));
        this->paintedImageOrigin.ry() = std::max(0., std::floor(float(this->size().height() - this->paintedImageSize.height())/2.));
    }

    void mouseMoveEvent(QMouseEvent* event) {
        //std::cout << event->pos().x() << std::endl;
        if(this->inMoveMode) {
            QPoint currentPosition = event->pos();
            this->paintedImageOrigin += (currentPosition - this->movementOrigin);
            this->movementOrigin = currentPosition;
            this->draw();
            event->setAccepted(true);
        }
        QRect target(paintedImageOrigin.x(), paintedImageOrigin.y(), this->paintedImageSize.width(), this->paintedImageSize.height());
        if(target.contains(event->pos())) {
            QPoint pointInPaintedImage = event->pos() - paintedImageOrigin;
            if(this->mirrorX)
                pointInPaintedImage.rx() = (paintedImageOrigin.x() + paintedImageSize.width()) - event->pos().x();
            if(this->mirrorY)
                pointInPaintedImage.ry() = (paintedImageOrigin.y() + paintedImageSize.height()) - event->pos().y();
            glm::vec2 ptInPaintedImage(pointInPaintedImage.x(), pointInPaintedImage.y());
            glm::vec2 targetSize = glm::vec2(dataImageSize.width(), dataImageSize.height());
            glm::vec2 srcSize = glm::vec2(paintedImageSize.width(), paintedImageSize.height());
            glm::ivec2 ptInImage = ptInPaintedImage * (targetSize / srcSize);
            Q_EMIT(mouseMovedIn2DPlanarViewer(ptInImage));
        }
    }

    void mousePressEvent(QMouseEvent* event) {
        std::cout << "Move mode" << std::endl;
        this->inMoveMode = true;
        this->movementOrigin = event->pos();
        Q_EMIT isSelected();
    }

    void mouseReleaseEvent(QMouseEvent* event) {
        std::cout << "Release mode false" << std::endl;
        this->inMoveMode = false;
    }

    void wheelEvent(QWheelEvent *event) {
        if(event->angleDelta().y() > 0) {
            //zoom += this->zoomSpeed;
            this->paintedImageSize.rwidth() += this->zoomSpeed;
            this->paintedImageSize.rheight() += this->zoomSpeed;
        } else if(event->angleDelta().y() < 0) {
            //zoom -= this->zoomSpeed;
            this->paintedImageSize.rwidth() -= this->zoomSpeed;
            this->paintedImageSize.rheight() -= this->zoomSpeed;
        }
        this->draw();
        event->setAccepted(false);
    }

signals:
    void isSelected();
    void mouseMovedIn2DPlanarViewer(const glm::ivec2& positionOfMouse2D);// Used for preview
};

class Image3DViewer : public QWidget {
    Q_OBJECT

public:

    QImage::Format imgFormat;
    QImage::Format mergedImgFormat;
    QString name;

    bool isInitialized;

    glm::vec3 direction;

    glm::ivec3 originalImgSize;// Usefull for getting back 3D point
    glm::ivec3 targetImgSize;
    std::vector<std::string> gridNames;
    std::vector<int> imagesToDraw;
    std::vector<int> alphaValues;
    std::vector<std::pair<QColor, QColor>> colors;
    Interpolation::Method interpolationMethod;
    int sliceIdx;

    Image2DViewer * viewer2D;

    std::vector<std::vector<bool>> upToDate;
    std::vector<Raw3DImage> imgData;

    Scene * scene;

    Image3DViewer(const QString& name, const glm::vec3& side, Scene * scene, QWidget * parent = nullptr): QWidget(parent), name(name), direction(side), scene(scene), isInitialized(false), viewer2D(nullptr), targetImgSize(glm::vec3(1., 1., 1.)) {initLayout(); connect(scene);}

    void init(const glm::vec3& originalImageSize, const glm::vec3& targetImageSize, const glm::vec3& optimalImageSize, const int& sliceIdx, const glm::vec3& side, std::vector<std::string> gridNames, std::vector<int> imgToDraw, std::vector<int> alphaValues, std::vector<std::pair<QColor, QColor>> colors, Interpolation::Method interpolationMethod, std::pair<bool, bool> mirror) {

        for(auto name : gridNames)
            if(name.empty())
                return;

        this->direction = side;

        this->targetImgSize = targetImageSize;
        this->originalImgSize = originalImageSize;

        this->gridNames = gridNames;
        this->imagesToDraw = imgToDraw;
        this->alphaValues = alphaValues;
        this->colors = colors;
        this->interpolationMethod = interpolationMethod;

        this->upToDate.clear();
        this->upToDate = std::vector<std::vector<bool>>(gridNames.size(), std::vector<bool>(this->targetImgSize.z, false));

        imgData.clear();
        imgData.reserve(gridNames.size());
        for(auto name : gridNames)
            imgData.push_back(Raw3DImage(this->targetImgSize, imgFormat));

        this->isInitialized = true;

        this->viewer2D->setImageSize(QSize(targetImageSize.x, targetImageSize.y), QSize(optimalImageSize.x, optimalImageSize.y), mirror.first, mirror.second);
        this->setSliceIdx(sliceIdx);

    }

    void setSliceIdx(int newSliceIdx) {
        this->sliceIdx = newSliceIdx;
        if(this->isInitialized) {
            this->drawImages();
        }
    }

    void saveImagesSlices(const QString& fileName) {
        this->fillAllImagesSlices();
        this->scene->writeGreyscaleTIFFImage(fileName.toStdString(), this->targetImgSize, this->imgData[this->imagesToDraw[0]].data);
        //for(int sliceIdx = 0; sliceIdx < this->upToDate[0].size(); ++sliceIdx) {
        //    QString fileName = path + QString("/slice") + QString(std::to_string(sliceIdx).c_str()) + QString(".png");
        //    this->getMergedImage(sliceIdx).save(fileName);
        //}
    }

private:

    void reset() {
        for(auto& line : this->upToDate)
            std::fill(line.begin(), line.end(), false);
    }

    void initLayout() {
        imgFormat = QImage::Format_RGB16;
        mergedImgFormat = QImage::Format_ARGB32;
        this->viewer2D = new Image2DViewer(mergedImgFormat, this);
    }

    void fillCurrentImages() {
        for(int i = 0; i < this->imagesToDraw.size(); ++i) {
            if(!this->upToDate[this->imagesToDraw[i]][this->sliceIdx]) {
                this->fillImage(this->imagesToDraw[i], this->sliceIdx);
                this->upToDate[this->imagesToDraw[i]][this->sliceIdx] = true;
            }
        }
    }

    void fillAllImagesSlices() {
        for(int sliceIdx = 0; sliceIdx < this->upToDate[0].size(); ++sliceIdx) {
            std::cout << "Fill image " << sliceIdx << std::endl;
            for(int i = 0; i < this->imagesToDraw.size(); ++i) {
                if(!this->upToDate[this->imagesToDraw[i]][sliceIdx]) {
                    this->fillImage(this->imagesToDraw[i], sliceIdx);
                    this->upToDate[this->imagesToDraw[i]][sliceIdx] = true;
                }
            }
        }
    }

    void fillImage(int imageIdx, int sliceIdx) {
        std::vector<uint16_t> data;
        auto bbox = scene->getBbox(this->gridNames[0]);
        glm::vec3 slices(-1, -1, sliceIdx);
        if(this->direction == glm::vec3(1., 0., 0.))
            std::swap(slices.x, slices.z);
        if(this->direction == glm::vec3(0., 1., 0.))
            std::swap(slices.y, slices.z);
        if(this->direction == glm::vec3(0., 0., 1.))
            std::swap(slices.z, slices.z);
        scene->getValues(this->gridNames[imageIdx], slices, bbox, this->targetImgSize, data, this->interpolationMethod);
        this->imgData[imageIdx].setSlice(sliceIdx, data);
    }

    void getColor(int idx, glm::ivec3 position, QColor& color) {
        color = this->imgData[idx].getImage(position.z).pixelColor(position.x, position.y);
    }

    QImage getCurrentMergedImage() {
        return this->mergeImages(this->imagesToDraw, this->sliceIdx);
    }

    QImage getMergedImage(int sliceIdx) {
        return this->mergeImages(this->imagesToDraw, sliceIdx);
    }

    QImage mergeImages(const std::vector<int>& indexes, const int& z) {
        QColor color;
        QPixmap result(this->targetImgSize.x, this->targetImgSize.y);
        //result.fill(Qt::black);
        result.fill(this->colors[indexes[0]].first);
        QPainter painter(&result);

        for(int k = 0; k < indexes.size(); ++k) {
            int idx = indexes[k];
            QImage img = this->imgData[idx].getImage(z).convertToFormat(mergedImgFormat);
            for(int i = 0; i < img.width(); ++i) {
                for(int j = 0; j < img.height(); ++j) {
                    QColor color = img.pixelColor(i, j);
                    if(color == Qt::black && idx > 0) {
                        color.setAlpha(0);
                    } else {
                        float value = float(color.red())/255.;
                        color = QColor(this->colors[idx].first.red()*(1.-value)+this->colors[idx].second.red()*value,
                                       this->colors[idx].first.green()*(1.-value)+this->colors[idx].second.green()*value,
                                       this->colors[idx].first.blue()*(1.-value)+this->colors[idx].second.blue()*value);
                        color.setAlpha(alphaValues[idx]);
                    }
                    img.setPixelColor(i, j, color);
                }
            }
            painter.drawPixmap(QPoint(0, 0), QPixmap::fromImage(img));
        }

        return result.toImage().convertToFormat(mergedImgFormat);
    }

    void drawImages() {
        if(this->viewer2D->activated) {
            this->fillCurrentImages();
            this->viewer2D->updateImageData(this->getCurrentMergedImage());
            this->viewer2D->draw();
        } else {
            std::cout << "Viewer deactivated !!" << std::endl;
        }
    }

    void convertVector(glm::vec3& vec) {
        if(this->direction == glm::vec3(1., 0., 0.))
            std::swap(vec.x, vec.z);
        if(this->direction == glm::vec3(0., 1., 0.))
            std::swap(vec.y, vec.z);
        if(this->direction == glm::vec3(0., 0., 1.))
            std::swap(vec.z, vec.z);
    }

    void mouseMovedIn2DViewer(const glm::ivec2& positionOfMouse2D) {
        std::cout << positionOfMouse2D << std::endl;
        glm::vec3 positionOfMouse3D(positionOfMouse2D.x, positionOfMouse2D.y, sliceIdx);
        glm::vec3 convertedTargetImgSize = this->targetImgSize;
        glm::vec3 convertedOriginalImgSize = this->originalImgSize;
        convertVector(convertedOriginalImgSize);
        convertVector(convertedTargetImgSize);
        convertVector(positionOfMouse3D);
        glm::vec3 fromTargetToOriginal = convertedOriginalImgSize / glm::vec3(convertedTargetImgSize);
        positionOfMouse3D *= fromTargetToOriginal;
        this->scene->grids[this->scene->getGridIdx(this->gridNames[0])]->grid->fromImageToWorld(positionOfMouse3D);
        Q_EMIT(mouseMovedInPlanarViewer(positionOfMouse3D));
    }

    void connect(Scene * scene) {
        QObject::connect(scene, &Scene::meshMoved, [this, scene](){
            this->reset();
            this->drawImages();
        });

        QObject::connect(this->viewer2D, &Image2DViewer::isSelected, this, &Image3DViewer::isSelected);
        QObject::connect(this->viewer2D, &Image2DViewer::mouseMovedIn2DPlanarViewer, this, &Image3DViewer::mouseMovedIn2DViewer);
        QObject::connect(this, &Image3DViewer::mouseMovedInPlanarViewer, scene, &Scene::previewPointInPlanarView);
        QObject::connect(this, &Image3DViewer::mouseMovedInPlanarViewer, scene, &Scene::pointIsClickedInPlanarViewer);
    }
signals:
    void isSelected();
    void mouseMovedInPlanarViewer(const glm::vec3& positionOfMouse3D);
};

class RangeOptionUnit : public QFrame {
    Q_OBJECT

public:
    int id;
    QPushButton * hideButton;
    QPushButton * hideColor;
    QPushButton * deleteButton;
    QPushButton * open;
    QPushButton * save;
    QPushButton * autoButton;
    QPushButton * hideOption;
    RangeOptionUnit(QWidget * parent = nullptr) : QFrame(parent)  {
        id = -1;
        this->setFrameShape(QFrame::Shape::StyledPanel);
        QVBoxLayout *unitLayout = new QVBoxLayout(this);
        //unitLayout->setAlignment(Qt::AlignTop);
        unitLayout->setAlignment(Qt::AlignVCenter);

        deleteButton = new QPushButton();
        deleteButton->setIcon(QIcon(QPixmap("../resources/cross.svg")));
        deleteButton->setIconSize(QPixmap("../resources/cross.svg").rect().size());
        deleteButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        QIcon icon;
        QSize size(80, 80);
        icon.addFile(QString("../resources/visible.svg"), size, QIcon::Normal, QIcon::Off);
        icon.addFile(QString("../resources/hidden.svg"), size, QIcon::Normal, QIcon::On);
        hideButton = new QPushButton();
        hideButton->setCheckable(true);
        hideButton->setIcon(icon);
        hideButton->setIconSize(QPixmap("../resources/visible.svg").rect().size());
        hideButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        hideColor = new QPushButton();
        QIcon icon2;
        icon2.addFile(QString("../resources/color.svg"), size, QIcon::Normal, QIcon::Off);
        icon2.addFile(QString("../resources/color_hide.svg"), size, QIcon::Normal, QIcon::On);
        hideColor = new QPushButton();
        hideColor->setCheckable(true);
        hideColor->setIcon(icon2);
        hideColor->setIconSize(QPixmap("../resources/color.svg").rect().size());
        hideColor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        open = new QPushButton();
        open->setIcon(QIcon(QPixmap("../resources/open.svg")));
        open->setIconSize(QPixmap("../resources/open.svg").rect().size());
        open->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        save = new QPushButton();
        save->setIcon(QIcon(QPixmap("../resources/save.svg")));
        save->setIconSize(QPixmap("../resources/save.svg").rect().size());
        save->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        autoButton = new QPushButton();
        autoButton->setIcon(QIcon(QPixmap("../resources/histo.svg")));
        autoButton->setIconSize(QPixmap("../resources/histo.svg").rect().size());
        autoButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        hideOption = new QPushButton();
        hideOption->setIcon(QIcon(QPixmap("../resources/arap.svg")));
        hideOption->setIconSize(QPixmap("../resources/arap.svg").rect().size());
        hideOption->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        hideOption->setCheckable(true);

        unitLayout->addStretch();
        unitLayout->addWidget(open);
        unitLayout->addWidget(save);
        unitLayout->addWidget(deleteButton);
        unitLayout->addWidget(hideButton);
        unitLayout->addWidget(hideColor);
        unitLayout->addWidget(autoButton);
        unitLayout->addWidget(hideOption);
        unitLayout->addStretch();

        QObject::connect(deleteButton, &QPushButton::clicked, [this](){Q_EMIT deleteCurrent(this->id);});
    }

signals:
    void deleteCurrent(int id);
};

class RangeUnit : public QFrame {
    Q_OBJECT

public:
    int id;
    QDoubleSpinBox * min;

    bool moreOptions;
    QDoubleSpinBox * max;
    QPushButton * left;
    QPushButton * right;
    QPushButton * deleteButton;

    QPushButton * hideButton;
    QCheckBox * hideColor;
    ColorButton * colorButton;
    RangeUnit(int id, QWidget * parent = nullptr) : id(id), QFrame(parent)  {
        this->setFrameShape(QFrame::Shape::StyledPanel);
        QVBoxLayout *unitLayout = new QVBoxLayout(this);
        unitLayout->setAlignment(Qt::AlignVCenter);

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist(0, 255);
        colorButton = new ColorButton(QColor(dist(rng), dist(rng), dist(rng)));

        hideColor = new QCheckBox();
        hideColor->setChecked(true);

        QWidget * colorWidget = new QWidget();
        QHBoxLayout * colorLayout = new QHBoxLayout(this);
        colorWidget->setLayout(colorLayout);

        colorLayout->addWidget(hideColor);
        colorLayout->addWidget(colorButton);
        colorLayout->setAlignment(colorButton, Qt::AlignHCenter);
        colorLayout->setContentsMargins(0, 0, 0, 0);
        colorLayout->setSpacing(0);
        colorWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        unitLayout->addWidget(colorWidget);

        unitLayout->setAlignment(colorButton, Qt::AlignHCenter);

        this->min = new QDoubleSpinBox();
        this->min->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        this->min->setMaximum(16000);
        this->min->setDecimals(0);
        this->min->setValue(0);

        this->max = new QDoubleSpinBox();
        this->max->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        this->max->setMaximum(16000);
        this->max->setDecimals(0);
        this->max->setValue(1);

        QHBoxLayout * moveLayout = new QHBoxLayout();
        left = new QPushButton();
        right = new QPushButton();
        left->setIcon(QIcon(QPixmap("../resources/left.svg")));
        left->setIconSize(QPixmap("../resources/left.svg").rect().size());
        left->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        right->setIcon(QIcon(QPixmap("../resources/right.svg")));
        right->setIconSize(QPixmap("../resources/right.svg").rect().size());
        right->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        moveLayout->addWidget(left);
        moveLayout->addWidget(right);

        deleteButton = new QPushButton();
        deleteButton->setIcon(QIcon(QPixmap("../resources/cross.svg")));
        deleteButton->setIconSize(QPixmap("../resources/cross.svg").rect().size());
        deleteButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        QIcon icon;
        QSize size(30, 30);
        icon.addFile(QString("../resources/visible.svg"), size, QIcon::Normal, QIcon::Off);
        icon.addFile(QString("../resources/hidden.svg"), size, QIcon::Normal, QIcon::On);
        hideButton = new QPushButton();
        hideButton->setCheckable(true);
        hideButton->setIcon(icon);
        hideButton->setIconSize(QPixmap("../resources/visible.svg").rect().size());
        hideButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        hideButton->setFixedSize(QSize(30, 25));

        colorLayout->addWidget(hideButton);

        unitLayout->addWidget(min);
        unitLayout->addWidget(max);
        unitLayout->addLayout(moveLayout);
        unitLayout->addWidget(deleteButton);
        //unitLayout->addWidget(hideButton);

        QObject::connect(left, &QPushButton::clicked, [this](){Q_EMIT leftMove(this->id);});
        QObject::connect(right, &QPushButton::clicked, [this](){Q_EMIT rightMove(this->id);});
        QObject::connect(deleteButton, &QPushButton::clicked, [this](){Q_EMIT deleteCurrent(this->id);});
        QObject::connect(colorButton->button, &QPushButton::clicked, [this](){Q_EMIT colorChanged(this->id);});
        QObject::connect(hideColor, &QCheckBox::clicked, [this](){Q_EMIT colorChanged(this->id);});
        QObject::connect(hideColor, &QCheckBox::stateChanged, [this](){this->colorButton->setDisabled(!this->hideColor->isChecked());});
        QObject::connect(min, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double d){
            if(!moreOptions || min->value() > max->value()) {
                this->max->blockSignals(true);
                this->max->setValue(min->value());
                this->max->blockSignals(false);
            }
            Q_EMIT rangeChanged(this->id);
        });
        QObject::connect(max, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double d){Q_EMIT rangeChanged(this->id);});
        QObject::connect(hideButton, &QPushButton::clicked, [this](){Q_EMIT rangeChanged(this->id);});
        QObject::connect(hideButton, &QPushButton::toggled, [this](){this->hideColor->setDisabled(this->hideButton->isChecked());
                                                                     this->colorButton->setDisabled(this->hideButton->isChecked());});
        moreOptions = false;
        this->setMoreOptions(this->moreOptions);
    }

    glm::vec3 getColor() {
        return glm::vec3(float(colorButton->getColor().red())/255., float(colorButton->getColor().green())/255., float(colorButton->getColor().blue())/255.);
    }

    void setMoreOptions(bool value) {
        this->moreOptions = value;
        if(value) {
           max->show();
           left->show();
           right->show();
           deleteButton->show();
        } else {
           max->hide();
           left->hide();
           right->hide();
           deleteButton->hide();
        }
    }

signals:
    void leftMove(int id);
    void rightMove(int id);
    void deleteCurrent(int id);
    void colorChanged(int id);
    void rangeChanged(int id);
};

class RangeControl : public QScrollArea {
    Q_OBJECT

public:
    Scene * scene;

    int maxNbUnits;

    RangeOptionUnit * rangeOptionUnit;

    QWidget * mainWidget;
    QHBoxLayout * mainLayout;
    QHBoxLayout * unitLayout;
    QHBoxLayout * addButtonLayout;
    QPushButton * buttonAdd;
    std::vector<RangeUnit*> units;
    bool initialized;

    RangeControl(Scene * scene, QWidget *parent = nullptr) : QScrollArea(parent), scene(scene) {init(); connect();}

    void init() {
        maxNbUnits = 500;

        this->units.reserve(maxNbUnits);
        this->buttonAdd = new QPushButton(QString("+"));
        this->buttonAdd->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        this->addButtonLayout = new QHBoxLayout();
        this->addButtonLayout->addWidget(this->buttonAdd);

        this->unitLayout = new QHBoxLayout();
        this->rangeOptionUnit = new RangeOptionUnit();
        //this->unitLayout->addWidget(this->rangeOptionUnit);
        this->addUnit();
        units.back()->blockSignals(true);
        units.back()->min->setValue(0);
        units.back()->max->setValue(16000);
        units.back()->colorButton->setColor(QColor(255., 255., 255.));
        units.back()->blockSignals(false);

        this->mainLayout = new QHBoxLayout();
        this->mainLayout->setAlignment(Qt::AlignLeft);
        this->mainLayout->addLayout(unitLayout);
        this->mainLayout->addLayout(addButtonLayout);

        this->mainWidget = new QWidget();
        this->mainWidget->setLayout(this->mainLayout);

        this->setWidget(this->mainWidget);
        this->setWidgetResizable(true);

        QObject::connect(rangeOptionUnit->hideColor, &QPushButton::clicked, this, [this](){this->toggleAll(Option::COLOR, !rangeOptionUnit->hideColor->isChecked());});
        QObject::connect(rangeOptionUnit->hideButton, &QPushButton::clicked, this, [this](){this->toggleAll(Option::VISU, rangeOptionUnit->hideButton->isChecked());});
        QObject::connect(rangeOptionUnit->deleteButton, &QPushButton::clicked, this, [this](){this->clearUnits(true); this->updateRanges();});
        QObject::connect(rangeOptionUnit->open, &QPushButton::clicked, this, [this](){this->readFromFile(); this->updateRanges();});
        QObject::connect(rangeOptionUnit->save, &QPushButton::clicked, this, [this](){this->writeToFile(); this->updateRanges();});
        QObject::connect(rangeOptionUnit->autoButton, &QPushButton::clicked, this, [this](){this->addUnitsAuto();});
        QObject::connect(rangeOptionUnit->hideOption, &QPushButton::clicked, this, [this](){this->toggleAll(Option::MORE_OPTIONS, rangeOptionUnit->hideOption->isChecked());});

        this->initialized = true;
    }

    void readFromFile() {
        QFile file;
        QString filename = QFileDialog::getOpenFileName(nullptr, "Open color map", QDir::currentPath(), "Json files (*.json)", 0, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty())
            return;

        this->clearUnits();
        file.setFileName(filename);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString values;
        values = file.readAll();
        file.close();

        QJsonDocument document = QJsonDocument::fromJson(values.toUtf8());
        QJsonObject json = document.object();

        for(int i = 0; i < json.count(); ++i) {
            const QString rangeId(std::to_string(i).c_str());
            QJsonObject values = json.value(rangeId).toObject();
            QJsonArray colorArray = values.value("color").toArray();
            glm::ivec3 color(0, 0, 0);
            for(int i = 0; i < 3; ++i)
                color[i] = std::stoi(colorArray.at(i).toString().toStdString());
            int min = std::stoi(values.value("min").toString().toStdString());
            int max = std::stoi(values.value("max").toString().toStdString());

            bool isDisplayed = std::stoi(values.value("display").toString().toStdString());
            this->addUnit(min, max, color, isDisplayed);
        }
    }

    void writeToFile() {
        QJsonObject obj;

        for(int i = 0; i < this->unitLayout->count(); ++i) {
            RangeUnit * unit = dynamic_cast<RangeUnit*>(this->unitLayout->itemAt(i)->widget());
            QJsonObject unitJson;

            QJsonArray color;
            color.insert(0, std::to_string(unit->colorButton->getColor().red()).c_str());
            color.insert(1, std::to_string(unit->colorButton->getColor().green()).c_str());
            color.insert(2, std::to_string(unit->colorButton->getColor().blue()).c_str());

            unitJson.insert("color", color);
            unitJson.insert("min", std::to_string(std::floor(unit->min->value())).c_str());
            unitJson.insert("max", std::to_string(std::floor(unit->max->value())).c_str());
            int isDisplayed = 0;
            if(!unit->hideButton->isChecked())
                isDisplayed = 1;
            unitJson.insert("display", std::to_string(isDisplayed).c_str());

            obj.insert(std::to_string(i).c_str(), unitJson);
        }

        QString filename = QFileDialog::getSaveFileName(nullptr, "Open color map", QDir::currentPath(), "Json files (*.json)", 0, QFileDialog::DontUseNativeDialog);
        QFile file;
        file.setFileName(filename);
        file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
        file.write(QJsonDocument(obj).toJson());
        file.close();
    }

    void clearUnits(bool addDefault = false) {
        for(int i = this->unitLayout->count()-1; i >= 0; --i) {
            this->deleteUnit(i);
        }
        if(addDefault) {
            this->addUnit(0, 16000);
            this->units.back()->hideColor->click();
        }
    }

    int findId(int id) {
       for(int i = 0; i < this->unitLayout->count(); ++i) {
           if(dynamic_cast<RangeUnit*>(this->unitLayout->itemAt(i)->widget())->id == id)
               return i;
       }
       return -1;
    }

    enum Direction {
        Left,
        Right
    };

    void deleteUnit(int id) {
        int idx = findId(id);
        this->unitLayout->removeWidget(this->unitLayout->itemAt(idx)->widget());
        delete this->units[id];
        this->units.erase(this->units.begin()+id);
        for(int i = id; i < this->units.size(); ++i)
            this->units[i]->id -= 1;
    }

    void moveUnit(int id, Direction direction) {
        int idx = this->findId(id);
        if(idx == -1 || (idx == 0 && direction == Direction::Left) || (idx == this->unitLayout->count()-1 && direction == Direction::Right))
            return;
        QWidget * widget = this->unitLayout->takeAt(idx)->widget();
        if(direction == Direction::Left)
            this->unitLayout->insertWidget(idx-1, widget);
        else
            this->unitLayout->insertWidget(idx+1, widget);
    }

    void addUnit(int min, int max) {
        this->addUnit();
        this->units.back()->min->setValue(min);
        this->units.back()->max->setValue(max);
    }

    void addUnit(int min, int max, glm::ivec3 color, bool display) {
        this->addUnit();
        this->units.back()->min->setValue(min);
        this->units.back()->max->setValue(max);
        this->units.back()->colorButton->setColor(QColor(color.r, color.g, color.b));
        if(!display)
            this->units.back()->hideButton->setChecked(true);
    }

    void addUnit() {
        this->units.push_back(new RangeUnit(this->units.size()));
        this->unitLayout->addWidget(units.back());
        QObject::connect(units.back(), &RangeUnit::leftMove, this, [this](int id){this->moveUnit(id, Direction::Left); this->updateRanges();});
        QObject::connect(units.back(), &RangeUnit::rightMove, this, [this](int id){this->moveUnit(id, Direction::Right); this->updateRanges();});
        QObject::connect(units.back(), &RangeUnit::deleteCurrent, this, [this](int id){this->deleteUnit(id); this->updateRanges();});
        QObject::connect(units.back(), &RangeUnit::rangeChanged, this, [this](int id){this->updateRanges();});
        QObject::connect(units.back(), &RangeUnit::colorChanged, this, [this](int id){this->updateRanges();});
        //QObject::connect(this->buttonAdd, &QPushButton::clicked, [this, scene](){this->updateRanges(scene);});
    }

    void updateRanges() {
        scene->resetRanges();
        for(int i = 0; i < this->unitLayout->count(); ++i) {
            RangeUnit * unit = dynamic_cast<RangeUnit*>(this->unitLayout->itemAt(i)->widget());
            if(!unit->hideButton->isChecked()) {
                if(unit->hideColor->isChecked()) {
                    scene->addRange(unit->min->value(), unit->max->value(), unit->getColor());
                } else {
                    scene->addRange(unit->min->value(), unit->max->value(), glm::vec3(1., 1., 1.));
                }
            }
        }
    }

    enum class Option {
        COLOR,
        VISU,
        MORE_OPTIONS
    };

    void toggleAll(Option option, bool value) {
        for(auto& unit : this->units) {
            switch(option) {
                case Option::COLOR:
                    unit->hideColor->setChecked(value);
                    break;
                case Option::VISU:
                    unit->hideButton->setChecked(value);
                    break;
                case Option::MORE_OPTIONS:
                    unit->setMoreOptions(value);
                    break;
            }
        }
        this->updateRanges();
    }

    void addUnitsAuto() {
        this->clearUnits();
        auto minMax = scene->getGridMinMaxValues();
        std::vector<bool> usage = scene->getGridUsageValues();
        int nbUnits = minMax.second - minMax.first;
        if(nbUnits < maxNbUnits) {
            for(int i = 1; i < usage.size(); ++i) {
                if(usage[i])
                    this->addUnit(i, i);
            }
        }
        this->updateRanges();
    }

    void connect() {
        QObject::connect(this->buttonAdd, &QPushButton::clicked, [this](){this->addUnit();});
    }
};

class PlanarViewForm : public Form {
    Q_OBJECT

public:
    Scene * scene;

    //Image3DViewer * imageViewer;

    std::map<QString, std::vector<std::pair<bool, bool>>> viewersMirror;   // Store viewers res    for each side for easier usage
    std::map<QString, std::vector<glm::vec3>> viewersRes;   // Store viewers res    for each side for easier usage
    std::map<QString, glm::ivec3> viewersValues;// Store viewers values for each side for easier usage
    std::map<QString, Image3DViewer*> viewers;
    QString selectedViewer;
    PlanarViewForm(Scene * scene, QWidget *parent = nullptr):Form(parent), scene(scene){init(scene);connect(scene);}

public slots:

    void addViewer(const QString& name, const glm::vec3& side = glm::vec3(0., 0., 1.)) {
        if(name.isEmpty())
            return;
        this->viewers[name] = new Image3DViewer(name, side, this->scene);
        QObject::connect(this->viewers[name], &Image3DViewer::isSelected, [=](){this->selectViewer(name);});

        this->viewersValues[name] = glm::ivec3(0, 0, 0);
        this->viewersRes[name] = {glm::vec3(1, 1, 1), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1)};
        this->viewersMirror[name] = {{false, false}, {false, false}, {false, false}};

        this->selectedViewer = name;
        this->labels["SelectedViewer"]->setText(name);

        glm::vec3 imgSize = this->getBackImgDimension(scene);
        if(side.x == 1.)
            std::swap(imgSize.x, imgSize.z);
        if(side.y == 1.)
            std::swap(imgSize.y, imgSize.z);
        this->setSpinBoxesValues(imgSize);

        this->updateImageViewer();
    }

    void selectViewer(const QString& name) {
        this->storeCurrentValues();
        this->selectedViewer = name;
        this->labels["SelectedViewer"]->setText(name);
        this->updateDefaultValues(name);
        this->recoverValues();
        this->updateSlice();
    }

    void updateDefaultValues(const QString& name) {
        const Image3DViewer * viewer = this->viewers[name];

        this->setSpinBoxesValues(viewer->targetImgSize);

        this->comboBoxes["Interpolation"]->blockSignals(true);
        int idx = this->comboBoxes["Interpolation"]->findText(Interpolation::toString(viewer->interpolationMethod).c_str());
        this->comboBoxes["Interpolation"]->setCurrentIndex(idx);
        this->comboBoxes["Interpolation"]->blockSignals(false);

        this->objectChoosers["From"]->blockSignals(true);
        idx = 0;
        if(viewer->gridNames.size() > 0) {
            this->spinBoxes["AlphaBack"]->blockSignals(true);
            this->spinBoxes["AlphaBack"]->setValue(viewer->alphaValues[0]);
            this->spinBoxes["AlphaBack"]->blockSignals(false);

            idx = this->objectChoosers["From"]->findText(viewer->gridNames[0].c_str());
        }
        this->objectChoosers["From"]->setCurrentIndex(idx);
        this->objectChoosers["From"]->blockSignals(false);

        this->objectChoosers["To"]->blockSignals(true);
        idx = 0;
        if(viewer->gridNames.size() > 1) {
            this->spinBoxes["AlphaFront"]->blockSignals(true);
            this->spinBoxes["AlphaFront"]->setValue(viewer->alphaValues[1]);
            this->spinBoxes["AlphaFront"]->blockSignals(false);

            idx = this->objectChoosers["To"]->findText(viewer->gridNames[1].c_str());
        }
        this->objectChoosers["To"]->setCurrentIndex(idx);
        this->objectChoosers["To"]->blockSignals(false);

        this->checkBoxes["UseBack"]->blockSignals(true);
        this->checkBoxes["UseFront"]->blockSignals(true);

        std::vector imgsToDraw = viewer->imagesToDraw;
        this->checkBoxes["UseBack"]->setChecked(std::find(imgsToDraw.begin(), imgsToDraw.end(), 0) != imgsToDraw.end());
        this->checkBoxes["UseFront"]->setChecked(std::find(imgsToDraw.begin(), imgsToDraw.end(), 1) != imgsToDraw.end());

        this->checkBoxes["UseBack"]->blockSignals(false);
        this->checkBoxes["UseFront"]->blockSignals(false);

        this->sliders["SliderX"]->blockSignals(true);
        this->sliders["SliderX"]->setValue(viewer->sliceIdx);
        this->sliders["SliderX"]->setMaximum(viewer->targetImgSize.z-1);
        this->sliders["SliderX"]->blockSignals(false);

        this->blockSignalsInGroup("GroupSide", true);
        if(viewer->direction == glm::vec3(1., 0., 0.)) {
            this->buttons["SideX"]->setChecked(true);
            this->buttons["SideY"]->setChecked(false);
            this->buttons["SideZ"]->setChecked(false);
        }
        if(viewer->direction == glm::vec3(0., 1., 0.)) {
            this->buttons["SideX"]->setChecked(false);
            this->buttons["SideY"]->setChecked(true);
            this->buttons["SideZ"]->setChecked(false);
        }
        if(viewer->direction == glm::vec3(0., 0., 1.)) {
            this->buttons["SideX"]->setChecked(false);
            this->buttons["SideY"]->setChecked(false);
            this->buttons["SideZ"]->setChecked(true);
        }
        this->blockSignalsInGroup("GroupSide", false);

    }

    void init(Scene * scene) {

        this->add(WidgetType::H_GROUP, "GroupHeader");
        this->addAllNextWidgetsToGroup("GroupHeader");

        this->add(WidgetType::LABEL, "SelectedViewer", "NONE");
        //this->add(WidgetType::BUTTON, "Rotate");
        //QPixmap pixmap(QString("../resources/rotate.svg"));
        //QIcon ButtonIcon(pixmap);
        //this->buttons["Rotate"]->setIcon(ButtonIcon);
        //this->buttons["Rotate"]->setText("");
        //this->buttons["Rotate"]->setIconSize(pixmap.rect().size());

        this->add(WidgetType::TIFF_SAVE, "Save");
        this->add(WidgetType::TIFF_SAVE, "SaveCur");

        this->add(WidgetType::BUTTON_CHECKABLE, "Link");
        QPixmap pixmap2(QString("../resources/link.svg"));
        QIcon ButtonIcon2(pixmap2);
        this->buttons["Link"]->setIcon(ButtonIcon2);
        this->buttons["Link"]->setText("");
        this->buttons["Link"]->setIconSize(pixmap2.rect().size());

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::SLIDER, "SliderX", "X");
        this->sliders["SliderX"]->setMinimum(0);
        this->sliders["SliderX"]->setMaximum(0);
        this->labels["SliderX"]->setFixedWidth(50);

        this->addWithLabel(WidgetType::H_GROUP, "GroupBack", "Back");
        this->addAllNextWidgetsToGroup("GroupBack");
        this->groups["GroupBack"]->setAlignment(Qt::AlignHCenter);

        this->add(WidgetType::GRID_CHOOSE, "From", "Back");
        this->setObjectTypeToChoose("From", ObjectToChoose::GRID);
        this->objectChoosers["From"]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        this->add(WidgetType::CHECK_BOX, "UseBack");
        this->add(WidgetType::SPIN_BOX, "AlphaBack");
        this->spinBoxes["AlphaBack"]->setSingleStep(20);

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::H_GROUP, "GroupFront", "Front");
        this->addAllNextWidgetsToGroup("GroupFront");
        this->groups["GroupFront"]->setAlignment(Qt::AlignHCenter);

        this->add(WidgetType::GRID_CHOOSE, "To", "Front");
        this->setObjectTypeToChoose("To", ObjectToChoose::GRID);
        this->objectChoosers["To"]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        this->add(WidgetType::CHECK_BOX, "UseFront");
        this->add(WidgetType::SPIN_BOX, "AlphaFront");
        this->spinBoxes["AlphaFront"]->setSingleStep(20);

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::COMBO_BOX, "Interpolation", "Interpolation");
        this->setComboChoices("Interpolation", Interpolation::toStringList());

        this->addWithLabel(WidgetType::H_GROUP, "GroupResolution", "Resolution");
        this->addAllNextWidgetsToGroup("GroupResolution");

        this->add(WidgetType::SPIN_BOX, "X");
        this->add(WidgetType::SPIN_BOX, "Y");
        this->add(WidgetType::SPIN_BOX, "Z");
        this->add(WidgetType::BUTTON, "Auto");

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::H_GROUP, "GroupSide", "Side");
        this->addAllNextWidgetsToGroup("GroupSide");

        this->add(WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE, "SideX", "X");
        this->add(WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE, "SideY", "Y");
        this->add(WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE, "SideZ", "Z");

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::H_GROUP, "GroupMirror", "Mirror");
        this->addAllNextWidgetsToGroup("GroupMirror");

        this->add(WidgetType::BUTTON_CHECKABLE, "MirrorX", "X");
        this->add(WidgetType::BUTTON_CHECKABLE, "MirrorY", "Y");

        this->addAllNextWidgetsToDefaultGroup();

        /****/

        this->checkBoxes["UseBack"]->setChecked(true);
        this->checkBoxes["UseFront"]->setChecked(true);

        this->spinBoxes["AlphaBack"]->setMinimum(0);
        this->spinBoxes["AlphaBack"]->setMaximum(255);
        this->spinBoxes["AlphaBack"]->setValue(255);
        this->spinBoxes["AlphaFront"]->setMinimum(0);
        this->spinBoxes["AlphaFront"]->setMaximum(255);
        this->spinBoxes["AlphaFront"]->setValue(255);

        this->setDisabled(true);
        /****/

        //this->addViewer("ViewZ");
    }

    void initViewer(const QString& name) {
        this->setDisabled(false);
        this->show();
        this->selectViewer(name);
        this->checkBoxes["UseBack"]->blockSignals(true);
        this->checkBoxes["UseFront"]->blockSignals(true);
        this->checkBoxes["UseBack"]->setChecked(true);
        this->checkBoxes["UseFront"]->setChecked(true);
        this->checkBoxes["UseBack"]->blockSignals(false);
        this->checkBoxes["UseFront"]->blockSignals(false);
        this->viewers[name]->viewer2D->activated = true;
        this->buttons["Link"]->setChecked(false);
        this->buttons["SideX"]->setChecked(false);
        this->buttons["SideY"]->setChecked(false);
        this->buttons["SideZ"]->setChecked(false);

        this->buttons["Link"]->click();
        if(this->getSide().x == 1.)
            this->buttons["SideX"]->click();
        if(this->getSide().y == 1.)
            this->buttons["SideY"]->click();
        if(this->getSide().z == 1.)
            this->buttons["SideZ"]->click();
    }

    glm::vec3 getBackImgDimension(Scene * scene) {
        glm::vec3 defaultValue = glm::vec3(1., 1., 1.);
        std::string name = this->getFromGridName();
        if(name == "")
            return defaultValue;
        return scene->grids[scene->getGridIdx(name)]->grid->getResolution();
    }

    void backImageChanged(Scene * scene) {
        glm::vec3 imgSize = this->getBackImgDimension(scene);
        if(this->getSide().x == 1.)
            std::swap(imgSize.x, imgSize.z);
        if(this->getSide().y == 1.)
            std::swap(imgSize.y, imgSize.z);
        this->setSpinBoxesValues(imgSize);
    }

    glm::ivec3 autoComputeBestSize(Scene * scene) {
        if(this->getFromGridName().empty() || this->getToGridName().empty())
            return glm::ivec3(1, 1, 1);
        glm::vec3 gridResolution = scene->getGridImgSize(this->getFromGridName());
        glm::vec3 voxelSize = scene->getGridVoxelSize(this->getFromGridName());
        float maxSize = std::min(voxelSize.x, std::min(voxelSize.y, voxelSize.z));
        glm::ivec3 finalSize(0., 0., 0.);
        for(int i = 0; i < 3; ++i) {
            float ratio = voxelSize[i]/maxSize;
            finalSize[i] = std::floor(ratio*gridResolution[i]);
        }
        std::cout << "Auto compute size: " << finalSize << std::endl;
        return finalSize;
    }

    glm::vec3 getSide() {
        if(this->noViewerSelected())
            return glm::vec3(0., 0., 1.);
        return this->viewers[this->selectedViewer]->direction;
    }

    void setAutoImageResolution() {
        glm::vec3 dim = this->autoComputeBestSize(this->scene);
        convertVector(dim);
        this->setSpinBoxesValues(dim);
        this->updateImageViewer();
    }

    void setSpinBoxesValues(const glm::vec3& values) {
        this->spinBoxes["X"]->blockSignals(true);
        this->spinBoxes["Y"]->blockSignals(true);
        this->spinBoxes["Z"]->blockSignals(true);
        this->spinBoxes["X"]->setMinimum(1);
        this->spinBoxes["Y"]->setMinimum(1);
        this->spinBoxes["Z"]->setMinimum(1);
        this->spinBoxes["X"]->setValue(values.x);
        this->spinBoxes["Y"]->setValue(values.y);
        this->spinBoxes["Z"]->setValue(values.z);
        this->spinBoxes["X"]->blockSignals(false);
        this->spinBoxes["Y"]->blockSignals(false);
        this->spinBoxes["Z"]->blockSignals(false);
    }

    bool noViewerSelected() {
        return !this->viewers[this->selectedViewer];
    }

    void convertVector(glm::vec3& vec) {
        if(this->getSide() == glm::vec3(1., 0., 0.))
            std::swap(vec.x, vec.z);
        if(this->getSide() == glm::vec3(0., 1., 0.))
            std::swap(vec.y, vec.z);
        if(this->getSide() == glm::vec3(0., 0., 1.))
            std::swap(vec.z, vec.z);
    }

    void updateImageViewer() {
        if(this->noViewerSelected())
            return;
        this->sliders["SliderX"]->setMinimum(0);
        this->sliders["SliderX"]->setMaximum(this->getImgDimension().z-1);
        glm::vec3 finalImageSize = autoComputeBestSize(scene);
        convertVector(finalImageSize);
        glm::vec3 originalImgDimension = this->getBackImgDimension(scene);
        convertVector(originalImgDimension);
        this->viewers[this->selectedViewer]->init(
                    originalImgDimension,
                    this->getImgDimension(),
                    finalImageSize, this->sliders["SliderX"]->value(),
                    this->getSide(),
                    {this->getFromGridName(), this->getToGridName()},
                    this->getImagesToDraw(),
                    {this->spinBoxes["AlphaBack"]->value(), this->spinBoxes["AlphaFront"]->value()},
                    {std::make_pair(
                        QColor(255.*this->scene->color0_second.x, 255.*this->scene->color0_second.y, 255.*this->scene->color0_second.z),
                        QColor(255.*this->scene->color1_second.x, 255.*this->scene->color1_second.y, 255.*this->scene->color1_second.z)),
                     std::make_pair(
                        QColor(255.*this->scene->color0.x, 255.*this->scene->color0.y, 255.*this->scene->color0.z),
                        QColor(255.*this->scene->color1.x, 255.*this->scene->color1.y, 255.*this->scene->color1.z))
                    },
                    this->getInterpolationMethod(),
                    {this->buttons["MirrorX"]->isChecked(), this->buttons["MirrorY"]->isChecked()});
    }

    void update(Scene * scene) {
        Form::update(scene);
    }

    void show() {
        Form::show();
    }

    glm::ivec3 getImgDimension() {
        glm::ivec3 value = glm::ivec3(this->spinBoxes["X"]->value(), this->spinBoxes["Y"]->value(), this->spinBoxes["Z"]->value());
        if(value.x > 100000 || value.y > 100000 || value.z > 100000)
            value = glm::ivec3(10, 10, 10);
        return value;
    }

    Interpolation::Method getInterpolationMethod() {
        QString method(this->comboBoxes["Interpolation"]->currentText());
        return Interpolation::fromString(method.toStdString());
    }

    std::vector<int> getImagesToDraw() {
        std::vector<int> imagesToDraw;
        if(this->checkBoxes["UseBack"]->isChecked())
            imagesToDraw.push_back(0);
        if(this->checkBoxes["UseFront"]->isChecked())
            imagesToDraw.push_back(1);
        return imagesToDraw;
    }

    std::string getFromGridName() {
        return this->objectChoosers["From"]->currentText().toStdString();
    }

    std::string getToGridName() {
        return this->objectChoosers["To"]->currentText().toStdString();
    }

    void storeCurrentValues() {
        if(this->noViewerSelected())
            return;
        glm::vec3 side = this->getSide();
        int value = this->sliders["SliderX"]->value();
        glm::vec3 res = this->getImgDimension();
        std::pair<bool, bool> mirror = {this->buttons["MirrorX"]->isChecked(), this->buttons["MirrorY"]->isChecked()};
        if(side.x > 0) {
            this->viewersValues[this->selectedViewer].x = value;
            this->viewersRes[this->selectedViewer][0] = res;
            this->viewersMirror[this->selectedViewer][0] = mirror;
        }

        if(side.y > 0) {
            this->viewersValues[this->selectedViewer].y = value;
            this->viewersRes[this->selectedViewer][1] = res;
            this->viewersMirror[this->selectedViewer][1] = mirror;
        }

        if(side.z > 0) {
            this->viewersValues[this->selectedViewer].z = value;
            this->viewersRes[this->selectedViewer][2] = res;
            this->viewersMirror[this->selectedViewer][2] = mirror;
        }
        std::cout << "Store value: " << value << std::endl;
    }

    void recoverValues() {
        if(this->noViewerSelected())
            return;
        glm::vec3 side = this->getSide();
        int value = 0;
        glm::vec3 res = glm::vec3(1, 1, 1);
        std::pair<bool, bool> mirror = {false, false};
        if(side.x > 0) {
            value = this->viewersValues[this->selectedViewer].x;
            res = this->viewersRes[this->selectedViewer][0];
            mirror = this->viewersMirror[this->selectedViewer][0];
        }

        if(side.y > 0) {
            value = this->viewersValues[this->selectedViewer].y;
            res = this->viewersRes[this->selectedViewer][1];
            mirror = this->viewersMirror[this->selectedViewer][1];
        }

        if(side.z > 0) {
            value = this->viewersValues[this->selectedViewer].z;
            res = this->viewersRes[this->selectedViewer][2];
            mirror = this->viewersMirror[this->selectedViewer][2];
        }
        if(res.x > 1 && res.y > 1 && res.z > 1)
            this->setSpinBoxesValues(res);
        this->sliders["SliderX"]->blockSignals(true);
        this->sliders["SliderX"]->setMinimum(0);
        this->sliders["SliderX"]->setMaximum(this->getImgDimension().z-1);
        this->sliders["SliderX"]->setValue(value);
        this->sliders["SliderX"]->blockSignals(false);
        this->buttons["MirrorX"]->blockSignals(true);
        this->buttons["MirrorX"]->setChecked(mirror.first);
        this->buttons["MirrorX"]->blockSignals(false);
        this->buttons["MirrorY"]->blockSignals(true);
        this->buttons["MirrorY"]->setChecked(mirror.second);
        this->buttons["MirrorY"]->blockSignals(false);
    }

    void updateSlice() {
        this->viewers[this->selectedViewer]->setSliceIdx(this->sliders["SliderX"]->value());
        if(this->buttons["Link"]->isChecked()) {
            if(this->viewers[this->selectedViewer]->direction == glm::vec3(1., 0., 0.)) {
                this->scene->slotSetPlaneDisplacementX(float(this->sliders["SliderX"]->value())/float(this->sliders["SliderX"]->maximum()));
                this->scene->slotSetPlaneDisplacementY(0.);
                this->scene->slotSetPlaneDisplacementZ(0.);
            }
            if(this->viewers[this->selectedViewer]->direction == glm::vec3(0., 1., 0.)) {
                this->scene->slotSetPlaneDisplacementY(float(this->sliders["SliderX"]->value())/float(this->sliders["SliderX"]->maximum()));
                this->scene->slotSetPlaneDisplacementZ(0.);
                this->scene->slotSetPlaneDisplacementX(0.);
            }
            if(this->viewers[this->selectedViewer]->direction == glm::vec3(0., 0., 1.)) {
                this->scene->slotSetPlaneDisplacementZ(float(this->sliders["SliderX"]->value())/float(this->sliders["SliderX"]->maximum()));
                this->scene->slotSetPlaneDisplacementX(0.);
                this->scene->slotSetPlaneDisplacementY(0.);
            }
            this->scene->updatePlaneControlWidget();
        }
        this->labels["SliderX"]->setText(std::to_string(this->sliders["SliderX"]->value()).c_str());
    }

    void connect(Scene * scene) {
        QObject::connect(this, &Form::widgetModified, [this, scene](const QString &id){
            if(this->noViewerSelected())
                return;
            if(id == "From")
                this->backImageChanged(scene);

            if(id == "Auto")
                this->setAutoImageResolution();

            if(id == "X" || id == "Y" || id == "Z" || id == "Interpolation" || id == "UseBack" || id == "UseFront" || id == "From" || id == "To" || id == "AlphaBack" || id == "AlphaFront" || "MirrorX" || "MirrorY")
                this->updateImageViewer();

            if(id == "SideX" || id == "SideY" || id == "SideZ") {
                this->storeCurrentValues();
                if(id == "SideX")
                    this->viewers[this->selectedViewer]->direction = glm::vec3(1., 0., 0.);
                if(id == "SideY")
                    this->viewers[this->selectedViewer]->direction = glm::vec3(0., 1., 0.);
                if(id == "SideZ")
                    this->viewers[this->selectedViewer]->direction = glm::vec3(0., 0., 1.);
                this->backImageChanged(scene);
                this->recoverValues();
                this->updateImageViewer();
            }

            if(id == "SliderX" || id == "SideX" || id == "SideY" || id == "SideZ" || id == "Link") {
                this->updateSlice();
            }
        });

        QObject::connect(this->fileChoosers["Save"], &FileChooser::fileSelected, [this](){
            //this->viewers[this->selectedViewer]->saveImagesSlices(this->fileChoosers["Save"]->filename);
            //this->scene->writeMapping(this->fileChoosers["Save"]->filename.toStdString(), this->getFromGridName(), this->getToGridName());
            //this->scene->sampleGridMapping(this->fileChoosers["Save"]->filename.toStdString(), this->getFromGridName(), this->getToGridName(), this->getImgDimension(), this->getInterpolationMethod());
            this->scene->sampleGridMapping(this->fileChoosers["Save"]->filename.toStdString(), this->getFromGridName(), this->getToGridName(), this->getImgDimension(), this->getInterpolationMethod());
        });

        QObject::connect(this->fileChoosers["SaveCur"], &FileChooser::fileSelected, [this](){
            this->viewers[this->selectedViewer]->saveImagesSlices(this->fileChoosers["SaveCur"]->filename);
        });

        QObject::connect(scene, &Scene::colorChanged, [this, scene](){
            if(this->noViewerSelected())
                return;
            updateImageViewer();
        });
    }
};

class PlanarViewer2D : public PlanarViewForm {
    Q_OBJECT

public:
    bool initialized;
    PlanarViewer2D(Scene * scene, QWidget *parent = nullptr):PlanarViewForm(scene, parent){
        this->initialized = false;
    }

    void initialize(Scene * scene) {
        this->addViewer("View_1", glm::vec3(1., 0., 0.));
        this->addViewer("View_2", glm::vec3(0., 1., 0.));
        this->addViewer("View_3", glm::vec3(0., 0., 1.));
        this->hide();
        this->initialized = true;
    }
};


class SaveImageForm : Form {
    Q_OBJECT

public:

    Image3DViewer * imageViewer;

    SaveImageForm(Scene * scene, QWidget *parent = nullptr):Form(parent){init(scene);connect(scene);}

public slots:

    void init(Scene * scene) {
        this->imageViewer = new Image3DViewer("PreviewX", glm::vec3(0., 0., 1.), scene);
        this->layout->addRow(imageViewer);

        this->addWithLabel(WidgetType::H_GROUP, "GroupBack", "Back");
        this->addAllNextWidgetsToGroup("GroupBack");
        this->groups["GroupBack"]->setAlignment(Qt::AlignHCenter);

        this->add(WidgetType::GRID_CHOOSE, "From", "Back");
        this->setObjectTypeToChoose("From", ObjectToChoose::GRID);
        this->objectChoosers["From"]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        this->add(WidgetType::CHECK_BOX, "UseBack");

        this->add(WidgetType::TIFF_SAVE, "Save image back", "Save");
        this->setFileChooserType("Save image back", FileChooserType::SAVE);
        this->fileChoosers["Save image back"]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::H_GROUP, "GroupFront", "Front");
        this->addAllNextWidgetsToGroup("GroupFront");
        this->groups["GroupFront"]->setAlignment(Qt::AlignHCenter);

        this->add(WidgetType::GRID_CHOOSE, "To", "Front");
        this->setObjectTypeToChoose("To", ObjectToChoose::GRID);
        this->objectChoosers["To"]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        this->add(WidgetType::CHECK_BOX, "UseFront");

        this->add(WidgetType::TIFF_SAVE, "Save image front", "Save");
        this->setFileChooserType("Save image front", FileChooserType::SAVE);
        this->fileChoosers["Save image front"]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        this->addAllNextWidgetsToDefaultGroup();

        this->add(WidgetType::COMBO_BOX, "Interpolation", "Interpolation");
        this->setComboChoices("Interpolation", Interpolation::toStringList());

        this->addWithLabel(WidgetType::H_GROUP, "GroupResolution", "Resolution");
        this->addAllNextWidgetsToGroup("GroupResolution");

        this->add(WidgetType::SPIN_BOX, "X");
        this->add(WidgetType::SPIN_BOX, "Y");
        this->add(WidgetType::SPIN_BOX, "Z");

        this->addAllNextWidgetsToDefaultGroup();

        this->add(WidgetType::BUTTON, "Preview");

        this->add(WidgetType::TIFF_SAVE, "Save image");
        this->setFileChooserType("Save image", FileChooserType::SAVE);

        /****/

        this->checkBoxes["UseBack"]->setChecked(true);
        this->checkBoxes["UseFront"]->setChecked(true);
    }

    void show() {
        Form::show();
    }

    void update(Scene * scene) {
        Form::update(scene);
    }

    void connect(Scene * scene) {
        QObject::connect(this->fileChoosers["Save image"], &FileChooser::fileSelected, [this, scene](){
        });

        QObject::connect(this->fileChoosers["Save image back"], &FileChooser::fileSelected, [this, scene](){
            scene->writeGreyscaleTIFFImage(this->fileChoosers["Save image back"]->filename.toStdString(), this->imageViewer->targetImgSize, this->imageViewer->imgData[0].data);
        });

        QObject::connect(this->fileChoosers["Save image front"], &FileChooser::fileSelected, [this, scene](){
            scene->writeGreyscaleTIFFImage(this->fileChoosers["Save image front"]->filename.toStdString(), this->imageViewer->targetImgSize, this->imageViewer->imgData[1].data);
        });

        QObject::connect(this->buttons["Preview"], &QPushButton::clicked, [this, scene](){
            this->imageViewer->show();
        });

        QObject::connect(this->objectChoosers["From"], QOverload<int>::of(&QComboBox::currentIndexChanged), [this, scene](int index){
        });
    }
};

class OpenImageForm : public Form {
    Q_OBJECT

public:

    bool useTetMesh;
    OpenImageForm(Scene * scene, QWidget *parent = nullptr):Form(parent){init();connect(scene);}

public slots:

    void init() {
        this->useTetMesh = false;
        //this->addFileChooser("Save image", FileChooserType::SAVE);

        this->addWithLabel(WidgetType::LINE_EDIT, "Name", "Name");

        this->add(WidgetType::SECTION, "Image");
        this->addAllNextWidgetsToSection("Image");

        /***/

        this->add(WidgetType::SECTION_CHECKABLE, "Image subsample");
        this->addAllNextWidgetsToSection("Image subsample");

        this->addWithLabel(WidgetType::SPIN_BOX, "Subsample", "Subsample");

        this->addAllNextWidgetsToSection("Image");

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

        this->add(WidgetType::SECTION, "Type");
        this->addAllNextWidgetsToSection("Type");
        this->addWithLabel(WidgetType::CHECK_BOX, "Segmented", "Segmented");

        this->addAllNextWidgetsToSection("Image");

        this->add(WidgetType::FILENAME, "Image filename");
        this->add(WidgetType::TIFF_CHOOSE, "Image choose", "Select image file");
        this->linkFileNameToFileChooser("Image filename", "Image choose");

        /***/

        this->addAllNextWidgetsToDefaultGroup();
        this->addAllNextWidgetsToDefaultSection();

        this->add(WidgetType::SECTION, "Mesh");
        this->addAllNextWidgetsToSection("Mesh");

        this->add(WidgetType::SECTION, "Tetrahedral mesh size");
        this->addAllNextWidgetsToSection("Tetrahedral mesh size");

        this->addWithLabel(WidgetType::H_GROUP, "GroupNbTet", "Nb tetrahedra");
        this->addAllNextWidgetsToGroup("GroupNbTet");

        this->add(WidgetType::SPIN_BOX, "NbTetX");
        this->add(WidgetType::SPIN_BOX, "NbTetY");
        this->add(WidgetType::SPIN_BOX, "NbTetZ");

        this->addAllNextWidgetsToSection("Mesh");

        this->add(WidgetType::SECTION, "Voxel size");
        this->addAllNextWidgetsToSection("Voxel size");

        this->addWithLabel(WidgetType::H_GROUP, "GroupVoxelSize", "Size");
        this->addAllNextWidgetsToGroup("GroupVoxelSize");

        this->add(WidgetType::SPIN_BOX_DOUBLE, "SizeVoxelX");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "SizeVoxelY");
        this->add(WidgetType::SPIN_BOX_DOUBLE, "SizeVoxelZ");

        this->addAllNextWidgetsToSection("Mesh");

        this->add(WidgetType::FILENAME, "Mesh filename");
        this->add(WidgetType::MESH_CHOOSE, "Mesh choose", "Select mesh file");
        this->linkFileNameToFileChooser("Mesh filename", "Mesh choose");

        /***/

        this->addAllNextWidgetsToDefaultGroup();
        this->addAllNextWidgetsToDefaultSection();

        this->add(WidgetType::SECTION, "Cage");
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

        this->sections["Image subsample"].first->hide();
        this->sections["Image subregion"].first->hide();
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

        this->sections["Image subsample"].first->setChecked(false);
        this->sections["Image subregion"].first->setChecked(false);

        this->spinBoxes["Subsample"]->setValue(1);
        this->spinBoxes["Subsample"]->setMinimum(1);

        this->spinBoxes["NbTetX"]->setValue(5);
        this->spinBoxes["NbTetX"]->setMinimum(1);
        this->spinBoxes["NbTetY"]->setValue(5);
        this->spinBoxes["NbTetY"]->setMinimum(1);
        this->spinBoxes["NbTetZ"]->setValue(5);
        this->spinBoxes["NbTetZ"]->setMinimum(1);

        this->doubleSpinBoxes["SizeVoxelX"]->setValue(1);
        this->doubleSpinBoxes["SizeVoxelX"]->setMinimum(0);
        this->doubleSpinBoxes["SizeVoxelY"]->setValue(1);
        this->doubleSpinBoxes["SizeVoxelY"]->setMinimum(0);
        this->doubleSpinBoxes["SizeVoxelZ"]->setValue(1);
        this->doubleSpinBoxes["SizeVoxelZ"]->setMinimum(0);

        this->sections["Image subregion"].first->setEnabled(false);
        this->sections["Image subsample"].first->setEnabled(false);

        this->sections["Mesh"].first->setEnabled(false);
        this->sections["Cage"].first->setEnabled(false);

        this->buttons["Load"]->setEnabled(false);
    }

    void update(Scene * scene) {
        Form::update(scene);
    }

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

    std::string getName() {
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

    glm::vec3 getSizeVoxel() {
        return glm::vec3(this->doubleSpinBoxes["SizeVoxelX"]->value(),
                         this->doubleSpinBoxes["SizeVoxelY"]->value(),
                         this->doubleSpinBoxes["SizeVoxelZ"]->value());
    }

    glm::vec3 getSizeTetmesh() {
        return glm::vec3(this->spinBoxes["NbTetX"]->value(),
                         this->spinBoxes["NbTetY"]->value(),
                         this->spinBoxes["NbTetZ"]->value());
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

    void connect(Scene * scene) {
        QObject::connect(this->fileChoosers["Image choose"], &FileChooser::fileSelected, [this](){
                this->buttons["Load"]->setEnabled(true);
                //this->sections["Image subregion"].first->setEnabled(true);// This functionnality isn't available yet
                this->sections["Image subsample"].first->setEnabled(true);
                this->sections["Mesh"].first->setEnabled(true);

                this->prefillFields({this->fileChoosers["Image choose"]->filename.toStdString()});
        });

        QObject::connect(this->fileChoosers["Mesh choose"], &FileChooser::fileSelected, [this](){
                this->sections["Tetrahedral mesh size"].first->setEnabled(false);
                this->useTetMesh = true;
                this->sections["Cage"].first->setEnabled(true);
        });

        QObject::connect(this->buttons["Load"], &QPushButton::clicked, [this, scene](){
                if(this->useTetMesh) {
                    scene->openGrid(this->getName(), this->getImgFilenames(), this->getSubsample(), this->getTetmeshFilename());
                } else {
                    scene->openGrid(this->getName(), this->getImgFilenames(), this->getSubsample(), this->getSizeVoxel(), this->getSizeTetmesh());
                }
                if(!this->fileChoosers["Cage choose"]->filename.isEmpty()) {
                    scene->openCage(this->getName() + "_cage", this->fileChoosers["Cage choose"]->filename.toStdString(), this->getName(), this->checkBoxes["mvc"]->isChecked());
                }
                this->hide();
                Q_EMIT loaded();
        });
    }
public:
signals:
    void loaded();
};

class InfoPannel : public QGroupBox {
    Q_OBJECT

public:
    
    InfoPannel(Scene * scene, QWidget *parent = nullptr):QGroupBox(parent){init();connect(scene);}
    InfoPannel(const QString &title, Scene * scene, QWidget *parent = nullptr): QGroupBox(title, parent){init();connect(scene);}

    QVBoxLayout * main_layout;
    QHBoxLayout * id_layout;
    QLabel      * info_id;
    QLabel      * info_position;
    QHBoxLayout * position_layout;
    QLabel      * info_id_data;
    QLabel      * info_position_data;

public slots:

    void init(){
        this->setCheckable(false);
        this->main_layout = new QVBoxLayout();
        this->main_layout->setAlignment(Qt::AlignTop);
        this->info_id = new QLabel("Id:");
        this->info_position = new QLabel("Pos:");

        this->info_id_data = new QLabel("-");
        this->info_position_data = new QLabel("[]");
        //this->info_position_data->setStyleSheet("font: 9pt;");

        this->id_layout = new QHBoxLayout();
        this->id_layout->addWidget(this->info_id);
        this->id_layout->addWidget(this->info_id_data);
        this->position_layout = new QHBoxLayout();
        this->position_layout->addWidget(this->info_position);
        this->position_layout->addWidget(this->info_position_data);

        this->id_layout->setAlignment(this->info_id_data, Qt::AlignHCenter);
        this->position_layout->setAlignment(this->info_position_data, Qt::AlignHCenter);

        this->main_layout->addLayout(this->id_layout);
        this->main_layout->addLayout(this->position_layout);
        this->setLayout(this->main_layout);
    }

    void connect(Scene * scene) {
        QObject::connect(scene, &Scene::selectedPointChanged, this, &InfoPannel::updatePointInfo);
    }

    void updatePointInfo(std::pair<int, glm::vec3> selectedPoint) {
        std::string idx = std::to_string(selectedPoint.first);
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << "[" << selectedPoint.second[0] << ", " << selectedPoint.second[1] << ", " << selectedPoint.second[2] << "]";
        std::string pt = stream.str();

        if(selectedPoint.first >= 0) {
            this->info_id_data->setText(QString(idx.c_str()));

            this->info_position_data->setToolTip(QString(pt.c_str()));
            this->info_position_data->setFixedWidth(175);
            //this->info_position_data->setText(QString(pt.c_str()));

            QFontMetrics metrics(this->info_position_data->font());
            QString elidedText = metrics.elidedText(QString(pt.c_str()), Qt::ElideRight, this->info_position_data->width());
            this->info_position_data->setText(elidedText);

        } else {
            //this->info_id_data->setText(QString("-"));
            //this->info_position_data->setText(QString("[]"));
        }
    }
};

class QActionManager : QWidget {
    Q_OBJECT
public:
    std::map<std::string, QToolButton *> menus;

    std::map<std::string, QAction *> actions;
    std::map<std::string, QActionGroup *> actionExclusiveGroups;

    std::map<std::string, QStringList> actionGroups;

    QAction * getAction(const QString& name) {
        return actions[name.toStdString()];
    }

    QToolButton * getMenu(const QString& name) {
        return menus[name.toStdString()];
    }

    void activateGroup(const QString& name) {
        this->showGroup(name);
        this->enableGroup(name);
    }

    void deactivateGroup(const QString& name) {
        this->hideGroup(name);
        this->disableGroup(name);
    }

    void hideGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setVisible(false);
        }
    }

    void showGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setVisible(true);
        }
    }

    void disableGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setEnabled(false);
        }
    }

    void enableGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setEnabled(true);
        }
    }

    void createQActionGroup(const QString& name, const QStringList& actionNames, bool exclusive = false) {
        this->actionGroups[name.toStdString()] = actionNames;
    }

    void createQExclusiveActionGroup(const QString& name, const QStringList& actionNames) {
        this->actionExclusiveGroups[name.toStdString()] = new QActionGroup(this);
        this->actionExclusiveGroups[name.toStdString()]->setExclusive(true);
        for(int i = 0; i < actionNames.size(); ++i) {
            if(this->actions.count(actionNames.at(i).toStdString())) {
                this->actionExclusiveGroups[name.toStdString()]->addAction(actions[actionNames.at(i).toStdString()]);
            } else {
                std::cout << "ERROR: the action [" << actionNames.at(i).toStdString() << "] doesn't exist !" << std::endl;
                throw std::runtime_error("[ERROR] wrong action name in the function createQActionGroup.");
            }
        }
    }

    QAction * createQAction(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon, bool checkable, bool checked) {
        QIcon icon;
        QSize size(80, 80);
        if(!defaultIcon.isEmpty())
            icon.addFile(QString("../resources/" + defaultIcon + QString(".svg")), size, QIcon::Normal, QIcon::Off);
        if(!pressedIcon.isEmpty())
            icon.addFile(QString("../resources/" + pressedIcon + QString(".svg")), size, QIcon::Normal, QIcon::On);
    
        QAction * action = new QAction(icon, text);
        if(checkable)
            action->setCheckable(true);
        action->setStatusTip(statusTip + QString(" - ") + keySequence);
        action->setToolTip(statusTip + QString(" - ") + keySequence);
    
        action->setShortcut(QKeySequence(keySequence));
        action->setIconVisibleInMenu(true);

        actions[name.toStdString()] = action;

        if(checked) {
            action->setChecked(true);
        }
    
        return action;
    }

    void createMenuButton(const QString& name, const QString& text, const QString& statusTip, const QString& defaultIcon, const QStringList& actions) {
        QIcon icon;
        QSize size(80, 80);
        if(!defaultIcon.isEmpty())
            icon.addFile(QString("../resources/" + defaultIcon + QString(".svg")), size, QIcon::Normal, QIcon::Off);

        QToolButton * button=new QToolButton(this);
        button->setStatusTip(statusTip);
        button->setToolTip(statusTip);
        button->setIcon(icon);
        this->menus[name.toStdString()] = button;
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        button->setText(text);
        button->setPopupMode(QToolButton::InstantPopup);

        QMenu *menu=new QMenu(button);
        for(auto& actionName : actions) {
            if(actionName == QString("-"))
                menu->addSeparator();
            else
                menu->addAction(this->actions[actionName.toStdString()]);
        }
        button->setMenu(menu);
    }

    QAction * createQActionButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip) {
        return createQAction(name, text, keySequence, statusTip, QString(), QString(), false, false);
    }

    QAction * createQActionButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, QString(), false, false);
    }

    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon, bool checked) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, pressedIcon, true, checked);
    }
    
    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, pressedIcon, true, false);
    }

    QAction * createQActionToggledButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, pressedIcon, true, true);
    }
    
    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon) {
        return createQActionToggleButton(name, text, keySequence, statusTip, defaultIcon, QString());
    }
    
    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip) {
        return createQActionToggleButton(name, text, keySequence, statusTip, QString(), QString());
    }

    QAction * createQActionToggledButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon) {
        return createQActionToggledButton(name, text, keySequence, statusTip, defaultIcon, QString());
    }
    
    QAction * createQActionToggledButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip) {
        return createQActionToggledButton(name, text, keySequence, statusTip, QString(), QString());
    }
};

class DisplayPannel : public QGroupBox {
    Q_OBJECT
public:
    DisplayPannel(const QString &title, QActionManager& actionManager, QWidget *parent = nullptr): QGroupBox(title, parent){init();connect(actionManager);}

    QVBoxLayout * mainLayout;
    QToolBar * toolBar;

public slots:
    void init() {
        this->mainLayout = new QVBoxLayout(this);
        this->mainLayout->setAlignment(Qt::AlignHCenter);

        this->toolBar = new QToolBar(this);
        this->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        this->mainLayout->addWidget(this->toolBar);
    };

    void connect(QActionManager& actionManager) {
        toolBar->addAction(actionManager.getAction("ToggleDisplayMesh"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayGrid"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayMultiView"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayWireframe"));
    };
};

class ToolPannel : public QGroupBox {
    Q_OBJECT

public:
    
    ToolPannel(QWidget *parent = nullptr):QGroupBox(parent){init();}
    ToolPannel(const QString &title, QActionManager& actionManager, QWidget *parent = nullptr): QGroupBox(title, parent){init();connect(actionManager);}

    QVBoxLayout * main_layout;

    QToolBar * toolbar;

    QToolBar * move_toolbar;
    QToolBar * arap_toolbar;
    QToolBar * fixedRegistration_toolbar;

public slots:
    void init(){
        this->main_layout = new QVBoxLayout(this);
        //this->main_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        this->toolbar = new QToolBar(this);
        this->toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        this->main_layout->addWidget(this->toolbar);
        this->main_layout->setAlignment(Qt::AlignHCenter);
        this->main_layout->addStretch();
    };

    void connect(QActionManager& actionManager) {
        // Spacer
        this->toolbar->addAction(actionManager.getAction("MoveTool_toggleEvenMode"));
        this->toolbar->addAction(actionManager.getAction("MoveTool_toggleMoveCage"));
        this->toolbar->addAction(actionManager.getAction("MoveTool_reset"));

        this->toolbar->addAction(actionManager.getAction("ARAPTool_moveMode"));
        this->toolbar->addAction(actionManager.getAction("ARAPTool_handleMode"));
        this->toolbar->addAction(actionManager.getAction("ARAPTool_toggleEvenMode"));
        
        this->toolbar->addAction(actionManager.getAction("FixedTool_apply"));
        this->toolbar->addAction(actionManager.getAction("FixedTool_clear"));
    };
};

class QuickSaveCage {
    FileChooser * fileChooser;
    Scene * scene;
    QString filePath;

public:
    QuickSaveCage(Scene * scene): scene(scene) {
        fileChooser = new FileChooser("file", FileChooserType::SAVE, FileChooserFormat::OFF);
    }

    void save() {
        if(filePath.isEmpty()) {
            this->fileChooser->click();
            filePath = this->fileChooser->filename;
            if(filePath.isEmpty())
                return;
        }

        bool saved = scene->saveActiveCage(filePath.toStdString());
        if(!saved)
            QMessageBox::critical(fileChooser, "Warning", "Selected mesh is not a cage, can't save.");
    }

    void saveAs() {
        this->fileChooser->click();
        filePath = this->fileChooser->filename;
        if(filePath.isEmpty())
            return;
        this->save();
    }
};

class MainWidget : public QMainWindow {
	Q_OBJECT
public:
	MainWidget();
	virtual ~MainWidget();
	Viewer* getViewer3D() const { return this->viewer; }

protected:
	void setupWidgets();
    void setupActions();
    void setupForms();
    void updateForms();
	/// @brief Allow to run code on any widget event
	/// @details In this case, set the minimum width and height of widgets in order to
	/// have them both square, and not too small.
	bool eventFilter(QObject* obj, QEvent* e) override;

private:
	Scene* scene;

    QActionManager* actionManager;

    QFrame* viewerFrame;
    QWidget* viewerCapsule;

    QSplitter* hSplit;
    QSplitter* vSplit1;
    QSplitter* vSplit2;
    QDockWidget * dockView_X;

	Viewer* viewer;

	GridLoaderWidget* loaderWidget;
	GridDeformationWidget* deformationWidget;

    RangeControl * range;
	ControlPanel* controlPanel;
	bool widgetSizeSet;

	QMenu* fileMenu;
    QMenu* editMenu;
    QMenu* windowsMenu;
    QMenu* otherMenu;

    QToolBar * toolbar;
	QAction* action_addGrid;
	QAction* action_saveGrid;
	QAction* action_exitProgram;
	QAction* action_showPlanarViewers;
	QAction* action_loadMesh;
	QAction* action_saveMesh;
	QAction* action_applyCage;
	QAction* action_openDevPannel;

    QComboBox* combo_mesh;

    CutPlaneGroupBox* cutPlane_pannel;
    DisplayPannel* display_pannel;
    InfoPannel* info_pannel;
    ToolPannel* tool_pannel;

	QAction* tool_open;
	QAction* tool_save;

	QAction* tool_none;
	QAction* tool_position;
	QAction* tool_direct;
	QAction* tool_ARAP;
	QAction* tool_registration;

	QStatusBar* statusBar;

    OpenMeshWidget * openMeshWidget;
    SaveMeshWidget * saveMeshWidget;
    ApplyCageWidget * applyCageWidget;

    DeformationForm * deformationForm;
    SaveImageForm * saveImageForm;
    QuickSaveCage * quickSaveCage;
    OpenImageForm * openImageForm;
    PlanarViewer2D * planarViewer;

    bool isShiftPressed = false;

public slots:
    void addNewMesh(const std::string& name, bool grid, bool cage) {
        this->combo_mesh->insertItem(this->combo_mesh->count(), QString(name.c_str()));
        if(this->scene->hasTwoOrMoreGrids()) {
            this->actionManager->getAction("ToggleDisplayMultiView")->setDisabled(false);
            this->actionManager->getAction("Transform")->setDisabled(false);
            this->actionManager->getAction("Boundaries")->setVisible(true);
            this->cutPlane_pannel->setDisabledAlpha(false);
        }
    }

    // *************** //
    // Connected to UI //
    // *************** //

    void changeCurrentTool(UITool::MeshManipulatorType newTool) {
        this->actionManager->deactivateGroup("MoveTool");
        this->actionManager->deactivateGroup("ARAPTool");
        this->actionManager->deactivateGroup("FixedTool");
        switch(newTool) {
            case UITool::MeshManipulatorType::POSITION:
                this->actionManager->activateGroup("MoveTool");
                break;
            case UITool::MeshManipulatorType::DIRECT:
                break;
            case UITool::MeshManipulatorType::ARAP:
                this->actionManager->activateGroup("ARAPTool");
                break;
            case UITool::MeshManipulatorType::FIXED_REGISTRATION:
                this->actionManager->activateGroup("FixedTool");
                break;
        }
    }

    void changeActiveMesh() {
        this->actionManager->getAction("ToggleNoneTool")->activate(QAction::Trigger);
        if(this->actionManager->getAction("ToggleDisplayWireframe")->isChecked())
            this->scene->toggleWireframe(false);
        else
            this->scene->toggleWireframe(true);
        if(this->scene->isGrid(this->combo_mesh->itemText(this->combo_mesh->currentIndex()).toStdString())) {
            Q_EMIT(this->gridSelected());
        } else {
            Q_EMIT(this->meshSelected());
        }
    }

    void initialize() {
        if(this->scene->demos.isDemo) {
            this->scene->changeActiveMesh(this->combo_mesh->itemText(this->combo_mesh->currentIndex()).toStdString());
            this->changeActiveMesh();
        }
    }
signals:
    void gridSelected();
    void meshSelected();
};

#endif	  // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
