#ifndef GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
#define GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_

#include "./discrete_grid.hpp"

class OutputGrid : public DiscreteGrid {
	public:
		OutputGrid(void);
		~OutputGrid(void);

		virtual void setVoxelData(sizevec3 idx, DataType value);

	protected:
};

#endif // GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
