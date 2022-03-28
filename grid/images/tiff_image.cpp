#include "tiff_image.hpp"
#include <algorithm>

SimpleTIFFImage::SimpleTIFFImage(const std::vector<std::string>& filename): tiffReader(new TIFFReader(filename)) {
    this->imgResolution = this->tiffReader->getImageResolution();
    this->imgDataType = this->tiffReader->getImageInternalDataType(); 
}

Image::ImageDataType SimpleTIFFImage::getInternalDataType() const {
    return this->imgDataType;
}

// Function to cast for the getValue
uint16_t getToLowPrecision(Image::ImageDataType imgDataType, const tdata_t& buf, int idx) {
    if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8)) {
        uint8_t * data = static_cast<std::uint8_t*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16)) {
        uint16_t * data = static_cast<std::uint16_t*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32)) {
        uint32_t * data = static_cast<std::uint32_t*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64)) {
        uint64_t * data = static_cast<std::uint64_t*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_8)) {
        int8_t * data = static_cast<std::int8_t*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_16)) {
        int16_t * data = static_cast<std::int16_t*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_32)) {
        int32_t * data = static_cast<std::int32_t*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_64)) {
        int64_t * data = static_cast<std::int64_t*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_32)) {
        float * data = static_cast<float*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_64)) {
        double * data = static_cast<double*>(buf);
        return static_cast<std::uint16_t>(data[idx]);
    } 
}

uint16_t SimpleTIFFImage::getValue(const glm::vec3& coord) const {
    // If we read directly from the raw image we use Nearest Neighbor interpolation
    const glm::vec3 newCoord{std::floor(coord[0]), std::floor(coord[1]), std::floor(coord[2])};
    int imageIdx = newCoord[2];
    this->tiffReader->setImageToRead(imageIdx);
    tdata_t buf = _TIFFmalloc(this->tiffReader->getScanLineSize());
    this->tiffReader->readScanline(buf, newCoord[1]);
    uint16_t res = getToLowPrecision(this->getInternalDataType(), buf, newCoord[0]);
    _TIFFfree(buf);
    return res;
}

//Function to cast and insert for the get slice
template <typename data_t>
void castToUintAndInsert(data_t * values, std::vector<uint16_t>& res, int duplicate, int offset, std::pair<glm::vec3, glm::vec3> bboxes) {
    for(int i = bboxes.first[0]; i < bboxes.second[0]; i+=offset) {
        for(int j = 0; j < duplicate; ++j)
            res.push_back(static_cast<std::uint16_t>(values[i])); 
    }
}

//Function to cast and insert for the get slice
void castToLowPrecision(Image::ImageDataType imgDataType, const tdata_t& buf, std::vector<uint16_t>& res, int duplicate, int offset, std::pair<glm::vec3, glm::vec3> bboxes) {
    if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8)) {
        uint8_t * data = static_cast<std::uint8_t*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16)) {
        uint16_t * data = static_cast<std::uint16_t*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32)) {
        uint32_t * data = static_cast<std::uint32_t*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64)) {
        uint64_t * data = static_cast<std::uint64_t*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_8)) {
        int8_t * data = static_cast<std::int8_t*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_16)) {
        int16_t * data = static_cast<std::int16_t*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_32)) {
        int32_t * data = static_cast<std::int32_t*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_64)) {
        int64_t * data = static_cast<std::int64_t*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_32)) {
        float * data = static_cast<float*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_64)) {
        double * data = static_cast<double*>(buf);
        castToUintAndInsert(data, res, duplicate, offset, bboxes);
    } 
}

void SimpleTIFFImage::getSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel, std::pair<int, int>  offsets, std::pair<glm::vec3, glm::vec3> bboxes) const {
    this->tiffReader->setImageToRead(sliceIdx);

    tdata_t buf;
    buf = _TIFFmalloc(this->tiffReader->getScanLineSize());

    uint32 row;
    for (row = bboxes.first[1]; row < bboxes.second[1]; row+=offsets.second) {
        this->tiffReader->readScanline(buf, row);
        castToLowPrecision(this->getInternalDataType(), buf, result, nbChannel, offsets.first, bboxes);
    }
    _TIFFfree(buf);
}

/***/

TIFFReader::TIFFReader(const std::vector<std::string>& filename): filenames(filename) {
    TIFFSetWarningHandler(nullptr); // Prevent to display warning
    this->tif = TIFFOpen(this->filenames[0].c_str(), "r");
    this->openedImage = 0;
}

void TIFFReader::openImage(int imageIdx) {
    TIFFClose(this->tif);
    this->tif = TIFFOpen(this->filenames[imageIdx].c_str(), "r");
}

void TIFFReader::closeImage() {
    TIFFClose(this->tif);
}

glm::vec3 TIFFReader::getImageResolution() const {
    uint32_t width = 0;
    uint32_t depth = 0;
    uint32_t length = 0;
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &length);
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    int dircount = 0;
    if(this->filenames.size() == 1) {
        do {
            dircount++;
        } while (TIFFReadDirectory(tif));
    } else {
        dircount = this->filenames.size(); 
    }
    return glm::vec3(width, length, dircount);
}

Image::ImageDataType TIFFReader::getImageInternalDataType() const {
    uint16_t sf = SAMPLEFORMAT_VOID;
    int result	= TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sf);
    if (result != 1) {
    	// Try to get the defaulted version of the field :
    	result = TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLEFORMAT, &sf);
    	// Some files might still not get the default info, in that case interpret as UINT
    	if (result != 1) {
    		sf = SAMPLEFORMAT_UINT;
    	}
    }
    
    uint16_t bps = 0;
    result	 = TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);

    switch (sf) {
    	case SAMPLEFORMAT_VOID:
    		throw std::runtime_error("Internal type of the frame was void.");
    		break;
    
    	case SAMPLEFORMAT_UINT: {
    		if (bps == 8) {
                std::cout << "Internal data type: uint8_t" << std::endl;
			    return (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8);
    		}
    		if (bps == 16) {
                std::cout << "Internal data type: uint16_t" << std::endl;
			    return (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16);
    		}
    		if (bps == 32) {
                std::cout << "Internal data type: uint32_t" << std::endl;
			    return (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32);
    		}
    		if (bps == 64) {
                std::cout << "Internal data type: uint64_t" << std::endl;
			    return (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64);
    		}
    	} break;
    
    	case SAMPLEFORMAT_INT: {
    		if (bps == 8) {
                std::cout << "Internal data type: int8_t" << std::endl;
			    return (Image::ImageDataType::Signed | Image::ImageDataType::Bit_8);
    		}
    		if (bps == 16) {
                std::cout << "Internal data type: int16_t" << std::endl;
			    return (Image::ImageDataType::Signed | Image::ImageDataType::Bit_16);
    		}
    		if (bps == 32) {
                std::cout << "Internal data type: int32_t" << std::endl;
			    return (Image::ImageDataType::Signed | Image::ImageDataType::Bit_32);
    		}
    		if (bps == 64) {
                std::cout << "Internal data type: int64_t" << std::endl;
			    return (Image::ImageDataType::Signed | Image::ImageDataType::Bit_64);
    		}
    	} break;
    
    	case SAMPLEFORMAT_IEEEFP: {
    		if (bps == 32) {
                std::cout << "Internal data type: float" << std::endl;
			    return (Image::ImageDataType::Floating | Image::ImageDataType::Bit_32);
    		}
    		if (bps == 64) {
                std::cout << "Internal data type: double" << std::endl;
			    return (Image::ImageDataType::Floating | Image::ImageDataType::Bit_64);
    		}
    	} break;
    
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

void TIFFReader::setImageToRead(int sliceIdx) {
    if(this->filenames.size() > 1) {
        this->openImage(sliceIdx);
    } else {
        TIFFSetDirectory(this->tif, sliceIdx);
    }
}

tsize_t TIFFReader::getScanLineSize() const {
    return TIFFScanlineSize(this->tif);
}

int TIFFReader::readScanline(tdata_t buf, uint32 row) const {
    return TIFFReadScanline(this->tif, buf, row);
}
