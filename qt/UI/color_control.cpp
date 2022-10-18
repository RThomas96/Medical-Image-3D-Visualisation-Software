#include "color_control.hpp"

#include "../scene.hpp"
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <random>

RangeOptionUnit::RangeOptionUnit(QWidget * parent) : QFrame(parent)  {
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
    hideOption->setIcon(QIcon(QPixmap("../resources/more_option.svg")));
    hideOption->setIconSize(QPixmap("../resources/more_option.svg").rect().size());
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

void RangeOptionUnit::reset() {
    this->hideButton->setChecked(false);
    this->hideColor->setChecked(false);
    this->deleteButton->setChecked(false);
    this->open->setChecked(false);
    this->save->setChecked(false);
    this->autoButton->setChecked(false);
    this->hideOption->setChecked(false);
}

///////////////

RangeUnit::RangeUnit(int id, QWidget * parent) : id(id), QFrame(parent)  {
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
    hideButton = new PreviewButton();
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
    //QObject::connect(hideColor, &QCheckBox::stateChanged, [this](){this->colorButton->setDisabled(!this->hideColor->isChecked());});
    QObject::connect(min, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double d){
            if(!moreOptions || min->value() > max->value()) {
            this->max->blockSignals(true);
            this->max->setValue(min->value());
            this->max->blockSignals(false);
            }
            Q_EMIT rangeChanged(this->id);
            });
    QObject::connect(max, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double d){Q_EMIT rangeChanged(this->id);});
    QObject::connect(hideButton, &PreviewButton::clicked, [this](){Q_EMIT rangeChanged(this->id);});
    QObject::connect(hideButton, &PreviewButton::toggled, [this](){this->hideColor->setDisabled(this->hideButton->isChecked());
            /*this->colorButton->setDisabled(this->hideButton->isChecked());*/});
    QObject::connect(hideButton, &PreviewButton::mouseEnter, [this](){setHighlightColor(true);});
    QObject::connect(hideButton, &PreviewButton::mouseLeave, [this](){setHighlightColor(false);});
    moreOptions = false;
    this->setMoreOptions(this->moreOptions);
}

void RangeUnit::setHighlightColor(bool value) {
    if(value) {
        this->originalColor = colorButton->getColor();
        this->colorButton->setColor(QColor(255., 255., 0.));
    } else {
        colorButton->setColor(this->originalColor);
    }
    Q_EMIT colorChanged(this->id);
}

glm::vec3 RangeUnit::getColor() {
    return glm::vec3(float(colorButton->getColor().red())/255., float(colorButton->getColor().green())/255., float(colorButton->getColor().blue())/255.);
}

void RangeUnit::setMoreOptions(bool value) {
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

////////////////////

void RangeControl::init() {
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

void RangeControl::readFromFile() {
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

void RangeControl::writeToFile() {
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

void RangeControl::clearUnits(bool addDefault) {
    this->rangeOptionUnit->reset();
    for(int i = this->unitLayout->count()-1; i >= 0; --i) {
        this->deleteUnit(i);
    }
    if(addDefault) {
        this->addUnit(0, 16000);
        this->units.back()->hideColor->click();
    }
}

int RangeControl::findId(int id) {
    for(int i = 0; i < this->unitLayout->count(); ++i) {
        if(dynamic_cast<RangeUnit*>(this->unitLayout->itemAt(i)->widget())->id == id)
            return i;
    }
    return -1;
}

void RangeControl::deleteUnit(int id) {
    int idx = findId(id);
    this->unitLayout->removeWidget(this->unitLayout->itemAt(idx)->widget());
    delete this->units[id];
    this->units.erase(this->units.begin()+id);
    for(int i = id; i < this->units.size(); ++i)
        this->units[i]->id -= 1;
}

void RangeControl::moveUnit(int id, Direction direction) {
    int idx = this->findId(id);
    if(idx == -1 || (idx == 0 && direction == Direction::Left) || (idx == this->unitLayout->count()-1 && direction == Direction::Right))
        return;
    QWidget * widget = this->unitLayout->takeAt(idx)->widget();
    if(direction == Direction::Left)
        this->unitLayout->insertWidget(idx-1, widget);
    else
        this->unitLayout->insertWidget(idx+1, widget);
}

void RangeControl::addUnit(int min, int max) {
    this->addUnit();

    this->units.back()->blockSignals(true);
    this->units.back()->min->setValue(min);
    this->units.back()->max->setValue(max);
    this->units.back()->blockSignals(false);
}

void RangeControl::addUnit(int min, int max, glm::ivec3 color, bool display) {
    this->addUnit();

    this->units.back()->blockSignals(true);
    this->units.back()->min->setValue(min);
    this->units.back()->max->setValue(max);
    this->units.back()->colorButton->setColor(QColor(color.r, color.g, color.b));
    if(!display)
        this->units.back()->hideButton->setChecked(true);
    this->units.back()->blockSignals(false);
}

void RangeControl::addUnit() {
    this->units.push_back(new RangeUnit(this->units.size()));
    this->unitLayout->addWidget(units.back());

    units.back()->setMoreOptions(rangeOptionUnit->hideOption->isChecked());
    units.back()->hideColor->setChecked(!rangeOptionUnit->hideColor->isChecked());
    units.back()->hideButton->setChecked(rangeOptionUnit->hideButton->isChecked());

    QObject::connect(units.back(), &RangeUnit::leftMove, this, [this](int id){this->moveUnit(id, Direction::Left); this->updateRanges();});
    QObject::connect(units.back(), &RangeUnit::rightMove, this, [this](int id){this->moveUnit(id, Direction::Right); this->updateRanges();});
    QObject::connect(units.back(), &RangeUnit::deleteCurrent, this, [this](int id){this->deleteUnit(id); this->updateRanges();});
    QObject::connect(units.back(), &RangeUnit::rangeChanged, this, [this](int id){this->updateRanges();});
    QObject::connect(units.back(), &RangeUnit::colorChanged, this, [this](int id){this->updateRanges();});
    //QObject::connect(this->buttonAdd, &QPushButton::clicked, [this, scene](){this->updateRanges(scene);});
}

void RangeControl::updateRanges() {
    scene->resetRanges();
    for(int i = 0; i < this->unitLayout->count(); ++i) {
        RangeUnit * unit = dynamic_cast<RangeUnit*>(this->unitLayout->itemAt(i)->widget());
        if(unit->hideColor->isChecked()) {
            scene->addRange(unit->min->value(), unit->max->value(), unit->getColor(), !unit->hideButton->isChecked(), false);
        } else {
            scene->addRange(unit->min->value(), unit->max->value(), glm::vec3(1., 1., 1.), !unit->hideButton->isChecked(), false);
        }
    }
    scene->updateMinMaxDisplayValues();
}

void RangeControl::toggleAll(Option option, bool value) {
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

void RangeControl::addUnitsAuto() {
    this->clearUnits();
    auto minMax = scene->getGridMinMaxValues();
    std::vector<bool> usage = scene->getGridUsageValues();
    int nbUnits = minMax.second - minMax.first;
    if(nbUnits < maxNbUnits) {
        for(int i = minMax.first+1; i < usage.size(); ++i) {
            if(usage[i])
                this->addUnit(i, i);
        }
    } else {
        this->clearUnits(true);
    }
    this->updateRanges();
}

void RangeControl::fillRangesFromScene() {
    std::vector<std::pair<uint16_t, uint16_t>> rangesMinMax;
    scene->getRanges(rangesMinMax);
    std::vector<glm::vec3> rangesColor;
    scene->getRangesColor(rangesColor);
    std::vector<bool> rangesVisi;
    scene->getRangesVisu(rangesVisi);

    this->clearUnits();
    for(int i = 0; i < rangesMinMax.size(); ++i) {
        this->addUnit(rangesMinMax[i].first, rangesMinMax[i].second, rangesColor[i]*glm::vec3(255., 255., 255.), rangesVisi[i]);
    }

    // If there is no ranges in the scene, it means that this is a newly created grid
    if(rangesMinMax.empty()) {
        this->clearUnits(true);
        this->updateRanges();
    }
}

void RangeControl::connect() {
    QObject::connect(this->buttonAdd, &QPushButton::clicked, [this](){this->addUnit(); this->updateRanges();});
    QObject::connect(this->scene, &Scene::activeMeshChanged, [this](){this->fillRangesFromScene();});
}
