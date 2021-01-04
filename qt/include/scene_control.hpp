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
		ControlPanel(Scene* const scene, Viewer* lv, Viewer* rv, QWidget* parent = nullptr);
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
		QDoubleSpinBox* xPlanePos; ///< Box to determine the coordinate of the minimum X cutting plane
		QDoubleSpinBox* yPlanePos; ///< Box to determine the coordinate of the minimum Y cutting plane
		QDoubleSpinBox* zPlanePos; ///< Box to determine the coordinate of the minimum Z cutting plane
		QWidget* controlContainer;
		Viewer* leftViewer;
		Viewer* rightViewer;

	public slots:
		void setTexCube(bool show);
		void setMinTexVal(int val);
		void setMaxTexVal(int val);
		void setCutPlaneXPos(double val);
		void setCutPlaneYPos(double val);
		void setCutPlaneZPos(double val);
};

#endif // QT_INCLUDE_SCENE_CONTROL_HPP_
