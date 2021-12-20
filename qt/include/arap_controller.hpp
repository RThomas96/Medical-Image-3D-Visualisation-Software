#ifndef VISUALISATION_QT_INCLUDE_ARAP_CONTROLLER_HPP_
#define VISUALISATION_QT_INCLUDE_ARAP_CONTROLLER_HPP_

#include "../../meshes/base_mesh/Mesh.hpp"
#include "../../meshes/deformable_curve/curve.hpp"
#include "../../meshes/operations/arap/Manipulator.h"
#include "../../meshes/operations/arap/AsRigidAsPossible.h"
#include "../../meshes/operations/arap/RectangleSelection.h"
#include "../../meshes/operations/arap/mesh_manip_interface.h"

#include <glm/glm.hpp>

#include <QLabel>
#include <QWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include <QListView>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

class Viewer;
class Scene;

class ARAPController : public QWidget {
public:
	ARAPController(Viewer* main_viewer, Scene* _scene);
	virtual ~ARAPController() = default;

public:
	void init();
protected:
	void initLayout();
	void initSignals();

	/**
	 * Functions for :
	 * - loading the mesh
	 * 		- loading the mesh itself from a file
	 * 		- loading the constraints, if found
	 * 		- re-loading the mesh from file
	 * - Interacting with constraints
	 * 		- Getting cursor position from Viewer
	 * 		- Adding mesh constraints
	 * 		- Adding image constraints
	 * 		- Modifying a constraint
	 * 		- Selecting a mesh constraint
	 * 		- Deleting a mesh constraint
	 * 		- Deleting an image constraint
	 * - Launching the deformation
	 * 		- Performing the first alignment of the mesh
	 * 		- Performing the rough scaling of the mesh
	 * 		- Performing the ARAP deformation itself
	 * - Saving the generated data
	 * 		- Saving the mesh to a file
	 * 		- Saving the curve to a file
	 */

public:
	/// @brief Accesses the internal ARAP manipulator.
	const std::shared_ptr<SimpleManipulator>& getARAPManipulator() const;
	/// @brief Accesses the internal mesh manipulation interface.
	const std::shared_ptr<MMInterface<glm::vec3>>& getMeshInterface() const;
	/// @brief Accesses the internal rectangle selection.
	const std::shared_ptr<RectangleSelection>& getRectangleSelection() const;

	/// @brief Returns the constraint that's currently being edited bu the user.
	std::size_t getCurrentlyEditedConstraint() const;
	/// @brief Get the currently loaded image constraints.
	std::vector<glm::vec4> getImageConstraints() const;
	/// @brief Get the currently loaded mesh constraints.
	std::vector<std::size_t> getMeshConstraints() const;

protected:
	/// @brief Uploads the mesh data to the Scene.
	void uploadMeshToScene();
	/// @brief Uploads the curve data to the Scene.
	void uploadCurveToScene();
	/// @brief Updates the mesh data in the Scene.
	void updateMeshDrawable();
	/// @brief Updates the curve data in the Scene.
	void updateCurveDrawable();

protected slots:
	/// @brief Slot called when the 'Load Image' button is pressed.
	void loadImageFromFile();
	/// @brief Slot called when the 'Load mesh' button is pressed.
	void loadMeshFromFile();
	/// @brief Slot called when the 'Load constraints' button is pressed.
	void loadConstraintsFromFile();

	/**
	 * Additionnal stuff :
	 * - Have some booleans keeping track of how the program data is handled/loaded/manipulated
	 * - Have the transformations be done only in-viewport until the 'Enable deformation' button is pressed.
	 */

protected:
	Viewer* viewer;	///< The viewer responsible for handling all the keyboard/mouse/draw events.
	Scene* scene;	///< The central repository of information.

	// TODO : add an image pointer here.
	Mesh::Ptr mesh;		///< The reference mesh.
	Curve::Ptr curve;	///< The curve going through the patient mesh.

	std::shared_ptr<MMInterface<glm::vec3>> mesh_interface;	///< The deformation interface for the mesh.
	std::shared_ptr<SimpleManipulator> arapManipulator;		///< The manipulator for selected vertices.
	std::shared_ptr<RectangleSelection> rectangleSelection;	///< The rectangle user selection.

	/**
	 * Buttons for :
	 * - loading the data
	 * 		- Loading the image
	 * 		- Loading the mesh
	 * 		- Possibly loading the constraints ???
	 * - Saving the data
	 * 		- Saving meshes to a file
	 * 		- Saving meshes to a file
	 * - Loading the data into the scene
	 * 		- Calls to the viewer to be made current
	 * 		- Then call the Scene functions
	 * 		- Update the Scene members
	 * 			- N.B. : leave the drawables in the scene
	 * - Launching the deformation
	 * 		- Performing the first alignment of the mesh
	 * 		- Performing the rough scaling of the mesh
	 * 		- Performing the ARAP deformation itself
	 *
	 * 	------- LATER -------
	 *
	 * - Settings of the program
	 * 		- ARAP Iterations
	 * 		- Removing constraints
	 */

	QPushButton* button_load_image;			///< Button to load an image from a file.
	QPushButton* button_load_mesh;			///< Button to load a mesh from a file.
	QPushButton* button_load_constraints;	///< Button to attempt loading the constraints file.
	QPushButton* button_load_curve;			///< Button to load a curve from file.

	QPushButton* button_save_mesh;	///< Button to save the mesh to a file.
	QPushButton* button_save_curve;	///< Button to save the curve to a file.

	QPushButton* button_align_arap;	///< Button to align the control points of the ARAP deformation.
	QPushButton* button_scale_arap;	///< Button to scale roughly the two BB of the control points of the ARAP deformation.
	QPushButton* button_start_arap;	///< Button to start the ARAP deformation using the currently available ARAP constraints.

	QLabel* label_mesh_name;	///< The mesh filename.
	QLabel* label_mesh_info;	///< The mesh information (nb of vertices, constraints found or not and size on disk).
	QLabel* label_grid_name;	///< The grid name.
	QLabel* label_grid_info;	///< The grid info.

	QString dir_last_accessed;	///< The directory last accessed by the user.

	QGridLayout* widget_layout;	///< The layout of this widget.

	// Qt layout/widgets stuff :
	QListView* listview_mesh_constraints;	///< The list view of all mesh constraints.
	QListView* listview_image_constraints;	///< The list view of all image constraints.

};

#endif	  // VISUALISATION_QT_INCLUDE_ARAP_CONTROLLER_HPP_
