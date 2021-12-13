#ifndef VISUALISATION_QT_INCLUDE_VIEWER_HEADER_HPP_
#define VISUALISATION_QT_INCLUDE_VIEWER_HEADER_HPP_

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QWidget>

#include "../../viewer/include/neighbor_visu_viewer.hpp"

// Fwd-declaration
class PlanarViewer;

/// @ingroup qtwidgets
/// @brief Header of a planar viewer, with the name of the plane controlled as well as a slider to control it
/// @details Will also include buttons to rotate the plane clockwise and counter-clockwise, as weel as include a button
/// to invert the plane's cutting direction
class ViewerHeader : public QWidget {
	Q_OBJECT
public:
	/// @brief Default constructor, creating an empty header
	ViewerHeader(QWidget* parent = nullptr);
	/// @brief Constructs a header with a given name, and planar viewer to control
	ViewerHeader(std::string name, QWidget* parent = nullptr);
	/// @brief Disconnects signals and destructs the object
	~ViewerHeader(void);

public:
	void setName(const std::string _name);
	/// @brief Allows to dynamically set a planar viewer to this header.
	void connectToViewer(PlanarViewer* _viewer);
	/// @brief Removes the connections between this object's members and the planar viewer.
	void unregisterPlaneViewer(void);
public slots:
	/**
		 * TODO here :
		 *	- slot to catch the slider signals → send as scalar in [0, 1] to planar viewer
		 *	- slot to catch the button signals OR maybe we can directly link them to planar viewer slots (?)
		 */
protected:
	/// @brief Announces this header will connect with the given planar viewer.
	/// @details This is the function that will setup the signals between the viewer and this widget.
	void registerWithViewer(void);
	/// @brief Allows to dynamically activate or de-activate the widgets in this class.
	void activateWidgets(bool activated = true);

protected:
	/// @brief Layout of the current widget
	QHBoxLayout* layout;
	/// @brief Viewer to control with the sliders and buttons
	PlanarViewer* viewerToControl;
	/// @brief 3D viewer, to update whenever a signal is raised.
	Viewer* viewerToUpdate;
	/// @brief Name of the plane controlled (could be 'X', or 'Transversal' for example)
	QLabel* label_PlaneName;
	/// @brief Invert the plane's cutting direction
	QPushButton* button_invertPlaneCut;
	/// @brief Button to rotate the cutting plane clockwise
	QPushButton* button_rotateClockwise;
	/// @brief Button to rotate the cutting plane counter-clockwise
	QPushButton* button_togglePlane;
	/// @brief Slider to control the cutting plane's depth
	QSlider* slider_planeDepth;
	/// @brief Color for the widget's background
	QColor color;

	QIcon* icon_togglePlane_On;
	QIcon* icon_togglePlane_Off;
	QIcon* icon_invertPlane;
	QIcon* icon_rotatePlane;
};

/// @ingroup qtwidgets
/// @brief Provides the same functionnality as the ViewerHeader class, but with more specialized buttons for the 3D
/// viewport.
/// @details Provides buttons to control the visualization mode, as well as a button to hide all planes at once.
class ViewerHeader3D : public QWidget {
	Q_OBJECT
public:
	/// @brief Default constructor, creating an empty header
	ViewerHeader3D(QWidget* parent = nullptr);
	/// @brief Constructs a header with a given name, and planar viewer to control
	ViewerHeader3D(Viewer* _viewer, Scene* _scene, QWidget* parent = nullptr);
	/// @brief Default dtor.
	~ViewerHeader3D(void);

public:
	void setupWidgets();
	void setupSignals();

protected:
	/// @brief Layout of the current widget
	QHBoxLayout* layout;
	/// @brief Viewer to control with the sliders and buttons
	Scene* sceneToControl;
	/// @brief 3D viewer, to update whenever a signal is raised.
	Viewer* viewerToUpdate;
	/// @brief Invert the plane's cutting direction
	QPushButton* button_invertPlaneCut;
	/// @brief Button to rotate the cutting plane counter-clockwise
	QPushButton* button_togglePlane;
	/// @brief Button to reset the visu box
	QPushButton* button_centerCamera;
	/// @brief Button to set solid drawing
	QPushButton* button_setSolid;
	/// @brief Button to set volumetric mode
	QPushButton* button_setVolumetric;
	/// @brief Button to set boxed view
	QPushButton* button_setVolumetricBoxed;
	/// @brief Color for the widget's background
	Qt::GlobalColor color;
	/// @brief Label for "All planes"
	QLabel* label_allPlanes;
	/// @brief Separator for the header
	QFrame* separator;
	/// @brief Icon to show the planes
	QIcon* icon_show;
	/// @brief Icon to hide the planes
	QIcon* icon_hide;
	/// @brief Icon to invert the planes
	QIcon* icon_invert;
	/// @brief icon for the solid draw mode
	QIcon* icon_solid;
	/// @brief icon for the volumetric draw mode
	QIcon* icon_volumetric;
	/// @brief icon for the volumetric boxed draw mode
	QIcon* icon_volumetric_boxed;
};

#endif	  // VIEWER_HEADER_HPP
