#include "../include/writer.hpp"
#include "../../grid/include/discrete_grid.hpp"

namespace IO {

	GenericGridWriter::GenericGridWriter(const std::string _baseName, bool _binaryMode) {
		this->baseName = _baseName;
		this->comment = "";
		this->bytesWritten = std::size_t(0);
		this->depthReached = std::size_t(0);
	}

	// The base destructor does nothing.
	GenericGridWriter::~GenericGridWriter() {}

	GenericGridWriter& GenericGridWriter::write(const std::shared_ptr<DiscreteGrid>& _vg) {
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

	void GenericGridWriter::openFile(bool _binaryMode) {
		return; // To be implemented in daughter classes.
	}

	void GenericGridWriter::openFileVersioned(std::size_t version, bool _binaryMode) {
		return; // To be implemented in daughter classes.
	}

	std::size_t GenericGridWriter::write_Once(const std::shared_ptr<DiscreteGrid>& _vg) {
		return 0; // To be implemented in daughter classes.
	}

	std::size_t GenericGridWriter::write_Depthwise(const std::shared_ptr<DiscreteGrid>& _vg, std::size_t depth) {
		return 0; // To be implemented in daughter classes.
	}

	DIMWriter::DIMWriter(const std::string _baseName) : GenericGridWriter(_baseName) {
		this->outputDIM = nullptr;
		this->outputIMA = nullptr;
		this->openFile();
	}

	DIMWriter::~DIMWriter() {
		this->outputDIM->close();
		this->outputIMA->close();
	}

	DIMWriter& DIMWriter::write(const std::shared_ptr<DiscreteGrid>& _vg) {
		this->bytesWritten = this->write_Once(_vg);
		this->depthReached = _vg->getGridDimensions().z;
		return *this;
	}

	void DIMWriter::openFile(bool _binaryMode) {
		// Note : binarymode is ignored here, DIM should always be non-binary, whilst the IMA should.

		std::string dimName = this->baseName + ".dim";
		std::string imaName = this->baseName + ".ima";

		std::ios::openmode openingMode = std::ios::out | std::ios::trunc;

		this->outputDIM = new std::ofstream(dimName, openingMode);
		this->outputIMA = new std::ofstream(imaName, openingMode | std::ios::binary);

		if (not this->outputDIM->is_open()) {
			std::cerr << __FUNCTION__ << " : Warning, couldn't open " << dimName << '\n';
			this->outputDIM->close();
			this->outputIMA->close();
			this->outputDIM = nullptr;
			this->outputIMA = nullptr;
		}
		if (not this->outputIMA->is_open()) {
			std::cerr << __FUNCTION__ << " : Warning, couldn't open " << imaName << '\n';
			this->outputDIM->close();
			this->outputIMA->close();
			this->outputDIM = nullptr;
			this->outputIMA = nullptr;
		}

		// Both files are opened, and ready to be written to.
		return;
	}

	std::size_t DIMWriter::write_Once(const std::shared_ptr<DiscreteGrid>& _vg) {
		if (this->outputDIM == nullptr || this->outputIMA == nullptr) {
			std::cerr << __FUNCTION__ << " : Could not write the contents to a file, one or more weren't opened." << '\n';
			return 0;
		}

		// Write the info about the grid to the dim file :
		this->writeDIMInfo(_vg);

		// Write the data about the grid in bulk to the IMA file :
		const std::vector<unsigned char>& data = _vg->getData();
		this->outputIMA->write((const char*)data.data(), data.size() * sizeof(unsigned char));
		// FIXME : think the cast might not work here ... to see and test

		// Fixes a bug where the contents of the file for DIM
		// would not fill the buffered ofstream enough to write
		// to the file, instead keeping the data in memory :
		this->outputDIM->flush();
		this->outputIMA->flush();

		// Return the number of bytes written by the call :
		return static_cast<std::size_t>(this->outputDIM->tellp() + this->outputIMA->tellp());
	}

	void DIMWriter::writeDIMInfo(const std::shared_ptr<DiscreteGrid>& _vg) {
		/* Writes the file all at once. */

		// Writes the grid's dimensions
		svec3 imDims = _vg->getGridDimensions();
		*this->outputDIM << imDims.x << " " << imDims.y << " " << imDims.z << '\n';
		*this->outputDIM << "-type U8\n";

		// Writes the voxel's dimensions within the grid :
		glm::vec3 vxDim	= _vg->getVoxelDimensions();
		*this->outputDIM << "-dx " << vxDim.x << '\n';
		*this->outputDIM << "-dy " << vxDim.y << '\n';
		*this->outputDIM << "-dz " << vxDim.z << '\n';

		// TODO : add other properties here ...

		return;
	}

	SingleTIFFWriter::SingleTIFFWriter(const std::string _baseName) : GenericGridWriter(_baseName) {
		this->tiffFile = nullptr;
	}

	SingleTIFFWriter::~SingleTIFFWriter() {
		if (this->tiffFile != nullptr) {
			TinyTIFFWriter_close(this->tiffFile);
			this->tiffFile = nullptr;
		}
	}

	SingleTIFFWriter& SingleTIFFWriter::write(const std::shared_ptr<DiscreteGrid>& _vg) {
		this->bytesWritten = this->write_Once(_vg);
		this->depthReached = _vg->getGridDimensions().z;
		return *this;
	}

	void SingleTIFFWriter::openTIFFFile(const std::shared_ptr<DiscreteGrid>& _vg) {
		uint16_t bps = static_cast<uint16_t>(sizeof(unsigned char));
		svec3 dims = _vg->getGridDimensions();
		uint32_t width = static_cast<uint32_t>(dims.x);
		uint32_t height = static_cast<uint32_t>(dims.y);

		std::string fileName = this->baseName + ".tif";

		this->tiffFile = TinyTIFFWriter_open(fileName.c_str(), bps, width, height);
		if (this->tiffFile == nullptr) {
			std::cerr << __FUNCTION__ << " : Warning : Tiff file could not be opened." << '\n';
		}

		return;
	}

	std::size_t SingleTIFFWriter::write_Once(const std::shared_ptr<DiscreteGrid>& _vg) {
		this->openTIFFFile(_vg);
		// Checks the file was opened :
		if (this->tiffFile == nullptr) {
			std::cerr << __FUNCTION__ << " : Warning : Could not write to file, since it was not opened." << '\n';
			return 0;
		}

		// Get the data :
		const std::vector<unsigned char>& data = _vg->getData();
		svec3 gridDims = _vg->getGridDimensions();
		std::size_t faceOffset = gridDims.x * gridDims.y;

		// Iterate on each 'face' :
		for (std::size_t i = 0; i < gridDims.z; ++i) {
			const uint8_t* frame = &(data[i * faceOffset]);
			TinyTIFFWriter_writeImage(this->tiffFile, (void*)frame);
		}

		// Close the file once finished :
		TinyTIFFWriter_close(this->tiffFile);
		this->tiffFile = nullptr;

		return static_cast<std::size_t>(data.size());
	}
}
