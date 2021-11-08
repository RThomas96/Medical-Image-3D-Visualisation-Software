#include "../include/grid_writer.hpp"

namespace Image {

GridWriter::GridWriter(GenericGridWriter::Ptr _existing_backend) : pImpl(std::move(_existing_backend)) {}

GridWriter::~GridWriter(void) {
	this->pImpl = nullptr; // reset the pointer and de-allocate it
}

void GridWriter::setBaseName(std::string basename) {
	if (basename.empty()) { return; }
	if (this->pImpl) { this->pImpl->setBaseName(basename); }
	return;
}

void GridWriter::setBasePath(std::string basepath) {
	if (basepath.empty()) { return; }
	if (this->pImpl) { this->pImpl->setBasePath(basepath); }
	return;
}

std::string GridWriter::getBaseName() const {
	if (this->pImpl) { return this->pImpl->getBaseName(); }
	return "";
}

std::string GridWriter::getBasePath() const {
	if (this->pImpl) { return this->pImpl->getBasePath(); }
	return "";
}

void GridWriter::setBackend(GenericGridWriter::Ptr _new_backend) {
	if (_new_backend == nullptr) { return; }
	this->pImpl = GenericGridWriter::Ptr(std::move(_new_backend)); // move the damn thing
	return;
}

bool GridWriter::writeSlice(Grid::Ptr src_grid, std::size_t slice, ThreadedTask::Ptr &task) {
	if (this->pImpl) { return this->pImpl->writeSlice(src_grid, slice, task); }
	return false;
}

bool GridWriter::writeGrid(Grid::Ptr src_grid, ThreadedTask::Ptr &task) {
	if (this->pImpl) { return this->pImpl->writeGrid(src_grid, task); }
	return false;
}

}
