#ifndef VISUALISATION_QT_INCLUDE_ARAP_CONTROLLER_HPP_
#define VISUALISATION_QT_INCLUDE_ARAP_CONTROLLER_HPP_

// Mesh, curve and ARAP structures :
#include "../../meshes/base_mesh/Mesh.hpp"
#include "../../meshes/deformable_curve/curve.hpp"
#include "../../meshes/operations/arap/Manipulator.h"
#include "../../meshes/operations/arap/AsRigidAsPossible.h"
#include "../../meshes/operations/arap/RectangleSelection.h"
#include "../../meshes/operations/arap/mesh_manip_interface.h"
// Image data :
#include "../../new_grid/include/grid.hpp"

#include <glm/glm.hpp>

#include <QLabel>
#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include <QListView>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

class Viewer;
class Scene;

class ARAPController : public QWidget {
	Q_OBJECT
public:
	/// @brief Default ctor for the controller. Initializes the viewer and scene fields and creates the widget.
	ARAPController(Viewer* main_viewer, Scene* _scene);
	/// @brief Default dtor. Declared as `default`.
	virtual ~ARAPController() = default;

	/// @brief Very simple state machine keeping track of which buttons should be activated
	enum States {
		Initialized					= 0,
		MeshLoaded					= 1,
		MeshLoadedWithConstraints	= 2,
		CurveLoaded					= 3,
		ImageLoaded					= 4,
		Deformed					= 5
	};

public:
	/// @brief Initializes the widget's various buttons, signals, and layouts.
	void init();

	/// @brief Set the state of the widget (dictates which buttons are activated)
	/// @note Called this way in order to not collide with any of the QWidget's functions.
	void setDeformationButtonsState(States state);
protected:
	/// @brief Creates the widget's layout.
	void initLayout();
	/// @brief Connects the widget's signals to other structures.
	void initSignals();

	/// @brief Called from setDeformationButtonsState(), this enables/disables the relevant buttons.
	void updateButtonsActivated();

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
	 * - Loading the data into the scene
	 * 		- Calls to the viewer to be made current
	 * 		- Then call the Scene functions
	 * 		- Update the Scene members
	 * 			- N.B. : leave the drawables in the scene
	 */

public:
	/// @brief Returns the currently loaded mesh.
	const Mesh::Ptr& getMesh() const;
	/// @brief Returns the currently loaded curve.
	const Curve::Ptr& getCurve() const;
	/// @brief Returns the currently loaded image.
	const Image::Grid::Ptr& getImage() const;

	/// @brief Accesses the internal ARAP manipulator.
	const std::shared_ptr<SimpleManipulator>& getARAPManipulator() const;
	/// @brief Accesses the internal mesh manipulation interface.
	const std::shared_ptr<MMInterface<glm::vec3>>& getMeshInterface() const;
	/// @brief Accesses the internal rectangle selection.
	const std::shared_ptr<RectangleSelection>& getRectangleSelection() const;

	/// @brief Returns the constraint that's currently being edited bu the user.
	/// @warning This returns a value in [1, #constraints] and 0 means no constraint is being edited right now !
	const size_t getCurrentlyEditedConstraint() const;
	/// @brief Get the currently loaded image constraints.
	const std::vector<glm::vec3>& getImageConstraints() const;
	/// @brief Get the currently loaded mesh constraints.
	const std::vector<std::size_t>& getMeshConstraints() const;
	/// @brief Get the compounded constraints.
	const std::vector<glm::vec3>& getCompoundedConstraints() const;

	/// @brief Updates the mesh data in the Scene.
	/// @note Since the curve is always deformed by the mesh, its update function is protected.
	void updateMeshDrawable();

	/// @brief Add a mesh constraint.
	void addMeshConstraint(std::size_t);
	/// @brief Add an image constraint.
	void addImageConstraint(glm::vec3);

public slots:
	/// @brief Sets the image pointer internally, whenever the loader widget has finished loading it.
	void setImagePointer(Image::Grid::Ptr&);

signals:
	/// @brief Called whenever the user pressed 'Load images'.
	void requestImageLoad();
	/// @brief Signal raised whenever the patient image is loaded in.
	void imageIsLoaded();
	/// @brief Signal raised whenever the mesh data is loaded in.
	void meshIsLoaded();
	/// @brief Signal raised whenever the curve data is loaded in.
	void curveIsLoaded();
	/// @brief Signal raised whenever the mesh data changed.
	void meshHasChanged();
	/// @brief Signal raised whenever the data contained in the curve changes.
	void curveHasChanged();

protected:
	/// @brief Slot called when the 'Load constraints' button is pressed.
	void loadConstraintDataFromFile(const std::string& file_name);

	/// @brief Uploads the mesh data to the Scene.
	void uploadMeshToScene();
	/// @brief Uploads the curve data to the Scene.
	void uploadCurveToScene();

	/// @brief Initializes the mesh interface, rectangle selection and ARAP manipulators.
	void initializeMeshInterface();
	/// @brief Resets the mesh interface, rectangle selection and ARAP manipulators.
	void resetMeshInterface();
	/// @brief Updates the curve data in the Scene.
	void updateCurveDrawable();

	/// @brief Updates the mesh info labels.
	void updateMeshInfoLabel();
	/// @brief Updates the grid info labels.
	void updateGridInfoLabel();

	/// @brief Used to reset the grid data from this class and from the Scene (and the GL context).
	void deleteGridData();
	/// @brief Used to reset the mesh data from this class and from the Scene (and the GL context).
	void deleteMeshData();
	/// @brief Used to reset the curve data from this class and from the Scene (and the GL context).
	void deleteCurveData();

protected slots:
	/// @brief Slot called when the 'Load Image' button is pressed.
	void loadImageFromFile();
	/// @brief Slot called when the 'Load mesh' button is pressed.
	void loadMeshFromFile();
	/// @brief Slot called when the 'Load constraints' button is pressed.
	void loadConstraintsFromFile();
	/// @brief Slot called when the 'Load Curve' button is pressed.
	void loadCurveFromFile();

	/// @brief Applies the computed (view-dependant) transformation to the mesh.
	void applyTransformation_Mesh();
	/// @brief Slot called whenever the mesh data is deformed, in order to update the curve positions.
	void updateCurveFromMesh();

	/// @brief Slot called whenever the program is in a state where the deformation can be enabled.
	/// @details Applies the transformation to the mesh and the curve, in order to enable real-time deformation
	/// of the loaded mesh data according to the user's wishes.
	void enableDeformation();
	/// @brief Slot called whenever the program cannot deform the loaded data.
	/// @details All transformations made when the deformation is not enabled will not be computed directly, but
	/// applied to the viewport and only applied when the deformation is re-enabled.
	void disableDeformation();
	/// @brief Slot called when the 'Align constraints' button is pressed.
	void arap_performAlignment();
	/// @brief Slot called when the 'Scale constraints' button is pressed.
	void arap_performScaling();
	/// @brief Slot called when the 'Compute deformation' button is pressed.
	void arap_computeDeformation();

	/// @brief Slot called whenever the 'Save mesh' button is pressed.
	void saveMesh();
	/// @brief Slot called whenever the 'Save curve' button is pressed.
	void saveCurve();

	/**
	 * Additionnal stuff :
	 * - Have some booleans keeping track of how the program data is handled/loaded/manipulated
	 * - Have the transformations be done only in-viewport until the 'Enable deformation' button is pressed.
	 */

protected:
	Viewer* viewer;	///< The viewer responsible for handling all the keyboard/mouse/draw events.
	Scene* scene;	///< The central repository of information (for now).

	Mesh::Ptr mesh;			///< The reference mesh, to deform.
	Curve::Ptr curve;		///< The curve of the reference mesh, to deform.
	Image::Grid::Ptr image;	///< The loaded patient image.

	std::shared_ptr<MMInterface<glm::vec3>> mesh_interface;	///< The deformation interface for the mesh.
	std::shared_ptr<SimpleManipulator> arapManipulator;		///< The manipulator for selected vertices.
	std::shared_ptr<RectangleSelection> rectangleSelection;	///< The rectangle user selection.

	std::vector<glm::vec3> image_constraints;	///< The image constraints (positions in 3D space).
	std::vector<std::size_t> mesh_constraints;	///< The mesh constraints (vertex indices).a
	std::vector<glm::vec3> compounded_constraints;	///< All of the constraints

	States state;	///< The current state of the application. Used to enable/disable buttons.

	/**
	 * 	------- LATER -------
	 * Buttons/sliders for :
	 * - Settings of the program
	 * 		- ARAP Iterations
	 * 		- Removing constraints
	 * 		- Modifying constraints
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

	QPushButton* button_manip_select_all;	///< Button to select all vertices of the mesh as handles.
	QPushButton* button_manip_select_none;	///< Button to unselect all vertices of the mesh.

	QCheckBox* checkbox_enable_deformation;	///< Checkbox which determines if the mesh can be deformed or not.

	QLabel* label_mesh_name;	///< The mesh filename.
	QLabel* label_mesh_info;	///< The mesh information (nb of vertices, constraints found or not and size on disk).
	QLabel* label_grid_name;	///< The grid name.
	QLabel* label_grid_info;	///< The grid info.

	QString dir_last_accessed;	///< The directory last accessed by the user.

	// Qt layout/widgets stuff :
	QListView* listview_mesh_constraints;	///< The list view of all mesh constraints.
	QListView* listview_image_constraints;	///< The list view of all image constraints.

};

#endif	  // VISUALISATION_QT_INCLUDE_ARAP_CONTROLLER_HPP_
