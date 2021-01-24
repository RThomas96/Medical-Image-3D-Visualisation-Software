#include "../include/reader.hpp"

#include <cstring>
#include <memory>

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
		this->downsampleLevel = DownsamplingLevel::Original;
		this->data.clear();
		this->filenames.clear();
		this->name = "";
		// init limits
		this->textureLimits.x = std::numeric_limits<data_t>::max();
		this->textureLimits.y = std::numeric_limits<data_t>::lowest();
		// interpolation struct :
		this->interpolator = nullptr;
	}

	GenericGridReader::~GenericGridReader() {
		this->filenames.clear();
		this->data.clear();
	}

	GenericGridReader& GenericGridReader::setDataThreshold(data_t _thresh) {
		this->threshold = _thresh;
		return *this;
	}

	GenericGridReader& GenericGridReader::setInterpolationMethod(std::shared_ptr<Interpolators::genericInterpolator<data_t>> &ptr) {
		this->interpolator = ptr;
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
		return *this;
	}

	GenericGridReader& GenericGridReader::loadImage() { return *this; }

	const std::vector<GenericGridReader::data_t>& GenericGridReader::getGrid() const { return this->data; }

	GenericGridReader::sizevec3 GenericGridReader::getGridDimensions() const { return this->gridDimensions; }

	glm::vec3 GenericGridReader::getVoxelDimensions() const { return this->voxelDimensions; }

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

		if (this->downsampleLevel != DownsamplingLevel::Original && this->interpolator == nullptr) {
			std::cerr << "[ERROR] No interpolation structure was set in this reader, even though a downsample was required.\n";
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

		// Check if dim/ima files are opened :
		if (this->dimFile == nullptr || this->imaFile == nullptr) {
			std::cerr << "Some files were not opened. Stopping loading now.\n";
			return *this;
		}

		std::cerr << "[LOG] Reading DIM data ...\n";

		// read number of voxels into image dimensions :
		(*this->dimFile) >> this->imageDimensions.x;
		(*this->dimFile) >> this->imageDimensions.y;
		(*this->dimFile) >> this->imageDimensions.z;

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

		// # of slices to load at once in order to downsample :
		std::size_t slicesToLoad = 1;
		// Compute voxel and grid sizes :
		sizevec3 dataDims = this->imageDimensions;
		if (this->downsampleLevel == DownsamplingLevel::Low) {
			dataDims /= std::size_t(2); this->voxelDimensions *= 2.; slicesToLoad = 2;
		}
		if (this->downsampleLevel == DownsamplingLevel::Lower) {
			dataDims /= std::size_t(4); this->voxelDimensions *= 4.; slicesToLoad = 4;
		}
		if (this->downsampleLevel == DownsamplingLevel::Lowest) {
			dataDims /= std::size_t(8); this->voxelDimensions *= 8.; slicesToLoad = 8;
		}
		this->gridDimensions = dataDims;

		// Get the max coord as a bounding box-type vector :
		using val_t = bbox_t::vec::value_type;
		bbox_t::vec maxCoord = bbox_t::vec(
			static_cast<val_t>(this->gridDimensions.x) * static_cast<val_t>(this->voxelDimensions.x),
			static_cast<val_t>(this->gridDimensions.y) * static_cast<val_t>(this->voxelDimensions.y),
			static_cast<val_t>(this->gridDimensions.z) * static_cast<val_t>(this->voxelDimensions.z)
		);

		// Set the bounding box to the grid*voxel dimensions :
		this->boundingBox.setMin(bbox_t::vec(0, 0, 0));
		this->boundingBox.setMax(maxCoord);

		std::cerr << "[LOG] Reading from IMA file ...\n";
		// resize data vector to fit all data :
		std::size_t dataSize = this->gridDimensions.x * this->gridDimensions.y * this->gridDimensions.z;
		this->data.resize(dataSize);

		std::vector<data_t> rawSlices;		// Raw slices loaded at full-res to downsample
		std::vector<data_t> singleSlice;	// Single raw slice (also full-res)
		std::vector<data_t> curSlice;		// Slice to output to this->data after processing

		curSlice.resize(this->gridDimensions.x * this->gridDimensions.y); // resize for one slice at grid dims
		singleSlice.resize(this->imageDimensions.x * this->imageDimensions.y); // Resize for 1 slice at full-res
		if (this->downsampleLevel != DownsamplingLevel::Original) {
			rawSlices.resize(singleSlice.size() * slicesToLoad);	// Resize to fit 'n' slices at full-res
		}

		// iterator pointing to positions in different vectors :
		std::vector<data_t>::iterator rawSlicesIterator = std::begin(rawSlices);

		// update data bounding box at the same time we're copying data:
		this->dataBoundingBox = bbox_t();
		for (std::size_t k = 0; k < this->gridDimensions.z; ++k) {
			rawSlicesIterator = std::begin(rawSlices);
			// Load slice(s) according to slicesToLoad :
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

			// Copy data from curSlice to data :
			for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
				for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
					std::size_t idx = i + j * this->gridDimensions.x;
					std::size_t data_idx = idx + k * this->gridDimensions.x * this->gridDimensions.y;
					this->data[data_idx] = curSlice[idx];
					if (this->data[data_idx] > this->threshold) {
						// Update data BB :
						bbox_t::vec v;
						v.x = static_cast<val_t>(i);
						v.y = static_cast<val_t>(j);
						v.z = static_cast<val_t>(k);
						this->dataBoundingBox.addPoint(v);
					}
				}
			}

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

	DIMReader& DIMReader::loadSlice(std::size_t idx, std::vector<data_t>& tgt) {
		if (this->imaFile == nullptr) {
			std::cerr << "[ERROR] IMA file was not open at the moment to call loadSlice()" << '\n';
			return *this;
		}
		if (not this->imaFile->good()) {
			std::cerr << "[ERROR] IMA file has suffered an irrecuperable error. (std::basic_ios::good() == false)\n";
			return *this;
		}

		// automatically advances the internal std::basic_ifstream position indicator in order to read the
		// next data whenever std::basic_ifstream::read is called another time :
		this->imaFile->read((char*)tgt.data(), this->imageDimensions.x * this->imageDimensions.y);

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

	StackedTIFFReader& StackedTIFFReader::loadImage() {
		// checks we have something to load :
		if (this->filenames.size() == 0) {
			std::cerr << "[LOG] No filenames were provided, nothing will be loaded." << '\n';
			return *this;
		}

		// Prepare storage of all the image's data :
		this->preAllocateStorage();

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

			// Copy data from curSlice into final buffer :
			for (std::size_t j = 0; j < this->gridDimensions.y; ++j) {
				for (std::size_t i = 0; i < this->gridDimensions.x; ++i) {
					// Copy into final buffer :
					std::size_t idx = i + j * this->gridDimensions.x;
					std::size_t data_idx = idx + k * this->gridDimensions.x * this->gridDimensions.y;
					this->data[data_idx] = curSlice[idx];
					if (this->data[data_idx] > this->threshold) {
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

		return *this;
	}

	StackedTIFFReader& StackedTIFFReader::preAllocateStorage() {
		// Count the # of frames in the stack :
		std::size_t nbFrames = 0;
		for (const std::string& fname : this->filenames) {
			// open the file, check the frames inside it and continue
			this->openFile(fname);
			do { nbFrames++; } while (TinyTIFFReader_hasNext(this->tiffFile));
		}
		std::cerr << "Finished analyzing the files. Found " << nbFrames << " frames in " << this->filenames.size() << " files.\n";
		this->currentFile = 0;
		// re-open the first file :
		this->openFile(filenames[0]);

		// File is opened, we can query its dimensions :
		std::size_t width = static_cast<std::size_t>(TinyTIFFReader_getWidth(this->tiffFile));
		std::size_t height = static_cast<std::size_t>(TinyTIFFReader_getHeight(this->tiffFile));
		std::size_t depth = static_cast<std::size_t>(nbFrames);

		// 'raw' image dimensions are what is originally read above :
		this->imageDimensions = sizevec3(width, height, depth);

		// multiplier to voxel dimensions :
		float voxelMultiplier = 1.f;
		// used to compute the voxel dimensions in the (possibly) downsampled grid loaded.
		// By default, the voxels in TIFF images are 1.f units since there's no way to specify
		// it in the TIFF spec. But is we downsample, we want to keep the same grid size, but
		// with less voxels inside. We thus need a multiplier for voxel sizes

		// If we downsample, apply the dimension reduction here on all axes
		// and modify the voxel multiplier accordingly :
		if (this->downsampleLevel == DownsamplingLevel::Low) {
			width /= 2; height /= 2; depth /= 2; voxelMultiplier = 2.f;
		} else if (this->downsampleLevel == DownsamplingLevel::Lower) {
			width /= 4; height /= 4; depth /= 4; voxelMultiplier = 4.f;
		} else if (this->downsampleLevel == DownsamplingLevel::Lowest) {
			width /= 8; height /= 8; depth /= 8; voxelMultiplier = 8.f;
		}

		// Compute the vector size to allocate :
		std::size_t finalSize = width * height * depth;

		// Compute the min/max values for the bounding box :
		bbox_t::vec minBB = bbox_t::vec(static_cast<bbox_t::vec::value_type>(0));
		bbox_t::vec maxBB = bbox_t::vec(
			static_cast<bbox_t::vec::value_type>(this->imageDimensions.x),
			static_cast<bbox_t::vec::value_type>(this->imageDimensions.y),
			static_cast<bbox_t::vec::value_type>(this->imageDimensions.z)
		);
		// Resize the vector :
		this->data.resize(finalSize);
		// Fill in the data for grid dimensions, bounding box, transform and voxel dimensions :
		this->gridDimensions = sizevec3(width, height, depth);
		this->transform = glm::mat4(1.f);
		this->voxelDimensions = glm::vec3(1.f, 1.f, 1.f) * voxelMultiplier;
		this->boundingBox = bbox_t(minBB, maxBB);
		this->dataBoundingBox = bbox_t();

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

		//std::cerr << "[LOG] Opening file named \"" << filename << "\" ..." << '\n';
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
		/*try {
			this->openFile(this->filenames[idx]);
		} catch (std::runtime_error& e) {
			std::cerr << "[ERROR] A runtime error occured while loading file \"" << this->filenames[idx]
				     << "\". Error message :\n";
			std::cerr << e.what() << '\n';
			std::abort();
		}*/

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
			this->textureLimits.x = std::min(this->textureLimits.x, d);
			this->textureLimits.y = std::max(this->textureLimits.y, d);
		});

		return *this;
/*
		// file is now opened, we can load the data into a vector :
		std::vector<data_t> rawData;
		rawData.resize(this->gridDimensions.x * this->gridDimensions.y);

		if (this->downsampleLevel == DownsamplingLevel::Original) {
			// If we aren't downsampled, then the grid size is the same as the image size.
			// load data :
			TinyTIFFReader_getSampleData(this->tiffFile, rawData.data(), 0);
		} else {
			#warning Check we read the proper data here. Height doesn't get skipped in for loop
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
*/
	}

}
