#ifndef VISUALISATION_QT_INCLUDE_VISU_BOX_CONTROLLER_HPP_
#define VISUALISATION_QT_INCLUDE_VISU_BOX_CONTROLLER_HPP_

#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>

class Scene; // Fwd-decl

class VisuBoxController : public QWidget {
	Q_OBJECT
	public:
		VisuBoxController(Scene* _scene);
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
		QDoubleSpinBox* input_BBMinX;	///< Controls the min point's X coordinate
		QDoubleSpinBox* input_BBMinY;	///< Controls the min point's Y coordinate
		QDoubleSpinBox* input_BBMinZ;	///< Controls the min point's Z coordinate
		QDoubleSpinBox* input_BBMaxX;	///< Controls the max point's X coordinate
		QDoubleSpinBox* input_BBMaxY;	///< Controls the max point's Y coordinate
		QDoubleSpinBox* input_BBMaxZ;	///< Controls the max point's Z coordinate
		QPushButton* button_resetBox;	///< Resets the visu box coordinates
		std::vector<QObject*> strayObj;	///< Stray objects created in the widget's setup process
};

#endif // VISUALISATION_QT_INCLUDE_VISU_BOX_CONTROLLER_HPP_
