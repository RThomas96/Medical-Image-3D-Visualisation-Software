#include "deformation_form.hpp"
#include "../scene.hpp"

#include<QTextBlock>

void DeformationForm::init() {
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

void DeformationForm::update(Scene * scene) {
    Form::update(scene);
}

void DeformationForm::show() {
    Form::show();
}

void DeformationForm::extractPointsFromText(std::vector<glm::vec3>& points) {
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

void DeformationForm::convertPoints(Scene * scene) {
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

std::string DeformationForm::getFromGridName() {
    return this->objectChoosers["From"]->currentText().toStdString();
}

std::string DeformationForm::getToGridName() {
    return this->objectChoosers["To"]->currentText().toStdString();
}

void DeformationForm::connect(Scene * scene) {
    QObject::connect(this->buttons["Deform"], &QPushButton::clicked, [this, scene](){this->convertPoints(scene);});
    QObject::connect(this->buttons["Preview"], &QPushButton::clicked, [this, scene](){
            scene->writeImageWithPoints("previewFrom.tiff", this->getFromGridName(), this->origins);
            scene->writeImageWithPoints("previewTo.tiff", this->getToGridName(), this->results);
            });
    QObject::connect(this->fileChoosers["Save image"], &FileChooser::fileSelected, [this, scene](){
            scene->writeDeformation(this->fileChoosers["Save image"]->filename.toStdString(), this->getFromGridName(), this->getToGridName());
            });
}
