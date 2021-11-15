#include "../include/generic_grid_writer.hpp"

namespace Image {

GenericGridWriter::GenericGridWriter(std::string bname) : file_base_name(bname), file_base_path("/") {}

GenericGridWriter::GenericGridWriter(std::string bname, std::string bpath) :
	file_base_name(bname), file_base_path(bpath) {}

void GenericGridWriter::setBaseName(std::string basename) { this->file_base_name = basename; return; }
void GenericGridWriter::setBasePath(std::string basepath) { this->file_base_path = basepath; return; }

std::string GenericGridWriter::getBaseName() const { return this->file_base_name; }
std::string GenericGridWriter::getBasePath() const { return this->file_base_path; }

} // namespace Image
