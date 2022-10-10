#ifndef TOOL_PANNEL 
#define TOOL_PANNEL 

#include "../helper/QActionManager.hpp"

#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>

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

        this->toolbar->addAction(actionManager.getAction("SliceTool_switchX"));
        this->toolbar->addAction(actionManager.getAction("SliceTool_switchY"));
        this->toolbar->addAction(actionManager.getAction("SliceTool_switchZ"));
    };
};


#endif
