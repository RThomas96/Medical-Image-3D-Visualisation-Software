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
		void initSignals();
		void updateViewers(void);
	public slots:
		void activatePanels(bool activeStatus = true);
		void setImageBoundaries(int bounds[6]);
	private:
		Scene* const sceneToControl;
		QSlider* xTex;
		QSlider* yTex;
		QSlider* zTex;
		QSpinBox* minValueTexture; ///< Spinbox to determine the min value in the texture which constitutes viable information
		QSpinBox* maxValueTexture; ///< Spinbox to determine the max value in the texture which constitutes viable information
		QSlider* xPlane_Min; ///< Slider to determine the coordinate of the minimum X cutting plane
		QSlider* yPlane_Min; ///< Slider to determine the coordinate of the minimum Y cutting plane
		QSlider* zPlane_Min; ///< Slider to determine the coordinate of the minimum Z cutting plane
		QSlider* xPlane_Max; ///< Slider to determine the coordinate of the maximum X cutting plane
		QSlider* yPlane_Max; ///< Slider to determine the coordinate of the maximum Y cutting plane
		QSlider* zPlane_Max; ///< Slider to determine the coordinate of the maximum Z cutting plane
		QWidget* controlContainer;
		Viewer* leftViewer;
		Viewer* rightViewer;

	public slots:
		void setXTexCoord(int coordX);
		void setYTexCoord(int coordY);
		void setZTexCoord(int coordZ);
		void setTexCube(bool show);
		void setMinTexVal(int val);
		void setMaxTexVal(int val);
		void setCutPlaneX_Min(int val);
		void setCutPlaneY_Min(int val);
		void setCutPlaneZ_Min(int val);
		void setCutPlaneX_Max(int val);
		void setCutPlaneY_Max(int val);
		void setCutPlaneZ_Max(int val);
};

#endif // QT_INCLUDE_SCENE_CONTROL_HPP_
