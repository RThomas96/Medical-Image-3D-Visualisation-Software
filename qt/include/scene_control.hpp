#ifndef QT_INCLUDE_SCENE_CONTROL_HPP_
#define QT_INCLUDE_SCENE_CONTROL_HPP_

#include "../../macros.hpp"
#include "../../features.hpp"

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
		ControlPanel(Scene* const scene, Viewer* lv, QWidget* parent = nullptr);
		~ControlPanel();
	protected:
		void initSignals(void);
		void updateViewers(void);
		void updateValues(void);
	public slots:
		void activatePanels(bool activeStatus = true);
	private:
		Scene* const sceneToControl;
		QSpinBox* minValueTexture; ///< Spinbox to determine the min value in the texture which constitutes viable information
		QSpinBox* maxValueTexture; ///< Spinbox to determine the max value in the texture which constitutes viable information
		QSpinBox* minValueColor; ///< Spinbox to determine the min color value to determine the color scale
		QSpinBox* maxValueColor; ///< Spinbox to determine the max color value to determine the color scale
		QWidget* controlContainer;
		Viewer* leftViewer;
		QDoubleSpinBox* clipDistance;

	public slots:
		void setTexCube(bool show);
		void setMinTexVal(int val);
		void setMaxTexVal(int val);
		void setMinColVal(int val);
		void setMaxColVal(int val);
		void setClipDistance(double val);
		void setCutPlaneXPos(double val);
		void setCutPlaneYPos(double val);
		void setCutPlaneZPos(double val);
};

#endif // QT_INCLUDE_SCENE_CONTROL_HPP_
