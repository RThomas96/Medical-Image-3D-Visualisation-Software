#ifndef COLOR_CONTROL
#define COLOR_CONTROL

#include<glm/glm.hpp>
#include<QFrame>
#include<QPushButton>
#include<QDoubleSpinBox>
#include<QCheckBox>
#include<QColor>
#include<QScrollArea>

#include "color_button.hpp"
#include "../scene_control.hpp"

class Scene;

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
    RangeOptionUnit(QWidget * parent = nullptr);
    void reset();

signals:
    void deleteCurrent(int id);
};

class PreviewButton : public QPushButton {
    Q_OBJECT

public:
    PreviewButton() : QPushButton() {
        this->setMouseTracking(true);
    }

    void enterEvent(QEvent* e) {
        Q_EMIT mouseEnter();
        QWidget::enterEvent(e);
    }

    void leaveEvent(QEvent* e) {
        Q_EMIT mouseLeave();
        QWidget::leaveEvent(e);
    }

signals:
    void mouseEnter();
    void mouseLeave();
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

    //QPushButton * hideButton;
    PreviewButton * hideButton;
    QCheckBox * hideColor;
    ColorButton * colorButton;
    QColor originalColor;

    RangeUnit(int id, QWidget * parent = nullptr);
    void setHighlightColor(bool value);
    glm::vec3 getColor();
    void setMoreOptions(bool value);

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

    void init();
    void readFromFile();
    void writeToFile();
    void clearUnits(bool addDefault = false);
    int findId(int id);
    void deleteUnit(int id);

    enum Direction {
        Left,
        Right
    };

    void moveUnit(int id, Direction direction);
    void addUnit(int min, int max);
    void addUnit(int min, int max, glm::ivec3 color, bool display);
    void addUnit();
    void updateRanges();
    void fillRangesFromScene();

    enum class Option {
        COLOR,
        VISU,
        MORE_OPTIONS
    };

    void toggleAll(Option option, bool value);
    void addUnitsAuto();
    void connect();
};

#endif
