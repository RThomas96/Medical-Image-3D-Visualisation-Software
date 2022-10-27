#ifndef QUICKSAVE_MESH 
#define QUICKSAVE_MESH 

#include "chooser.hpp"
#include <QString>

class Scene;

class QuickSaveMesh {
    FileChooser * fileChooser;
    Scene * scene;
    QString filePath;

    public:
    QuickSaveMesh(Scene * scene): scene(scene) {
        fileChooser = new FileChooser("file", FileChooserType::SAVE, FileChooserFormat::OFF);
    }

    void save();

    void saveAs() {
        this->fileChooser->click();
        filePath = this->fileChooser->filename;
        if(filePath.isEmpty())
            return;
        this->save();
    }
};


#endif
