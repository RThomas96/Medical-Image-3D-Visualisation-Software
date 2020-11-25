#ifndef GRID_INCLUDE_GRID_POOL_HPP_
#define GRID_INCLUDE_GRID_POOL_HPP_

/* The class here will hold a vector of shared_ptrs to DiscreteGrids, with methods to add a stack of images, remove a
 * stack, modify a grid (return a shared_ptr which can modify the grid). it also holds
 */

#include "../../grid/include/tetmesh.hpp"
#include "../../grid/include/discrete_grid.hpp"
#include "../../grid/include/input_discrete_grid.hpp"
#include "../../grid/include/output_discrete_grid.hpp"

#include <vector>
#include <memory>

/// @brief This class holds all the grids needed and to be generated.
/// @details This class holds input grids from disk, output grids to disk and
class GridPool {
	public:
		/// @b Creates an empty grid pool, which will contain no data.
		GridPool(void);
		/// @b Destroys a grid pool, and frees up the allocated memory.
		~GridPool(void);
	public:
		/// @b Add an input grid previously created.
		GridPool& addInputGrid(const std::shared_ptr<DiscreteGrid>&);
		/// @b Create an empty output grid.
		GridPool& createOutputGrid(void);
		/// @b Create an output grid from the given input grids in parameter.
		GridPool& createOutputGrid(const std::initializer_list<const std::shared_ptr<InputGrid>>&);
	public:
		/// @b Generates a specific output grid.
		void generateOutputGrid(const std::shared_ptr<OutputGrid>&);
		/// @b Generates all output grids currently managed by the pool.
		void generateOutputGrids(void);
	public:
		/// @b Saves a grid pool configuration to a JSON file.
		void toJSONFile(const std::string& path) const;
		/// @b Loads a grid pool configuration from a JSON file.
		void fromJSONFile(const std::string& path);
	public:
		/// @b Returns all grids, regardless of their input/output status
		std::vector<const std::shared_ptr<DiscreteGrid>> getAllGrids(void) const;
		/// @b Returns all input grids
		std::vector<const std::shared_ptr<InputGrid>> getInputGrids(void) const;
		/// @b Returns all output grids
		std::vector<const std::shared_ptr<OutputGrid>> getOutputGrids(void) const;
	protected:
		std::vector<std::shared_ptr<InputGrid>> inputGrids; ///< Input grids
		std::vector<std::shared_ptr<OutputGrid>> outputGrids; ///< Output grids
		std::vector<std::shared_ptr<TetMesh>> tetMeshes; ///< Tetrahedral meshes
};

#endif // GRID_INCLUDE_GRID_POOL_HPP_
