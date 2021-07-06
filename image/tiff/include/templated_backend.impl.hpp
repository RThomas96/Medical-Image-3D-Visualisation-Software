#ifndef	VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_IMPL_HPP_
#define	VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_IMPL_HPP_

#ifndef  VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_HPP_
#include "./templated_backend.hpp"		// Included between header guards to have code completion in QtCreator.
#endif

namespace Image {

namespace Tiff {

template <typename unsupported_element_t>
TIFFBackendDetail<unsupported_element_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	UNUSED(w); UNUSED(h); UNUSED(_dim);
	throw std::runtime_error("Error : unsupported type passed to the TIFFBackendDetail constructor.");
}

template <>
TIFFBackendDetail<std::uint8_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_8;

	this->bitsPerSample = 8;
	this->sampleFormat = SAMPLEFORMAT_UINT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint8_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::uint16_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_16;

	this->bitsPerSample = 16;
	this->sampleFormat = SAMPLEFORMAT_UINT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint16_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::uint32_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_32;

	this->bitsPerSample = 32;
	this->sampleFormat = SAMPLEFORMAT_UINT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint32_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::uint64_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_64;

	this->bitsPerSample = 64;
	this->sampleFormat = SAMPLEFORMAT_UINT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint64_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::int8_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_8;

	this->bitsPerSample = 8;
	this->sampleFormat = SAMPLEFORMAT_INT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int8_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::int16_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_16;

	this->bitsPerSample = 16;
	this->sampleFormat = SAMPLEFORMAT_INT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int16_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::int32_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_32;

	this->bitsPerSample = 32;
	this->sampleFormat = SAMPLEFORMAT_INT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int32_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::int64_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_64;

	this->bitsPerSample = 64;
	this->sampleFormat = SAMPLEFORMAT_INT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int64_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<float>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_32;

	this->bitsPerSample = 32;
	this->sampleFormat = SAMPLEFORMAT_IEEEFP;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(float) << '\n';
	#endif
}

template <>
TIFFBackendDetail<double>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_64;

	this->bitsPerSample = 64;
	this->sampleFormat = SAMPLEFORMAT_IEEEFP;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(double) << '\n';
	#endif
}

template <typename element_t> typename TIFFBackendDetail<element_t>::Ptr
TIFFBackendDetail<element_t>::createBackend(uint32_t width, uint32_t height, std::size_t _dim) {
	return Ptr(new TIFFBackendDetail<pixel_t>(width, height, _dim));
}

template <typename element_t>
void TIFFBackendDetail<element_t>::cleanResources() {
	// Clear cache and loaded image data :
	this->cachedSlices.clearCache();
	this->images.clear();
	// Reset all variables to either their original state, or a known error value (like voxdims = -1) :
	this->internal_data_type = ImageDataType::Unknown;
	this->resolution = svec3{0, 0, 0};
	this->voxel_dimensions = glm::vec3{-1.f, -1.f, -1.f};
	this->voxel_dimensionality = 0;
	this->bitsPerSample = 0;
	this->sampleFormat = SAMPLEFORMAT_VOID;
	this->samplesPerPixel = 0;
}

template <typename element_t>
void TIFFBackendDetail<element_t>::compute_stack_basename(void) {
	if (this->images.empty()) { this->stack_base_name = "<undefined>"; }
	this->stack_base_name = "default_grid_name";
}

template <typename element_t>
std::size_t TIFFBackendDetail<element_t>::loadSlice(std::size_t i) {
	// TODO: import from the existing loadslice funtion
}

template <typename element_t>
template <typename out_t>
bool TIFFBackendDetail<element_t>::template_tiff_read_sub_region(svec3 origin, svec3 size, std::vector<out_t>& values) {
	// TODO: import from the existing read subregion function
}

template <typename element_t>
template <typename range_t>
bool TIFFBackendDetail<element_t>::template_tiff_get_sub_range_values(std::size_t channel, glm::tvec2<range_t>& range) {
	// If channel unavailable, return false and leave the ranges uninitialized.
	if (channel >= this->voxel_dimensionality) { return false; }
	// If the channel is available, return the value ranges :
	range = this->value_ranges[channel];
	return true;
}

template <typename element_t>
void TIFFBackendDetail<element_t>::parse_info_in_separate_thread(ThreadedTask::Ptr _task,
												const std::vector<std::vector<std::string>>& user_filenames) {
	// Should be checked in the launcher of this thread, but just to be sure :
	if (user_filenames.empty()) {
		_task->pushMessage("No filenames were provided.");
		_task->end(false);
		return;
	}

	// First of all, we should check the current object is of the right type !
	Frame::Ptr reference_frame = nullptr;
	try {
		reference_frame = std::make_shared<Frame>(user_filenames[0][0], 0);
	}  catch (std::exception& e) {
		_task->pushMessage(std::string("Error : could not create a ref.frame for file parsing. Message : \n")+e.what());
		_task->end(false);
		return;
	}
	if (not this->is_frame_compatible_with_backend(_task, reference_frame)) {
		_task->end(false);
		this->cleanResources();
		return;
	}

	// Check the same number of files are here per stack of filenames.
	// If there's only one do nothing. Otherwise :
	if (user_filenames.size() > 1) {
		// Otherwise, check each additionnal stack and see if they have the same # of files :
		bool valid = true;
		std::size_t ref_count = user_filenames[0].size(); // reference count

		for (std::size_t i = 1; i < user_filenames.size()-1; i++) {
			if (user_filenames[i].size() != ref_count) {
				// signal it's not valid, but don't stop here.
				valid = false;
				_task->pushMessage(
					std::string("Stack of files number <")+std::to_string(i)+std::string("> does not contain the"
					" requisite amount of filenames (got ")+std::to_string(user_filenames[i].size())+std::string(
					", instead of the expected ")+std::to_string(ref_count)+std::string(").")
				);
			}
		}

		// after all stacks are done, stop here if an error occured.
		if (not valid) { this->cleanResources(); _task->end(false); return; }
	}

	// The number of file stacks :
	std::size_t file_channels = user_filenames.size();

	// Then, check the files are the 'same' (same encoding [bitsperpixel, samplesperpixel] and size and frame count)
	for (std::size_t file_index = 0; file_index < user_filenames[0].size(); ++file_index) {

		// Get the number of available directories :
		tdir_t current_dir_count = countDirectories(std::string_view(user_filenames[0][file_index]));
		// Check all other files in the different stacks have the same number of directories :
		for (std::size_t c = 1; c < file_channels; ++c) {
			tdir_t temp_dir_count = countDirectories(std::string_view(user_filenames[c][file_index]));
			if (temp_dir_count != current_dir_count) {
				_task->pushMessage(
					std::string("Error : file \"")+user_filenames[c][file_index]+std::string("\" does not have the"
					" expected count of TIFF images (expected ")+std::to_string(current_dir_count)+std::string(", "
					"but got ")+std::to_string(temp_dir_count)+std::string(" instead).")
				);
				this->cleanResources();
				_task->end(false);
				return;
			}
		}

		// Now, we _know_ all files have the same number of directories.
		// Iterate, and build the image representations out of the directories :
		for (std::size_t dir = 0; dir < current_dir_count; ++dir) {
			// Create a container for the image :
			TIFFImage current_image = TIFFImage();
			Tiff::Frame::Ptr current_frame = nullptr;

			// iterate on all file stacks :
			for (std::size_t c = 0; c < file_channels; ++c) {
				try {
					// Attempt to create a frame from this file, at directory index 'dir'
					current_frame = std::make_shared<Tiff::Frame>(user_filenames[c][file_index], dir);
				}  catch (std::exception e) {
					_task->pushMessage(
						std::string("Error : parsing file \"")+user_filenames[c][file_index]+std::string("produced"
						"an error at directory ")+std::to_string(dir)+std::string(". Error message : ")+e.what()
					);
					this->cleanResources();
					_task->end(false);
					return;
				}

				// Check if the frame is compatible with the reference frame :
				if (not reference_frame->isCompatibleWith(*current_frame)) {
					_task->pushMessage(
						std::string("Error : frame ")+std::to_string(dir)+std::string(" of file ")+
						user_filenames[c][file_index]+std::string(" is not compatible with the rest of the frames.")
					);
					this->cleanResources();
					_task->end(false);
					return;
				}

				// push the full slice onto the stack :
				current_image.push_back(current_frame);
			}

			// push current image to the stack :
			this->images.push_back(current_image);
		}
	}

	// Reminder : in the ctor, we must specify the width and height of the grid, as well as te voxel dimensionality

	// populate the fields with the right information from the loaded data :
	std::size_t total_samples = 0;
	for (std::size_t i = 0; i < file_channels; ++i) {
		total_samples += this->images[0][i]->samplesPerPixel;
	}
	// set samples per pixel :
	this->samplesPerPixel = total_samples;
	// set stack depth :
	this->resolution.z = this->images.size();
}

template <typename element_t>
bool TIFFBackendDetail<element_t>::is_frame_compatible_with_backend(ThreadedTask::Ptr& _task,
																	Frame::Ptr& reference_frame) const {
	// Open the file :
	TIFF* lib_handle = reference_frame->getLibraryHandle();

	bool valid = true;
	std::uint16_t bits_per_sample = reference_frame->bitsPerSample(lib_handle);
	std::uint16_t sample_format = reference_frame->sampleFormat(lib_handle);
	std::uint16_t samples_per_pixel = reference_frame->samplesPerPixel;
	std::uint32_t width = reference_frame->width(lib_handle);
	std::uint32_t height = reference_frame->height(lib_handle);

	if (this->sampleFormat != sample_format) {
		_task->pushMessage("Error : sample format was not identical between reference frame and current TIFF backend.");
		valid = false;
	}
	if (this->samplesPerPixel != samples_per_pixel) {
		_task->pushMessage("Error : samples/pixel was not identical between reference frame and current TIFF backend.");
		valid = false;
	}
	if (this->bitsPerSample != bits_per_sample) {
		_task->pushMessage("Error : bits/sample was not identical between reference frame and current TIFF backend.");
		valid = false;
	}
	if (this->resolution.x != width) {
		_task->pushMessage("Error : frame width was not identical between reference frame and current TIFF backend.");
		valid = false;
	}
	if (this->resolution.y != height) {
		_task->pushMessage("Error : frame height was not identical between reference frame and current TIFF backend.");
		valid = false;
	}

	TIFFClose(lib_handle);
	return valid;
}

} // namespace Tiff

} // namespace Image

#endif //  VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_IMPL_HPP_
