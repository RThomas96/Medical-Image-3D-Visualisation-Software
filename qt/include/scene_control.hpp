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

#define CONTROLLER_USE_SLIDERS

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
	public slots:
		void activatePanels(bool activeStatus = true);
		void updateValues(void);
	private:
		Scene* const sceneToControl;

		QSlider* minValueTexture; ///< Slider to determine the min value in the texture which constitutes viable information
		QSlider* maxValueTexture; ///< Slider to determine the max value in the texture which constitutes viable information
		QSlider* minValueColor; ///< Slider to determine the min color value to determine the color scale
		QSlider* maxValueColor; ///< Slider to determine the max color value to determine the color scale

		QWidget* controlContainer;
		Viewer* leftViewer;
		QDoubleSpinBox* clipDistance;

	public slots:
		void setMinTexVal(int val);
		void setMaxTexVal(int val);
		void setMinColVal(int val);
		void setMaxColVal(int val);
		void setClipDistance(double val);
};

#endif // QT_INCLUDE_SCENE_CONTROL_HPP_
