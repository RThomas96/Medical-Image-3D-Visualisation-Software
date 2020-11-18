#include "../include/reader.hpp"

#include <cstring>

namespace IO {

	bool FileExists(const char* filename) {
		// We could do a more time-optimized version of it
		// by returning fileTester == nullptr, but I still
		// prefer to properly close the inode associated
		// with it, to sanitize the system.

		// Try to open the file :
		FILE* fileTester = fopen(filename, "r");
		// Is it open ? If not, return false :
		if (fileTester == nullptr) return false;
		// Otherwise, close the file and return true :
		fclose(fileTester);
		return true;
	}

	char* FileBaseName(const char* filename) {
		// Note : this function does not check if the file exists.
		// It only extracts the basename from the string provided.

		// Locate last occurence of the character dot '.'
		const char* separator = strrchr(filename, '.');
		const char* path_slash = strrchr(filename, '/');
		// If none was found, return nulltpr :
		if (separator == nullptr) { return nullptr; }
		// Otherwise, allocate a new string and copy contents :
		std::size_t length = static_cast<std::size_t>(separator - &filename[0]);
		std::size_t length_to_path = static_cast<std::size_t>(separator - path_slash);
		// If the dot is a for relative folder access, discard the file (since it has no extension)
		if (length_to_path < 0) { return nullptr; }
		// Allocate memory for file's basename
		char* basename = static_cast<char*>(calloc(length + 1, sizeof(char)));
		memcpy(basename, filename, length * sizeof(char));
		// null-terminate the string
		basename[length] = '\0';
		// return it !
		return basename;
	}

	char* AppendExtension(char* basename, const char* extension) {
		// Early exits
		if (basename == nullptr) return nullptr;
		if (extension == nullptr) return nullptr;

		std::size_t baselength = strlen(basename); //strlen ignores terminating \0 !
		std::size_t extlength = strlen(extension); //strlen ignores terminating \0 !

		// If basename is empty string, return nothing
		if (baselength == 0) return nullptr;
		// If extension is empty string, return basename
		if (extlength == 0) return basename;

		// Realloc basename (add one for the dot, and one for the terminating char) :
		char* new_basename = static_cast<char*>(calloc(baselength+extlength+2, sizeof(char)));
		if (basename == nullptr) {
			return nullptr;
		}
		memcpy(new_basename, basename, baselength);
		new_basename[baselength] = '.';
		memcpy(new_basename+baselength+1, extension, extlength);
		new_basename[baselength+extlength+1] = '\0';
		return new_basename;
	}

	GenericGridReader::GenericGridReader(data_t _thres) : threshold(_thres) {
		this->boundingBox = bbox_t();
		this->dataBoundingBox = bbox_t();
		this->threshold = data_t(0);
		this->gridDimensions = sizevec3(0,0,0);
		this->voxelDimensions = glm::vec3(0.f);
		this->transform = glm::mat4(1.f);
		this->downsampled = false;
		this->data.clear();
		this->filenames.clear();
		this->name = "";
	}

	GenericGridReader::~GenericGridReader() {
		this->filenames.clear();
		this->data.clear();
	}

	GenericGridReader& GenericGridReader::setDataThreshold(data_t _thresh) {
		this->threshold = _thresh;
		return *this;
	}

	GenericGridReader& GenericGridReader::setFilenames(std::vector<std::string> &names) {
		// remove old filenames :
		this->filenames.clear();
		// Copy filenames, inserting them at the back of 'filenames' :
		std::copy(names.begin(), names.end(), std::back_inserter(this->filenames));

		return *this;
	}

	GenericGridReader& GenericGridReader::loadImage() { return *this; }

	const std::vector<GenericGridReader::data_t>& GenericGridReader::getGrid() const { return this->data; }

	GenericGridReader::sizevec3 GenericGridReader::getGridDimensions() const { return this->gridDimensions; }

	glm::vec3 GenericGridReader::getVoxelDimensions() const { return this->voxelDimensions; }

	glm::mat4 GenericGridReader::getTransform() const { return this->transform; }

	GenericGridReader::bbox_t GenericGridReader::getBoundingBox() const { return this->boundingBox; }

	GenericGridReader::bbox_t GenericGridReader::getDataBoundingBox() const { return this->dataBoundingBox; }

	GenericGridReader& GenericGridReader::openFile(std::string &name) { return *this; }

	GenericGridReader& GenericGridReader::loadImageIndexed(std::size_t idx) { return *this; }

	GenericGridReader& GenericGridReader::loadGrid() { return *this; }

	GenericGridReader::data_t GenericGridReader::getDataThreshold() const { return this->threshold; }

	GenericGridReader& GenericGridReader::swapData(std::vector<data_t> &target) {
		this->data.swap(target);
		return *this;
	}

	DIMReader::DIMReader(data_t thresh) : GenericGridReader(thresh) {
		this->dimFile = nullptr;
		this->imaFile = nullptr;
	}

	DIMReader::~DIMReader() {
		if (this->dimFile != nullptr) { this->dimFile->close(); }
		if (this->imaFile != nullptr) { this->imaFile->close(); }
		std::cerr << "[LOG] Destroying DIM reader" << '\n';
	}

	DIMReader& DIMReader::loadImage() {
		if (this->filenames.size() == 0) {
			std::cerr << "[LOG] No filenames were provided, nothing will be loaded." << '\n';
			return *this;
		}

		try {
			// Open with the given filename :
			this->openFile(this->filenames[0]);
		} catch (const std::runtime_error& e) {
			std::cerr << "[ERROR] Could not open the file \""<<this->filenames[0]<<"\". Error message :\n";
			std::cerr << e.what() << '\n';
			std::abort();
		}

		this->loadGrid();

		return *this;
	}

	DIMReader& DIMReader::openFile(std::string &name) {
		// Get IMA/DIM names from the open file :
		char* basename = FileBaseName(name.c_str());
		char* ima_input_file_path = nullptr;
		char* dim_input_file_path = nullptr;

		ima_input_file_path = AppendExtension(basename, "ima");
		dim_input_file_path = AppendExtension(basename, "dim");

		// check the filenames are properly created :
		if (dim_input_file_path == nullptr) {
			std::cerr << "base and ext concat could not be performed for dim" << '\n';
			free(basename);
			return *this;
		}
		if (ima_input_file_path == nullptr) {
			std::cerr << "base and ext concat could not be performed for ima" << '\n';
			free(basename);
			free(dim_input_file_path);
			return *this;
		}

		// Open the files, and check they are open
		this->dimFile = new std::ifstream(dim_input_file_path);
		this->imaFile = new std::ifstream(ima_input_file_path);

		if ((not this->dimFile->is_open()) or (not this->imaFile->is_open())) {
			this->dimFile->close();
			this->imaFile->close();
			this->dimFile = nullptr;
			this->imaFile = nullptr;
			throw std::runtime_error("Could not open files.");
		}

		free(basename);
		free(dim_input_file_path);
		free(ima_input_file_path);

		return *this;
	}

	DIMReader& DIMReader::loadGrid() {
		// Check if dim/ima files are opened :
		if (this->dimFile == nullptr || this->imaFile == nullptr) { return *this; }

		std::cerr << "[LOG] Reading DIM data ...\n";

		// read number of voxels :
		(*this->dimFile) >> this->gridDimensions.x;
		(*this->dimFile) >> this->gridDimensions.y;
		(*this->dimFile) >> this->gridDimensions.z;

		// Get the max coord as a bounding box-type vector :
		bbox_t::vec maxCoord = bbox_t::vec(
			static_cast<bbox_t::vec::value_type>(this->gridDimensions.x),
			static_cast<bbox_t::vec::value_type>(this->gridDimensions.y),
			static_cast<bbox_t::vec::value_type>(this->gridDimensions.z)
		);

		// By default, set the bounding box to the grid dimensions :
		this->boundingBox.setMin(bbox_t::vec(0, 0, 0));
		this->boundingBox.setMax(maxCoord);
		// (can be overriden by the dim file's info)

		// read info from the DIM file (extended with our properties)
		std::string token, type;
		do {
			(*this->dimFile) >> token;
			if (token.find("-type") != std::string::npos) { (*this->dimFile) >> type; }
			else if (token.find("-dx") != std::string::npos) {(*this->dimFile) >> this->voxelDimensions.x;}
			else if (token.find("-dy") != std::string::npos) {(*this->dimFile) >> this->voxelDimensions.y;}
			else if (token.find("-dz") != std::string::npos) {(*this->dimFile) >> this->voxelDimensions.z;}
			else if (token.find("-bbmin") != std::string::npos) {
				bbox_t::vec v; (*this->dimFile) >> v.x >> v.y >> v.z; this->boundingBox.setMin(v);
			}
			else if (token.find("-bbmax") != std::string::npos) {
				bbox_t::vec v; (*this->dimFile) >> v.x >> v.y >> v.z; this->boundingBox.setMax(v);
			}
			else {
				std::cerr << "[DIMReader - ERROR] token "<<token<<" did not represent anything"<<'\n';
			}
		} while (not this->dimFile->eof());

		std::cerr << "[LOG] Reading from IMA file ...\n";
		// resize data vector :
		std::size_t dataSize = this->gridDimensions.x * this->gridDimensions.y * this->gridDimensions.z;
		this->data.resize(dataSize);
		// read from file directly :
		this->imaFile->read((char*)this->data.data(), static_cast<std::size_t>(dataSize));

		std::cerr << "[LOG] Updating DIM/IMA data bounding box with threshold " << +this->threshold << " ...\n";

		// update data bounding box :
		this->dataBoundingBox = bbox_t();
		for (std::size_t k = 0; k < this->gridDimensions.z; ++k) {
			for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
				for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
					std::size_t idx = i + j * this->gridDimensions.x +
							  k * this->gridDimensions.x * this->gridDimensions.y;
					if (this->data[idx] > this->threshold) {
						// Update data BB :
						bbox_t::vec v;
						v.x = static_cast<bbox_t::vec::value_type>(i);
						v.y = static_cast<bbox_t::vec::value_type>(j);
						v.z = static_cast<bbox_t::vec::value_type>(k);
						this->dataBoundingBox.addPoint(v);
					}
				}
			}
		}

		std::cerr << "[LOG] Updated data bounding box\n";

		return *this;
	}

	StackedTIFFReader::StackedTIFFReader(StackedTIFFReader::data_t thresh) : GenericGridReader(thresh) {
		this->tiffFile = nullptr;
	}

	StackedTIFFReader::~StackedTIFFReader() {
		// if a file was opened :
		if (this->tiffFile != nullptr) {
			// close the file, and free up memory
			TinyTIFFReader_close(this->tiffFile);
			this->tiffFile = nullptr;
		}
		std::cerr << "[LOG] Destroying stacked TIFF reader\n";
	}

	StackedTIFFReader& StackedTIFFReader::loadImage() {
		// checks we have something to load :
		if (this->filenames.size() == 0) {
			std::cerr << "[LOG] No filenames were provided, nothing will be loaded." << '\n';
			return *this;
		}

		// Prepare storage of all the image's data :
		this->preAllocateStorage();

		for (std::size_t i = 0; i < this->filenames.size(); ++i) {
			this->loadImageIndexed(i);
		}

		return *this;
	}

	StackedTIFFReader& StackedTIFFReader::preAllocateStorage() {
		try {
			this->openFile(this->filenames[0]);
		}  catch (std::runtime_error& e) {
			std::cerr << "[ERROR] A runtime error occured while preallocating storage. Error message : \n";
			std::cerr << e.what() << '\n';
			std::abort();
		}

		// File is opened, we can query its dimensions :
		std::size_t width = static_cast<std::size_t>(TinyTIFFReader_getWidth(this->tiffFile));
		std::size_t height = static_cast<std::size_t>(TinyTIFFReader_getHeight(this->tiffFile));
		std::size_t depth = static_cast<std::size_t>(this->filenames.size());
		// If we downsample, apply the dimension reduction here :
		if (this->downsampled) { width /= 2; height /= 2; }
		// Compute the vector size :
		std::size_t finalSize = width * height * depth;

		// Compute the min/max values for the bounding box :
		bbox_t::vec minBB = bbox_t::vec(static_cast<bbox_t::vec::value_type>(0));
		bbox_t::vec maxBB = bbox_t::vec(
			static_cast<bbox_t::vec::value_type>(width),
			static_cast<bbox_t::vec::value_type>(height),
			static_cast<bbox_t::vec::value_type>(depth)
		);
		// Resize the vector :
		this->data.resize(finalSize);
		// Fill in the data for grid dimensions, bounding box, transform and voxel dimensions :
		this->gridDimensions = sizevec3(width, height, depth);
		this->transform = glm::mat4(1.f);
		this->voxelDimensions = glm::vec3(1.f, 1.f, 1.f);
		this->boundingBox = bbox_t(minBB, maxBB);
		this->dataBoundingBox = bbox_t();

		// Most of those values assigned above are default values, since we cannot gather
		// that info from a TIFF file. (Not easily, anyway).
		return *this;
	}

	StackedTIFFReader& StackedTIFFReader::openFile(std::string& filename) {
		if (this->tiffFile != nullptr) {
			TinyTIFFReader_close(this->tiffFile);
			this->tiffFile = nullptr;
		}

		std::cerr << "[LOG] Opening file named \"" << filename << "\" ..." << '\n';
		this->tiffFile = TinyTIFFReader_open(filename.c_str());

		if (TinyTIFFReader_wasError(this->tiffFile)) {
			std::cerr << "[WARNING] Errors occured while loading \"" << filename
				  << "\" with TinyTIFF. Error messages :" << '\n';
			while (TinyTIFFReader_wasError(this->tiffFile)) {
				std::cerr << "[WARNING]\t" << TinyTIFFReader_getLastError(this->tiffFile) << '\n';
			}
		}

		return *this;
	}

	StackedTIFFReader& StackedTIFFReader::loadImageIndexed(std::size_t idx) {
		try {
			this->openFile(this->filenames[idx]);
		} catch (std::runtime_error& e) {
			std::cerr << "[ERROR] A runtime error occured while loading file \"" << this->filenames[idx]
				     << "\". Error message :\n";
			std::cerr << e.what() << '\n';
			std::abort();
		}

		// file is now opened, we can load the data into a vector :
		std::vector<data_t> rawData;
		rawData.resize(this->gridDimensions.x * this->gridDimensions.y);

		if (not this->downsampled) {
			// If we aren't downsampled, then the grid size is the same as the image size.
			// load data :
			TinyTIFFReader_getSampleData(this->tiffFile, rawData.data(), 0);
		} else {
			// Need a vector to hold the whole image :
			std::vector<data_t> wholeImage;
			// Resize it to the image canvas' size :
			std::size_t width = TinyTIFFReader_getWidth(this->tiffFile);
			std::size_t height = TinyTIFFReader_getHeight(this->tiffFile);
			std::cerr << "[LOG] Loading image " << this->filenames[idx] << "at native resolution of " <<
				     width << "x" << height << " ...\n";
			wholeImage.resize(width*height);
			// Load image :
			TinyTIFFReader_getSampleData(this->tiffFile, wholeImage.data(), 0);
			// Take only the data needed :
			for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
				for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
					// width should be equal to this->gridDimensions.x, since we're downsampled :
					rawData[i + j*this->gridDimensions.x] = wholeImage[i*2 + j*width];
				}
			}
			std::cerr << "[LOG]Outputting a " << width << "x" << height << " 2D image into a " <<
				  this->gridDimensions.x << "x" << this->gridDimensions.y << " 3D image\n";
		}

		// Analyse the image (for bounding box) :
		for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
			for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
				const data_t& val = rawData[i + j*this->gridDimensions.x];
				if (val > this->threshold) {
					bbox_t::vec v;
					v.x = static_cast<bbox_t::vec::value_type>(i);
					v.y = static_cast<bbox_t::vec::value_type>(j);
					v.z = static_cast<bbox_t::vec::value_type>(idx);
					this->dataBoundingBox.addPoint(v);
				}
			}
		}

		// Copy it into the main image buffer :
		std::size_t offset = this->gridDimensions.x * this->gridDimensions.y * idx;
		std::copy(rawData.begin(), rawData.end(), this->data.data()+offset);

		return *this;
	}

	StackedTIFFReader& StackedTIFFReader::enableDownsampling(bool enabled) {
		this->downsampled = enabled;
		return *this;
	}

}
