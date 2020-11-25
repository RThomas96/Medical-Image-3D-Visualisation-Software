#include "../include/grid_pool.hpp"

GridPool::GridPool(void) {
	// Nothing here yet
}

GridPool::~GridPool(void) {
	// Nothing here yet
}

void GridPool::toJSONFile(const std::string &path) const {
	std::cerr << "[ERROR][" << __FUNCTION__ << "] Does not support JSON configuration exporting yet.\n";
	return;
}

void GridPool::fromJSONFile(const std::string &path) {
	std::cerr << "[ERROR][" << __FUNCTION__ << "] Does not support JSON configuration loading yet.\n";
	return;
}
