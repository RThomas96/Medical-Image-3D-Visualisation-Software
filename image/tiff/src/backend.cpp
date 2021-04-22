#include "../include/backend.hpp"

#include <thread>

namespace Image {

	TIFFBackend::TIFFBackend(const std::vector<std::vector<std::string>>& fns) : ImageBackendImpl(fns) {
	}

	bool TIFFBackend::canReadImage(const std::string& image_name) {
		// Check the file ends with a known and good extension, and then check we can open it.
		std::size_t extPos = image_name.find_last_of(".");
		// if no extension is present, the file type is unknown, return false :
		if (extPos == std::string::npos) { return false; }

		// Extract the extension from the name :
		std::string ext = image_name.substr(extPos, std::string::npos);

		#warning All file extensions of TIFF are not specified here.
		// If the extension is known, return true !
		if (ext == ".tif" || ext == ".tiff") {
			// If the extension is valid, then try to parse the first frame of the file :
			try {
				#warning Doesn't take into account the templated backends of this class.
				Tiff::Frame::Ptr test_frame = std::make_shared<Tiff::Frame>(image_name.data(), 0);
				return true;
			} catch (std::exception _e) {
				#warning Should we alert the user here ?
				return false;
			}
		}

		// Otherwise, return false
		return false;
	}

	ThreadedTask::Ptr TIFFBackend::parseImageInfo(void) {
		/* Parses the image in a separate thread. Detaches it from the main thread, in order to
		 * let it free up its own resources. */

		// Create a task object :
		ThreadedTask::Ptr parseTask = std::make_shared<ThreadedTask>();

		// If no filenames are provided, just return and end the task :
		if (this->filenames.empty()) { parseTask->end(); return parseTask; }

		// Launch the parsing of files in a separate thread :
		std::thread parseThread = std::thread(&TIFFBackend::parseImageInfo_thread, this, parseTask);
		// Wait for the task to be initialized, in 5ms increments :
		while (parseTask->getMaxSteps() == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(5)); }
		// Detach the thread, allowing it to continue executing outside of this scope :
		parseThread.detach();

		return parseTask;
	}

}
