#include "../include/grid.hpp"

#include "../../image/tiff/include/backend.hpp"

namespace Image {

	Grid::Ptr Grid::createGrid(std::vector<std::vector<std::string>> &_filenames) {
		if (_filenames.empty()) { return nullptr; }
		if (_filenames[0].empty()) { return nullptr; }
		// Get the extension of the first filename :
		const std::string& reference_file= _filenames[0][0];
		std::size_t iter = reference_file.find_last_of('.');
		if (iter == std::string::npos) { return nullptr; }

		// extract substring of extension :
		std::string ext	= reference_file.substr(iter, std::string::npos);
		if (ext == ".tif" || ext == ".tiff") {
			return Grid::Ptr(new Grid(Tiff::createBackend(reference_file)));
		}

		return nullptr;
	}

}
