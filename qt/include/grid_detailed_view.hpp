#ifndef GRID_INFO_VIEW_HPP
#define GRID_INFO_VIEW_HPP

#include "../../macros.hpp"
#include "../../features.hpp"

#include "./grid_list_view.hpp"
#include "../../grid/include/discrete_grid.hpp"

#include "./qt_proxies.hpp"

#include <QLabel>
#include <QFrame>
#include <QLineEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <vector>
#include <array>
#include <variant>

class GridDetailedView : public QWidget {
	public:
		/// @brief Creates an empty detailed view of a grid.
		GridDetailedView(void);
		/// @brief Removes all objects and frees allocated memory.
		~GridDetailedView(void);
	public:
		/// @brief Sets most of the fields to an editable state.
		void setEditable();
		/// @brief Sets most of the fields to an immutable state.
		/// @details The grid name field will not be set to read only, however the rest of the fields will.
		void setNonEditable();
		/// @brief Updates the signals to target the currently selected grid.
		void updateSignals(void);
	public slots:
		/// @brief Shows the information of the given grid.
		void showGrid(GridView* caller, const std::shared_ptr<DiscreteGrid>&);
		/// @brief Signals for the QLineEdit send QStrings, not std::strings, tihs converts them.
		void proxy_ChangeGridName(const QString& newName);
	protected:
		/// @brief Creates all the QWidgets for this UI.
		void createWidgets(void);
		/// @brief Deletes all the QWidgets for this UI.
		void deleteWidgets(void);
		/// @brief Disconnects all signals from the UI.
		void disconnectSignals(void);
		/// @brief Blocks all signals from the elements present in this UI.
		void blockEverySignal(bool _block = true);
		/// @brief Updates the panel's values with the grid's data.
		void updateValues(void);
	protected:
		///@brief The grid to display information from, set to nullptr at construction.
		std::shared_ptr<DiscreteGrid> grid;
		/// @brief The proxy to the bounding box, to control it with signals from QT
		std::shared_ptr<Proxies::BoundingBox> proxy_boundingBox;
		/// @brief Proxy for the grid resolution
		std::shared_ptr<Proxies::Resolution> proxy_gridResolution;
		/// @brief The list item which requested to show a grid
		GridView* gridListItem;

		QVBoxLayout* layout_leftPane; ///< The layout holding the left panel of the UI : the grid details.
		QVBoxLayout* layout_rightPane; ///< Layout holding the right panel of the UI.
		QHBoxLayout* layout_sideBySide; ///< Layout holding the grid details and viewer/grid list.
		QVBoxLayout* layout_widget; ///< Layout for the entire widget.

		QFrame* layout_separator; ///< Separator between the two panes of the widget's UI.

		QGroupBox* groupBox_gridDetails; ///< Group box holding the grid details.
		QGroupBox* groupBox_viewer; ///< Group box holding the viewer to the side (can also be grid picker).
		QGroupBox* groupBox_boundingBox; ///< Group box holding the bounding box data.
		QGroupBox* groupBox_transformation; ///< The group box holding the transformation picker.
		QGroupBox* groupBox_transformationDetails; ///< The group box with the transformation parameters.

		QLabel* label_headerDetailedView; ///< Header for the field.
		QLabel* label_headerGridName; ///< Header for the grid name.
		QLabel* label_headerGridResolution; ///< Header for the grid's size.
		QLabel* label_headerTransformation; ///< Label to show in the transformation box.
		QLabel* label_gridResolution; ///< Label holding the grid resolution.
		std::array<QLabel*, 6> label_BBControls; ///< Labels for the editable bounding box controls.

		QLineEdit* editable_gridName; ///< Text field to edit the grid's name.
		std::array<QSpinBox*, 3> editable_GridResolution; ///< Controls for the grid resolution.
		std::array<QDoubleSpinBox*, 6> editable_BBControls; ///< Controls for the grid's bounding box.

		std::vector<QLayout*> otherBoxLayouts; ///< Stray box layouts we had to create

#ifdef GRID_DETAILS_ENABLE_TRANSFORMATION_PICKER
		QComboBox* picker_transformationStyle; ///< The style of transformation requested by the user
#endif

#ifdef GRID_DETAILS_ENABLE_TRANSFORMATION_DETAILS
		SomeOtherClass* widget_transformationViewer; ///< The viewer and picker for the transformation
#endif
#ifdef GRID_DETAILS_ENABLE_SIDE_PANEL
		SomeClass* widget_SideWidget; ///< The viewer of the stack or picker of input grids
#endif
};

#endif // GRID_INFO_VIEW_HPP
