#ifndef QT_INCLUDE_GRID_CONTROL_HPP_
#define QT_INCLUDE_GRID_CONTROL_HPP_

#include "../../macros.hpp"
#include "../../features.hpp"

#include "../../grid/include/tetmesh.hpp"
#include "../../grid/include/discrete_grid.hpp"

#include <glm/glm.hpp>

#include <QDir>
#include <QLabel>
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QRadioButton>

#include <memory>
#include <iostream>

#define ENABLE_SINGLE_DIALOGBOX

class Scene; // Fwd-declaration

/// @brief Simple widget allowing the control (if possible) of a grid's resolution, voxel sizes and bounding box.
/// @note In this class, the bounding box controlled will always be defined as existing within grid space. It makes no
/// sense to try and convert to world space the bounding box defined by the spinboxes, to then re-convert this bounding
/// box into grid space, which would give another bounding volume. As such, this will only be defined in grid space.
/// @note Since the bounding box is defined in grid space, then the TetMesh that is responsible for the computation of
/// the output grid will also go through the grid in grid space.
class GridControl : public QWidget {
		Q_OBJECT
	public:
		/// @brief Default constructor of the class, providing a valid scene pointer.
		GridControl(std::shared_ptr<DiscreteGrid> _vg, std::shared_ptr<TetMesh> _tm, Scene* _scene, QWidget* parent = nullptr);
		/// @brief Default destructor of the class. Removes the output grid and unregisters itself from the scene.
		~GridControl(void);
		/// @brief Ovverides the default show event in order to lock its size, disabling later resize events.
		virtual void showEvent(QShowEvent* _e) override;

	public slots:
		/// @brief Update the values and labels of this widget in order to be synced with the grid controlled.
		void updateValues(void);
		/// @brief Updates the method picked for interpolation.
		void pickMethod(int m);
		/// @brief Triggers the TetMesh::populateOutputGrid() function.
		void launchGridFill(void);
		/// @brief Saves the grid to a file, after having populated it (GridControl::launchGridFill()).
		void saveToFile(void);
		/// @brief Updates the grid's resolution whenever a spinbox for it changes.
		void setGridResolution(void);
		/// @brief Updates the grid's voxel sizes whenever a spinbox for it changes.
		void setGridVoxelSize(void);
		/// @brief Updates the grid's bounding box whenever a spinbox for it changes.
		void setGridBoundingBox(void);

	protected:
		/// @brief Allows to block all signals from this widget in one fell swoop.
		void blockSignals(bool blocked = true);
		/// @brief Sets up the widgets and the layout of the window.
		void setupWidgets();
		/// @brief Connects the different spinboxes to the functions to modify the grid.
		void setupSignals();
		/// @brief Enables each field in the window, allowing it to modify a grid.
		void enableWidgets();
		/// @brief Disables each field in the window, disallowing it to modify a grid.
		void disableWidgets();
		/// @brief Sets up the bounds of a QSpinBox object.
		void setupSpinBoxBounds(QSpinBox* sb);
		/// @brief Sets up the bounds of a QDoubleSpinBox object. If second param is true, starts at 0.
		void setupDoubleSpinBoxBounds(QDoubleSpinBox* dsb, bool lowOrMin);

	protected:
		// Different output grids available to the user :
		std::shared_ptr<OutputGrid> output_R;
		std::shared_ptr<OutputGrid> output_B;
		std::shared_ptr<OutputGrid> output_RGB;
		// Different interpolation structures available to the user :
		std::shared_ptr<TetMesh> interpolator_R;
		std::shared_ptr<TetMesh> interpolator_G;
		std::shared_ptr<TetMesh> interpolator_RGB;

		// Variables retrieved from the constructor :
		std::shared_ptr<DiscreteGrid> voxelGrid; ///< Voxel grid to control.
		std::shared_ptr<TetMesh> mesh; ///< Tetrahedral mesh responsible for the generation of the grid
		Scene* scene; ///< The scene to control.

		// Qt stuff :
		InterpolationMethods method; ///< The method with which to interpolate the grid
		QSpinBox* input_GridSizeX; ///< Controls the grid size along X
		QSpinBox* input_GridSizeY; ///< Controls the grid size along Y
		QSpinBox* input_GridSizeZ; ///< Controls the grid size along Z
		QDoubleSpinBox* input_VoxelSizeX; ///< Controls the voxel size on X
		QDoubleSpinBox* input_VoxelSizeY; ///< Controls the voxel size on Y
		QDoubleSpinBox* input_VoxelSizeZ; ///< Controls the voxel size on Z
		QComboBox* methodPicker; ///< Picker for the interpolation method.
		QLabel* info_GridSizeTotal; ///< Displays the total number of voxels in the grid
		QLabel* info_VoxelSize; ///< Displays the size of voxels, in units.

		QDir baseDir; ///< User save dir, by default set to QDir::homePath()
		QPushButton* button_modifyBaseDir;
		QLabel* label_baseDir;
		QLineEdit* lineEdit_baseName;
		QLabel* label_baseName;
		QComboBox* comboBox_filetype;

		QPushButton* button_SaveButton; ///< Button to save the grid, triggers a text dialog and saves the file.
		QDoubleSpinBox* input_GridBBMinX; ///< Controls the coordinates of the minimum bound of the grid's render bounding box along X
		QDoubleSpinBox* input_GridBBMinY; ///< Controls the coordinates of the minimum bound of the grid's render bounding box along Y
		QDoubleSpinBox* input_GridBBMinZ; ///< Controls the coordinates of the minimum bound of the grid's render bounding box along Z
		QDoubleSpinBox* input_GridBBMaxX; ///< Controls the coordinates of the maximum bound of the grid's render bounding box along X
		QDoubleSpinBox* input_GridBBMaxY; ///< Controls the coordinates of the maximum bound of the grid's render bounding box along Y
		QDoubleSpinBox* input_GridBBMaxZ; ///< Controls the coordinates of the maximum bound of the grid's render bounding box along Z
		QGroupBox* groupBox_outputType; ///< Decides on the grid output type. Either single channel 2 stacks, or RGB
		QRadioButton* radioButton_2stacks; ///< Button to choose the output type as being two stacks
		QRadioButton* radioButton_rgb; ///< Button to choose the output type as one RGB stack
		std::vector<QObject*> strayObj; ///< All temporary objects allocated for the viewer, that need to be deleted

		#ifdef ENABLE_SINGLE_DIALOGBOX
		QMessageBox* dialogBox;
		#endif

	private:
		QLabel* info_TotalTime; ///< Total time it took to fill the grid
		QLabel* info_VoxelRate; ///< Rate of filling, in gigavoxels/hour.
		QLabel* info_MemorySize; ///< Size of the voxel grid, in GB.
		/// @b Updates the fields relating to the time/size/memory footprint of the generated grid.
		void updateDebugInfoFields(void);
};

#endif // QT_INCLUDE_GRID_CONTROL_HPP_
