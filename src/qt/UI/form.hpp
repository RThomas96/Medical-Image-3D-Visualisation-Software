#ifndef QT_FORM
#define QT_FORM

#include "chooser.hpp"

#include<QString>
#include<QBoxLayout>
#include<QSlider>
#include<QCheckBox>
#include<QComboBox>
#include<QFormLayout>
#include<QGroupBox>
#include<QDoubleSpinBox>
#include<QSpinBox>
#include<QPushButton>
#include<QTextEdit>
#include<QLineEdit>
#include<QLabel>

class Scene;

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

    void init();
    void blockSignalsInGroup(const QString& group, bool blockSignals);

    void addGroup(QLayout * group);
    void addGroup(QWidget * label, QLayout * group);

    void addWidget(QWidget * widget1, QWidget * widget2 = nullptr);

    void addAllNextWidgetsToSection(const QString& name);
    void addAllNextWidgetsToDefaultSection();
    void addAllNextWidgetsToGroup(const QString& name);
    void addAllNextWidgetsToDefaultGroup();

    void setSectionCheckable(const QString& name, bool checkable);
    void setAllWidgetsVisibilityInSection(const QString& name, bool value);
    void setTextEditEditable(const QString& id, bool editable);

    void linkFileNameToFileChooser(const QString& filenameId, const QString& fileChooserId);

    void setObjectTypeToChoose(const QString& id, ObjectToChoose objectToChoose);
    void setFileChooserType(const QString& id, FileChooserType type);

    void setComboChoices(const QString& id, const std::vector<QString>& choices);
    void setComboChoices(const QString& id, const std::vector<std::string>& choices);

    void addWithLabel(const WidgetType& type, const QString& id, const QString& label);

    void add(const WidgetType& type, const QString& id);
    void add(const WidgetType& type, const QString& id, const QString& name);
    void add(const WidgetType& type, const QString& id, const QString& name, const QString& label);

    void update(Scene * scene);

private:
    void addWidgetToSection(const QString& name, QWidget * widget1, QWidget * widget2 = nullptr);
    void addWidgetToGroup(const QString& name, QWidget * widget1, QWidget * widget2 = nullptr);

    void addGroupToSection(const QString& name, QWidget * label, QLayout * group);
    void addGroupToSection(const QString& name, QLayout * group);

    void addGroupToGroup(const QString& name, QWidget * label, QLayout * group);
    void addGroupToGroup(const QString& name, QLayout * group);
};

#endif
