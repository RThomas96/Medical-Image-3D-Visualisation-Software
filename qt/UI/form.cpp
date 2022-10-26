#include "form.hpp"

void Form::init() {
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->layout = new QFormLayout();
    this->setLayout(this->layout);

    insertNextWidgetInSection = false;
    sectionToInsertIn = QString("");

    insertNextWidgetInGroup = false;
    groupToInsertIn = QString("");
}

void Form::blockSignalsInGroup(const QString& group, bool blockSignals) {
    QLayout * layout = groups[group];
    for (int i = 0; i < layout->count(); ++i) {
        layout->itemAt(i)->widget()->blockSignals(blockSignals);
    }
}

void Form::addGroup(QLayout * group) {
    if(insertNextWidgetInSection) {
        this->addGroupToSection(this->sectionToInsertIn, group);
    } else if(insertNextWidgetInGroup) {
        this->addGroupToGroup(this->groupToInsertIn, group);
    } else {
        this->layout->addRow(group);
    }
}

void Form::addGroup(QWidget * label, QLayout * group) {
    if(insertNextWidgetInSection) {
        this->addGroupToSection(this->sectionToInsertIn, label, group);
    } else if(insertNextWidgetInGroup) {
        this->addGroupToGroup(this->groupToInsertIn, label, group);
    } else {
        this->layout->addRow(label, group);
    }
}

void Form::addWidget(QWidget * widget1, QWidget * widget2) {
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

void Form::addAllNextWidgetsToSection(const QString& name) {
    this->insertNextWidgetInSection = true;
    this->insertNextWidgetInGroup = false;
    this->sectionToInsertIn = name;
}

void Form::addAllNextWidgetsToDefaultSection() {
    this->insertNextWidgetInSection = false;
}

void Form::addAllNextWidgetsToGroup(const QString& name) {
    this->insertNextWidgetInGroup = true;
    this->insertNextWidgetInSection = false;
    this->groupToInsertIn = name;
}

void Form::addAllNextWidgetsToDefaultGroup() {
    this->insertNextWidgetInGroup = false;
}

void Form::setSectionCheckable(const QString& name, bool checkable) {
    this->sections[name].first->setCheckable(checkable);
}

void Form::setAllWidgetsVisibilityInSection(const QString& name, bool value) {
    QLayout *layout = this->sections[name].second;
    if (layout) {
        for (int i = 0; i < layout->count(); ++i) {
            if(layout->itemAt(i)->widget()) {
                layout->itemAt(i)->widget()->setVisible(value);
            }
        }
    }
}

void Form::setTextEditEditable(const QString& id, bool editable) {
    textEdits[id]->setReadOnly(!editable);
}

void Form::linkFileNameToFileChooser(const QString& filenameId, const QString& fileChooserId) {
    this->fileNames[filenameId]->init(this->fileChoosers[fileChooserId]);
}

void Form::setObjectTypeToChoose(const QString& id, ObjectToChoose objectToChoose) {
    objectChoosers[id]->objectToChoose = objectToChoose;
}

void Form::setFileChooserType(const QString& id, FileChooserType type) {
    fileChoosers[id]->setType(type);
}

void Form::setComboChoices(const QString& id, const std::vector<QString>& choices) {
    this->comboBoxes[id]->blockSignals(true);
    for(auto choice : choices) {
        this->comboBoxes[id]->addItem(choice);
    }
    this->comboBoxes[id]->blockSignals(false);
}

void Form::setComboChoices(const QString& id, const std::vector<std::string>& choices) {

    this->comboBoxes[id]->blockSignals(true);
    for(auto choice : choices) {
        this->comboBoxes[id]->addItem(QString(choice.c_str()));
    }
    this->comboBoxes[id]->blockSignals(false);
}

void Form::addWithLabel(const WidgetType& type, const QString& id, const QString& label) {
    this->add(type, id, id, label);
}

void Form::add(const WidgetType& type, const QString& id) {
    this->add(type, id, id, "");
}

void Form::add(const WidgetType& type, const QString& id, const QString& name) {
    this->add(type, id, name, "");
}

void Form::add(const WidgetType& type, const QString& id, const QString& name, const QString& label) {
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
            if(!this->insertNextWidgetInGroup)
                buttons[id] = new QPushButton(name);
            else
                buttons[id] = new QPushButton(name, this->groups[this->groupToInsertIn]->widget());
            buttons[id]->setCheckable(true);
            buttons[id]->setAutoExclusive(true);
            newWidget = buttons[id];
            connect(buttons[id], &QPushButton::clicked, [this, id] { widgetModified(id); });
            break;
        case WidgetType::FILENAME:
            fileNames[id] = new FileName();
            fileNames[id]->setAlignment(Qt::AlignHCenter);
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

void Form::update(Scene * scene) {
    for(auto& chooser : objectChoosers) {
        chooser.second->fillChoices(scene);
    }
}

void Form::addWidgetToSection(const QString& name, QWidget * widget1, QWidget * widget2) {
    if(widget2)
        this->sections[name].second->addRow(widget1, widget2);
    else
        this->sections[name].second->addRow(widget1);
}

void Form::addWidgetToGroup(const QString& name, QWidget * widget1, QWidget * widget2) {
    this->groups[name]->addWidget(widget1);
    if(widget2)
        this->groups[name]->addWidget(widget2);
}

void Form::addGroupToSection(const QString& name, QWidget * label, QLayout * group) {
    this->sections[name].second->addRow(label, group);
}

void Form::addGroupToSection(const QString& name, QLayout * group) {
    this->sections[name].second->addRow(group);
}

void Form::addGroupToGroup(const QString& name, QWidget * label, QLayout * group) {
    this->groups[name]->addLayout(group);
}

void Form::addGroupToGroup(const QString& name, QLayout * group) {
    this->groups[name]->addLayout(group);
}
