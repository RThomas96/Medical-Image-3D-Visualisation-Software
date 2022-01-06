#include "../include/tiff_image.hpp"

TIFFImage::TIFFImage(const std::string& filename) {
    //this->tif = TIFFOpen("../../../../Data/v3_a5_100_150_8bit_normalized.tif", "r");
    //this->tif = TIFFOpen("../../../../Data/myTiff.tif", "r");
    //this->tif = TIFFOpen("../../../../../../../data/datasets/tulane/v3/registration_subset/v3_a5_100_150_8bit_normalized_25.tif", "r");
    //this->tif = TIFFOpen("../../../../../../../data/datasets/tulane/v3/registration_subset/v3_a5_100_150_8bit_normalized.tif", "r");
    this->tif = TIFFOpen(filename.c_str(), "r");
    TIFFSetWarningHandler(nullptr); // Prevent to display warning
    uint32_t width = 0;
    uint32_t depth = 0;
    uint32_t length = 0;
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &length);
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    int dircount = 0;
    do {
        dircount++;
    } while (TIFFReadDirectory(tif));
    imgDimensions = glm::vec3(width, length, dircount);

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
			    this->imgDataType = Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8;
    		}
    		if (bps == 16) {
                std::cout << "Internal data type: uint16_t" << std::endl;
			    this->imgDataType = Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16;
    		}
    		if (bps == 32) {
                std::cout << "Internal data type: uint32_t" << std::endl;
			    this->imgDataType = Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32;
    		}
    		if (bps == 64) {
                std::cout << "Internal data type: uint64_t" << std::endl;
			    this->imgDataType = Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64;
    		}
    	} break;
    
    	case SAMPLEFORMAT_INT: {
    		if (bps == 8) {
                std::cout << "Internal data type: int8_t" << std::endl;
			    this->imgDataType = Image::ImageDataType::Signed | Image::ImageDataType::Bit_8;
    		}
    		if (bps == 16) {
                std::cout << "Internal data type: int16_t" << std::endl;
			    this->imgDataType = Image::ImageDataType::Signed | Image::ImageDataType::Bit_16;
    		}
    		if (bps == 32) {
                std::cout << "Internal data type: int32_t" << std::endl;
			    this->imgDataType = Image::ImageDataType::Signed | Image::ImageDataType::Bit_32;
    		}
    		if (bps == 64) {
                std::cout << "Internal data type: int64_t" << std::endl;
			    this->imgDataType = Image::ImageDataType::Signed | Image::ImageDataType::Bit_64;
    		}
    	} break;
    
    	case SAMPLEFORMAT_IEEEFP: {
    		if (bps == 32) {
                std::cout << "Internal data type: float" << std::endl;
			    this->imgDataType = Image::ImageDataType::Floating | Image::ImageDataType::Bit_32;
    		}
    		if (bps == 64) {
                std::cout << "Internal data type: double" << std::endl;
			    this->imgDataType = Image::ImageDataType::Floating | Image::ImageDataType::Bit_64;
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

Image::ImageDataType TIFFImage::getInternalDataType() const {
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

uint16_t TIFFImage::getValue(const glm::vec3& coord) const {
    if (this->tif) {
        TIFFSetDirectory(tif, static_cast<int>(std::floor(coord[2])));
        tdata_t buf;
        uint32 row; 
        buf = _TIFFmalloc(TIFFScanlineSize(tif));
        TIFFReadScanline(tif, buf, static_cast<int>(std::floor(coord[1])));
        //uint8_t res = static_cast<uint8_t*>(buf)[static_cast<int>(std::floor(coord[0]))];
        uint16_t res = getToLowPrecision(this->getInternalDataType(), buf, std::floor(coord[0])); 
        _TIFFfree(buf);
        return res;
    }
    return 0.;
}

//Function to cast and insert for the get slice
template <typename data_t>
void castToUintAndInsert(data_t * values, std::vector<uint16_t>& res, size_t size, int duplicate, int offset) {
    for(int i = 0; i < size; i+=offset) {
        for(int j = 0; j < duplicate; ++j)
            res.push_back(static_cast<std::uint16_t>(values[i])); 
    }
}

//Function to cast and insert for the get slice
void castToLowPrecision(Image::ImageDataType imgDataType, const tdata_t& buf, std::vector<uint16_t>& res, size_t size, int duplicate, int offset) {
    if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8)) {
        uint8_t * data = static_cast<std::uint8_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16)) {
        uint16_t * data = static_cast<std::uint16_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32)) {
        uint32_t * data = static_cast<std::uint32_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64)) {
        uint64_t * data = static_cast<std::uint64_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_8)) {
        int8_t * data = static_cast<std::int8_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_16)) {
        int16_t * data = static_cast<std::int16_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_32)) {
        int32_t * data = static_cast<std::int32_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_64)) {
        int64_t * data = static_cast<std::int64_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_32)) {
        float * data = static_cast<float*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_64)) {
        double * data = static_cast<double*>(buf);
        castToUintAndInsert(data, res, size, duplicate, offset);
    } 
}

void TIFFImage::getSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel, std::pair<int, int>  offsets) const {
    TIFFSetDirectory(this->tif, sliceIdx);
    uint32 imagelength;
    tdata_t buf;
    uint32 row;

    TIFFGetField(this->tif, TIFFTAG_IMAGELENGTH, &imagelength);
    buf = _TIFFmalloc(TIFFScanlineSize(this->tif));

    for (row = 0; row < imagelength; row+=offsets.second) {
        TIFFReadScanline(this->tif, buf, row);
        int imageSize = static_cast<int>(this->imgDimensions[0]);
        castToLowPrecision(this->getInternalDataType(), buf, result, imageSize, nbChannel, offsets.first);
    }
    _TIFFfree(buf);
}
