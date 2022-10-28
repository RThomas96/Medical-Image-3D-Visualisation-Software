#ifndef DISPLAY_PANNEL 
#define DISPLAY_PANNEL 

#include "../helper/QActionManager.hpp"
#include<QToolBar>
#include<QVBoxLayout>

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
        toolBar->addAction(actionManager.getAction("ToggleDisplayWireframe"));
        toolBar->addAction(actionManager.getAction("ToggleDisplayMultiView"));
    };
};


#endif
