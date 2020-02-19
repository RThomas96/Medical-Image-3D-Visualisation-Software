#include "../include/image_stack_loader.hpp"

#include <iostream>
#include <algorithm>
#include <iterator>

#include <QImage>
#include <QString>
#include <QFileDialog>
#include <QErrorMessage>

#define CLAMP_VAL(x) (x>128)?255:x<<2
#define THRESHOLD 5
#define BINARIZE_VAL(x) (x>THRESHOLD) ? 128 : 0

image_stack_loader::image_stack_loader() {
	// Start all durations to 0 :
	this->raw_file_opening_duration = DEFAULT_TIMEKEEPING_DURATION(0);
	this->raw_file_copying_duration = DEFAULT_TIMEKEEPING_DURATION(0);
	this->raw_file_reading_duration = DEFAULT_TIMEKEEPING_DURATION(0);
	this->std_data_copying_duration = DEFAULT_TIMEKEEPING_DURATION(0);
	this->complete_stack_loading_duration = DEFAULT_TIMEKEEPING_DURATION(0);

	// Start number of file operations to 0 :
	this->number_of_file_opening_operations = 0;
	this->number_of_file_reading_operations = 0;
	this->number_of_file_copying_operations = 0;

	// No file is yet open :
	this->tiff_file = nullptr;

	// The data has not been read yet :
	this->image_data = nullptr;
	this->end_iterator = 0;

	// Number of color channels in the image
	this->color_channels = 1;

	// No image was yet loaded, initialize the sizes to max :
	this->image_width = std::numeric_limits<std::size_t>::max();
	this->image_height = std::numeric_limits<std::size_t>::max();
	this->image_depth = std::numeric_limits<std::size_t>::max();
}

unsigned char* image_stack_loader::load_stack_from_folder() {

	// Open up a file dialog :
	QStringList file_names = QFileDialog::getOpenFileNames(nullptr, "Open TIF images", "../../", "TIFF Image files (*.tif *.tiff)");
	// Check if files are selected :
	while (file_names.size() == 0) {
		QErrorMessage errorMsg(nullptr);
		errorMsg.showMessage("No files selected !");
		file_names = QFileDialog::getOpenFileNames(nullptr, "Open TIF images", "../../", "TIFF Image files (*.tif *.tiff)");
	}

	// TIME TRACKING :
	std::chrono::time_point<std::chrono::_V2::system_clock, DEFAULT_TIMEKEEPING_DURATION> start_point_for_stack_loading = std::chrono::high_resolution_clock::now();

	// Once images paths are set, load in the images :
	this->load_all_images(file_names);

	// TIME TRACKING :
	std::chrono::time_point<std::chrono::_V2::system_clock, DEFAULT_TIMEKEEPING_DURATION> end_point_for_stack_loading = std::chrono::high_resolution_clock::now();
	this->complete_stack_loading_duration = end_point_for_stack_loading - start_point_for_stack_loading;

	this->print_debug_info();

	return this->image_data;
}

void image_stack_loader::load_single_image(QString absolute_path) {
	this->tiff_file = nullptr;
	// Timekeeping variables
	std::chrono::time_point<std::chrono::_V2::system_clock, DEFAULT_TIMEKEEPING_DURATION>
		start_point_for_image_loading, start_point_for_image_copying, start_point_for_image_reading,
		end_point_for_image_loading, end_point_for_image_copying, end_point_for_image_reading;

	// Get file name as const char*.
	// WARNING : the function toStdString() does not allocate a memory space, it merely
	// copies the data onto the stack, which then pushes it to the next function. Thus,
	// we need to call c_str() on a locally allocated memory space, which is why we are
	// creating a string for this single purpose.
	std::string fpath = absolute_path.toStdString();
	const char* file_name = fpath.c_str();

	// Open file and track time and operations :
	start_point_for_image_loading = std::chrono::high_resolution_clock::now();
	this->tiff_file = TinyTIFFReader_open(file_name);
	end_point_for_image_loading = std::chrono::high_resolution_clock::now();
	this->raw_file_opening_duration += (end_point_for_image_loading - start_point_for_image_loading);
	this->number_of_file_opening_operations++;

	// Check if the file has been opened :
	if (!this->tiff_file) {
		std::cerr << "This file was not opened : " << fpath << std::endl;
		// Get TinyTIFF loading error :
		if (TinyTIFFReader_wasError(this->tiff_file)) {
			std::cerr << "TinyTIFF error string : " << TinyTIFFReader_getLastError(this->tiff_file) << std::endl;
		} else {
			std::cerr << "No TinyTIFF error to report " << std::endl;
		}
		std::cerr << "Returning empty vector " << std::endl;
		this->tiff_file = nullptr;
		// no need to close it, just return empty vector :
		return;
	}

	if (TinyTIFFReader_wasError(this->tiff_file)) {
		std::cerr << "File opened, but error happened. TinyTIFF OPENED error string : " << TinyTIFFReader_getLastError(tiff_file) << std::endl;
	}

	// Image stride
	uint32_t stride = static_cast<uint32_t>(this->image_width * this->image_height);

	// Read data :
	start_point_for_image_reading = std::chrono::high_resolution_clock::now();
	TinyTIFFReader_readFrame<unsigned char>(this->tiff_file, this->raw_data_storage);
	end_point_for_image_reading = std::chrono::high_resolution_clock::now();
	this->raw_file_reading_duration += (end_point_for_image_reading - start_point_for_image_reading);
	this->number_of_file_reading_operations++;

	// Copy data, in RGB format :
	start_point_for_image_copying = std::chrono::high_resolution_clock::now();
	if (downsampling_enabled == false ) {
		for (std::size_t i = 0; i < stride; ++i) {
			for (std::size_t j = 0; j < this->color_channels; ++j) {
				this->image_data[this->end_iterator + j] = CLAMP_VAL(this->raw_data_storage[i]);
			}
			this->end_iterator += this->color_channels;
		}
	} else {
		for (std::size_t i = 0; i < this->image_height/2; ++i) {
			for (std::size_t j = 0; j < this->image_width/2; ++j) {
				for (std::size_t k = 0; k < this->color_channels; ++k) {
					this->image_data[this->end_iterator + k] = CLAMP_VAL(this->raw_data_storage[i*2*this->image_width + j*2]);
				}
				this->end_iterator += this->color_channels;
			}
		}
	}
	end_point_for_image_copying = std::chrono::high_resolution_clock::now();
	this->raw_file_copying_duration += (end_point_for_image_copying - start_point_for_image_copying);
	this->number_of_file_copying_operations++;

	TinyTIFFReader_close(this->tiff_file);
	this->tiff_file = nullptr;

	return;
}

void image_stack_loader::load_all_images(QStringList absolute_paths) {
	// If no images are present, exit :
	if (absolute_paths.size() == 0) {
		std::cerr << "image_stack_loader::load_all_images() : no paths were provided" << std::endl;
		return;
	}

	this->initialize_image_data(absolute_paths[0], absolute_paths.size());

	this->raw_data_storage = static_cast<unsigned char*>(calloc(this->image_width * this->image_height, sizeof(unsigned char)));
	// For all paths, add them into a vector
	for (int i = 0; i < absolute_paths.size(); i++) {
		this->load_single_image(absolute_paths[i]);
	}
	free(this->raw_data_storage);
}

void image_stack_loader::initialize_image_data(QString test_file, int nb_images) {
	std::chrono::time_point<std::chrono::_V2::system_clock, DEFAULT_TIMEKEEPING_DURATION>
		start_point_for_image_loading, end_point_for_image_loading;

	std::string fpath = test_file.toStdString();
	const char* file_name = fpath.c_str();

	// Open file and track time and operations :
	start_point_for_image_loading = std::chrono::high_resolution_clock::now();
	this->tiff_file = TinyTIFFReader_open(file_name);
	end_point_for_image_loading = std::chrono::high_resolution_clock::now();
	this->raw_file_opening_duration += (end_point_for_image_loading - start_point_for_image_loading);
	this->number_of_file_opening_operations++;

	uint32_t w = TinyTIFFReader_getWidth(this->tiff_file);
	uint32_t h = TinyTIFFReader_getHeight(this->tiff_file);

	// Set sizes :
	this->image_width = static_cast<std::size_t>(w);
	this->image_height = static_cast<std::size_t>(h);
	this->image_depth = static_cast<std::size_t>(nb_images);

	// Size of vector to allocate :
	std::size_t size_to_allocate = this->downsampling_enabled ?
					       (this->image_width * this->image_height) / 4 * this->image_depth * this->color_channels :
					       this->image_width * this->image_depth * this->image_height * this->color_channels;

	std::cout << "Downsampling is " << (this->downsampling_enabled ? "enabled." : "disabled.") << std::endl;
	std::cout << "Will allocate " << size_to_allocate << " elements (" << (static_cast<double>(size_to_allocate)/1024.)/1024. <<
		     "Mo) instead of " << this->image_width * this->image_depth * this->image_height * this->color_channels << " elements (" <<
		     (static_cast<double>(this->image_width * this->image_depth * this->image_height * this->color_channels)/1024.)/1024.<< "Mo)." <<std::endl;

	// Set data :
	this->image_data = static_cast<unsigned char*>(calloc(size_to_allocate, sizeof(unsigned char)));
	this->end_iterator = 0;

	return;
}

void image_stack_loader::print_debug_info() const {
	std::cout << "--------------------------------- TIME INFO ---------------------------------" << std::endl;
	std::cout << "Total time to open images : " << this->raw_file_opening_duration.count() << " ms for " << this->number_of_file_opening_operations << " operations. Around " << this->raw_file_opening_duration.count() / static_cast<double>(this->number_of_file_opening_operations) << " ms per operation" << std::endl;
	std::cout << "Total time to read images : " << this->raw_file_reading_duration.count() << " ms for " << this->number_of_file_reading_operations << " operations. Around " << this->raw_file_reading_duration.count() / static_cast<double>(this->number_of_file_reading_operations) << " ms per operation" << std::endl;
	std::cout << "Total time to copy images : " << this->raw_file_copying_duration.count() << " ms for " << this->number_of_file_copying_operations << " operations. Around " << this->raw_file_copying_duration.count() / static_cast<double>(this->number_of_file_copying_operations) << " ms per operation" << std::endl;
	std::cout << "Total time spent opening, reading, copying : " << this->raw_file_opening_duration.count() + this->raw_file_reading_duration.count() + this->raw_file_copying_duration.count() << "ms.\nTotal time elapsed since loading began : " << this->complete_stack_loading_duration.count() << " ms." << std::endl;
	std::cout << "--------------------------------- END  INFO ---------------------------------" << std::endl;
}

std::size_t image_stack_loader::get_image_depth() const { return this->image_depth; }

std::size_t image_stack_loader::get_image_width() const { return not this->downsampling_enabled ? this->image_width : this->image_width/2; }

std::size_t image_stack_loader::get_image_height() const { return not this->downsampling_enabled ? this->image_height : this->image_height/2; }

void image_stack_loader::enable_downsampling(bool _is_enabled) { this->downsampling_enabled = _is_enabled; }
