#ifndef QT_INCLUDE_OFFLINE_RENDERER_WIDGET_HPP_
#define QT_INCLUDE_OFFLINE_RENDERER_WIDGET_HPP_

#include "../../image/include/image_storage.hpp"
#include "../../grid/include/discrete_grid.hpp"
#include "../../grid/include/grid_pool.hpp"

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QGridLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QScrollArea>

#include <memory>
#include <vector>

class OfflineWidget : public QWidget {
	protected:
		class GridWidget; // Fwd-declaration
	public:
		OfflineWidget();
		~OfflineWidget(void);
	protected:
		QGridLayout* mainLayout; ///< Main layout containing the UI

		QMenuBar* menuBar; ///< The menu bar of the program
		QStatusBar* statusBar; ///< Status bar for the widget

		QScrollArea* gridList; ///< The scroll area where grids are displayed.


	protected:
		class GridWidget : public QWidget {
			public:
				GridWidget(const std::shared_ptr<DiscreteGrid>& grid);
				~GridWidget();
			protected:
		};
};

#endif // QT_INCLUDE_OFFLINE_RENDERER_WIDGET_HPP_
