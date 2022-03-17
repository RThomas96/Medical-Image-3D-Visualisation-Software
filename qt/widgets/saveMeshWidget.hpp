#ifndef QT_INCLUDE_SAVE_MESH_WIDGET_HPP_
#define QT_INCLUDE_SAVE_MESH_WIDGET_HPP_

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QFrame>

class Scene;

class SaveMeshWidget : public QWidget {
    Q_OBJECT

public:
	SaveMeshWidget(Scene* scene, QWidget* parent = nullptr);
    void setPotentialMeshToSave(const QStringList& namesOfPotentialMeshesToSave);
    void saveMesh(Scene * scene);
	~SaveMeshWidget() {};

    QVBoxLayout * mainLayout;
    QVBoxLayout * fileLayout;
    QHBoxLayout * selectLayout;
    QHBoxLayout * nameLayout;

    QPushButton * chooseFileName;
    QLabel * selectedFile;

	QComboBox * comboSave;

    QPushButton * saveMeshButton;

    QFrame * line;

    void selectFile();
};

#endif
