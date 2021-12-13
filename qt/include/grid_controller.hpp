#ifndef QT_INCLUDE_OFFLINE_RENDERER_WIDGET_HPP_
#define QT_INCLUDE_OFFLINE_RENDERER_WIDGET_HPP_

#include "../../features.hpp"
#include "../../macros.hpp"

#include "../../grid/include/discrete_grid.hpp"
//#include "../../grid/include/grid_pool.hpp"
#include "./grid_detailed_view.hpp"
#include "./grid_list_view.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolBar>
#include <QWidget>

#include <memory>
#include <vector>

/// @ingroup qtwidgets
/// @brief This was supposed to be a widget where we could see all the loaded grids, and their details.
/// @warning Legacy code, never used. Do not use directly.
/// @note For a more convenient way to do this, see Qt's help page on Model-View architecture.
class GridController : public QMainWindow {
public:
	/// @brief Constructor of the offline grid generation widget.
	GridController(const std::shared_ptr<GridPool> _pool) :
		gridPool(_pool) {}

	~GridController(void);

public:
	/// @brief A proxy function allowing to create an input grid.
	void proxy_createInputGrid();
	/// @brief A proxy function allowing to create an output grid.
	void proxy_createOutputGrid();

protected:
	QGridLayout* mainLayout;	///< Main layout containing the UI

	QToolBar* menuBar;	  ///< The menu bar of the program
	QStatusBar* statusBar;	  ///< Status bar for the widget

	QScrollArea* view_gridList;	   ///< The scroll area where grids are displayed.
	std::vector<GridView*> view_gridListItems;	  ///< The list of grid list items (list views).
	GridDetailedView* view_gridDetails;	   ///< The grid details panel.

	QPushButton* button_addInputGrid;	 ///< Button to add an input grid to the loaded pool.
	QPushButton* button_addOutputGrid;	  ///< Button to add an output grid to the loaded pool.

	const std::shared_ptr<GridPool> gridPool;	 ///< The grid pool, holding references to all grids loaded.
};

#endif	  // QT_INCLUDE_OFFLINE_RENDERER_WIDGET_HPP_
