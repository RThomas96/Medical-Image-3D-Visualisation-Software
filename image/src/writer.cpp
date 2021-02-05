#include "../include/writer.hpp"
#include "../../grid/include/discrete_grid.hpp"

namespace IO {

	GenericGridWriter::GenericGridWriter(const std::string _baseName, const std::string _basePath) {
		this->baseName = _baseName;
		this->basePath = _basePath;
		this->grid = nullptr;
		this->comment = "";
		this->bytesWritten = std::size_t(0);
		this->depthReached = std::size_t(0);
		this->isPreallocated = false;
		this->isOpen = false;
	}

	// The base destructor does nothing.
	GenericGridWriter::~GenericGridWriter() {
		std::cerr << "Deleting a grid writer ..." <<'\n';
	}

	GenericGridWriter& GenericGridWriter::preAllocateData() { return *this; }
	GenericGridWriter& GenericGridWriter::setBaseName(std::string bname) { this->baseName = bname; return *this; }
	GenericGridWriter& GenericGridWriter::setBasePath(std::string bpath) { this->basePath = bpath; return *this; }
	GenericGridWriter& GenericGridWriter::setGrid(const std::shared_ptr<DiscreteGrid>& g) { this->grid = g; return *this; }

	GenericGridWriter& GenericGridWriter::write() {
		return *this; // To be implemented in daughter classes.
	}

	GenericGridWriter& GenericGridWriter::setComment(const std::string _comment) {
		this->comment = _comment;
		return *this;
	}

	GenericGridWriter& GenericGridWriter::appendComment(const std::string _comment) {
		if (this->comment.empty()) {
			this->comment = _comment;
		} else {
			this->comment += '\n' + _comment;
		}
		return *this;
	}

	std::size_t GenericGridWriter::sizeWritten() const {
		return this->bytesWritten;
	}

	GenericGridWriter& GenericGridWriter::writeSlice(const std::vector<data_t> &sliceData, std::size_t sliceIdx) {
		return *this;
	}

	void GenericGridWriter::openFile() {
		return; // To be implemented in daughter classes.
	}

	void GenericGridWriter::openFileVersioned(std::size_t version, bool _binaryMode) {
		return; // To be implemented in daughter classes.
	}

	std::size_t GenericGridWriter::write_Once() {
		return 0; // To be implemented in daughter classes.
	}

	std::size_t GenericGridWriter::write_Depthwise(std::size_t depth) {
		return 0; // To be implemented in daughter classes.
	}

	DIMWriter::DIMWriter(const std::string _baseName, const std::string _basePath)
	: GenericGridWriter(_baseName, _basePath) {
		this->outputDIM = nullptr;
		this->outputIMA = nullptr;
	}

	DIMWriter::~DIMWriter() {
		std::cerr << "Deleting a grid writer of DIM ..." <<'\n';
		this->outputDIM->flush();
		this->outputIMA->flush();
		this->outputDIM->close();
		this->outputIMA->close();
	}

	DIMWriter& DIMWriter::preAllocateData() {
		if (this->grid == nullptr) { return *this; }
		if (this->isPreallocated) { return *this; }
		std::cerr << "Preallocating data ..." << '\n';
		this->isPreallocated = true;
		this->openFile();

		auto dims = this->grid->getResolution();
		std::cerr << "[LOG] preallocate() : Grid dimensions : " << dims.x << ',' << dims.y << ',' << dims.z << '\n';
		std::size_t streamsize = dims.x * dims.y;
		std::vector<data_t> emptydata(streamsize);
		for (std::size_t i = 0; i < dims.z; ++i) {
			this->outputIMA->write((const char*)emptydata.data(), streamsize);
		}
		this->outputIMA->seekp(0);

		return *this;
	}

	DIMWriter& DIMWriter::write() {
		this->bytesWritten = this->write_Once();
		this->depthReached = (this->bytesWritten == 0) ? 0 : this->grid->getResolution().z;
		return *this;
	}

	DIMWriter& DIMWriter::writeSlice(const std::vector<data_t> &sliceData, std::size_t sliceIdx) {
		if (this->grid == nullptr) { return *this; }
		if (not this->isOpen) { return *this; }
		if (not this->isPreallocated) { this->preAllocateData(); }

		std::size_t framesize = this->grid->getResolution().x * this->grid->getResolution().y * sizeof(data_t);
		this->outputIMA->seekp(framesize*sliceIdx);
		this->outputIMA->write((const char*)sliceData.data(), framesize);
		return *this;
	}

	void DIMWriter::openFile() {
		if (this->isOpen) { return; }
		this->isOpen = true;
		std::string dimName = this->basePath + '/' + this->baseName + ".dim";
		std::string imaName = this->basePath + '/' + this->baseName + ".ima";

		std::ios::openmode openingMode = std::ios::out | std::ios::trunc | std::ios::binary;

		this->outputDIM = new std::ofstream(dimName, openingMode);
		this->outputIMA = new std::ofstream(imaName, openingMode);

		if (not this->outputDIM->is_open()) {
			std::cerr << __FUNCTION__ << " : Warning, couldn't open " << dimName << '\n';
			this->outputDIM->close();
			this->outputIMA->close();
			this->outputDIM = nullptr;
			this->outputIMA = nullptr;
			return;
		}
		if (not this->outputIMA->is_open()) {
			std::cerr << __FUNCTION__ << " : Warning, couldn't open " << imaName << '\n';
			this->outputDIM->close();
			this->outputIMA->close();
			this->outputDIM = nullptr;
			this->outputIMA = nullptr;
			return;
		}

		this->writeDIMInfo();

		this->outputDIM->flush();
		this->outputIMA->flush();

		// Both files are opened, and ready to be written to.
		return;
	}

	std::size_t DIMWriter::write_Once() {
		if (this->grid == nullptr) { return 0; }
		if (this->outputDIM == nullptr || this->outputIMA == nullptr) {
			std::cerr << __FUNCTION__ << " : Could not write the contents to a file, one or more weren't opened." << '\n';
			return 0;
		}

		if (this->grid->hasData() == false) {
			std::cerr << "[ERROR] : The voxel grid passed to this writer did not have any data !\n";
			return 0;
		}

		if (not this->isOpen) { this->openFile(); }
		// Write the info about the grid to the dim file :
		if (not this->isPreallocated) { this->writeDIMInfo(); }

		// Write the data about the grid in bulk to the IMA file :
		const data_t* data = this->grid->getDataPtr();
		DiscreteGrid::sizevec3 size = this->grid->getResolution();
		this->outputIMA->write(reinterpret_cast<const char*>(data), size.x * size.y * size.z * sizeof(data_t));
		// FIXME : think the cast might not work here ... to see and test

		// Fixes a bug where the contents of the file for DIM
		// would not fill the buffered ofstream enough to write
		// to the file, instead keeping the data in memory :
		this->outputDIM->flush();
		this->outputIMA->flush();

		// Return the number of bytes written by the call :
		return static_cast<std::size_t>(this->outputDIM->tellp() + this->outputIMA->tellp());
	}

	void DIMWriter::writeDIMInfo() {
		/* Writes the file all at once. */
		if (this->grid == nullptr) { return; }

		// Writes the grid's dimensions
		svec3 imDims = this->grid->getResolution();
		*this->outputDIM << imDims.x << " " << imDims.y << " " << imDims.z << '\n';
		#ifdef VISUALISATION_USE_UINT8
		*this->outputDIM << "-type U8\n";
		#endif
		#ifdef VISUALISATION_USE_UINT16
		*this->outputDIM << "-type U16\n";
		#endif

		// Writes the voxel's dimensions within the grid :
		glm::vec3 vxDim	= this->grid->getVoxelDimensions();
		*this->outputDIM << "-dx " << vxDim.x << '\n';
		*this->outputDIM << "-dy " << vxDim.y << '\n';
		*this->outputDIM << "-dz " << vxDim.z << '\n';

		return;
	}

	SingleTIFFWriter::SingleTIFFWriter(const std::string _baseName, const std::string _basePath)
	: GenericGridWriter(_baseName, _basePath) {
		this->tiffFile = nullptr;
	}

	SingleTIFFWriter::~SingleTIFFWriter() {
		if (this->tiffFile != nullptr) {
			TinyTIFFWriter_close(this->tiffFile);
			this->tiffFile = nullptr;
		}
		std::cerr << "Deleting a grid writer of SINGLE TIFF..." <<'\n';
	}

	SingleTIFFWriter& SingleTIFFWriter::write() {
		this->bytesWritten = this->write_Once();
		this->depthReached = (this->bytesWritten == 0) ? 0 : this->grid->getResolution().z;
		return *this;
	}

	void SingleTIFFWriter::openTIFFFile(const std::shared_ptr<DiscreteGrid>& _vg) {
		uint16_t bps = static_cast<uint16_t>(sizeof(data_t)*8);
		svec3 dims = _vg->getResolution();
		uint32_t width = static_cast<uint32_t>(dims.x);
		uint32_t height = static_cast<uint32_t>(dims.y);

		std::string fileName = this->basePath + '/' + this->baseName + ".tif";
		uint16_t samples = 1; // 1 for grayscale, 3 for RGB (...)

		#warning To change if we change the data type
		TinyTIFFWriterSampleFormat sf = TinyTIFFWriter_UInt;

		#warning TinyTIFFWriter has not been tested here !
		this->tiffFile = TinyTIFFWriter_open(fileName.c_str(), bps, sf, samples, width, height, TinyTIFFWriter_Greyscale);
		if (this->tiffFile == nullptr) {
			std::cerr << __FUNCTION__ << " : Warning : Tiff file could not be opened." << '\n';
		}

		return;
	}

	std::size_t SingleTIFFWriter::write_Once() {
		if (this->grid == nullptr) { return 0; }
		this->openTIFFFile(this->grid);
		// Checks the file was opened :
		if (this->tiffFile == nullptr) {
			std::cerr << __FUNCTION__ << " : Warning : Could not write to file, since it was not opened." << '\n';
			return 0;
		}

		if (this->grid->hasData() == false) {
			std::cerr << "[ERROR] : The voxel grid associated with this writer did not have any data !\n";
			return 0;
		}

		// Get the data :
		const data_t* data = this->grid->getDataPtr();
		svec3 gridDims = this->grid->getResolution();
		std::size_t faceOffset = gridDims.x * gridDims.y;

		// Iterate on each 'face' :
		for (std::size_t i = 0; i < gridDims.z; ++i) {
			const data_t* frame = &(data[i * faceOffset]);
			TinyTIFFWriter_writeImage(this->tiffFile, (void*)frame);
		}

		// Close the file once finished :
		TinyTIFFWriter_close(this->tiffFile);
		this->tiffFile = nullptr;

		return faceOffset * gridDims.z;
	}

	StaggeredTIFFWriter::StaggeredTIFFWriter(const std::string _bN, const std::string _bP) : GenericGridWriter(_bN, _bP) {
		this->tiffFile = nullptr;
	}

	StaggeredTIFFWriter::~StaggeredTIFFWriter(void) {
		if (this->tiffFile != nullptr) {
			TinyTIFFWriter_close(this->tiffFile);
			if (TinyTIFFWriter_wasError(this->tiffFile)) {
				std::cerr << "[ERROR] : " << "Could not close file in StackedTIFFWriter.\n";
			}
			this->tiffFile = nullptr;
		}
		std::cerr << "Deleting a grid writer of multiple TIFF..." <<'\n';
	}

	StaggeredTIFFWriter& StaggeredTIFFWriter::preAllocateData() {
		if (this->grid == nullptr) { return *this; }
		if (this->isPreallocated) { return *this; }
		this->isPreallocated = true;

		// get total files to process :
		this->totalFiles = this->grid->getResolution().z;
		std::cerr << "Staggerered : max slices = "<< this->totalFiles << '\n';
		this->currentFile = 0;

		return *this;
	}

	StaggeredTIFFWriter& StaggeredTIFFWriter::write() {
		if (this->grid->hasData() == false) {
			std::cerr << "Error : cannot write a grid which has no data inside !" << '\n';
			return *this;
		}

		if (not this->isPreallocated) { this->preAllocateData(); }

		// Get the data :
		const data_t* data = this->grid->getDataPtr();
		svec3 gridDims = this->grid->getResolution();
		std::size_t faceOffset = gridDims.x * gridDims.y;

		// Iterate on each 'face' :
		for (std::size_t i = 0; i < gridDims.z; ++i) {
			//open file:
			this->openVersionnedTIFFFile(i);
			//get data:
			const data_t* frame = &(data[i * faceOffset]);
			// write and check for errors :
			TinyTIFFWriter_writeImage(this->tiffFile, (void*)frame);
			if (TinyTIFFWriter_wasError(this->tiffFile)) {
				std::cerr << "Error : writing slice " <<i << " to files resulted in an error.\n";
				std::cerr << "Message : " << TinyTIFFWriter_getLastError(this->tiffFile);
			}
		}

		return *this;
	}

	StaggeredTIFFWriter& StaggeredTIFFWriter::writeSlice(const std::vector<data_t> &sliceData, std::size_t sliceIdx) {
		if (sliceData.empty()) { return *this; }
		// open and write :
		this->openVersionnedTIFFFile(sliceIdx);
		TinyTIFFWriter_writeImage(this->tiffFile, sliceData.data());
		return *this;
	}

	void StaggeredTIFFWriter::openVersionnedTIFFFile(std::size_t index) {
		std::string fullpath = this->basePath + '/' + this->createSuffixedFilename(index) + ".tif";
		if (this->tiffFile != nullptr) {
			TinyTIFFWriter_close(this->tiffFile);
		}
		// Size of samples in bits :
		uint16_t bps = sizeof(data_t) * 8;
		// We deal in Uints here :
		TinyTIFFWriterSampleFormat sf = TinyTIFFWriter_UInt;
		// we only have grayscale :
		uint16_t samples = 1;
		TinyTIFFWriterSampleInterpretation si = TinyTIFFWriter_Greyscale;
		// set width/height :
		auto dims = this->grid->getResolution();
		std::cerr << "Image dimensions : " << dims.x << ',' << dims.y;
		uint32_t width = dims.x;
		uint32_t height = dims.y;

		// finally, open file :
		this->tiffFile = TinyTIFFWriter_open(fullpath.c_str(), bps, sf, 0, width, height, si);

		if (TinyTIFFWriter_wasError(this->tiffFile)) {
			std::cerr << "Error occured while trying to open " << fullpath << "\n";
			std::cerr << "Message : " << TinyTIFFWriter_getLastError(this->tiffFile);
		}

		return;
	}

	std::size_t StaggeredTIFFWriter::computeSuffixLength(std::size_t maxidx) {
		return std::to_string(maxidx).length();
	}

	std::string StaggeredTIFFWriter::createSuffixedFilename(std::size_t idx) {
		std::string suffixed = this->baseName + "_";
		std::size_t completeLength = this->computeSuffixLength(this->totalFiles);
		std::size_t currentLength = this->computeSuffixLength(idx);
		// add as many 0's as necessary :
		for (; completeLength > currentLength; --completeLength) { suffixed += "0"; }
		// add number at the end :
		suffixed += std::to_string(idx);
		// return complete
		return suffixed;
	}

}
