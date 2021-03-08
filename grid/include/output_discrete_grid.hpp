#ifndef GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
#define GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_

#include "../../macros.hpp"
#include "../../features.hpp"

#include "./discrete_grid.hpp"
#include "./input_discrete_grid.hpp"

class OutputGrid : public DiscreteGrid {
	public:
		OutputGrid(void);
		OutputGrid(const std::shared_ptr<OutputGrid>&);
		OutputGrid(sizevec3 resolution, bbox_t renderWindow);
		OutputGrid(const OutputGrid& other) = delete;
		OutputGrid(OutputGrid&& other) = delete;
		OutputGrid& operator= (const OutputGrid& other) = delete;
		OutputGrid& operator= (OutputGrid&& other) = delete;
		~OutputGrid(void);

		virtual OutputGrid& preallocateData(void);
		virtual OutputGrid& preallocateData(sizevec3 dims);
		virtual OutputGrid& updateRenderBox(const bbox_t& newbox);
		virtual OutputGrid& writeSlice();
		virtual OutputGrid& setCurrentSlice(std::size_t currentSlice);
		virtual OutputGrid& setPixel(std::size_t i, std::size_t j, std::size_t k, DataType _data) override;
	protected:
		std::size_t currentSlice;
};

#endif // GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
