#ifndef QT_INCLUDE_GRID_CONTROL_HPP_
#define QT_INCLUDE_GRID_CONTROL_HPP_

#include "../../grid/include/voxel_grid.hpp"

#include <glm/glm.hpp>

#include <QLabel>
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

#include <memory>
#include <iostream>

class GridControl : public QWidget {
		Q_OBJECT
	public:
		GridControl(VoxelGrid* vxg, QWidget* parent = nullptr); ///< Default constructor
		~GridControl(void); ///< Default destructor
		void setVoxelGrid(std::shared_ptr<VoxelGrid> vg);

	public slots:
		/// @brief Upates the labels on the grid controller.
		void updateGridLabels(void);
		/// @brief Updates the grid dimensions from the grid stored in memory.
		void updateGridDimensions(void);
		/// @brief Updates the method picked for interpolation.
		void pickMethod(int m);
		/// @brief Asks the user if it wants to launch the grid filling operation, and then does it if it's okay.
		void launchGridFill(void);
		/// @brief Saves the grid to a file. Currently, only DIM/IMA writers are tested.
		void saveToFile(void);

	protected:
		void setupWidgets();
		void setupSignals();
		void setupSpinBoxBounds(QSpinBox* sb);
		void setupDoubleSpinBoxBounds(QDoubleSpinBox* dsb);
		InterpolationMethods method; ///< The method with which to interpolate the grid
		QSpinBox* input_GridSizeX; ///< Controls the grid size along X
		QSpinBox* input_GridSizeY; ///< Controls the grid size along Y
		QSpinBox* input_GridSizeZ; ///< Controls the grid size along Z
		QComboBox* methodPicker; ///< Picker for the interpolation method.
		QLabel* info_GridSizeTotal; ///< Displays the total number of voxels in the grid
		QLabel* info_VoxelSize; ///< Displays the size of voxels, in units.
		QPushButton* button_FillButton; ///< Button to start the process of filling the grid
		QPushButton* button_SaveButton; ///< Button to save the grid, triggers a text dialog and saves the file.
		QDoubleSpinBox* input_GridBBMinX; ///< Controls the coordinates of the minimum bound of the grid's render bounding box along X
		QDoubleSpinBox* input_GridBBMinY; ///< Controls the coordinates of the minimum bound of the grid's render bounding box along Y
		QDoubleSpinBox* input_GridBBMinZ; ///< Controls the coordinates of the minimum bound of the grid's render bounding box along Z
		QDoubleSpinBox* input_GridBBMaxX; ///< Controls the coordinates of the maximum bound of the grid's render bounding box along X
		QDoubleSpinBox* input_GridBBMaxY; ///< Controls the coordinates of the maximum bound of the grid's render bounding box along Y
		QDoubleSpinBox* input_GridBBMaxZ; ///< Controls the coordinates of the maximum bound of the grid's render bounding box along Z
		std::shared_ptr<VoxelGrid> voxelGrid; ///< Voxel grid to control.

	private:
		QLabel* info_TotalTime; ///< Total time it took to fill the grid
		QLabel* info_VoxelRate; ///< Rate of filling, in gigavoxels/hour.
		QLabel* info_MemorySize; ///< Size of the voxel grid, in GB.
		void updateDebugInfoFields(void);
};

#endif // QT_INCLUDE_GRID_CONTROL_HPP_
