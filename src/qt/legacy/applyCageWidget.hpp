#ifndef QT_INCLUDE_APPLY_CAGE_WIDGET_HPP_
#define QT_INCLUDE_APPLY_CAGE_WIDGET_HPP_

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

class ApplyCageWidget : public QWidget {
    Q_OBJECT

public:
	ApplyCageWidget(Scene* scene, QWidget* parent = nullptr);
    void setPotentialCageToApply(const QStringList& namesOfPotentialCageesToApply);
    void applyCage(Scene * scene);
	~ApplyCageWidget() {};

    QVBoxLayout * mainLayout;
    QVBoxLayout * fileLayout;
    QHBoxLayout * selectLayout;
    QHBoxLayout * nameLayout;

    QPushButton * chooseFileName;
    QLabel * selectedFile;

	QComboBox * comboApply;

    QPushButton * saveCageButton;

    QFrame * line;

    void selectFile();
};

#endif
