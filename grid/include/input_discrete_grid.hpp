#ifndef GRID_INCLUDE_INPUT_DISCRETE_GRID_HPP_
#define GRID_INCLUDE_INPUT_DISCRETE_GRID_HPP_

#include "../../macros.hpp"
#include "../../features.hpp"

#include "./discrete_grid.hpp"

class InputGrid : public DiscreteGrid {
	public:
		/// @brief Nothing to be done in the constructor.
		InputGrid(void);
		/// @brief Nothing to be done in the constructor.
		InputGrid(const InputGrid& other) = delete;
		/// @brief Nothing to be done in the constructor.
		InputGrid(InputGrid&& other) = delete;
		/// @brief Nothing to be done in the constructor.
		InputGrid& operator= (const InputGrid& other) = delete;
		/// @brief Nothing to be done in the constructor.
		InputGrid& operator= (InputGrid&& other) = delete;

		/// @brief Nothing will be done in the destructor here.
		~InputGrid(void);

		/// @brief Pre-allocate the data vector, in order to have enough space to copy the data into it.
		virtual InputGrid& preAllocateImageData(sizevec3 dimensions);

		/// @brief Add an image to the input grid.
		virtual InputGrid& addImage(std::vector<DataType> imgData, std::size_t imgIndex);

		/// @brief For file formats where the image is stored as one chunk of data, sets the while image at once.
		virtual InputGrid& setGrid(std::vector<DataType> imgData, sizevec3 dimensions);

		/// @brief Override of the modifiable flag in the discrete grid, removing the ability for this grid to be modified.
		virtual InputGrid& setModifiable(bool b) override;

		/// @brief Disallows setting the resolution to anything else than already loaded in.
		virtual InputGrid& setResolution(sizevec3 newRes) override;

		/// @brief Disallows setting a new bounding box to the input grid.
		virtual InputGrid& setBoundingBox(bbox_t renderWindow) override;
};

class OfflineInputGrid : public InputGrid {
	public:
		/// @brief Nothing to be done in the constructor.
		OfflineInputGrid(void);
		/// @brief Nothing to be done in the constructor.
		OfflineInputGrid(const OfflineInputGrid& other) = delete;
		/// @brief Nothing to be done in the constructor.
		OfflineInputGrid(OfflineInputGrid&& other) = delete;
		/// @brief Nothing to be done in the constructor.
		OfflineInputGrid& operator= (const OfflineInputGrid& other) = delete;
		/// @brief Nothing to be done in the constructor.
		OfflineInputGrid& operator= (OfflineInputGrid&& other) = delete;

		/// @brief Nothing will be done in the destructor here.
		~OfflineInputGrid(void);

		/// @b Gathers info from an existing inputgrid, and loads the info for itself.
		virtual OfflineInputGrid& fromInputGrid(const std::shared_ptr<InputGrid>& igrid);

		virtual DataType getPixel(std::size_t x, std::size_t y, std::size_t z) const override;
	protected:
		mutable std::ifstream* dimFile; // mutable needed for proper overloads of getPixel()
		mutable std::ifstream* imaFile; // mutable needed for proper overloads of getPixel()
};

#endif // GRID_INCLUDE_INPUT_DISCRETE_GRID_HPP_
