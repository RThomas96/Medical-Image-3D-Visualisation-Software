#ifndef GRID_LIST_VIEW_HPP
#define GRID_LIST_VIEW_HPP

#include "../../macros.hpp"
#include "../../features.hpp"

#include "../../grid/include/discrete_grid.hpp"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

#include <vector>
#include <memory>

class GridDetailedView; // [Fwd-declaration]

/// @ingroup qtwidgets
/// @brief This was supposed to be a list of loaded grids that we could interact with. Never used.
/// @warning Legacy code. Do not use directly.
/// @note For a more convenient way to do this, see Qt's help page on Model-View architecture.
class GridView : public QWidget {
	public:
		using resolution_t = DiscreteGrid::sizevec3; ///< Typedef for the size vector of DiscreteGrid.
	public:
		/// @brief Basic constructor linking this grid view to a grid, and to the detailed view widget.
		GridView(GridDetailedView* const _details, const std::shared_ptr<DiscreteGrid>& _grid);
		/// @brief Destroys the grid view, and frees up any allocated memory.
		~GridView(void);

	public:
		/// @brief Reads the properties of the associated grid and populates its fields accordingly.
		void readValuesFromGrid(void);
		/// @brief Sets up the connection to the grid 'detailed' view.
		void setupWidgetConnections(void);

	protected:
		/// @brief Blocks every signal from any QObject in this class.
		void blockEverySignal(bool block = true);
		/// @brief Takes a resolution and outputs a string detailing the resolution.
		/// @details Given a resolution (a vec3 from GLM), it outputs a string of the following format :
		/// " size_X x size_Y x size_Z, or XYZ [unit]voxels ". XYZ will be scaled to show only 3 decimal points
		/// of precision, modifying the unit shown accordingly.
		/// @returns A printable, formatted string detailing the resolution of the grid.
		std::string resolutionToString(const resolution_t& _res) const;

	private:
		/// @brief Places the widgets on the grid, in order to display them later.
		void placeWidgetsOnGrid(void);

	signals:
		/// @brief Signals the user's request to view/modify the grid data of this grid view.
		void gridModifyRequest(GridView* _caller, const std::shared_ptr<DiscreteGrid>&);

	public slots:
		/// @brief Request to update the labels on this widget to the grid's latest information.
		void updateValues(void);
		/// @brief Proxy to handle a grid modification request.
		void proxy_viewGrid(void);

	protected:
		/// @brief Deletes and frees up the memory allocated by all the QObjects of this class.
		void deleteQObjects(void);

	protected:
		QLabel* label_gridType; ///< Displays whether the grid is an input grid or output grid.
		QLabel* label_gridName; ///< Label showing the grid name
		QLabel* label_gridDiskSize; ///< Label showing the grid's size on disk (projected, or real)
		QLabel* label_gridResolution; ///< Label showing the grid's resolution.
		QLabel* label_gridImageCount; ///< Label showing the number of files composing the image stack.

		QLabel* label_headerName; ///< Header for label_gridName.
		QLabel* label_headerDiskSize; ///< Header for label_gridDiskSize
		QLabel* label_headerResolution; ///< Header for label_gridResolution.
		QLabel* label_headerImageCount; ///< Header for label_gridImageCount.

		QPushButton* button_modifyGrid; ///< Button to modify the grid.
		QGridLayout* gridLayout; ///< Layout to position the items on the widget.
		GridDetailedView* detailedView; ///< Detailed view of the grid (another panel in the UI).

		const std::shared_ptr<DiscreteGrid> grid; ///< Pointer to the grid we want to display info from.
};

#endif // GRID_LIST_VIEW_HPP
