#ifndef GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
#define GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_

#include "./discrete_grid.hpp"
#include "./input_discrete_grid.hpp"

class OutputGrid : public DiscreteGrid {
	public:
		OutputGrid(void);
		OutputGrid(sizevec3 resolution, bbox_t renderWindow);
		~OutputGrid(void);

		virtual OutputGrid& preallocateData(void);
		virtual OutputGrid& preallocateData(sizevec3 dims);
		virtual void setVoxelData(sizevec3 idx, DataType value);
#ifdef ENABLE_BASIC_BB
		virtual OutputGrid& setBoundingBox(bbox_t renderWindow) override;
#ifdef ENABLE_BB_TRANSFORM
		virtual OutputGrid& updateRenderBox(const bbox_t& newbox);
#endif
#endif
};

#endif // GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
