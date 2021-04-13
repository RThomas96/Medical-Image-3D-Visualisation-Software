#include "../include/reader.hpp"

#include <QXmlStreamReader>
#include <QDir>

#include <cstring>
#include <memory>
#include <iomanip>

namespace IO {

	void nullify_tiff_errors(const char* module, const char* fmt, va_list _va_) { return; /* do nothing ! */ }

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
		this->gridDimensions = sizevec3(0,0,0);
		this->voxelDimensions = glm::vec3(1.f);
		this->voxelMultiplier = glm::vec3(1.f);
		this->transform = glm::mat4(1.f);
		this->downsampleLevel = DownsamplingLevel::Original;
		this->data.clear();
		this->filenames.clear();
		this->name = "";
		// init limits
		this->userLimits.x = std::numeric_limits<data_t>::lowest();
		this->userLimits.y = std::numeric_limits<data_t>::max();
		this->textureLimits.x = std::numeric_limits<data_t>::max();
		this->textureLimits.y = std::numeric_limits<data_t>::lowest();
		// interpolation struct :
		this->interpolator = nullptr;
		this->isAnalyzed = false;
		this->hasUserBounds = false;
	}

	GenericGridReader::~GenericGridReader() {
		this->filenames.clear();
		this->data.clear();
	}

	GenericGridReader& GenericGridReader::parseImageInfo(ThreadedTask::Ptr& task) { task->setSteps(0); return *this; }

	GenericGridReader& GenericGridReader::setDataThreshold(data_t _thresh) {
		this->threshold = _thresh;
		return *this;
	}

	GenericGridReader& GenericGridReader::setInterpolationMethod(std::shared_ptr<Interpolators::genericInterpolator<data_t>> &ptr) {
		this->interpolator = ptr;
		return *this;
	}

	GenericGridReader& GenericGridReader::setUserIntensityLimits(data_t min, data_t max) {
		this->hasUserBounds = true;
		this->userLimits = glm::vec<2, data_t, glm::defaultp>(min, max);
		return *this;
	}

	GenericGridReader& GenericGridReader::setFilenames(std::vector<std::string> &names) {
		// remove old filenames :
		this->filenames.clear();
		// Copy filenames, inserting them at the back of 'filenames' :
		std::copy(names.begin(), names.end(), std::back_inserter(this->filenames));

		return *this;
	}

	GenericGridReader& GenericGridReader::enableDownsampling(DownsamplingLevel _level) {
		this->downsampleLevel = _level;
		// Update voxel multiplier accordingly :
		switch (_level) {
			case IO::DownsamplingLevel::Original:
				this->voxelMultiplier = glm::vec3(1.f, 1.f, 1.f);
				break;
			case IO::DownsamplingLevel::Low :
				this->voxelMultiplier = glm::vec3(2.f, 2.f, 2.f);
				break;
			case IO::DownsamplingLevel::Lower :
				this->voxelMultiplier = glm::vec3(4.f, 4.f, 4.f);
				break;
			case IO::DownsamplingLevel::Lowest :
				this->voxelMultiplier = glm::vec3(8.f, 8.f, 8.f);
				break;
		}
		return *this;
	}

	DownsamplingLevel GenericGridReader::downsamplingLevel() { return this->downsampleLevel; }

	GenericGridReader& GenericGridReader::loadImage(ThreadedTask::Ptr& task) { task->setSteps(0); return *this; }

	const std::vector<GenericGridReader::data_t>& GenericGridReader::getGrid() const { return this->data; }

	GenericGridReader::sizevec3 GenericGridReader::getGridDimensions() const {
		if (this->downsampleLevel == DownsamplingLevel::Low) {
			return (this->imageDimensions / std::size_t(2));
		} else if (this->downsampleLevel == DownsamplingLevel::Lower) {
			return (this->imageDimensions / std::size_t(4));
		} else if (this->downsampleLevel == DownsamplingLevel::Lowest) {
			return (this->imageDimensions / std::size_t(8));
		}
		return this->gridDimensions;
	}

	std::size_t GenericGridReader::getGridSizeBytes() const {
		std::size_t sizeBytes = 0;
		sizeBytes = this->imageDimensions.x * this->imageDimensions.y * this->imageDimensions.z * sizeof(data_t);
		if (this->downsampleLevel == DownsamplingLevel::Low) { sizeBytes /= 2*2*2; }
		if (this->downsampleLevel == DownsamplingLevel::Lower) { sizeBytes /= 4*4*4; }
		if (this->downsampleLevel == DownsamplingLevel::Lowest) { sizeBytes /= 8*8*8; }
		return sizeBytes;
	}

	glm::vec3 GenericGridReader::getVoxelDimensions() const { return this->voxelDimensions * this->voxelMultiplier; }

	glm::vec3 GenericGridReader::getOriginalVoxelDimensions() const { return this->voxelDimensions; }

	glm::mat4 GenericGridReader::getTransform() const { return this->transform; }

	GenericGridReader::bbox_t GenericGridReader::getBoundingBox() const { return this->boundingBox; }

	GenericGridReader::bbox_t GenericGridReader::getDataBoundingBox() const { return this->dataBoundingBox; }

	GenericGridReader& GenericGridReader::openFile(const std::string &name) { return *this; }

	GenericGridReader& GenericGridReader::loadSlice(std::size_t idx, std::vector<data_t>& tgt) { return *this; }

	GenericGridReader::data_t GenericGridReader::getDataThreshold() const { return this->threshold; }

	glm::vec<2, GenericGridReader::data_t, glm::defaultp> GenericGridReader::getTextureLimits() const {
		return this->textureLimits;
	}

	GenericGridReader& GenericGridReader::swapData(std::vector<data_t> &target) {
		this->data.swap(target);
		return *this;
	}

	std::vector<std::string> GenericGridReader::getFilenames(void) const { return this->filenames; }

	GenericGridReader& GenericGridReader::setUserVoxelSize(float _x, float _y, float _z) {
		this->voxelDimensions = glm::vec3(_x, _y, _z);
		return *this;
	}

	DIMReader::DIMReader(data_t thresh) : GenericGridReader(thresh) {
		this->dimFile = nullptr;
		this->imaFile = nullptr;
	}

	DIMReader::~DIMReader() {
		if (this->dimFile != nullptr) { this->dimFile->close(); }
		if (this->imaFile != nullptr) { this->imaFile->close(); }
	}

	DIMReader& DIMReader::parseImageInfo(ThreadedTask::Ptr& task) {
		if (this->isAnalyzed == true) {
			task->end();
			return *this;
		}
		if (this->filenames.size() == 0) {
			std::cerr << "[LOG] No filenames were provided, nothing will be loaded." << '\n';
			task->end();
			return *this;
		}
		this->isAnalyzed = true;

		// if the files have already been opened, no need
		// to re-open them (data was already precomputed) :
		if (this->dimFile != nullptr) { task->end(); return *this; }
		if (this->imaFile != nullptr) { task->end(); return *this; }

		// Steps (2) :
		// read image dimensions
		// compute the default variables
		task->setSteps(2);

		try {
			// Open with the given filename :
			this->openFile(this->filenames[0]);
		} catch (const std::runtime_error& e) {
			std::cerr << "[ERROR] Could not open the file \""<<this->filenames[0]<<"\". Error message :\n";
			std::cerr << e.what() << '\n';
			std::abort();
		}

		// read number of voxels into image dimensions :
		(*this->dimFile) >> this->imageDimensions.x;
		(*this->dimFile) >> this->imageDimensions.y;
		(*this->dimFile) >> this->imageDimensions.z;

		task->advance();

		std::cerr << "[LOG] parseImageInfo() : Image dimensions : " << this->imageDimensions.x << ',' << this->imageDimensions.y << ',' << this->imageDimensions.z << '\n';

		// read info from the DIM file (extended with our properties)
		std::string token, type;
		do {
			(*this->dimFile) >> token;
			if (token.find("-type") != std::string::npos) { (*this->dimFile) >> type; }
			else if (token.find("-dx") != std::string::npos) {(*this->dimFile) >> this->voxelDimensions.x;}
			else if (token.find("-dy") != std::string::npos) {(*this->dimFile) >> this->voxelDimensions.y;}
			else if (token.find("-dz") != std::string::npos) {(*this->dimFile) >> this->voxelDimensions.z;}
			else {
				std::cerr << "[ERROR] DIMReader - token "<< token <<" did not represent anything"<<'\n';
			}
		} while (not this->dimFile->eof());

		std::cerr << "[LOG] DIM file's " << this->filenames[0] << " type is " << type << '\n';

		// Get the max coord as a bounding box-type vector :
		using val_t = bbox_t::vec::value_type;
		bbox_t::vec maxCoord = bbox_t::vec(
			static_cast<val_t>(this->imageDimensions.x) * static_cast<val_t>(this->voxelDimensions.x),
			static_cast<val_t>(this->imageDimensions.y) * static_cast<val_t>(this->voxelDimensions.y),
			static_cast<val_t>(this->imageDimensions.z) * static_cast<val_t>(this->voxelDimensions.z)
		);
		// N.B. : This BB will only be valid as long as the downsampling factor doesn't change. Recomputed in loadImage.

		// Set the bounding box to the grid*voxel dimensions :
		this->boundingBox.setMin(bbox_t::vec(0, 0, 0));
		this->boundingBox.setMax(maxCoord);

		this->gridDimensions = this->imageDimensions;

		task->advance();

		return *this;
	}

	DIMReader::data_t DIMReader::getPixel(std::size_t i, std::size_t j, std::size_t k) {
		if (this->imaFile != nullptr) {
			auto lastPos = this->imaFile->tellg();
			// FF to the file location specified :
			this->imaFile->seekg(i*j*k*sizeof(data_t));
			data_t val = 0;
			this->imaFile->read((char*)&val, sizeof(data_t));
			// put stream back to where it was
			this->imaFile->seekg(lastPos);
			return val;
		} else {
			return data_t(0);
		}
	}

	DIMReader::data_t DIMReader::getPixel_ImageSpace(glm::vec4 pos) {
		// assumes the point is given in image space.
		// We still need to check if if actually point to somewhere in image space, though.

		// checks the point is actually in the bb :
		if (this->boundingBox.contains(bbox_t::vec(glm::convert_to<bbox_t::data_t>(pos)))) {
			// compute the actual coordinates of the point :
			glm::vec3 inside_pos = glm::vec3(pos.x, pos.y, pos.z) - glm::convert_to<float>(this->boundingBox.getMin());
			// divide by 'actual' voxel dimensions (we emulate no downsampling here) :
			glm::uvec3 coords = inside_pos / this->voxelDimensions;
			// Get the value !
			return this->getPixel(coords.x, coords.y, coords.z);
		}

		return data_t(0);
	}

	DIMReader& DIMReader::loadImage(ThreadedTask::Ptr& task) {
		if (this->downsampleLevel != DownsamplingLevel::Original && this->interpolator == nullptr) {
			std::cerr << "[ERROR] No interpolation structure was set in this reader, even though a downsample was required.\n";
			return *this;
		}

		if (this->isAnalyzed == false) {
			this->parseImageInfo(task);
		}
		// Compute voxel and grid sizes :
		sizevec3 dataDims = this->imageDimensions;

		// # of slices to load at once in order to downsample :
		std::size_t slicesToLoad = 1;

		if (this->downsampleLevel == DownsamplingLevel::Low) { dataDims /= std::size_t(2); slicesToLoad = 2; }
		if (this->downsampleLevel == DownsamplingLevel::Lower) { dataDims /= std::size_t(4); slicesToLoad = 4; }
		if (this->downsampleLevel == DownsamplingLevel::Lowest) { dataDims /= std::size_t(8); slicesToLoad = 8; }
		this->gridDimensions = dataDims;

		// Set max nb of steps ...
		std::size_t nbsteps = this->gridDimensions.z;
		task->setSteps(nbsteps);

		std::cerr << "[LOG] Reading DIM data ...\n";

		std::cerr << "\tOriginal file dimensions : {" << dataDims.x << ',' << dataDims.y << ',' << dataDims.z << "}\n";
		std::cerr << "\tOriginal vox  dimensions : {" << this->voxelDimensions.x << ',' << this->voxelDimensions.y << ',' << this->voxelDimensions.z << "}\n";

		// Update the bounding box :
		// if the downsampling rate changed between precomputeData() and now, the bounding box
		// is no longer of the right size (by a factor of k \in [2,4,8]) :
		using val_t = bbox_t::vec::value_type;
		bbox_t::vec maxCoord = bbox_t::vec(
			static_cast<val_t>(this->gridDimensions.x) * static_cast<val_t>(this->voxelDimensions.x*this->voxelMultiplier.x),
			static_cast<val_t>(this->gridDimensions.y) * static_cast<val_t>(this->voxelDimensions.y*this->voxelMultiplier.y),
			static_cast<val_t>(this->gridDimensions.z) * static_cast<val_t>(this->voxelDimensions.z*this->voxelMultiplier.z)
		);
		// Set the bounding box to the grid*voxel dimensions :
		this->boundingBox.setMin(bbox_t::vec(0, 0, 0));
		this->boundingBox.setMax(maxCoord);

		std::cerr << "\tNew      img  dimensions : {" << dataDims.x << ',' << dataDims.y << ',' << dataDims.z << "}\n";
		std::cerr << "\tNew      vox  dimensions : {" << this->voxelDimensions.x * this->voxelMultiplier.x << ',' <<
				this->voxelDimensions.y * this->voxelMultiplier.y << ',' <<
				this->voxelDimensions.z * this->voxelMultiplier.z << "}\n";

		std::cerr << "[LOG] load() : Grid dimensions : " << this->gridDimensions.x << ',' << this->gridDimensions.y << ',' << this->gridDimensions.z << '\n';

		std::cerr << "[LOG] Reading from IMA file ...\n";
		// resize data vector to fit all data :
		std::size_t dataSize = this->gridDimensions.x * this->gridDimensions.y * this->gridDimensions.z;
		this->data.resize(dataSize);

		using val_t = bbox_t::vec::value_type;	// Typedef of BB vec value type
		std::vector<data_t> rawSlices;		// Raw slices loaded at full-res to downsample
		std::vector<data_t> singleSlice;	// Single raw slice (also full-res)
		std::vector<data_t> curSlice;		// Slice to output to this->data after processing

		curSlice.resize(this->gridDimensions.x * this->gridDimensions.y); // resize for one slice at grid dims
		singleSlice.resize(this->imageDimensions.x * this->imageDimensions.y); // Resize for 1 slice at full-res
		// For multiple slices, only allocate memory if necessary :
		if (this->downsampleLevel != DownsamplingLevel::Original) {
			rawSlices.resize(singleSlice.size() * slicesToLoad);	// Resize to fit 'n' slices at full-res
		}

		// iterator pointing to positions in different vectors :
		std::vector<data_t>::iterator rawSlicesIterator = std::begin(rawSlices);

		// update data bounding box at the same time we're copying data:
		this->dataBoundingBox = bbox_t();
		// for each slice of the desired image size :
		for (std::size_t k = 0; k < this->gridDimensions.z; ++k) {
			rawSlicesIterator = std::begin(rawSlices);
			// Load slice(s) from file according to slicesToLoad :
			for (std::size_t i = 0; i < slicesToLoad; ++i) {
				this->loadSlice(k, singleSlice);
				// if we have multiple of them to load, do so :
				if (this->downsampleLevel != DownsamplingLevel::Original) {
					rawSlicesIterator = std::copy(std::begin(singleSlice), std::end(singleSlice), rawSlicesIterator);
				}
			}

			if (this->downsampleLevel == DownsamplingLevel::Original) {
				// straight copy into current slice vector :
				std::copy(std::begin(singleSlice), std::end(singleSlice), std::begin(curSlice));
			} else {
				for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
					for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
						// Interpolation happens here !
						// we start at i*slice, j*slice, k*slice

						std::vector<data_t> interpolationData;
						std::vector<data_t>::iterator interpolationIterator;

						// Gather necessary data :
						for (std::size_t z = 0; z < slicesToLoad; ++z) {
							for (std::size_t y = 0; y < slicesToLoad; ++y) {
								std::size_t i_x = i * slicesToLoad;
								std::size_t i_y = (j * slicesToLoad + y) * this->imageDimensions.x;
								std::size_t i_z = (z) * this->imageDimensions.x * this->imageDimensions.y;
								// here z is untouched because we have only loaded the slices to
								// 'merge' with the interpolator. we don't need to do (k*slice + z)
								std::size_t iter = i_x + i_y + i_z;
								// read 'slicesToLoad' elements from the grid :
								interpolationIterator = interpolationData.insert(interpolationIterator, rawSlices.begin()+iter, rawSlices.begin()+iter+slicesToLoad);
							}
						}

						data_t pixelVal = (*this->interpolator)(slicesToLoad, interpolationData);
						interpolationData.clear();
						std::size_t idx = i + j * this->gridDimensions.x;
						curSlice[idx] = pixelVal;
					}
				}
			}

			bbox_t::vec minBB = this->boundingBox.getMin();
			// Copy data from curSlice to data :
			for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
				for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
					std::size_t idx = i + j * this->gridDimensions.x;
					std::size_t data_idx = idx + k * this->gridDimensions.x * this->gridDimensions.y;
					this->data[data_idx] = curSlice[idx];
					if (this->hasUserBounds) {
						// If the user has supplied bounds for the ROI
						if (this->data[data_idx] > this->userLimits.x && this->data[data_idx] < this->userLimits.y) {
							// Update data BB :
							bbox_t::vec v;
							v.x = minBB.x + static_cast<bbox_t::vec::value_type>(i) * this->voxelDimensions.x * this->voxelMultiplier.x;
							v.y = minBB.y + static_cast<bbox_t::vec::value_type>(j) * this->voxelDimensions.y * this->voxelMultiplier.y;
							v.z = minBB.z + static_cast<bbox_t::vec::value_type>(k) * this->voxelDimensions.z * this->voxelMultiplier.z;
							this->dataBoundingBox.addPoint(v);
						}
					} else {
						// regular bb update, only above threshold :
						if (this->data[data_idx] > this->threshold) {
							// Update data BB :
							bbox_t::vec v;
							v.x = minBB.x + static_cast<bbox_t::vec::value_type>(i) * this->voxelDimensions.x * this->voxelMultiplier.x;
							v.y = minBB.y + static_cast<bbox_t::vec::value_type>(j) * this->voxelDimensions.y * this->voxelMultiplier.y;
							v.z = minBB.z + static_cast<bbox_t::vec::value_type>(k) * this->voxelDimensions.z * this->voxelMultiplier.z;
							this->dataBoundingBox.addPoint(v);
						}
					}
				}
			}
			task->advance();
		}

		return *this;
	}

	DIMReader& DIMReader::openFile(const std::string &name) {
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
		this->dimFile = new std::ifstream(dim_input_file_path, std::ios_base::in | std::ios_base::binary);
		this->imaFile = new std::ifstream(ima_input_file_path, std::ios_base::in | std::ios_base::binary);

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

	DIMReader& DIMReader::loadSlice(std::size_t idx, std::vector<data_t>& tgt) {
		if (this->imaFile == nullptr) {
			std::cerr << "[ERROR] IMA file was not open at the moment to call loadSlice()" << '\n';
			return *this;
		}
		if (this->imaFile->fail()) {
			std::cerr << "[ERROR] IMA file has suffered an irrecuperable error at slice " << idx << ". (std::basic_ios::fail() == true)\n";
			std::cerr << "Error message : " << strerror(errno) << "\n";
			return *this;
		}

		// automatically advances the internal std::basic_ifstream position indicator in order to read the
		// next data whenever std::basic_ifstream::read is called another time :
		this->imaFile->read((char*)tgt.data(), this->imageDimensions.x * this->imageDimensions.y * sizeof(data_t));

		// analyze to find raw min/max values :
		std::for_each(std::begin(tgt), std::end(tgt), [this](const data_t& d) -> void {
			this->textureLimits.x = std::min(this->textureLimits.x, d);
			this->textureLimits.y = std::max(this->textureLimits.y, d);
		});

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

	StackedTIFFReader& StackedTIFFReader::parseImageInfo(ThreadedTask::Ptr& task) {
		if (this->isAnalyzed == true) { return *this; }
		if (this->filenames.size() == 0) {
			std::cerr << "[LOG] No filenames were provided, nothing will be loaded." << '\n';
			task->end();
			return *this;
		}
		this->isAnalyzed = true;

		task->setSteps(this->filenames.size());

		// Count the # of frames in the stack :
		std::size_t nbFrames = 0;
		for (std::size_t i = 0; i < this->filenames.size(); ++i) {
			std::size_t localNBFrames = 0;
			// open the file :
			this->openFile(this->filenames[i]);
			do {
				if (TinyTIFFReader_wasError(this->tiffFile)) {
					std::cerr << "[WARNING] Reading of TIFF frames for filename " <<
							this->filenames[i] << " caused an error.\n";
					std::cerr << "[WARNING] Error message : " <<
							TinyTIFFReader_getLastError(this->tiffFile) << '\n';
				}
				// assign each frame (image slice) to this particular filename
				// with the frame index within the file :
				this->sliceToFilename.push_back(std::make_pair(i, localNBFrames));
				nbFrames++; localNBFrames++;
			} while (TinyTIFFReader_hasNext(this->tiffFile));

			task->advance();
		}

		this->currentFile = 0;
		this->openFile(this->filenames[0]);

		std::size_t width = static_cast<std::size_t>(TinyTIFFReader_getWidth(this->tiffFile));
		std::size_t height = static_cast<std::size_t>(TinyTIFFReader_getHeight(this->tiffFile));
		std::size_t depth = static_cast<std::size_t>(nbFrames);

		// Compute the min/max values for the bounding box :
		bbox_t::vec minBB = bbox_t::vec(static_cast<bbox_t::vec::value_type>(0));
		// no multiplication by voxel dimensions here because they're undefined in this file format :
		bbox_t::vec maxBB = bbox_t::vec(
			static_cast<bbox_t::vec::value_type>(width),
			static_cast<bbox_t::vec::value_type>(height),
			static_cast<bbox_t::vec::value_type>(depth)
		);

		// 'raw' image dimensions are what is originally read above :
		this->imageDimensions = sizevec3(width, height, depth);
		// Fill in the data for grid dimensions, bounding box, transform and voxel dimensions :
		this->gridDimensions = sizevec3(width, height, depth);
		this->transform = glm::mat4(1.f);
		this->voxelDimensions = glm::vec3(1.f, 1.f, 1.f);
		this->boundingBox = bbox_t(minBB, maxBB);
		this->dataBoundingBox = bbox_t();

		task->advance();

		return *this;
	}

	StackedTIFFReader& StackedTIFFReader::loadImage(ThreadedTask::Ptr& task) {
		// checks we have something to load :
		if (this->filenames.size() == 0) {
			std::cerr << "[LOG] No filenames were provided, nothing will be loaded." << '\n';
			return *this;
		}

		if (this->isAnalyzed == false) {
			this->parseImageInfo(task);
			task->advance();
		}

		// Prepare storage of all the image's data :
		this->preAllocateStorage();

		// Set max nb of steps ...
		std::size_t nbsteps = this->gridDimensions.z;
		task->setSteps(nbsteps);

		// holds the data of the (possibly) downsampled file, to insert into this->data
		std::vector<data_t> curSlice;
		// holds the data of the raw slice(s) from the filesystem.
		std::vector<data_t> rawSlices;
		// holds the data of a single slice, raw from the filesystem
		std::vector<data_t> singleSlice;

		// The # of slices to load according to the downsampling level :
		std::size_t slicesToLoad = 1;
		if (this->downsampleLevel == DownsamplingLevel::Low) { slicesToLoad = 2; }
		if (this->downsampleLevel == DownsamplingLevel::Lower) { slicesToLoad = 4; }
		if (this->downsampleLevel == DownsamplingLevel::Lowest) { slicesToLoad = 8; }

		// resize :
		std::cerr << "[LOG] load() : Grid dimensions : " << this->gridDimensions.x << ',' << this->gridDimensions.y << ',' << this->gridDimensions.z << '\n';
		std::cerr << "[LOG] load() : Voxel dimensions : " << this->voxelDimensions.x * this->voxelMultiplier.x << ','
				<< this->voxelDimensions.y * this->voxelMultiplier.y << ','
				<< this->voxelDimensions.z * this->voxelMultiplier.z << '\n';

		// Recompute bounding box according to the voxel multiplier used here :
		using val_t = bbox_t::vec::value_type;
		bbox_t::vec maxCoord = bbox_t::vec(
			static_cast<val_t>(this->gridDimensions.x) * static_cast<val_t>(this->voxelDimensions.x*this->voxelMultiplier.x),
			static_cast<val_t>(this->gridDimensions.y) * static_cast<val_t>(this->voxelDimensions.y*this->voxelMultiplier.y),
			static_cast<val_t>(this->gridDimensions.z) * static_cast<val_t>(this->voxelDimensions.z*this->voxelMultiplier.z)
		);
		// Set the bounding box to the grid*voxel dimensions :
		this->boundingBox.setMin(bbox_t::vec(0, 0, 0));
		this->boundingBox.setMax(maxCoord);

		curSlice.resize(this->gridDimensions.x * this->gridDimensions.y);
		// Resize to accept one slice at full-res :
		singleSlice.resize(this->imageDimensions.x * this->imageDimensions.y);
		// only uses memory if we have to downsample :
		if (this->downsampleLevel != DownsamplingLevel::Original) {
			// resize to accept multiple slices at full res :
			rawSlices.resize(singleSlice.size() * slicesToLoad);
		}

		// iterator for raw slices :
		std::vector<data_t>::iterator rawSlicesIterator = std::begin(rawSlices);

		// Go through the grid :
		for (std::size_t k = 0; k < this->gridDimensions.z; ++k) {
			rawSlicesIterator = std::begin(rawSlices);
			// Load slice(s) here :
			for (std::size_t i = 0; i < slicesToLoad; ++i) {
				this->loadSlice(k*slicesToLoad+i, singleSlice);
				// Do this only if we have multiple slices to load, otherwise skip it
				if (this->downsampleLevel != DownsamplingLevel::Original) {
					// copy into rawslices :
					rawSlicesIterator = std::copy(std::begin(singleSlice), std::end(singleSlice), rawSlicesIterator);
				}
			}

			if (this->downsampleLevel == DownsamplingLevel::Original) {
				// Straight copy from loaded data to current slice data, updating the insertion iterator :
				std::copy(std::begin(singleSlice), std::end(singleSlice), std::begin(curSlice));
			} else {
				// input data into curSlice, one pixel at a time :
				for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
					for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
						// Interpolation code here !

						std::vector<data_t> interpolationData;
						std::vector<data_t>::iterator interpolationIterator;

						// Gather necessary data :
						for (std::size_t z = 0; z < slicesToLoad; ++z) {
							for (std::size_t y = 0; y < slicesToLoad; ++y) {
								std::size_t i_x = i * slicesToLoad;
								std::size_t i_y = (j * slicesToLoad + y) * this->imageDimensions.x;
								std::size_t i_z = (z) * this->imageDimensions.x * this->imageDimensions.y;
								std::size_t iter = i_x + i_y + i_z;
								// read 'slicesToLoad' elements from the grid :
								interpolationIterator = interpolationData.insert(interpolationIterator, rawSlices.begin()+iter, rawSlices.begin()+iter+slicesToLoad);
							}
						}

						data_t pixelVal = (*this->interpolator)(slicesToLoad, interpolationData);
						std::size_t idx = i + j * this->gridDimensions.x;
						curSlice[idx] = pixelVal;
					}
				}
			}

			bbox_t::vec minBB = this->boundingBox.getMin();
			// Copy data from curSlice into final buffer :
			for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
				for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
					// Copy into final buffer :
					std::size_t idx = i + j * this->gridDimensions.x;
					std::size_t data_idx = idx + k * this->gridDimensions.x * this->gridDimensions.y;
					this->data[data_idx] = curSlice[idx];
					if (this->hasUserBounds) {
						// If the user has supplied bounds for the ROI
						if (this->data[data_idx] > this->userLimits.x && this->data[data_idx] < this->userLimits.y) {
							// Update data BB :
							bbox_t::vec v;
							v.x = minBB.x + static_cast<bbox_t::vec::value_type>(i) * this->voxelDimensions.x * this->voxelMultiplier.x;
							v.y = minBB.y + static_cast<bbox_t::vec::value_type>(j) * this->voxelDimensions.y * this->voxelMultiplier.y;
							v.z = minBB.z + static_cast<bbox_t::vec::value_type>(k) * this->voxelDimensions.z * this->voxelMultiplier.z;
							this->dataBoundingBox.addPoint(v);
						}
					} else {
						// regular bb update, only above threshold :
						if (this->data[data_idx] > this->threshold) {
							// Update data BB :
							bbox_t::vec v;
							v.x = minBB.x + static_cast<bbox_t::vec::value_type>(i) * this->voxelDimensions.x * this->voxelMultiplier.x;
							v.y = minBB.y + static_cast<bbox_t::vec::value_type>(j) * this->voxelDimensions.y * this->voxelMultiplier.y;
							v.z = minBB.z + static_cast<bbox_t::vec::value_type>(k) * this->voxelDimensions.z * this->voxelMultiplier.z;
							this->dataBoundingBox.addPoint(v);
						}
					}
				}
			}
			task->advance();
		}

		return *this;
	}

	StackedTIFFReader::data_t StackedTIFFReader::getPixel(std::size_t i, std::size_t j, std::size_t k) {
		if (this->isAnalyzed == false) { return data_t(0); }

		const std::pair<std::size_t, std::size_t>& file = this->sliceToFilename[k];
		std::size_t idxFile = file.first;
		std::size_t idxFrame = file.second;

		this->openFile(this->filenames[idxFile]);

		for (std::size_t i = 0; i < idxFrame; ++i) {
			TinyTIFFReader_readNext(this->tiffFile);
		}

		data_t* imgData = new data_t[this->imageDimensions.x * this->imageDimensions.y];
		TinyTIFFReader_getSampleData(this->tiffFile, imgData, 0);

		data_t retVal = imgData[j*this->imageDimensions.x+i];
		delete[] imgData;

		return retVal;
	}

	StackedTIFFReader::data_t StackedTIFFReader::getPixel_ImageSpace(glm::vec4 pos) {
		// assumes the point is given in image space.
		// We still need to check if if actually point to somewhere in image space, though.

		// checks the point is actually in the bb :
		if (this->boundingBox.contains(bbox_t::vec(glm::convert_to<bbox_t::data_t>(pos)))) {
			// compute the actual coordinates of the point :
			glm::vec3 inside_pos = glm::vec3(pos.x, pos.y, pos.z) - glm::convert_to<float>(this->boundingBox.getMin());
			// divide by 'actual' voxel dimensions (we emulate no downsampling here) :
			glm::uvec3 coords = inside_pos / this->voxelDimensions;
			// Get the value !
			return this->getPixel(coords.x, coords.y, coords.z);
		}
		return data_t(0);
	}

	StackedTIFFReader& StackedTIFFReader::preAllocateStorage() {
		// used to compute the voxel dimensions in the (possibly) downsampled grid loaded.
		// By default, the voxels in TIFF images are 1.f units since there's no way to specify
		// it in the TIFF spec. But is we downsample, we want to keep the same grid size, but
		// with less voxels inside. We thus need a multiplier for voxel sizes

		std::size_t width = this->imageDimensions.x;
		std::size_t height= this->imageDimensions.y;
		std::size_t depth = this->imageDimensions.z;

		// If we downsample, apply the dimension reduction here on all axes
		// and modify the voxel multiplier accordingly :
		if (this->downsampleLevel == DownsamplingLevel::Low) { width /= 2; height /= 2; depth /= 2; }
		else if (this->downsampleLevel == DownsamplingLevel::Lower) { width /= 4; height /= 4; depth /= 4; }
		else if (this->downsampleLevel == DownsamplingLevel::Lowest) { width /= 8; height /= 8; depth /= 8; }

		// Voxel dimensions will be scaled with voxelMultiplier.
		// However, grid dimensions needs to be set here :
		this->gridDimensions = sizevec3(width, height, depth);

		// Compute the vector size to allocate :
		std::size_t finalSize = width * height * depth;
		// Resize the vector :
		this->data.resize(finalSize);

		// Most of those values assigned above are default values, since we cannot gather
		// that info from a TIFF file. (Not easily, anyway).
		return *this;
	}

	StackedTIFFReader& StackedTIFFReader::openFile(const std::string& filename) {
		if (this->tiffFile != nullptr) {
			TinyTIFFReader_close(this->tiffFile);
			this->tiffFile = nullptr;
		}
		if (filename.empty()) {
			TinyTIFFReader_close(this->tiffFile);
			this->tiffFile = nullptr;
		}

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

	StackedTIFFReader& StackedTIFFReader::loadSlice(std::size_t idx, std::vector<data_t>& tgt) {
		/**
		Here, we want to load directly an entire frame. We will read the current frame of the opened TIFF, and
		then go onto either the next frame in the file, OR the next file.
		*/
		#warning Defaults to using 8 bit values for all images loaded !
		#warning Assumes the vector is of the right size !
		TinyTIFFReader_getSampleData(this->tiffFile, tgt.data(), 0);

		if (TinyTIFFReader_hasNext(this->tiffFile)) {
			int ret = TinyTIFFReader_readNext(this->tiffFile);
			if (ret == 0) {
				std::cerr << "[ERROR] Could not read the next frame of the TIFF file, even tough it has one.\n";
				std::cerr << "        Loading the next image instead.\n";
				this->currentFile++;
				this->openFile(this->filenames[this->currentFile]);
			}
		} else {
			this->currentFile++;
			if (this->currentFile < this->filenames.size()) {
				this->openFile(this->filenames[this->currentFile]);
			} else {
				this->currentFile = 0;
			}
		}

		//look for min/max values per-slice :
		std::for_each(std::begin(tgt), std::end(tgt), [this](const data_t& d) -> void {
			if (this->hasUserBounds) {
				this->textureLimits.x = std::min(std::max(this->textureLimits.x, this->userLimits.x), d);
				this->textureLimits.y = std::max(std::min(this->textureLimits.y, this->userLimits.y), d);
			} else {
				this->textureLimits.x = std::min(this->textureLimits.x, d);
				this->textureLimits.y = std::max(this->textureLimits.y, d);
			}
		});

		return *this;
	}

	libTIFFReader::TIFFFrame::TIFFFrame(void) {
		this->filename = "";
		this->directoryOffset = 0;
		// By default, one 8-bit pixel per
		this->samplesPerPixel = 1;
		this->bitsPerSample.clear();
		this->width = 0;
		this->height = 0;
		this->rowsPerStrip = 0;
	}

	libTIFFReader::TIFFFrame::TIFFFrame(std::string _filename, tdir_t idx) : TIFFFrame() {
		this->filename = _filename;
		if (this->filename.empty()) {
			throw std::runtime_error("The TIFF file pointer passed was NULL.");
		}

		this->loadTIFFInfo(idx);
	}

	libTIFFReader::TIFFFrame::~TIFFFrame() { /* For now, nothing needs to be explicitely destructed here. */ }

	void libTIFFReader::TIFFFrame::loadTIFFInfo(tdir_t index) {
		this->directoryOffset = index;

		int result = 0; // result for all TIFF operations

		if (this->filename.empty() == false) {
			TIFFSetErrorHandler(nullify_tiff_errors);
			TIFFSetWarningHandler(nullify_tiff_errors);
			TIFF* file = TIFFOpen(this->filename.c_str(), "r");
			// We don't currently support tiled images :
			if (TIFFIsTiled(file)) {
				throw std::runtime_error("cannot read tiled images");
			}

			// Set the current directory (should already be there, but just to be sure) :
			result = TIFFSetDirectory(file, this->directoryOffset);
			// for some reason, we might read past-the-end on some directories. Handle the case :
			if (result != 1) { throw std::runtime_error("cannot set file to directory "+std::to_string(index)); }

			// Check the sample format of the directory :
			uint16_t sampleformat = SAMPLEFORMAT_UINT;
			result = TIFFGetField(file, TIFFTAG_SAMPLEFORMAT, &sampleformat);
			if (result != 1) {
				result = TIFFGetFieldDefaulted(file, TIFFTAG_SAMPLEFORMAT, &sampleformat);
				if (result != 1) {
					std::cerr << "[MAJOR_WARNING] The file \"" << TIFFFileName(file) << "\" has no SAMPLEFORMAT.";
					std::cerr << "[MAJOR_WARNING] Interpreting the data as unsigned ints by default.";
					sampleformat = SAMPLEFORMAT_UINT;
				}
			}
			if (sampleformat != SAMPLEFORMAT_UINT) {
				throw std::runtime_error("Cannot read values other than unsigned int yet");
			}

			uint16_t pconfig = 0;
			result = TIFFGetField(file, TIFFTAG_PLANARCONFIG, &pconfig);
			if (result != 1) { throw std::runtime_error("cannot get planarconfig."); }
			if (pconfig != 1) {
				throw std::runtime_error("Currently do not support planar configurations other than 1 (interleaved)");
			}

			// Get the number of samples per-pixel :
			result = TIFFGetField(file, TIFFTAG_SAMPLESPERPIXEL, &this->samplesPerPixel);
			if (result != 1) { throw std::runtime_error("cannot read samplesperpixel."); }

			// Get the number of bits per sample :
			this->bitsPerSample.resize(this->samplesPerPixel);
			result = TIFFGetField(file, TIFFTAG_BITSPERSAMPLE, this->bitsPerSample.data());
			if (result != 1) { throw std::runtime_error("cannot read bitsperpixel."); }

			// If there are more than 3 samples per pixel, then extra samples are used.
			// We don't support this currently.
			if (this->samplesPerPixel > 3u) { throw std::runtime_error("cant parse images with more than 3 samples."); }

			// Get image width and height :
			result = TIFFGetField(file, TIFFTAG_IMAGEWIDTH, &this->width);
			if (result != 1) { throw std::runtime_error("cannot read width field in this directory"); }
			result = TIFFGetField(file, TIFFTAG_IMAGELENGTH, &this->height);
			if (result != 1) { throw std::runtime_error("cannot read height field in this directory"); }

			// Get the number of rows per strip :
			result = TIFFGetField(file, TIFFTAG_ROWSPERSTRIP, &this->rowsPerStrip);
			if (result != 1) { throw std::runtime_error("cannot read rowsperstrip. maybe the image was tiled ?"); }

			// Compute the number of stripoffsets to return :
			this->stripsPerImage = (this->height + this->rowsPerStrip - 1)/this->rowsPerStrip;

			TIFFClose(file);
			std::cerr.flush();
		}
	}

	void libTIFFReader::TIFFFrame::printInfo(std::string prefix) {
		std::cerr << prefix << "TIFF frame \"" << this->filename << "\", directory " << this->directoryOffset
					<< " contents :\n";
		std::cerr << prefix << '\t' << "Image dimensions : {" << +this->width << 'x' << +this->height << "}\n";
		std::cerr << prefix << '\t' << "Strips per image : " << +this->stripsPerImage << '\n';
		std::cerr << prefix << '\t' << "Rows per strip : " << +this->rowsPerStrip << '\n';
		std::cerr << prefix << '\t' << "Samples per pixel : " << this->samplesPerPixel << '\n';
		std::cerr << prefix << '\t' << "Bits per sample : ";
		std::for_each(this->bitsPerSample.cbegin(), this->bitsPerSample.cend(), [](const uint16_t bps) {
			std::cerr << bps << ", ";
		});
		std::cerr << '\n';
		std::cerr << "\n" << prefix << "Finished TIFF directory " << this->directoryOffset << "\n";
	}

	libTIFFReader::libTIFFReader(data_t thresh) : GenericGridReader(thresh) {
		this->frames.clear();
		#ifdef VISUALIZATION_USE_READ_CACHE
		this->cache.clearCache();
		#endif
	}

	libTIFFReader::~libTIFFReader() {
		this->frames.clear();
	}

	libTIFFReader& libTIFFReader::parseImageInfo(ThreadedTask::Ptr& task) noexcept(false) {
		// if no filenames given, cannot preallocate data ...
		if (this->filenames.empty()) { task->end(); return *this; }

		// if already analyzed, do nothing :
		if (this->isAnalyzed == true) { task->end(); return *this; }
		this->isAnalyzed = true;

		this->frames.clear();

		// add one more than necessary, to not finish the task and add more elements later
		task->setSteps(this->filenames.size()+1);

		// Open files :
		std::for_each(this->filenames.cbegin(), this->filenames.cend(), [this, &task](const std::string f) {
			this->openFile(f);
			task->advance();
		});

		// Since TIFF doesn't support any spatial information akin to voxel sizes, or bounding boxes, we can
		// define a few default values here :

		std::cerr << "Files opened, frame count : " << this->frames.size() << "\n";

		task->setAdvancement(0);
		task->setSteps(this->frames.size());

		this->imageDimensions.x = this->frames[0].width;
		this->imageDimensions.y = this->frames[0].height;
		this->imageDimensions.z = this->frames.size();
		this->gridDimensions = this->imageDimensions;

		// TODO : check if we're not better off throwing an exception here ...
		std::for_each(this->frames.cbegin(), this->frames.cend(), [this, &task](const TIFFFrame& f) {
			if (f.width != this->imageDimensions.x || f.height != this->imageDimensions.y) {
				std::cerr << "[ERROR_DIMENSIONS] Frame " << f.directoryOffset << " of file " << f.filename <<
					" is not of the same dimensions. fDims = [" << f.width << ", " << f.height << "] compared to [" <<
					this->imageDimensions.x << ", " << this->imageDimensions.y << "]\n";
			}
			task->advance();
		});

		this->voxelDimensions = glm::vec3(1., 1., 1.);
		this->voxelMultiplier = glm::vec3(1., 1., 1.);
		this->transform = glm::mat4(1.f);

		// Update bounding box and data bounding box :
		this->boundingBox = bbox_t();
		this->dataBoundingBox = bbox_t();

		return *this;
	}

	libTIFFReader& libTIFFReader::loadImage(ThreadedTask::Ptr& task) {
		if (this->filenames.size() == 0) { return *this; }

		if (not this->isAnalyzed) {
			this->parseImageInfo(task);
			task->advance();
		}

		this->preAllocateStorage();

		// Set max nb of steps ...
		std::size_t nbsteps = this->gridDimensions.z;
		task->setSteps(nbsteps);

		// We need to update the bounding box size to reflect the user-defined size :
		using val_t = bbox_t::vec::value_type;
		bbox_t::vec maxCoord = bbox_t::vec(
			static_cast<val_t>(this->gridDimensions.x) * static_cast<val_t>(this->voxelDimensions.x*this->voxelMultiplier.x),
			static_cast<val_t>(this->gridDimensions.y) * static_cast<val_t>(this->voxelDimensions.y*this->voxelMultiplier.y),
			static_cast<val_t>(this->gridDimensions.z) * static_cast<val_t>(this->voxelDimensions.z*this->voxelMultiplier.z)
		);
		// Set the bounding box to the grid*voxel dimensions :
		this->boundingBox.setMin(bbox_t::vec(0, 0, 0));
		this->boundingBox.setMax(maxCoord);

		// The # of slices to load according to the downsampling level :
		std::size_t slicesToLoad = 1;
		if (this->downsampleLevel == DownsamplingLevel::Low) { slicesToLoad = 2; }
		if (this->downsampleLevel == DownsamplingLevel::Lower) { slicesToLoad = 4; }
		if (this->downsampleLevel == DownsamplingLevel::Lowest) { slicesToLoad = 8; }

		// holds the data of the (possibly) downsampled file, to insert into this->data
		std::vector<data_t> curSlice;
		// holds the data of the raw slice(s) from the filesystem.
		std::vector<data_t> rawSlices;
		// holds the data of a single slice, raw from the filesystem
		std::vector<data_t> singleSlice;

		curSlice.resize(this->gridDimensions.x * this->gridDimensions.y);
		// Resize to accept one slice at full-res :
		singleSlice.resize(this->imageDimensions.x * this->imageDimensions.y);
		// only uses memory if we have to downsample :
		if (this->downsampleLevel != DownsamplingLevel::Original) {
			// resize to accept multiple slices at full res :
			rawSlices.resize(singleSlice.size() * slicesToLoad);
		}

		// iterator for raw slices :
		std::vector<data_t>::iterator rawSlicesIterator = std::begin(rawSlices);

		// Go through the grid :
		for (std::size_t k = 0; k < this->gridDimensions.z; ++k) {
			rawSlicesIterator = std::begin(rawSlices);
			// Load slice(s) here :
			for (std::size_t i = 0; i < slicesToLoad; ++i) {
				this->loadSlice(k*slicesToLoad+i, singleSlice);
				// Do this only if we have multiple slices to load, otherwise skip it
				if (this->downsampleLevel != DownsamplingLevel::Original) {
					// copy into rawslices :
					rawSlicesIterator = std::copy(std::begin(singleSlice), std::end(singleSlice), rawSlicesIterator);
				}
			}

			if (this->downsampleLevel == DownsamplingLevel::Original) {
				// Straight copy from loaded data to current slice data, updating the insertion iterator :
				std::copy(std::begin(singleSlice), std::end(singleSlice), std::begin(curSlice));
			} else {
				// input data into curSlice, one pixel at a time :
				for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
					for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
						// Interpolation code here !

						std::vector<data_t> interpolationData;
						std::vector<data_t>::iterator interpolationIterator;

						// Gather necessary data :
						for (std::size_t z = 0; z < slicesToLoad; ++z) {
							for (std::size_t y = 0; y < slicesToLoad; ++y) {
								std::size_t i_x = i * slicesToLoad;
								std::size_t i_y = (j * slicesToLoad + y) * this->imageDimensions.x;
								std::size_t i_z = (z) * this->imageDimensions.x * this->imageDimensions.y;
								std::size_t iter = i_x + i_y + i_z;
								// read 'slicesToLoad' elements from the grid :
								interpolationIterator = interpolationData.insert(interpolationIterator, rawSlices.begin()+iter, rawSlices.begin()+iter+slicesToLoad);
							}
						}

						data_t pixelVal = (*this->interpolator)(slicesToLoad, interpolationData);
						std::size_t idx = i + j * this->gridDimensions.x;
						curSlice[idx] = pixelVal;
					}
				}
			}

			bbox_t::vec minBB = this->boundingBox.getMin();
			// Copy data from curSlice into final buffer :
			for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
				for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
					// Copy into final buffer :
					std::size_t idx = i + j * this->gridDimensions.x;
					std::size_t data_idx = idx + k * this->gridDimensions.x * this->gridDimensions.y;
					this->data[data_idx] = curSlice[idx];
					if (this->hasUserBounds) {
						// If the user has supplied bounds for the ROI
						if (this->data[data_idx] > this->userLimits.x && this->data[data_idx] < this->userLimits.y) {
							// Update data BB :
							bbox_t::vec v;
							v.x = minBB.x + static_cast<bbox_t::vec::value_type>(i) * this->voxelDimensions.x * this->voxelMultiplier.x;
							v.y = minBB.y + static_cast<bbox_t::vec::value_type>(j) * this->voxelDimensions.y * this->voxelMultiplier.y;
							v.z = minBB.z + static_cast<bbox_t::vec::value_type>(k) * this->voxelDimensions.z * this->voxelMultiplier.z;
							this->dataBoundingBox.addPoint(v);
						}
					} else {
						// regular bb update, only above threshold :
						if (this->data[data_idx] > this->threshold) {
							// Update data BB :
							bbox_t::vec v;
							v.x = minBB.x + static_cast<bbox_t::vec::value_type>(i) * this->voxelDimensions.x * this->voxelMultiplier.x;
							v.y = minBB.y + static_cast<bbox_t::vec::value_type>(j) * this->voxelDimensions.y * this->voxelMultiplier.y;
							v.z = minBB.z + static_cast<bbox_t::vec::value_type>(k) * this->voxelDimensions.z * this->voxelMultiplier.z;
							this->dataBoundingBox.addPoint(v);
						}
					}
				}
			}
			task->advance();
		}
		#ifdef VISUALIZATION_USE_READ_CACHE
		// Clear the cache, we don't need it anymore :
		this->cache.clearCache();
		#endif
		return *this;
	}

	libTIFFReader& libTIFFReader::preAllocateStorage() {
		// Check the downsampling rate :
		std::size_t divider = 1;
		if (this->downsampleLevel == IO::DownsamplingLevel::Low   ) { divider = 2u; }
		if (this->downsampleLevel == IO::DownsamplingLevel::Lower ) { divider = 4u; }
		if (this->downsampleLevel == IO::DownsamplingLevel::Lowest) { divider = 8u; }

		this->gridDimensions = this->imageDimensions / sizevec3(divider, divider, divider);
		this->data.resize(this->gridDimensions.x * this->gridDimensions.y * this->gridDimensions.z);

		using val_t = bbox_t::vec::value_type;
		bbox_t::vec minCoord = bbox_t::vec(0, 0, 0);
		bbox_t::vec maxCoord = bbox_t::vec(
			static_cast<val_t>(this->imageDimensions.x) * static_cast<val_t>(this->voxelDimensions.x),
			static_cast<val_t>(this->imageDimensions.y) * static_cast<val_t>(this->voxelDimensions.y),
			static_cast<val_t>(this->imageDimensions.z) * static_cast<val_t>(this->voxelDimensions.z)
		);

		this->boundingBox = bbox_t(minCoord, maxCoord);
		this->dataBoundingBox = bbox_t();

		return *this;
	}

	libTIFFReader& libTIFFReader::openFile(const std::string &filename) {
		TIFFSetErrorHandler(nullify_tiff_errors);
		TIFFSetWarningHandler(nullify_tiff_errors);
		TIFF* handle = TIFFOpen(filename.c_str(), "r");
		if (handle == nullptr) {
			throw std::runtime_error("Cannot open " + filename + " !");
		}

		// Count its directories :
		tdir_t nod = TIFFNumberOfDirectories(handle);
		if (nod == 0) { throw std::runtime_error("File " + filename +  " had no directories !"); }

		// Parse each frame of the file :
		for (tdir_t i = 0; i < nod; ++i) {
			try {
				this->frames.emplace_back(filename, i);
			} catch (std::exception& e) {
				std::cerr << "[ERROR] Directory " << i << " of file " << filename << " generated an error. Message :\n";
				std::cerr << '\t' << e.what() << '\n';
			}
		}

		TIFFClose(handle);

		return *this;
	}

	libTIFFReader& libTIFFReader::loadSlice(std::size_t idx, std::vector<data_t> &tgt) {
		if (idx >= this->frames.size()) {
			std::cerr << "ERROR : no slice present at index " << idx << ".\n";
			return *this;
		}

		const TIFFFrame& frame = this->frames[idx];
		TIFFSetErrorHandler(nullify_tiff_errors);
		TIFFSetWarningHandler(nullify_tiff_errors);
		TIFF* file = TIFFOpen(frame.filename.c_str(), "r");
		// Set the tiff frame to be current in its designated file :
		int result = TIFFSetDirectory(file, frame.directoryOffset);
		if (result != 1) { throw std::runtime_error("Could not set the TIFF directory to"+std::to_string(frame.directoryOffset)); }

		// Compute offset for the pixels :
		uint64_t pixelOffset = 0;
		for (uint16_t px = 0; px < frame.samplesPerPixel; ++px) { pixelOffset += frame.bitsPerSample[px]; }

		// Read each strip :
		for (uint64_t i = 0; i < frame.stripsPerImage; ++i) {
			tsize_t readPixelSize = 0;
			if (i == frame.stripsPerImage-1) {
				// The last strip is handled differently than the rest. Fewer bytes should be read.
				// compute the remaining rows, to get the number of bytes :
				tsize_t last_strip_row_count = frame.height - (frame.stripsPerImage - 1)*frame.rowsPerStrip;
				if (last_strip_row_count == 0) { throw std::runtime_error("Last strip was 0 rows tall ! Something went wrong beforehand."); }
				readPixelSize = last_strip_row_count * frame.width;
			} else {
				readPixelSize = frame.width * frame.rowsPerStrip;	// strip size, in pixels
			}

			tsize_t readBytesSize = readPixelSize*(pixelOffset/8);		// strip size, in bytes
			tsize_t stripPixelSize = frame.width * frame.rowsPerStrip;	// The

			tmsize_t read = TIFFReadEncodedStrip(file, i, tgt.data()+i*stripPixelSize, readBytesSize);
			if (read < 0) {
				// print width for numbers :
				std::size_t pwidth = static_cast<std::size_t>(std::ceil(std::log10(static_cast<double>(frame.stripsPerImage))));
				std::cerr << "[DEBUG]\t\t(" << std::setw(pwidth) << i << "/" << frame.stripsPerImage << ") ERROR : Could not read strip " << i << " from the frame.\n";
			}
		}

		// Update texture bounds :
		std::for_each(tgt.cbegin(), tgt.cend(), [this](const data_t& d) -> void {
			if (this->hasUserBounds) {
				this->textureLimits.x = std::min(std::max(this->textureLimits.x, this->userLimits.x), d);
				this->textureLimits.y = std::max(std::min(this->textureLimits.y, this->userLimits.y), d);
			} else {
				this->textureLimits.x = std::min(this->textureLimits.x, d);
				this->textureLimits.y = std::max(this->textureLimits.y, d);
			}
		});
		TIFFClose(file);

		return *this;
	}

	libTIFFReader::data_t libTIFFReader::getPixel(std::size_t i, std::size_t j, std::size_t k) {
		if (k >= this->frames.size()) { return 0; }
		// k will be used to get the right frame :
		const TIFFFrame& frame = this->frames[k];
		TIFFSetErrorHandler(nullify_tiff_errors);
		TIFFSetWarningHandler(nullify_tiff_errors);
		// The loaded frame data :
		std::shared_ptr<frame_data_t> frameData = nullptr;
		// The resulting pixel value :
		data_t result;

		// If the cache doesn't have the loaded frame, load it :
		if ( (frameData = this->cache.getData(k)) == nullptr) {
			frameData = std::make_shared<frame_data_t>(frame.width*frame.height);
			this->loadSlice(k, *frameData);

			// Add to the cache :
			this->cache.loadData(k, frameData);
		}

		// frameData has the entire frame loaded, get the right pixel :
		std::size_t idx = frame.width * j + i;
		result = (*frameData)[idx];

		return result;
	}

	libTIFFReader::data_t libTIFFReader::getPixel_ImageSpace(glm::vec4 pos) {
		// assumes the point is given in image space.
		// We still need to check if if actually point to somewhere in image space, though.

		// checks the point is actually in the bb :
		if (this->boundingBox.contains(bbox_t::vec(glm::convert_to<bbox_t::data_t>(pos)))) {
			// compute the actual coordinates of the point :
			glm::vec3 inside_pos = glm::vec3(pos.x, pos.y, pos.z) - glm::convert_to<float>(this->boundingBox.getMin());
			// divide by 'actual' voxel dimensions (we emulate no downsampling here) :
			glm::uvec3 coords = inside_pos / this->voxelDimensions;
			// Get the value !
			return this->getPixel(coords.x, coords.y, coords.z);
		}

		return data_t(0);
	}

	OMETIFFReader::OMETIFFReader(data_t thresh) : libTIFFReader(thresh) {}

	OMETIFFReader& OMETIFFReader::parseImageInfo(ThreadedTask::Ptr &task) {
		if (this->filenames.size() == 0) { task->end(); return *this; }

		// Suppress TIFF warnings and errors.
		TIFFSetErrorHandler(nullify_tiff_errors);
		TIFFSetWarningHandler(nullify_tiff_errors);

		QDir defaultDir = QFileInfo(this->filenames[0].c_str()).absoluteDir();

		// Open the first file (supposed to have header) :
		TIFF* file_with_header = TIFFOpen(this->filenames[0].c_str(), "r");
		char* desc = nullptr;

		// Get the description :
		int result = TIFFGetField(file_with_header, TIFFTAG_IMAGEDESCRIPTION, &desc);
		if (result != 1) {
			throw std::runtime_error("Could not read the image description of file " + this->filenames[0]);
		}

		// If the description is empty, retun here :
		if (strlen(desc) == 0) { throw std::runtime_error("Error : TIFF description did not contain any data"); }

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
				// If we're in the image tag :
				if (reader.name().compare(QString("Image"), Qt::CaseSensitivity::CaseSensitive) == 0) {
					auto attributes = reader.attributes();
					if (attributes.hasAttribute("Name")) {
						this->name = attributes.value("Name").toString().toStdString();
					}
				}

				// If we're in the Pixels tag (where the data is) :
				if (reader.name().compare(QString("Pixels"), Qt::CaseSensitivity::CaseSensitive) == 0) {
					// Get attributes of the image :
					auto attributes = reader.attributes();

					// Attributes for the voxel dimensions :
					if (attributes.hasAttribute("PhysicalSizeX")) {
						this->voxelDimensions.x = attributes.value("PhysicalSizeX").toFloat();
					}
					if (attributes.hasAttribute("PhysicalSizeY")) {
						this->voxelDimensions.y = attributes.value("PhysicalSizeY").toFloat();
					}
					if (attributes.hasAttribute("PhysicalSizeZ")) {
						this->voxelDimensions.z = attributes.value("PhysicalSizeZ").toFloat();
					}

					// Attributes for the image dimensions :
					if (attributes.hasAttribute("SizeX")) {
						this->imageDimensions.x = attributes.value("SizeX").toUInt();
					}
					if (attributes.hasAttribute("SizeY")) {
						this->imageDimensions.y = attributes.value("SizeY").toUInt();
					}
					if (attributes.hasAttribute("SizeZ")) {
						this->imageDimensions.z = attributes.value("SizeZ").toUInt();
						task->setSteps(this->imageDimensions.z);
						task->setAdvancement(0);
					}
				}

				// If we have an image identifier, open it :
				if (reader.name().compare(QString("TiffData"), Qt::CaseSensitivity::CaseSensitive) == 0) {
					tdir_t ifd_index = 0;
					QString filename;
					QStringRef ifdRef, filenameRef;
					if ((ifdRef = reader.attributes().value("IFD")).isNull() == false) {
						ifd_index = ifdRef.toString().toUInt();
					}
					// Go to the UUID element :
					reader.readNextStartElement();
					// In the UUID element, the filename is under the "FileName" attribute, the text is only
					// the actual URN:UUID scheme for the OME-TIFF specification. Get the file name :
					if ((filenameRef = reader.attributes().value("FileName")).isNull() == false) {
						filename = filenameRef.toString();
						/// Check if file exists :
						if (QFileInfo::exists(defaultDir.path() + "/" + filename)) {
							#warning Here, we exploit the fact the data is separated. To fix with new interface(soon)
							// File exists, we can add it to the list:
							std::string real_filename = defaultDir.path().toStdString() + "/" + filename.toStdString();
							this->frames.emplace_back(real_filename, ifd_index);
							task->advance();
						} else {
							std::cerr << "[Error] File " << filename.toStdString() << " does not exist in " <<
										 defaultDir.path().toStdString() << '\n';
						}
					}
					// Get out of the UUID tag
					reader.readNext();
				}
			}
		}

		reader.clear();

		// Default values for the other data members :
		this->gridDimensions = this->imageDimensions;
		this->voxelMultiplier = glm::vec3(1., 1., 1.);
		this->transform = glm::mat4(1.f);

		// Update bounding box and data bounding box :
		this->boundingBox = bbox_t();
		this->dataBoundingBox = bbox_t();

		this->isAnalyzed = true;

		task->end();

		return *this;
	}

}
