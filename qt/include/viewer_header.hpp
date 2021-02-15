#ifndef VISUALISATION_QT_INCLUDE_VIEWER_HEADER_HPP_
#define VISUALISATION_QT_INCLUDE_VIEWER_HEADER_HPP_

#include <QIcon>
#include <QLabel>
#include <QWidget>
#include <QSlider>
#include <QPixmap>
#include <QPushButton>
#include <QHBoxLayout>

#include "../../viewer/include/neighbor_visu_viewer.hpp"

// Fwd-declaration
class PlanarViewer;

/// @b Header of a planar viewer, with the name of the plane controlled as well as a slider to control it
/// @details Will also include buttons to rotate the plane clockwise and counter-clockwise, as weel as include a button
/// to invert the plane's cutting direction
class ViewerHeader : public QWidget {
	Q_OBJECT
	public:
		/// @b Default constructor, creating an empty header
		ViewerHeader(QWidget* parent = nullptr);
		/// @b Constructs a header with a given name, and planar viewer to control
		ViewerHeader(std::string name, QWidget* parent = nullptr);
		/// @b Disconnects signals and destructs the object
		~ViewerHeader(void);
	public:
		void setName(const std::string _name);
		/// @b Allows to dynamically set a planar viewer to this header.
		void connectToViewer(PlanarViewer* _viewer);
		/// @b Removes the connections between this object's members and the planar viewer.
		void unregisterPlaneViewer(void);
	public slots:
		/**
		 * TODO here :
		 *	- slot to catch the slider signals → send as scalar in [0, 1] to planar viewer
		 *	- slot to catch the button signals OR maybe we can directly link them to planar viewer slots (?)
		 */
	protected:
		/// @b Announces this header will connect with the given planar viewer.
		/// @details This is the function that will setup the signals between the viewer and this widget.
		void registerWithViewer(void);
		/// @b Allows to dynamically activate or de-activate the widgets in this class.
		void activateWidgets(bool activated = true);
	protected:
		/// @b Layout of the current widget
		QHBoxLayout* layout;
		/// @b Viewer to control with the sliders and buttons
		PlanarViewer* viewerToControl;
		/// @b 3D viewer, to update whenever a signal is raised.
		Viewer* viewerToUpdate;
		/// @b Name of the plane controlled (could be 'X', or 'Transversal' for example)
		QLabel* label_PlaneName;
		/// @b Invert the plane's cutting direction
		QPushButton* button_invertPlaneCut;
		/// @b Button to rotate the cutting plane clockwise
		QPushButton* button_rotateClockwise;
		/// @b Button to rotate the cutting plane counter-clockwise
		QPushButton* button_togglePlane;
		/// @b Slider to control the cutting plane's depth
		QSlider* slider_planeDepth;
		/// @b Color for the widget's background
		QColor color;

		QIcon* icon_togglePlane_On;
		QIcon* icon_togglePlane_Off;
		QIcon* icon_invertPlane;
		QIcon* icon_rotatePlane;
};

class ViewerHeader3D : public QWidget {
		Q_OBJECT
	public:
		/// @b Default constructor, creating an empty header
		ViewerHeader3D(QWidget* parent = nullptr);
		/// @b Constructs a header with a given name, and planar viewer to control
		ViewerHeader3D(Viewer* _viewer, Scene* _scene, QWidget* parent = nullptr);
		/// @b Default dtor.
		~ViewerHeader3D(void);
	public:
		void setupWidgets();
		void setupSignals();
	protected:
		/// @b Layout of the current widget
		QHBoxLayout* layout;
		/// @b Viewer to control with the sliders and buttons
		Scene* sceneToControl;
		/// @b 3D viewer, to update whenever a signal is raised.
		Viewer* viewerToUpdate;
		/// @b Invert the plane's cutting direction
		QPushButton* button_invertPlaneCut;
		/// @b Button to rotate the cutting plane counter-clockwise
		QPushButton* button_togglePlane;
		/// @b Button to reset the visu box
		QPushButton* button_centerCamera;
		/// @b Button to set solid drawing
		QPushButton* button_setSolid;
		/// @b Button to set volumetric mode
		QPushButton* button_setVolumetric;
		/// @b Button to set boxed view
		QPushButton* button_setVolumetricBoxed;
		/// @b Color for the widget's background
		Qt::GlobalColor color;
		/// @b Label for "All planes"
		QLabel* label_allPlanes;
		/// @b Separator for the header
		QFrame* separator;
		/// @b Icon to show the planes
		QIcon* icon_show;
		/// @b Icon to hide the planes
		QIcon* icon_hide;
		/// @b Icon to invert the planes
		QIcon* icon_invert;
		/// @b icon for the solid draw mode
		QIcon* icon_solid;
		/// @b icon for the volumetric draw mode
		QIcon* icon_volumetric;
		/// @b icon for the volumetric boxed draw mode
		QIcon* icon_volumetric_boxed;
};

#endif // VIEWER_HEADER_HPP
