#include "../include/reader.hpp"

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

	GenericGridReader& GenericGridReader::setDataThreshold(data_t _thresh) { this->threshold = _thresh; }

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

	DIMReader::DIMReader(data_t thresh) : GenericGridReader(thresh) {}

	DIMReader::~DIMReader() {
		this->dimFile.close();
		this->imaFile.close();
	}

	DIMReader& DIMReader::loadImage() {
		if (this->filenames.size() != 0) {
			this->loadGrid();
		}
		return *this;
	}

	DIMReader& DIMReader::openFile(std::string &name) {
		// Get IMA/DIM names from the open file :
		char* basename = FileBaseName(name.c_str());
		char* ima_input_file_path = nullptr; ima_input_file_path = AppendExtension(basename, "ima");
		char* dim_input_file_path = nullptr; dim_input_file_path = AppendExtension(basename, "dim");

		// check the filenames are properly created :
		if (dim_input_file_path == nullptr) { std::cerr << "base and ext concat could not be performed for dim" << '\n'; return *this; }
		if (ima_input_file_path == nullptr) { std::cerr << "base and ext concat could not be performed for ima" << '\n'; return *this; }

		// Open the files, and check they are open
		this->dimFile.open(dim_input_file_path);
		this->imaFile.open(ima_input_file_path);

		if ((not this->dimFile.is_open()) or (not this->imaFile.is_open())) {
			this->dimFile.close();
			this->imaFile.close();
			throw std::runtime_error("Could not open files.");
		}

		return *this;
	}

	DIMReader& DIMReader::loadGrid() {
		// Read info from DIM file :
		sizevec3 dims;

		// read number of voxels :
		this->dimFile >> this->gridDimensions.x;
		this->dimFile >> this->gridDimensions.y;
		this->dimFile >> this->gridDimensions.z;

		// read info :
		std::string token, type;

		do {
			this->dimFile >> token;
			if (token.find("-type") != std::string::npos) { this->dimFile >> type; }
			else if (token.find("-dx") != std::string::npos) { this->dimFile >> this->voxelDimensions.x; }
			else if (token.find("-dy") != std::string::npos) { this->dimFile >> this->voxelDimensions.y; }
			else if (token.find("-dz") != std::string::npos) { this->dimFile >> this->voxelDimensions.z; }
			else if (token.find("-bbmin") != std::string::npos) { bbox_t::vec v; this->dimFile >> v.x >> v.y >> v.z; this->boundingBox.setMin(v); }
			else if (token.find("-bbmax") != std::string::npos) { bbox_t::vec v; this->dimFile >> v.x >> v.y >> v.z; this->boundingBox.setMax(v); }
			else if (token.find("-databbmin") != std::string::npos) { bbox_t::vec v; this->dimFile >> v.x >> v.y >> v.z; this->dataBoundingBox.setMin(v); }
			else if (token.find("-databbmax") != std::string::npos) { bbox_t::vec v; this->dimFile >> v.x >> v.y >> v.z; this->dataBoundingBox.setMax(v); }
			else if (token.find("-threshold") != std::string::npos) { this->dimFile >> this->threshold; }
			else if (token.find("-transform") != std::string::npos) {
				glm::mat4 t;
				for (std::size_t i = 0; i < 4; ++i) {
					for (std::size_t j = 0; j < 4; ++j) {
						this->dimFile >> t[i][j];
					}
				}
				this->transform = t;
			}
			else {
				std::cerr << "[DIMReader - ERROR] token " << token << " did not represent anything" << '\n';
			}
		} while (not this->dimFile.eof());

		// resize data vector :
		std::size_t dataSize = this->gridDimensions.x * this->gridDimensions.y * this->gridDimensions.z;
		this->data.resize(dataSize);
		// read from file directly :
		this->imaFile.read((char*)this->data.data(), static_cast<std::size_t>(dataSize));

		return *this;
	}

}
