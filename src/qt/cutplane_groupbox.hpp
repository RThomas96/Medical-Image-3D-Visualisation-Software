#ifndef CUTPLANEGROUPBOX_H
#define CUTPLANEGROUPBOX_H

#include <glm/glm.hpp>
#include <QGroupBox>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <assert.h>

//*****************************************************************************************************//
//  CuttingPlaneGroupBox class                                                                         //
//*****************************************************************************************************//
// Creates a box with 3 sliders to manipulate cutting planes                                           //
// Each ligne correspond to an axis: text Qlabel, slider, normal direction, plane display status       //
//*****************************************************************************************************//
// Usage:                                                                                              //
// Create class                                                                                        //
// Then cutting planes ranges (maximum values) as integer setMaxCutPlanes(xMax,yMax,zMax)              //
// To update colors use setColors()                                                                    //
// Connect signals to get user input information                                                       //
//      iSliderValueChanged(i);   i in {x,y,z} signaling when slider value changed                     //
//      clickedInvertIPushButton();   I in {X,Y,Z} signaling inversion of half-space visibility        //
//      clickedDisplayICut(bool display);    I in {X,Y,Z} signaling change in plane display status     //
//*****************************************************************************************************//

class CutPlaneGroupBox : public QGroupBox
{
    Q_OBJECT
public:
    CutPlaneGroupBox(QWidget *parent = nullptr):QGroupBox(parent){init();}
    CutPlaneGroupBox(const QString &title, QWidget *parent = nullptr): QGroupBox(title, parent)
    {
        init();
    }

public slots:
    void setMaxCutPlanes(int x, int y, int z){

        //Setting cutting plane maxium values ie range
        assert(x>0 && y>0 && z>0 && "Slider for cutting planes should be positive");

        xHSlider->setRange(0,x);
        yHSlider->setRange(0,y);
        zHSlider->setRange(0,z);

        xHSlider->setValue(0);
        yHSlider->setValue(0);
        zHSlider->setValue(0);
    }

    void setValues(float x, float y, float z) {
        xHSlider->blockSignals(true);
        yHSlider->blockSignals(true);
        zHSlider->blockSignals(true);
        xHSlider->setValue(x*float(xHSlider->maximum()));
        yHSlider->setValue(y*float(yHSlider->maximum()));
        zHSlider->setValue(z*float(zHSlider->maximum()));
        this->labelValueX->setText(std::to_string(int(std::floor(float(x)*float(imageSize.x)))).c_str());
        this->labelValueY->setText(std::to_string(int(std::floor(float(y)*float(imageSize.y)))).c_str());
        this->labelValueZ->setText(std::to_string(int(std::floor(float(z)*float(imageSize.z)))).c_str());
        xHSlider->blockSignals(false);
        yHSlider->blockSignals(false);
        zHSlider->blockSignals(false);
    }

    void setImageSize(const glm::ivec3& imageSize) {
        this->imageSize = imageSize;
        // To update the slice indicator
        this->setValues(xHSlider->value()/float(xHSlider->maximum()), yHSlider->value()/float(yHSlider->maximum()), zHSlider->value()/float(zHSlider->maximum()));
    }

    void setDisabledAlpha(bool value) {
        if(value) {
            this->aHSlider->hide();
            this->labelCutA->hide();
        } else {
            this->aHSlider->show();
            this->labelCutA->show();
        }
    }

signals:
    void xSliderValueChanged(float x);
    void ySliderValueChanged(float y);
    void zSliderValueChanged(float z);
    void aSliderValueChanged(float a);

    void clickedInvertXPushButton();
    void clickedInvertYPushButton();
    void clickedInvertZPushButton();

    void clickedDisplayXCut(bool display);
    void clickedDisplayYCut(bool display);
    void clickedDisplayZCut(bool display);
protected:

    void init(){
        this->setCheckable(true);

        QVBoxLayout * vBoxLayout = new QVBoxLayout(this);

        //Building default cutting planes box
        //Setting horizontal layouts for all the axis aligned directions
        QHBoxLayout * xHBoxLayout = new QHBoxLayout();
        vBoxLayout->addLayout(xHBoxLayout);
        QHBoxLayout * yHBoxLayout = new QHBoxLayout();
        vBoxLayout->addLayout(yHBoxLayout);
        QHBoxLayout * zHBoxLayout = new QHBoxLayout();
        vBoxLayout->addLayout(zHBoxLayout);
        QHBoxLayout * aHBoxLayout = new QHBoxLayout();
        vBoxLayout->addLayout(aHBoxLayout);

        //Naming labels to identify each axis
        QLabel * labelCutX = new QLabel("x", this);
        xHBoxLayout->addWidget(labelCutX);
        QLabel * labelCutY = new QLabel("y", this);
        yHBoxLayout->addWidget(labelCutY);
        QLabel * labelCutZ = new QLabel("z", this);
        zHBoxLayout->addWidget(labelCutZ);
        labelCutA = new QLabel("a", this);
        aHBoxLayout->addWidget(labelCutA);

        labelValueX = new QLabel("0", this);
        labelValueX->setMinimumWidth(26);
        xHBoxLayout->addWidget(labelValueX);
        labelValueY = new QLabel("0", this);
        labelValueY->setMinimumWidth(26);
        yHBoxLayout->addWidget(labelValueY);
        labelValueZ = new QLabel("0", this);
        labelValueZ->setMinimumWidth(26);
        zHBoxLayout->addWidget(labelValueZ);
        labelValueX->setHidden(true);
        labelValueY->setHidden(true);
        labelValueZ->setHidden(true);

        //Sliders definitions
        xHSlider = new QSlider(this);
        xHSlider->setOrientation(Qt::Horizontal);
        xHBoxLayout->addWidget(xHSlider);

        yHSlider = new QSlider(this);
        yHSlider->setOrientation(Qt::Horizontal);
        yHBoxLayout->addWidget(yHSlider);

        zHSlider = new QSlider(this);
        //zHSlider->setMaximum(1);
        zHSlider->setOrientation(Qt::Horizontal);
        zHBoxLayout->addWidget(zHSlider);

        aHSlider = new QSlider(this);
        //zHSlider->setMaximum(1);
        aHSlider->setOrientation(Qt::Horizontal);
        aHBoxLayout->addWidget(aHSlider);

        //Push buttons to invert visibility direction
        QPushButton * invertXPushButton = new QPushButton("invert", this);
        invertXPushButton->setCheckable(true);
        xHBoxLayout->addWidget(invertXPushButton);
        QPushButton * invertYPushButton = new QPushButton("invert", this);
        invertYPushButton->setCheckable(true);
        yHBoxLayout->addWidget(invertYPushButton);
        QPushButton * invertZPushButton = new QPushButton("invert", this);
        invertZPushButton->setCheckable(true);
        zHBoxLayout->addWidget(invertZPushButton);

        //Checkbox to toggle plane display
        QCheckBox * displayXCut = new QCheckBox(this);
        displayXCut->setChecked(true);
        xHBoxLayout->addWidget(displayXCut);
        QCheckBox * displayYCut = new QCheckBox(this);
        displayYCut->setChecked(true);
        yHBoxLayout->addWidget(displayYCut);
        QCheckBox * displayZCut = new QCheckBox(this);
        displayZCut->setChecked(true);
        zHBoxLayout->addWidget(displayZCut);

        //QCheckBox * displayPlanes = new QCheckBox("Display planes", this);
        //displayPlanes->setChecked(true);
        //vBoxLayout->addWidget(displayPlanes);

        //Connecting signals notify for user interactions
        connect(xHSlider, SIGNAL(valueChanged(int)), this, SLOT(xSValueChanged(int)));
        connect(yHSlider, SIGNAL(valueChanged(int)), this, SLOT(ySValueChanged(int)));
        connect(zHSlider, SIGNAL(valueChanged(int)), this, SLOT(zSValueChanged(int)));
        connect(aHSlider, SIGNAL(valueChanged(int)), this, SLOT(aSValueChanged(int)));

        connect(invertXPushButton, SIGNAL(clicked()), this, SLOT(clickedIXPushButton()));
        connect(invertYPushButton, SIGNAL(clicked()), this, SLOT(clickedIYPushButton()));
        connect(invertZPushButton, SIGNAL(clicked()), this, SLOT(clickedIZPushButton()));

        connect(displayXCut, SIGNAL(clicked(bool)), this, SLOT(clickedDXCut(bool)));
        connect(displayYCut, SIGNAL(clicked(bool)), this, SLOT(clickedDYCut(bool)));
        connect(displayZCut, SIGNAL(clicked(bool)), this, SLOT(clickedDZCut(bool)));

        this->imageSize = glm::ivec3(100, 100, 100);
    }

    // The value stored in the slider are normalized between [0., 99.]
    // To display the image slice instead of the normalized value, we use this image size
    glm::ivec3 imageSize;

    QLabel *labelCutA;
    QLabel *labelValueX;
    QLabel *labelValueY;
    QLabel *labelValueZ;
    QLabel *labelValueA;
    QSlider *xHSlider;
    QSlider *yHSlider;
    QSlider *zHSlider;
    QSlider *aHSlider;
    QPushButton *invertXPushButton;
    QPushButton *invertYPushButton;
    QPushButton *invertZPushButton;
    QCheckBox *displayXCut;
    QCheckBox *displayYCut;
    QCheckBox *displayZCut;

protected slots:
    void xSValueChanged(int x){ this->labelValueX->setText(std::to_string(int(std::floor((float(x)/99.)*float(imageSize.x)))).c_str()); emit xSliderValueChanged(float(x)/99.);}
    void ySValueChanged(int y){ this->labelValueY->setText(std::to_string(int(std::floor((float(y)/99.)*float(imageSize.y)))).c_str()); emit ySliderValueChanged(float(y)/99.);}
    void zSValueChanged(int z){ this->labelValueZ->setText(std::to_string(int(std::floor((float(z)/99.)*float(imageSize.z)))).c_str()); emit zSliderValueChanged(float(z)/99.);}
    void aSValueChanged(int a){ emit aSliderValueChanged(float(a)/99.);}

    void clickedIXPushButton(){ emit clickedInvertXPushButton();}
    void clickedIYPushButton(){ emit clickedInvertYPushButton();}
    void clickedIZPushButton(){ emit clickedInvertZPushButton();}

    void clickedDXCut(bool display){ emit clickedDisplayXCut(display);}
    void clickedDYCut(bool display){ emit clickedDisplayYCut(display);}
    void clickedDZCut(bool display){ emit clickedDisplayZCut(display);}
};

#endif // CUTPLANEGROUPBOX_H
