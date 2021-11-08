#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TEMPLATED_WRITER_BACKEND_IMPL_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TEMPLATED_WRITER_BACKEND_IMPL_HPP_

#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TEMPLATED_WRITER_BACKEND_HPP_
#include "./templated_writer_backend.hpp"
#endif

namespace Image {

namespace Tiff {

	template <typename unsupported_element_t>
	TIFFWriterDetail<unsupported_element_t>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		throw std::runtime_error("Error : cannot create a TIFF writer backend with unsupported type.");
	}

	template <typename unsupported_element_t>
	TIFFWriterDetail<unsupported_element_t>::TIFFWriterDetail(std::string bname, std::string bpath) :
	TIFFWriter(bname, bpath) {
		throw std::runtime_error("Error : cannot create a TIFF writer backend with unsupported type.");
	}

	template <>
	TIFFWriterDetail<std::uint8_t>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_UINT;
		this->bits_per_sample = 8;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::uint16_t>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_UINT;
		this->bits_per_sample = 16;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::uint32_t>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_UINT;
		this->bits_per_sample = 32;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::uint64_t>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_UINT;
		this->bits_per_sample = 64;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::uint8_t>::TIFFWriterDetail(std::string bname, std::string bpath) :
	TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_UINT;
		this->bits_per_sample = 8;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::uint16_t>::TIFFWriterDetail(std::string bname, std::string bpath) :
	TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_UINT;
		this->bits_per_sample = 16;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::uint32_t>::TIFFWriterDetail(std::string bname, std::string bpath) :
	TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_UINT;
		this->bits_per_sample = 32;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::uint64_t>::TIFFWriterDetail(std::string bname, std::string bpath) :
	TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_UINT;
		this->bits_per_sample = 64;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::int8_t>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_INT;
		this->bits_per_sample = 8;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::int16_t>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_INT;
		this->bits_per_sample = 16;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::int32_t>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_INT;
		this->bits_per_sample = 32;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::int64_t>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_INT;
		this->bits_per_sample = 64;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::int8_t>::TIFFWriterDetail(std::string bname, std::string bpath) :
	TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_INT;
		this->bits_per_sample = 8;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::int16_t>::TIFFWriterDetail(std::string bname, std::string bpath) :
	TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_INT;
		this->bits_per_sample = 16;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::int32_t>::TIFFWriterDetail(std::string bname, std::string bpath) :
	TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_INT;
		this->bits_per_sample = 32;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<std::int64_t>::TIFFWriterDetail(std::string bname, std::string bpath) :
	TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_INT;
		this->bits_per_sample = 64;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<float>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_IEEEFP;
		this->bits_per_sample = 32;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<double>::TIFFWriterDetail(std::string bname) : TIFFWriter(bname) {
		this->sample_format = SAMPLEFORMAT_IEEEFP;
		this->bits_per_sample = 64;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<float>::TIFFWriterDetail(std::string bname, std::string bpath) : TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_IEEEFP;
		this->bits_per_sample = 32;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <>
	TIFFWriterDetail<double>::TIFFWriterDetail(std::string bname, std::string bpath) : TIFFWriter(bname, bpath) {
		this->sample_format = SAMPLEFORMAT_IEEEFP;
		this->bits_per_sample = 64;
		this->planar_config = PLANARCONFIG_CONTIG;
	}

	template <typename element_t>
	TIFFWriterDetail<element_t>::~TIFFWriterDetail() {}

	template <typename element_t>
	bool TIFFWriterDetail<element_t>::writeSlice(Grid::Ptr src_grid, std::size_t slice, ThreadedTask::Ptr task) {
		// Warning : will only write SINGLE-CHANNEL IMAGES.

		// if nothing is given, nothing'll be written :
		if (src_grid == nullptr) { return false; }

		// check resolution :
		svec3 resolution = src_grid->getResolution();
		if (resolution.z == 0 || resolution.y == 0 || resolution.x == 0) {
			std::cerr << "Error : cannot write with a dimension at 0.\n";
			return false;
		}
		if (slice >= resolution.z) { std::cerr << "Error : cannot write past-the-end of the grid.\n"; return false; }

		// get voxel dimensionality :
		std::size_t dimensionality = src_grid->getVoxelDimensionality();
		if (dimensionality == 0) { std::cerr << "Error : cannot write 0 samples.\n"; return false; }

		// get data from the grid :
		std::vector<pixel_t> slice_values;
		src_grid->readSlice(slice, slice_values);
		bool successful_write = true;

		// start to iterate on the voxel dimensionality :
		for (std::size_t channel = 0; channel < dimensionality; ++channel) {
			// build file name :
			std::string target_name = this->build_iterative_filename(slice, channel);
			// open TIFF file :
			TIFF* file = this->open_file(target_name);
			// write directory headers :
			std::uint32_t strip_size = this->write_tiff_tags(file, src_grid, slice, channel);
			// check no error occured :
			if (strip_size == 0) {
				std::cerr << "Error : slice " << slice << ", channel " << channel << " of file \"" << target_name <<
							 "\" cannot be written to (no strip size).";
				successful_write = false;
				continue;
			}
			// extract data from data vector :
			std::vector<pixel_t> current_data = this->read_subpixels_from_slice(slice_values, dimensionality, channel);
			// write the data to file :
			this->write_tiff_strips(file, current_data, resolution.x, resolution.y, strip_size);
			// close the file, and switch to the next one
			this->close_file(file);
		}

		return successful_write;
	}

	template <typename element_t>
	bool TIFFWriterDetail<element_t>::writeGrid(Grid::Ptr src_grid, ThreadedTask::Ptr task) {
		if (src_grid == nullptr) { return false; }

		// Call write slice on all slices of the grid :
		svec3 resolution = src_grid->getResolution();
		if (resolution.z == 0) { return false; }

		//write all slices and check if something went wrong :
		bool all_right = true;
		for (std::size_t slice = 0; slice < resolution.z; ++slice) {
			if (not this->writeSlice(src_grid, slice, task)) {
				std::cerr << "Warning : error writing slice " << slice << " to disk.\n";
				all_right = false;
			}
		}

		// if task available, set to end with current state (bad or good)
		if (task != nullptr) { task->end(all_right); }

		return all_right;
	}

	template <typename element_t>
	std::uint32_t TIFFWriterDetail<element_t>::write_tiff_tags(TIFF* file, Grid::Ptr grid, std::size_t slice_idx, std::size_t start) {
		// cannot write to non-open file :
		if (file == nullptr || file == NULL) { return 0; }

		// gather info from the grid first :
		svec3 resolution = grid->getResolution();
		std::size_t vox_dimensionality = grid->getVoxelDimensionality();
		// if trying to write with a start sample bigger than the number of samples, return.
		if (start >= vox_dimensionality) { return 0; }

		// REMINDER : will only write single-channel TIFF images.
		// as such, set photometric interpretation to minisblack :
		uint16_t photometric_interpretation = PHOTOMETRIC_MINISBLACK;

		/**
		 * WARNING : The TIFF spec states that greyscale images can only have 4 or 8 bits per pixel, thus limiting them
		 * to 16 or 256 grey values. We'll see what happens when loading the image afterwards.
		 */

		// start with the width and height :
		TIFFSetField(file, TIFFTAG_IMAGEWIDTH, resolution.x);
		TIFFSetField(file, TIFFTAG_IMAGELENGTH, resolution.y);
		TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 1);
		TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, this->bits_per_sample);
		TIFFSetField(file, TIFFTAG_PHOTOMETRIC, photometric_interpretation);
		TIFFSetField(file, TIFFTAG_EXTRASAMPLES, 0, nullptr); // no extra samples [ MIGHT SEGFAULT ] because of nullptr

		// Required fields from the TIFF spec for greyscale images :
		TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_NONE);		// No compression applied
		TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);	// sets contiguous arrays in mem
		TIFFSetField(file, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);	// Top-left is 0,0

		// Set the X and Y resolutions (with corresponding units) :
		TIFFSetField(file, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER);
		// get voxel dimensions (expressed in micrometers, divided to get back to centimeters and inverted because the
		// value expresses how many pixels in one ResolutionUnit, apparently) :
		glm::vec3 voxel_dimensions = 1.0f / (grid->getVoxelDimensions() * 1.e-4f);
		TIFFSetField(file, TIFFTAG_XRESOLUTION, voxel_dimensions.x);
		TIFFSetField(file, TIFFTAG_YRESOLUTION, voxel_dimensions.y);

		// Compute the expected rowspersample :
		// note : last param of TIFFDefaultStripSize is an estimate of the desired size of the strip
		std::uint32_t rows_per_strip = TIFFDefaultStripSize(file, 0);
		// And set it in the file :
		TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, rows_per_strip);

		return rows_per_strip;
	}

	template <typename element_t>
	std::vector<element_t> TIFFWriterDetail<element_t>::read_subpixels_from_slice(std::vector<pixel_t>& src,
																				  std::size_t samples_in_src,
																				  std::size_t beg) {
		// check fi the # of samples in src are all filled :
		if (src.size() % samples_in_src > 0) { std::cerr << "Warning : not enough samples in the src vector !\n"; }
		// if reading no samples or wrong arguments, return null vector :
		if (beg >= samples_in_src) {
			std::cerr << "Error : trying to read past-the-end of data vector in writer\n";
			return std::vector<pixel_t>();
		}

		// compute the number of pixels in src :
		std::size_t pixel_count = src.size() / samples_in_src;
		// allocate output vector
		std::vector<pixel_t> out(pixel_count);

		// # of samples read from src, acts as iter into out vector :
		std::size_t samples_read = 0;
		// iterate on all pixels of src, starting at [beg] and with a stride of [samples_in_src] :
		for (std::size_t src_iter = beg; src_iter < src.size(); src_iter += samples_in_src) {
			out[samples_read] = src[src_iter];
			// increment read counter :
			samples_read++;
		}
		// vector should be all read.

		return out;
	}

	template <typename element_t>
	bool TIFFWriterDetail<element_t>::write_tiff_strips(TIFF* file, std::vector<element_t>& data, std::size_t width,
														std::size_t height, std::uint32_t estimated_strip_size) {
		// check parameters are valid :
		if (file == NULL || file == nullptr) { std::cerr << "Tried to write to NULL.\n"; return false; }
		if (data.size() == 0) { std::cerr << "Tried to write no data.\n"; return false; }

		// ADDITIONAL CHECK : see if data has a round multiple of 'width' elements :
		if ((data.size() % width) != 0) { std::cerr<<"ERROR : too many components (over="<<data.size()%width<<")\n"; }

		// pre-allocate a copy buffer for libtiff :
		void* tiff_buffer = _TIFFmalloc(width*sizeof(pixel_t));
		if (tiff_buffer == NULL) { std::cerr << "couldn't allocate tiff buffer"; return false; }

		// tracks if any strips have not been written to the file
		bool write_successful = true;
		// WARNING : doing it this way may produce a corrupted TIFF file, where some rows/strips are missing !!!

		for (std::size_t current_row = 0; current_row < height; current_row++) {
			// copy data over from the width buffer :
			_TIFFmemcpy(tiff_buffer, &(data[current_row*width]), width*sizeof(pixel_t));
			// attempt to write the scanline to the file :
			if (TIFFWriteScanline(file, tiff_buffer, current_row) < 0) {
				std::cerr << "ERROR could not write the contents of the row " << current_row << " of the file \"" <<
							 TIFFFileName(file) << "\"\n";
				write_successful = false;
			}
		}

		// free up the local buffer :
		_TIFFfree(tiff_buffer);

		return write_successful;
	}

} // namespace Tiff

} // namespace Image

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TEMPLATED_WRITER_BACKEND_IMPL_HPP_
