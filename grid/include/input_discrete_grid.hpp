#ifndef GRID_INCLUDE_INPUT_DISCRETE_GRID_HPP_
#define GRID_INCLUDE_INPUT_DISCRETE_GRID_HPP_

#include "./discrete_grid.hpp"

class InputGrid : public DiscreteGrid {
	public:
		/// @brief Nothing to be done in the constructor.
		InputGrid(void);

		/// @brief Nothing will be done in the destructor here.
		~InputGrid(void) {}

		/// @brief Pre-allocate the data vector, in order to have enough space to copy the data into it.
		virtual InputGrid& preAllocateImageData(sizevec3 dimensions);

		/// @brief Add an image to the input grid.
		virtual InputGrid& addImage(std::vector<DataType> imgData, std::size_t imgIndex);

		/// @brief Override of the modifiable flag in the discrete grid, removing the ability for this grid to be modified.
		virtual InputGrid& setModifiable(bool b) override;

		/// @brief Disallows any bounding box changes.
		virtual InputGrid& setBoundingBox(glm::vec4 min, glm::vec4 max) override;

		/// @brief Disallows setting the resolution to anything else than already loaded in.
		virtual InputGrid& setResolution(sizevec3 newRes) override;
};

#endif // GRID_INCLUDE_INPUT_DISCRETE_GRID_HPP_
