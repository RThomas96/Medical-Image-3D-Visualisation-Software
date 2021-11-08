#include "../include/writer.hpp"

namespace Image {

Writer::Writer(WriterBackendImpl::Ptr _existing_backend) : pImpl(std::move(_existing_backend)) {}

Writer::~Writer(void) {
	this->pImpl = nullptr; // reset the pointer and de-allocate it
}

void Writer::setBaseName(std::string basename) {
	if (basename.empty()) { return; }
	if (this->pImpl) { this->pImpl->setBaseName(basename); }
	return;
}

void Writer::setBasePath(std::string basepath) {
	if (basepath.empty()) { return; }
	if (this->pImpl) { this->pImpl->setBasePath(basepath); }
	return;
}

std::string Writer::getBaseName() const {
	if (this->pImpl) { return this->pImpl->getBaseName(); }
	return "";
}

std::string Writer::getBasePath() const {
	if (this->pImpl) { return this->pImpl->getBasePath(); }
	return "";
}

void Writer::setBackend(WriterBackendImpl::Ptr _new_backend) {
	if (_new_backend == nullptr) { return; }
	this->pImpl = WriterBackendImpl::Ptr(std::move(_new_backend)); // move the damn thing
	return;
}

bool Writer::writeSlice(Grid::Ptr src_grid, std::size_t slice, ThreadedTask::Ptr &task) {
	if (this->pImpl) { return this->pImpl->writeSlice(src_grid, slice, task); }
	return false;
}

bool Writer::writeGrid(Grid::Ptr src_grid, ThreadedTask::Ptr &task) {
	if (this->pImpl) { return this->pImpl->writeGrid(src_grid, task); }
	return false;
}

}
