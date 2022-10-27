#ifndef QT_INCLUDE_OPEN_MESH_WIDGET_HPP_
#define QT_INCLUDE_OPEN_MESH_WIDGET_HPP_

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

class OpenMeshWidget : public QWidget {
    Q_OBJECT

public:
	OpenMeshWidget(Scene* scene, QWidget* parent = nullptr);
    void setPotentialCages(const QStringList& namesOfPotentialCages);
    void loadMeshToScene(Scene * scene);
	~OpenMeshWidget() {};

    QVBoxLayout * mainLayout;
    QVBoxLayout * fileLayout;
    QHBoxLayout * selectLayout;
    QHBoxLayout * nameLayout;
    QVBoxLayout * cageLayout;
    QHBoxLayout * radioLayout;

    QGroupBox * groupName;
    QLabel * labelName;
    QLineEdit * name;

    QPushButton * chooseFileName;
    QLabel * selectedFile;

	QGroupBox * cageGroup;
    QRadioButton * radioMVC;
    QRadioButton * radioGreen;
	QComboBox * comboCages;

    QPushButton * loadMesh;

    QFrame * line;

    void selectFile();

signals:
    void loaded();
};

#endif
