#include "../include/tiff_reader.hpp"

#include "../include/tiff_reader_templated.hpp"

#include <thread>
#include <memory>
#include <vector>
#include <utility>
#include <set>

#include <QXmlStreamReader>
#include <cstring>

namespace Image {

namespace Tiff {

	TIFFReader::TIFFReader(void) : GenericImageReader() {}

	std::string TIFFReader::getImageName() const { return this->stack_base_name; }

	void TIFFReader::setImageName(std::string &_user_defined_name_) {
		if (_user_defined_name_.empty()) { return; }
		this->stack_base_name = _user_defined_name_;
	}

	std::size_t TIFFReader::getVoxelDimensionality() const { return this->voxel_dimensionality; }

	glm::vec3 TIFFReader::getVoxelDimensions() const { return this->voxel_dimensions; }

	svec3 TIFFReader::getResolution() const { return this->resolution; }

	ImageDataType TIFFReader::getInternalDataType() const { return this->internal_data_type; }

	BoundingBox_General<float> TIFFReader::getBoundingBox() const {
		// By default, in image space bounding box is resolution * voxel dimensions
		glm::vec3 dimensions = glm::convert_to<float>(this->resolution);
		glm::vec3 size = dimensions * this->voxel_dimensions;
		return BoundingBox_General<float>(glm::vec3{.0f, .0f, .0f}, size);
	}

	ThreadedTask::Ptr TIFFReader::parseImageInfo(ThreadedTask::Ptr pre_existing_task,
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
		std::thread parsing_thread = std::thread(&TIFFReader::parse_info_in_separate_thread,
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

	/// @brief Static function to extract info from the first OME TIFF image in order to create right backend. 
	/// @details Open the first image and extract bits per pixel, image width, image height, sample format.
    /// The implementation do not use the frame implementation as the OME TIFF format do not contain classic fields.
    /// Information are extracted from the XML file contain in the OME TIFF first image comment.
    static void extractInfoFromFirstImageOMETIFF(const std::string reference_filename, uint16_t& bps, uint32_t& w, uint32_t& h, uint16_t& sf, uint32_t& z, glm::vec3& voxel_dim){
        if (reference_filename.size() == 0) { return; }

        //Open the first file (supposed to have header) :
        TIFF* file_with_header = TIFFOpen(reference_filename.c_str(), "r");
        char* desc = nullptr;

        // Get the description :
        int result = TIFFGetField(file_with_header, TIFFTAG_IMAGEDESCRIPTION, &desc);
        if (result != 1) {
            throw std::runtime_error("Error: Could not read the first image used for the specification of the OME TIFF format: " + reference_filename);
        }

        // If the description is empty, retun here :
        if (strlen(desc) == 0) { throw std::runtime_error("Error: the first image used for the specification of the OME TIFF format did not contain any description"); }

        QXmlStreamReader reader;
        reader.addData(desc);

        // Check if the XML data is ill-formed or not :
        if (reader.hasError()) {
            QString msg = "[" + QString::number(reader.lineNumber()) + "," + QString::number(reader.columnNumber()) +
                "] Error : " + reader.errorString();
            reader.clear();
            throw std::runtime_error(msg.toStdString());
        }

        // For all tokens in the document :
        while (not reader.atEnd()) {
            if (reader.readNext() == QXmlStreamReader::TokenType::StartElement) {
                // If we're in the Pixels tag (where the data is) :
                if (reader.name().compare(QString("Pixels"), Qt::CaseSensitivity::CaseSensitive) == 0) {
                    // Get attributes of the image :
                    auto attributes = reader.attributes();

                    // Attributes for the voxel dimensions :
                    if (attributes.hasAttribute("PhysicalSizeX"))
                        voxel_dim.x = attributes.value("PhysicalSizeX").toFloat();
                    if (attributes.hasAttribute("PhysicalSizeY"))
                        voxel_dim.y = attributes.value("PhysicalSizeY").toFloat();
                    if (attributes.hasAttribute("PhysicalSizeZ"))
                        voxel_dim.z = attributes.value("PhysicalSizeZ").toFloat();

                    // Attributes for the image dimensions :
                    if (attributes.hasAttribute("SizeX"))
                        w = attributes.value("SizeX").toUInt();
                    if (attributes.hasAttribute("SizeY"))
                        h = attributes.value("SizeY").toUInt();
                    if (attributes.hasAttribute("SizeZ"))
                        z = attributes.value("SizeZ").toUInt();

                    // Data type :
                    if (attributes.hasAttribute("PixelType")) {
                        std::string full_data_type = attributes.value("PixelType").toString().toStdString();
                        uint16_t parsed_bps = 8;
                        bool found_valid_dataType = false;
                        std::size_t found_pos = 0;
                        while(parsed_bps <= 64) {
                            found_pos = full_data_type.find_first_of(std::to_string(parsed_bps));
                            found_valid_dataType = (found_pos != std::string::npos);
                            if(found_valid_dataType)
                                break;
                            parsed_bps = parsed_bps * 2;
                        }

                        if(!found_valid_dataType)
                            throw std::runtime_error("Error: Wrong datatype in the PixelType attribute of the XML commentary in OMETIFF file: " + full_data_type);
                        else
                            bps = parsed_bps;

                        std::string dataType = full_data_type.substr(0, found_pos);
                        if(dataType == "uint") {
                            sf = SAMPLEFORMAT_UINT;
                        } else if(dataType == "int") {
                            sf = SAMPLEFORMAT_INT;
                        } else if(dataType == "float") {
                            sf = SAMPLEFORMAT_IEEEFP;
                        } else { 
                            throw std::runtime_error("The file's internal type was not recognized (not in libTIFF's types).");
                        }
                    }
                }
            }
        }
    }

	/// @brief Static function to extract info from the first TIFF image in order to create right backend. 
	/// @details Open the first image and extract bits per pixel, image width, image height, sample format.
    /// The implementation use the frame implementation to parse the first image. 
    static bool extractInfoFromFirstImageTIFF(const std::string reference_filename, uint16_t& bps, uint32_t& w, uint32_t& h, uint16_t& sf){
        
        Frame::Ptr reference_frame = nullptr;

        try {
            reference_frame = std::make_shared<Frame>(reference_filename, 0);
        }  catch (const std::exception& _e) {
            std::cerr << "Error : could not create backend. Error message : " << _e.what() << '\n';
            return false;
        }

        // get the file handle by libtiff :
        TIFF* f = reference_frame->getLibraryHandle();
        // get the number of bits per pixel :
        bps = reference_frame->bitsPerSample(f);
        // get frame information :
        w = reference_frame->width(f);
        h = reference_frame->height(f);
        sf = reference_frame->sampleFormat(f);
		std::cerr << "Reference frame's samples per pixel : " << reference_frame->samplesPerPixel << "\n";
        TIFFClose(f);
        return true;
    }

	TIFFReader::Ptr createBackend(const std::string reference_filename, bool isOMETIFF) {
        
        // Bits per sample
        uint16_t bps = 0;
        // Image width
        uint32_t w = 0;
        // Image height
        uint32_t h = 0;
        // Sample format
        uint16_t sf = SAMPLEFORMAT_INT;
        // Image dimensionnality (number of channel)
        std::size_t _dim = 0;
        // Image depth
	    uint32_t z = 0;
        // Voxel size in reel world (in order to keep multiple grids at scale)
        glm::vec3 voxel_dim = glm::vec3{1.f, 1.f, 1.f};

        if(isOMETIFF){
            extractInfoFromFirstImageOMETIFF(reference_filename, bps, w, h, sf, z, voxel_dim);
        } else {
            // Regular tiff format do not contain z and voxel_dim data...
            // They need to be manually set by the user
            extractInfoFromFirstImageTIFF(reference_filename, bps, w, h, sf);
        }

        TIFFReader::Ptr pImpl = nullptr;
		// Chooses the right type based on sample formats, and bit widths of the samples :
		switch (sf) {
			case SAMPLEFORMAT_VOID:
				throw std::runtime_error("Internal type of the frame was void.");
			break;

			case SAMPLEFORMAT_UINT: {
				if (bps == 8) { pImpl = TIFFReaderTemplated<std::uint8_t>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
				if (bps == 16) { pImpl = TIFFReaderTemplated<std::uint16_t>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
				if (bps == 32) { pImpl = TIFFReaderTemplated<std::uint32_t>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
				if (bps == 64) { pImpl = TIFFReaderTemplated<std::uint64_t>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
				std::string err = "Sample was UInt, but no matching ctor was found for "+std::to_string(bps)+" bits.";
				throw std::runtime_error(err);
			}
			break;

			case SAMPLEFORMAT_INT: {
				if (bps == 8) { pImpl = TIFFReaderTemplated<std::int8_t>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
				if (bps == 16) { pImpl = TIFFReaderTemplated<std::int16_t>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
				if (bps == 32) { pImpl = TIFFReaderTemplated<std::int32_t>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
				if (bps == 64) { pImpl = TIFFReaderTemplated<std::int64_t>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
				std::string err = "Sample was Int, but no matching ctor was found for "+std::to_string(bps) + " bits.";
				throw std::runtime_error(err);
			}
			break;

			case SAMPLEFORMAT_IEEEFP: {
				if (bps == 32) { pImpl = TIFFReaderTemplated<float>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
				if (bps == 64) { pImpl = TIFFReaderTemplated<double>::createBackend(w, h, _dim, z, voxel_dim); return pImpl; }
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
