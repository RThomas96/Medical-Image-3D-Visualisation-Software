#ifndef GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
#define GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_

#include "./discrete_grid.hpp"
#include "./input_discrete_grid.hpp"

class OutputGrid : public DiscreteGrid {
	public:
		OutputGrid(void);
		OutputGrid(sizevec3 resolution, bbox_t renderWindow);
		~OutputGrid(void);

		virtual OutputGrid& updateRenderBox(const std::shared_ptr<InputGrid> input);
		virtual OutputGrid& preallocateData(void);
		virtual OutputGrid& preallocateData(sizevec3 dims);
		virtual void setVoxelData(sizevec3 idx, DataType value);
		virtual OutputGrid& setBoundingBox(bbox_t renderWindow) override;

	protected:
};

#endif // GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
