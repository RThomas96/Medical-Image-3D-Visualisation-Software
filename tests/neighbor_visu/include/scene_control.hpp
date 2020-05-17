#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_CONTROL_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_CONTROL_HPP_

#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>

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
		QSlider* xSlider;
		QSlider* ySlider;
		QSlider* zSlider;
		QCheckBox* toggleTexCubeCheckbox;
		QWidget* controlContainer;
		Viewer* leftViewer;
		Viewer* rightViewer;

	public slots:
		void setXCoord(int coordX);
		void setYCoord(int coordY);
		void setZCoord(int coordZ);
		void setTexCube(bool show);
};

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_SCENE_CONTROL_HPP_
