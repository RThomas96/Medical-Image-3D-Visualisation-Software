#ifndef QT_CHOOSER
#define QT_CHOOSER

#include <QLabel>
#include <QComboBox>
#include <QPushButton>

class Scene;

enum ObjectToChoose {
    ALL,
    GRID
};

class ObjectChooser : public QComboBox {
    Q_OBJECT

public:
    ObjectToChoose objectToChoose;
    ObjectChooser(QWidget *parent = nullptr):QComboBox(parent){}

    void fillChoices(const ObjectToChoose& objectToChoose, Scene * scene);
    void fillChoices(Scene * scene);
};

///

enum FileChooserType {
    SELECT,
    SAVE
};

enum class FileChooserFormat {
    TIFF,
    MESH,
    PATH,
    OFF
};

class FileChooser : public QPushButton {
    Q_OBJECT

public:
    FileChooserFormat format;
    FileChooserType type;
	QString filename;
    FileChooser(QString name, FileChooserType type = FileChooserType::SELECT, FileChooserFormat format = FileChooserFormat::TIFF, QWidget *parent = nullptr):QPushButton(name, parent){init(type, format);}

public slots:
    void init(FileChooserType type, FileChooserFormat format);
    void resetValues();
    void setType(FileChooserType type);
    void setFormat(FileChooserFormat format);
    void click();

signals:
    void fileSelected();
};

///

class FileName : public QLabel {
    Q_OBJECT

public:
    FileName(QWidget *parent = nullptr):QLabel(parent){init(nullptr);}

public slots:
    void init(FileChooser * fileChooser);
    void resetValues();
};

#endif
