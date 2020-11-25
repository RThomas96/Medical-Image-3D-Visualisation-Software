#ifndef GRID_INFO_VIEW_HPP
#define GRID_INFO_VIEW_HPP

#include "../../grid/include/discrete_grid.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>

#include <vector>

class GridDetailedView : public QWidget {
	public:
		GridDetailedView(void);
		~GridDetailedView(void);
		void showGrid(const std::shared_ptr<DiscreteGrid>&);
	protected:
		// Find a way to incorporate the grid controller here
		// Find a way to incorporate a widget which either :
		//	- displays a view of one of the stack images
		//	- displays input grids to sample from (for output grids)
		std::shared_ptr<DiscreteGrid>& grid;
};

#endif // GRID_INFO_VIEW_HPP
