#include "../include/backend.hpp"

#include "../include/templated_backend.hpp"

#include <thread>
#include <memory>
#include <vector>
#include <utility>
#include <set>

namespace Image {

namespace Tiff {

	TIFFBackendImpl::TIFFBackendImpl(void) : ImageBackendImpl() {}

	std::string TIFFBackendImpl::getImageName() const { return this->stack_base_name; }

	void TIFFBackendImpl::setImageName(std::string &_user_defined_name_) {
		if (_user_defined_name_.empty()) { return; }
		this->stack_base_name = _user_defined_name_;
	}

	std::size_t TIFFBackendImpl::getVoxelDimensionality() const { return this->voxel_dimensionality; }

	glm::vec3 TIFFBackendImpl::getVoxelDimensions() const { return this->voxel_dimensions; }

	svec3 TIFFBackendImpl::getResolution() const { return this->resolution; }

	ImageDataType TIFFBackendImpl::getInternalDataType() const { return this->internal_data_type; }

	BoundingBox_General<float> TIFFBackendImpl::getBoundingBox() const {
		// By default, in image space bounding box is resolution * voxel dimensions
		glm::vec3 dimensions = glm::convert_to<float>(this->resolution);
		glm::vec3 size = dimensions * this->voxel_dimensions;
		return BoundingBox_General<float>(glm::vec3{.0f, .0f, .0f}, size);
	}

	ThreadedTask::Ptr TIFFBackendImpl::parseImageInfo(ThreadedTask::Ptr pre_existing_task,
													  const std::vector<std::vector<std::string>>& filenames) {
		// Use the high-resolution clock and its typedefs for time points and durations
		using clock_t = std::chrono::high_resolution_clock;
		using time_t = clock_t::time_point;
		using duration_t = clock_t::duration;
		using known_duration_t = std::chrono::duration<double, std::milli>; // duration : ms, expressed in double
		// Used to measure the time taken to launch the thread :
		time_t before_thread_launch = time_t(), after_thread_launch = time_t();
		duration_t thread_lauch_time = duration_t::zero();

		// Check the task exists, and if not, create it :
		if (pre_existing_task == nullptr) { pre_existing_task = std::make_shared<ThreadedTask>(); }

		// Return if no filenames are provided :
		if (filenames.empty()) {
			pre_existing_task->pushMessage("No filenames were provided.");
			pre_existing_task->end(false);
			return pre_existing_task;
		}

		pre_existing_task->setState(TaskState::Ready);

		// Launch the parsing of files :
		before_thread_launch = clock_t::now();
		std::thread parsing_thread = std::thread(&TIFFBackendImpl::parse_info_in_separate_thread,
												 this, pre_existing_task, std::cref(filenames));
		after_thread_launch = clock_t::now();
		// Let the thread detach naturally :
		parsing_thread.detach();

		// Compute duration and output it in a human-readable format :
		thread_lauch_time = after_thread_launch - before_thread_launch;
		std::cerr << "Thread launch operation took "
				<< std::chrono::duration_cast<known_duration_t>(thread_lauch_time).count()
				<< "ms\n";

		return pre_existing_task;
	}

	TIFFBackendImpl::Ptr createBackend(const std::string reference_filename) {
		Frame::Ptr reference_frame = nullptr;

		try {
			reference_frame = std::make_shared<Frame>(reference_filename, 0);
		}  catch (const std::exception& _e) {
			std::cerr << "Error : could not create backend. Error message : " << _e.what() << '\n';
			return nullptr;
		}

		TIFFBackendImpl::Ptr pImpl = nullptr;
		std::size_t _dim = 0;

		// get the file handle by libtiff :
		TIFF* f = reference_frame->getLibraryHandle();
		// get the number of bits per pixel :
		uint16_t bps = reference_frame->bitsPerSample(f);
		// get frame information :
		uint32_t w = reference_frame->width(f);
		uint32_t h = reference_frame->height(f);
		uint16_t sf = reference_frame->sampleFormat(f);
		TIFFClose(f);

		// Chooses the right type based on sample formats, and bit widths of the samples :
		switch (sf) {
			case SAMPLEFORMAT_VOID:
				throw std::runtime_error("Internal type of the frame was void.");
			break;

			case SAMPLEFORMAT_UINT: {
				if (bps == 8) { pImpl = TIFFBackendDetail<std::uint8_t>::createBackend(w, h, _dim); return pImpl; }
				if (bps == 16) { pImpl = TIFFBackendDetail<std::uint16_t>::createBackend(w, h, _dim); return pImpl; }
				if (bps == 32) { pImpl = TIFFBackendDetail<std::uint32_t>::createBackend(w, h, _dim); return pImpl; }
				if (bps == 64) { pImpl = TIFFBackendDetail<std::uint64_t>::createBackend(w, h, _dim); return pImpl; }
				std::string err = "Sample was UInt, but no matching ctor was found for "+std::to_string(bps)+" bits.";
				throw std::runtime_error(err);
			}
			break;

			case SAMPLEFORMAT_INT: {
				if (bps == 8) { pImpl = TIFFBackendDetail<std::int8_t>::createBackend(w, h, _dim); return pImpl; }
				if (bps == 16) { pImpl = TIFFBackendDetail<std::int16_t>::createBackend(w, h, _dim); return pImpl; }
				if (bps == 32) { pImpl = TIFFBackendDetail<std::int32_t>::createBackend(w, h, _dim); return pImpl; }
				if (bps == 64) { pImpl = TIFFBackendDetail<std::int64_t>::createBackend(w, h, _dim); return pImpl; }
				std::string err = "Sample was Int, but no matching ctor was found for "+std::to_string(bps) + " bits.";
				throw std::runtime_error(err);
			}
			break;

			case SAMPLEFORMAT_IEEEFP: {
				if (bps == 32) { pImpl = TIFFBackendDetail<float>::createBackend(w, h, _dim); return pImpl; }
				if (bps == 64) { pImpl = TIFFBackendDetail<double>::createBackend(w, h, _dim); return pImpl; }
				std::string err = "Sample was floating point , but no matching ctor was found for "+std::to_string(bps)
								+ " bits.";
				throw std::runtime_error(err);
			}
			break;

			case SAMPLEFORMAT_COMPLEXINT:
				throw std::runtime_error("The file's internal type was complex integers (not supported).");
			break;

			case SAMPLEFORMAT_COMPLEXIEEEFP:
				throw std::runtime_error("The file's internal type was complex floating points (not supported).");
			break;

			default:
				throw std::runtime_error("The file's internal type was not recognized (not in libTIFF's types).");
			break;
		}
	}

}

}
