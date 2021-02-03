#ifndef GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
#define GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_

#include "../../macros.hpp"
#include "../../features.hpp"

#include "./discrete_grid.hpp"
#include "./input_discrete_grid.hpp"

class OutputGrid : public DiscreteGrid {
	public:
		OutputGrid(void);
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

class OfflineOutputGrid : public OutputGrid {
	public:
		OfflineOutputGrid(void);
		OfflineOutputGrid(sizevec3 resolution, bbox_t renderWindow);
		OfflineOutputGrid(const OfflineOutputGrid& other) = delete;
		OfflineOutputGrid(OfflineOutputGrid&& other) = delete;
		OfflineOutputGrid& operator= (const OfflineOutputGrid& other) = delete;
		OfflineOutputGrid& operator= (OfflineOutputGrid&& other) = delete;
		~OfflineOutputGrid(void);

		virtual bool hasData(void) const override;
		virtual OfflineOutputGrid& preallocateData(void) override;
		virtual OfflineOutputGrid& preallocateData(sizevec3 dims) override;
		/// @b Override of the setPixel() function to directly write the data to file
		virtual OfflineOutputGrid& setPixel(std::size_t i, std::size_t j, std::size_t k, DataType _data) override;
	protected:
		std::ofstream* outputDIM;
		std::ofstream* outputIMA;
};

#endif // GRID_INCLUDE_OUTPUT_DISCRETE_GRID_HPP_
