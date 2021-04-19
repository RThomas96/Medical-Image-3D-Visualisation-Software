#ifndef VISUALISATION_QT_INCLUDE_VISU_BOX_CONTROLLER_HPP_
#define VISUALISATION_QT_INCLUDE_VISU_BOX_CONTROLLER_HPP_

#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>

class Scene; // Fwd-decl
class MainWidget; // Fwd-decl

class VisuBoxController : public QWidget {
	Q_OBJECT
	public:
		VisuBoxController(Scene* _scene, MainWidget* _widget);
		~VisuBoxController(void);
	public:
		void updateValues();
	protected:
		void blockSignals(bool block = true);
		void setupWidgets(void);
		void setupSignals(void);
		void updateBox(void);
	protected:
		Scene* scene;			///< The scene to control
		MainWidget* main;		///< The main widget to call
		QSpinBox* input_coordMinX;
		QSpinBox* input_coordMinY;
		QSpinBox* input_coordMinZ;
		QSpinBox* input_coordMaxX;
		QSpinBox* input_coordMaxY;
		QSpinBox* input_coordMaxZ;
		QPushButton* button_resetBox;	///< Resets the visu box coordinates
		QPushButton* button_loadROI;	///< Loads the defined coordinates as the current image
		std::vector<QObject*> strayObj;	///< Stray objects created in the widget's setup process
};

#endif // VISUALISATION_QT_INCLUDE_VISU_BOX_CONTROLLER_HPP_
