#include "../include/grid_writer_interface.hpp"

namespace Image {

WriterBackendImpl::WriterBackendImpl(std::string bname) : file_base_name(bname), file_base_path("/") {}

WriterBackendImpl::WriterBackendImpl(std::string bname, std::string bpath) :
	file_base_name(bname), file_base_path(bpath) {}

void WriterBackendImpl::setBaseName(std::string basename) { this->file_base_name = basename; return; }
void WriterBackendImpl::setBasePath(std::string basepath) { this->file_base_path = basepath; return; }

std::string WriterBackendImpl::getBaseName() const { return this->file_base_name; }
std::string WriterBackendImpl::getBasePath() const { return this->file_base_path; }

} // namespace Image
