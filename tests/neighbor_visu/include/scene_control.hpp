#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_CONTROL_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_CONTROL_HPP_

#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include <iostream>

class Scene; // forward declaration
class Viewer; // forward declaration

class ControlPanel : public QWidget {
		Q_OBJECT
	public:
		ControlPanel(Scene* const scene, Viewer* lv, Viewer* rv, QWidget* parent = nullptr);
		~ControlPanel();
	protected:
		void initSignals();
	public slots:
		void activatePanels(bool activeStatus = true);
		void setImageBoundaries(int bounds[6]);
	private:
		Scene* const sceneToControl;
		QDoubleSpinBox* xPicker;
		QDoubleSpinBox* yPicker;
		QDoubleSpinBox* zPicker;
		QSlider* xTex;
		QSlider* yTex;
		QSlider* zTex;
		QSpinBox* minValueTexture;
		QSpinBox* maxValueTexture;
		QCheckBox* toggleTexCubeCheckbox;
		QWidget* controlContainer;
		Viewer* leftViewer;
		Viewer* rightViewer;

	public slots:
		void setXCoord(double coordX);
		void setYCoord(double coordY);
		void setZCoord(double coordZ);
		void setXTexCoord(int coordX);
		void setYTexCoord(int coordY);
		void setZTexCoord(int coordZ);
		void setTexCube(bool show);
		void setMinTexVal(int val);
		void setMaxTexVal(int val);
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_CONTROL_HPP_
