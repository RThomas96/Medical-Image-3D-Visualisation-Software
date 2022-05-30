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

#include <map>

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
    MESH
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
                else
	                filename = QFileDialog::getOpenFileName(nullptr, "Open mesh file", QDir::currentPath(), "MESH files (*.mesh)", 0, QFileDialog::DontUseNativeDialog);
                break;

            case FileChooserType::SAVE:
                if(this->format == FileChooserFormat::TIFF)
	                filename = QFileDialog::getSaveFileName(nullptr, "Select the image to save", QDir::currentPath(), tr("TIFF Files (*.tiff)"), 0, QFileDialog::DontUseNativeDialog);
                else
	                filename = QFileDialog::getSaveFileName(nullptr, "Select the mesh to save", QDir::currentPath(), tr("OFF Files (*.off)"), 0, QFileDialog::DontUseNativeDialog);
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
        this->setText("[Select a file]");
        if(fileChooser)
            QObject::connect(fileChooser, &FileChooser::fileSelected, [this, fileChooser](){this->setText(fileChooser->filename);});
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
    TIFF_SAVE,
    MESH_CHOOSE,
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
            case WidgetType::TIFF_CHOOSE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SELECT, FileChooserFormat::TIFF);
                newWidget = fileChoosers[id];
                break;
            case WidgetType::MESH_SAVE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SAVE, FileChooserFormat::MESH);
                newWidget = fileChoosers[id];
                break;
            case WidgetType::TIFF_SAVE:
                fileChoosers[id] = new FileChooser(name, FileChooserType::SAVE, FileChooserFormat::TIFF);
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

        this->add(WidgetType::TEXT_EDIT, "PtToDeform", "Points to deform");
        this->setTextEditEditable("PtToDeform", true);

        this->add(WidgetType::TEXT_EDIT, "Results", "Results");
        this->setTextEditEditable("Results", true);

        this->add(WidgetType::BUTTON, "Deform");
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

    // Data
    QImage imageData;// Image data to be painted
    QSize originalImageSize;// Original image size
    QSize targetImageSize;// Optimal image size according to voxel size, etc

    QPoint paintedImageOrigin;
    QSize paintedImageSize;

    //Display variables
    QLabel * display;
    QHBoxLayout * layout;
    QImage::Format format;

    Image2DViewer(QImage::Format format, QWidget *parent = nullptr): QWidget(parent), format(format){init();}

    void init() {
        this->zoomSpeed = 30;
        this->zoom = 1;
        this->imageData = QImage(10., 10., format);
        this->imageData.fill(QColor(0., 0., 0.));
        this->targetImageSize = this->imageData.size();
        this->paintedImageOrigin = QPoint(0, 0);
        this->paintedImageSize = this->imageData.size();

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

    void setImageSize(const QSize& originalImageSize, const QSize& targetImageSize) {
        this->originalImageSize = originalImageSize;
        this->targetImageSize = targetImageSize;
        this->zoomSpeed = std::max(this->targetImageSize.width(), this->targetImageSize.height())/10;

        this->paintedImageOrigin = QPoint(0, 0);
        this->paintedImageSize = targetImageSize;
        this->fitToWindow();
    }

    void updateImageData(const QImage& image) {
        if(image.size() == this->originalImageSize) {
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
        painter.drawPixmap(target, QPixmap::fromImage(this->imageData.scaled(this->paintedImageSize, Qt::IgnoreAspectRatio, Qt::FastTransformation)));
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
        std::cout << event->pos().x() << std::endl;
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
            glm::vec2 ptInPaintedImage(pointInPaintedImage.x(), pointInPaintedImage.y());
            glm::vec2 targetSize = glm::vec2(originalImageSize.width(), originalImageSize.height());
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

    glm::ivec3 imgSize;
    glm::ivec3 imgNewSize;
    std::vector<std::string> gridNames;
    std::vector<int> imagesToDraw;
    std::vector<int> alphaValues;
    std::vector<QColor> colors;
    Interpolation::Method interpolationMethod;
    int sliceIdx;

    Image2DViewer * viewer2D;

    std::vector<std::vector<bool>> upToDate;
    std::vector<Raw3DImage> imgData;

    Scene * scene;

    Image3DViewer(const QString& name, const glm::vec3& side, Scene * scene, QWidget * parent = nullptr): QWidget(parent), name(name), direction(side), scene(scene), isInitialized(false), viewer2D(nullptr) {initLayout(); connect(scene);}

    void init(const glm::vec3& imageSize, const glm::vec3& imageResolution, const int& sliceIdx, const glm::vec3& side, std::vector<std::string> gridNames, std::vector<int> imgToDraw, std::vector<int> alphaValues, std::vector<QColor> colors, Interpolation::Method interpolationMethod) {

        for(auto name : gridNames)
            if(name.empty())
                return;

        this->direction = side;

        this->imgSize = imageResolution;

        this->gridNames = gridNames;
        this->imagesToDraw = imgToDraw;
        this->alphaValues = alphaValues;
        this->colors = colors;
        this->interpolationMethod = interpolationMethod;

        this->upToDate.clear();
        this->upToDate = std::vector<std::vector<bool>>(gridNames.size(), std::vector<bool>(this->imgSize.z, false));

        imgData.clear();
        imgData.reserve(gridNames.size());
        for(auto name : gridNames)
            imgData.push_back(Raw3DImage(this->imgSize, imgFormat));

        this->isInitialized = true;

        this->viewer2D->setImageSize(QSize(imageResolution.x, imageResolution.y), QSize(imageSize.x, imageSize.y));
        this->setSliceIdx(sliceIdx);

    }

    void setSliceIdx(int newSliceIdx) {
        this->sliceIdx = newSliceIdx;
        if(this->isInitialized) {
            this->drawImages();
        }
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
        scene->getValues(this->gridNames[imageIdx], slices, bbox, this->imgSize, data, this->interpolationMethod);
        this->imgData[imageIdx].setSlice(sliceIdx, data);
    }

    void getColor(int idx, glm::ivec3 position, QColor& color) {
        color = this->imgData[idx].getImage(position.z).pixelColor(position.x, position.y);
    }

    QImage getCurrentMergedImage() {
        return this->mergeImages(this->imagesToDraw, this->sliceIdx);
    }

    QImage mergeImages(const std::vector<int>& indexes, const int& z) {
        QColor color;
        QPixmap result(this->imgSize.x, this->imgSize.y);
        result.fill(Qt::black);
        QPainter painter(&result);

        for(int idx = 0; idx < indexes.size(); ++idx) {
            QImage img = this->imgData[idx].getImage(z).convertToFormat(mergedImgFormat);
            for(int i = 0; i < img.width(); ++i) {
                for(int j = 0; j < img.height(); ++j) {
                    QColor color = img.pixelColor(i, j);
                    if(color == Qt::black) {
                        color.setAlpha(0);
                    } else {
                        float value = float(color.red())/255.;
                        color = QColor(value*this->colors[idx].red(), value*this->colors[idx].green(), value*this->colors[idx].blue());
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

    void mouseMovedIn2DViewer(const glm::ivec2& positionOfMouse2D) {
        std::cout << positionOfMouse2D << std::endl;
        glm::vec3 positionOfMouse3D(positionOfMouse2D.x, positionOfMouse2D.y, sliceIdx);
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
    }
signals:
    void isSelected();
    void mouseMovedInPlanarViewer(const glm::vec3& positionOfMouse3D);
};

class PlanarViewForm : public Form {
    Q_OBJECT

public:
    Scene * scene;

    //Image3DViewer * imageViewer;

    std::map<QString, Image3DViewer*> viewers;
    QString selectedViewer;
    PlanarViewForm(Scene * scene, QWidget *parent = nullptr):Form(parent), scene(scene){init(scene);connect(scene);}

public slots:

    void addViewer(const QString& name, const glm::vec3& side = glm::vec3(0., 0., 1.)) {
        if(name.isEmpty())
            return;
        this->viewers[name] = new Image3DViewer(name, side, this->scene);
        QObject::connect(this->viewers[name], &Image3DViewer::isSelected, [=](){this->selectViewer(name);});

        this->selectedViewer = name;
        this->labels["SelectedViewer"]->setText(name);

        glm::vec3 imgSize = this->getBackImgDimension(scene);
        if(this->checkBoxes["Auto"]->isChecked())
            imgSize = this->autoComputeBestSize(scene);
        if(side.x == 1.)
            std::swap(imgSize.x, imgSize.z);
        if(side.y == 1.)
            std::swap(imgSize.y, imgSize.z);
        this->setSpinBoxesValues(imgSize);

        this->updateImageViewer();
    }

    void selectViewer(const QString& name) {
        this->selectedViewer = name;
        this->labels["SelectedViewer"]->setText(name);
        this->updateDefaultValues(name);
    }

    void updateDefaultValues(const QString& name) {
        const Image3DViewer * viewer = this->viewers[name];

        this->setSpinBoxesValues(viewer->imgSize);

        this->comboBoxes["Interpolation"]->blockSignals(true);
        int idx = this->comboBoxes["Interpolation"]->findText(Interpolation::toString(viewer->interpolationMethod).c_str());
        this->comboBoxes["Interpolation"]->setCurrentIndex(idx);
        this->comboBoxes["Interpolation"]->blockSignals(false);

        this->objectChoosers["From"]->blockSignals(true);
        idx = 0;
        if(viewer->gridNames.size() > 0)
            idx = this->objectChoosers["From"]->findText(viewer->gridNames[0].c_str());
        this->objectChoosers["From"]->setCurrentIndex(idx);
        this->objectChoosers["From"]->blockSignals(false);

        this->objectChoosers["To"]->blockSignals(true);
        idx = 0;
        if(viewer->gridNames.size() > 1)
            idx = this->objectChoosers["To"]->findText(viewer->gridNames[1].c_str());
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
        this->sliders["SliderX"]->setMaximum(viewer->imgSize.z-1);
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
        this->add(WidgetType::LABEL, "SelectedViewer", "NONE");

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

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::H_GROUP, "GroupFront", "Front");
        this->addAllNextWidgetsToGroup("GroupFront");
        this->groups["GroupFront"]->setAlignment(Qt::AlignHCenter);

        this->add(WidgetType::GRID_CHOOSE, "To", "Front");
        this->setObjectTypeToChoose("To", ObjectToChoose::GRID);
        this->objectChoosers["To"]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        this->add(WidgetType::CHECK_BOX, "UseFront");
        this->add(WidgetType::SPIN_BOX, "AlphaFront");

        this->addAllNextWidgetsToDefaultGroup();

        this->add(WidgetType::COMBO_BOX, "Interpolation", "Interpolation");
        this->setComboChoices("Interpolation", Interpolation::toStringList());

        this->addWithLabel(WidgetType::H_GROUP, "GroupResolution", "Resolution");
        this->addAllNextWidgetsToGroup("GroupResolution");

        this->add(WidgetType::SPIN_BOX, "X");
        this->add(WidgetType::SPIN_BOX, "Y");
        this->add(WidgetType::SPIN_BOX, "Z");
        this->addWithLabel(WidgetType::CHECK_BOX, "Auto", "Auto");

        this->addAllNextWidgetsToDefaultGroup();

        this->addWithLabel(WidgetType::H_GROUP, "GroupSide", "Side");
        this->addAllNextWidgetsToGroup("GroupSide");

        this->add(WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE, "SideX", "X");
        this->add(WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE, "SideY", "Y");
        this->add(WidgetType::BUTTON_CHECKABLE_AUTOEXCLUSIVE, "SideZ", "Z");

        this->addAllNextWidgetsToDefaultGroup();

        /****/

        this->checkBoxes["UseBack"]->setChecked(true);
        this->checkBoxes["UseFront"]->setChecked(true);

        this->spinBoxes["AlphaBack"]->setMinimum(0);
        this->spinBoxes["AlphaBack"]->setMaximum(255);
        this->spinBoxes["AlphaFront"]->setMinimum(0);
        this->spinBoxes["AlphaFront"]->setMaximum(255);

        /****/

        //this->addViewer("ViewZ");
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
        if(this->checkBoxes["Auto"]->isChecked())
            imgSize = this->autoComputeBestSize(scene);
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

    void updateImageViewer() {
        if(this->noViewerSelected())
            return;
        this->sliders["SliderX"]->setMinimum(0);
        this->sliders["SliderX"]->setMaximum(this->getImgDimension().z-1);
        this->viewers[this->selectedViewer]->init(this->autoComputeBestSize(this->scene), this->getImgDimension(), this->sliders["SliderX"]->value(), this->getSide(), {this->getFromGridName(), this->getToGridName()}, this->getImagesToDraw(), {this->spinBoxes["AlphaBack"]->value(), this->spinBoxes["AlphaFront"]->value()}, {QColor(255.*this->scene->color0.x, 255.*this->scene->color0.y, 255.*this->scene->color0.z), QColor(255.*this->scene->color0_second.x, 255.*this->scene->color0_second.y, 255.*this->scene->color0_second.z)}, this->getInterpolationMethod());
    }

    void update(Scene * scene) {
        Form::update(scene);
    }

    void show() {
        Form::show();
    }

    glm::ivec3 getImgDimension() {
        return glm::ivec3(this->spinBoxes["X"]->value(), this->spinBoxes["Y"]->value(), this->spinBoxes["Z"]->value());
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

    void connect(Scene * scene) {
        QObject::connect(this, &Form::widgetModified, [this, scene](const QString &id){
            if(id == "From" || id == "Auto")
                this->backImageChanged(scene);

            if(id == "X" || id == "Y" || id == "Z" || id == "Interpolation" || id == "UseBack" || id == "UseFront" || id == "From" || id == "To" || id == "Auto")
                this->updateImageViewer();

            if(id == "SideX" || id == "SideY" || id == "SideZ") {
                if(!this->noViewerSelected()) {
                    if(id == "SideX")
                        this->viewers[this->selectedViewer]->direction = glm::vec3(1., 0., 0.);
                    if(id == "SideY")
                        this->viewers[this->selectedViewer]->direction = glm::vec3(0., 1., 0.);
                    if(id == "SideZ")
                        this->viewers[this->selectedViewer]->direction = glm::vec3(0., 0., 1.);
                    this->backImageChanged(scene);
                    this->updateImageViewer();
                }
            }

            if(id == "SliderX") {
                this->viewers[this->selectedViewer]->setSliceIdx(this->sliders["SliderX"]->value());
                this->labels["SliderX"]->setText(std::to_string(this->sliders["SliderX"]->value()).c_str());
            }
        });

        QObject::connect(scene, &Scene::colorChanged, [this, scene](){
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
            scene->writeGreyscaleTIFFImage(this->fileChoosers["Save image back"]->filename.toStdString(), this->imageViewer->imgSize, this->imageViewer->imgData[0].data);
        });

        QObject::connect(this->fileChoosers["Save image front"], &FileChooser::fileSelected, [this, scene](){
            scene->writeGreyscaleTIFFImage(this->fileChoosers["Save image front"]->filename.toStdString(), this->imageViewer->imgSize, this->imageViewer->imgData[1].data);
        });

        QObject::connect(this->buttons["Preview"], &QPushButton::clicked, [this, scene](){
            this->imageViewer->show();
        });

        QObject::connect(this->objectChoosers["From"], QOverload<int>::of(&QComboBox::currentIndexChanged), [this, scene](int index){
        });
    }
};

class OpenImageForm : Form {
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

        this->add(WidgetType::BUTTON, "Load");

        /***/

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

        this->buttons["Load"]->setEnabled(false);
    }

    void update(Scene * scene) {
        Form::update(scene);
    }

    void show() {
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
        SimpleImage image(files);        
        this->doubleSpinBoxes["SizeVoxelX"]->setValue(image.voxelSize.x);
        this->doubleSpinBoxes["SizeVoxelY"]->setValue(image.voxelSize.y);
        this->doubleSpinBoxes["SizeVoxelZ"]->setValue(image.voxelSize.z);
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
        });

        QObject::connect(this->buttons["Load"], &QPushButton::clicked, [this, scene](){
                if(this->useTetMesh) {
                    scene->openGrid(this->getName(), this->getImgFilenames(), this->getSubsample(), this->getTetmeshFilename());
                } else {
                    scene->openGrid(this->getName(), this->getImgFilenames(), this->getSubsample(), this->getSizeVoxel(), this->getSizeTetmesh());
                }
        });
    }
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
            this->info_position_data->setText(QString(pt.c_str()));
        } else {
            this->info_id_data->setText(QString("-"));
            this->info_position_data->setText(QString("[]"));
        }
    }
};

class QActionManager : QWidget {
    Q_OBJECT
public:
    std::map<std::string, QAction *> actions;
    std::map<std::string, QActionGroup *> actionExclusiveGroups;

    std::map<std::string, QStringList> actionGroups;

    QAction * getAction(const QString& name) {
        return actions[name.toStdString()];
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
        //QFont font = this->font();
        //font.setPointSize(8);
        //this->setFont(font);

        this->toolBar = new QToolBar(this);
        this->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        //this->toolBar->setIconSize(QSize(150, 150));

        this->mainLayout->addWidget(this->toolBar);
        //this->mainLayout->addWidget(this->toolBarRow2);
    };
    void connect(QActionManager& actionManager) {
        toolBar->addAction(actionManager.getAction("ToggleDisplayMesh"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayGrid"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayPlanarViewers"));
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

	ControlPanel* controlPanel;
	bool widgetSizeSet;

	QMenu* fileMenu;
	QMenu* viewMenu;
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
    OpenImageForm * openImageForm;
    PlanarViewer2D * planarViewer;

    bool isShiftPressed = false;

public slots:
    void addNewMesh(const std::string& name, bool grid, bool cage) {
        //this->combo_mesh->insertItem(this->combo_mesh->count(), QString(this->meshNames.back().c_str()));
        this->combo_mesh->insertItem(this->combo_mesh->count(), QString(name.c_str()));
        //if(!this->gridOrCage.back().first)
        //    this->combo_mesh_register->insertItem(this->combo_mesh_register->count(), QString(this->meshNames.back().c_str()));
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
    }

    void initialize() {
        if(this->scene->demos.isDemo) {
            this->scene->changeActiveMesh(this->combo_mesh->itemText(this->combo_mesh->currentIndex()).toStdString());
            this->changeActiveMesh();
        }
    }
};

#endif	  // QT_INCLUDE_NEIGHBOR_VISU_MAIN_WIDGET_HPP_
