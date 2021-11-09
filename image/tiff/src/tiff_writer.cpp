#include "../include/tiff_writer.hpp"

namespace Image {

namespace Tiff {

	TIFFWriter::TIFFWriter(std::string bname) : GenericGridWriter(bname) {}

	TIFFWriter::TIFFWriter(std::string bname, std::string bpath) : GenericGridWriter(bname, bpath) {}

	TIFFWriter::~TIFFWriter() {}

	std::string TIFFWriter::build_iterative_filename(std::size_t slice_idx, std::size_t channel) {
		// take base name, append after base path (with '/' if not present) :
		//	if ends with '_' do nothing, otherwise add it
		//	add numbered channel count (with 1 leading 0s, as if there could be up to 99 channels)
		//	add '_'
		//	add numbered slice count (with 4 leading 0s, as if there could be up to 100k slices)

		// start by defining some defaults :
		std::string final_name = "", path = this->file_base_path, name = this->file_base_name, channel_string = "", slice_string = "";

		// if empty file name or path, replace with defaults.
		if (this->file_base_name.empty()) { name = "default_grid"; }
		if (this->file_base_path.empty()) { path = "/"; }

		// compute number suffixes :
		if (channel < 10) { channel_string = "0" + std::to_string(channel); }
		// slice index can be up to 99'999, so count the # of leading 0s there should be :
		float f_slice_idx = static_cast<float>(slice_idx);
		std::size_t leading_0_slice = 5 - static_cast<std::size_t>(std::floor(std::log10(f_slice_idx)) + 1.f);
		if (slice_idx == 0) { leading_0_slice = 4; } // special case for 0 since log10(0) == -inf
		std::string number_prefix = "";
		if (leading_0_slice > 0) { number_prefix = std::string(leading_0_slice, '0'); }
		slice_string = number_prefix + std::to_string(slice_idx);

		final_name = path;													// start with base path
		if (final_name[final_name.size()-1] != '/') { final_name += "/"; }	// add path separator at the end, if needed
		final_name += name;													// add file name
		if (final_name[final_name.size()-1] != '_') { final_name += "_"; }	// add underscore for numbering if not
		final_name += channel_string + "_" + slice_string + ".tiff";		// add suffixes and file extension

		return final_name;
	}

	TIFF* TIFFWriter::open_file(std::string file_name, std::string permissions) {
		TIFF* file = TIFFOpen(file_name.data(), permissions.data());
		if (file == NULL) {} // empty for now
		return file;
	}

	void TIFFWriter::close_file(TIFF *handle) {
		if (handle == nullptr || handle == NULL) { return; }
		TIFFClose(handle);
		return;
	}

} // namespace Tiff

} // namespace Image
