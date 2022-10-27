#ifndef INFO_PANNEL 
#define INFO_PANNEL 

#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "glm/glm.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

class Scene;

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

    void connect(Scene * scene);

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

#endif
