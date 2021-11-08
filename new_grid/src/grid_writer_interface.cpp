#include "../include/grid_writer_interface.hpp"

namespace Image {

GridWriterInterface::GridWriterInterface(std::string bname) : file_base_name(bname), file_base_path("/") {}

GridWriterInterface::GridWriterInterface(std::string bname, std::string bpath) :
	file_base_name(bname), file_base_path(bpath) {}

void GridWriterInterface::setBaseName(std::string basename) { this->file_base_name = basename; return; }
void GridWriterInterface::setBasePath(std::string basepath) { this->file_base_path = basepath; return; }

std::string GridWriterInterface::getBaseName() const { return this->file_base_name; }
std::string GridWriterInterface::getBasePath() const { return this->file_base_path; }

} // namespace Image
