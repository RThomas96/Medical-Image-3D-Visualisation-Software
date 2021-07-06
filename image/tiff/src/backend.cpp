#include "../include/backend.hpp"

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
		// Using the high-resolution clock and its derivatives typedefs for time points and durations
		using clock_t = std::chrono::high_resolution_clock;
		using time_t = clock_t::time_point;
		using duration_t = clock_t::duration;
		// used for the last duration_cast, to have the time in ms, but still expressed as doubles
		using known_duration_t = std::chrono::duration<double, std::milli>;
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

		// Launch the parsing of files :
		before_thread_launch = clock_t::now();
		std::thread parsing_thread = std::thread(&TIFFBackendImpl::parse_info_in_separate_thread,
												 this, pre_existing_task, std::cref(filenames));
		after_thread_launch = clock_t::now();

		parsing_thread.detach();

		// Compute duration and output it in a human-readable format :
		thread_lauch_time = after_thread_launch - before_thread_launch;
		std::cerr << "Thread launch operation took "
				<< std::chrono::duration_cast<known_duration_t>(thread_lauch_time).count()
				<< "ms\n";

		return pre_existing_task;
	}

}

}
