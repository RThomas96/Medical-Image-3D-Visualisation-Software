#include "../include/grid.hpp"
#include <chrono>

bool isPtInBB(const glm::vec3& p, const glm::vec3& bbmin, const glm::vec3& bbmax) {
    for(int i = 0; i < 3; ++i) {
        if(p[i] < bbmin[i] || p[i] > bbmax[i])
            return false;
    }
    return true;
}

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

template <typename data_t>
void castToUintAndInsert(data_t * values, std::vector<uint16_t>& res, size_t size, int duplicate) {
    for(int i = 0; i < size; ++i) {
        for(int j = 0; j < duplicate; ++j)
            res.push_back(static_cast<std::uint16_t>(values[i])); 
    }
}

void castToLowPrecision(Image::ImageDataType imgDataType, const tdata_t& buf, std::vector<uint16_t>& res, size_t size, int duplicate) {
    if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8)) {
        uint8_t * data = static_cast<std::uint8_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16)) {
        uint16_t * data = static_cast<std::uint16_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32)) {
        uint32_t * data = static_cast<std::uint32_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64)) {
        uint64_t * data = static_cast<std::uint64_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_8)) {
        int8_t * data = static_cast<std::int8_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_16)) {
        int16_t * data = static_cast<std::int16_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_32)) {
        int32_t * data = static_cast<std::int32_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_64)) {
        int64_t * data = static_cast<std::int64_t*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_32)) {
        float * data = static_cast<float*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_64)) {
        double * data = static_cast<double*>(buf);
        castToUintAndInsert(data, res, size, duplicate);
    } 
}

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

// TODO: nbChannel is bugged for now
void TIFFImage::getSlice(int imgIdx, int lineIdx, std::vector<uint16_t>& result, int nbChannel) const {
    if (this->tif) {
        TIFFSetDirectory(tif, imgIdx);
        tdata_t buf;
        uint32 row = static_cast<uint32>(lineIdx); 
        buf = _TIFFmalloc(TIFFScanlineSize(tif));
        TIFFReadScanline(tif, buf, row);
        uint8_t * res = static_cast<uint8_t*>(buf);
        //uint16_t * res = static_cast<uint16_t*>(buf);
        int sliceSize = static_cast<int>(this->imgDimensions[0]);
        castToLowPrecision(this->getInternalDataType(), buf, result, sliceSize, nbChannel);
        _TIFFfree(buf);
    }
}

void TIFFImage::getImage(int imgIdx, std::vector<std::uint16_t>& result, int nbChannel) const {
    if (this->tif) {
        TIFFSetDirectory(tif, imgIdx);
        uint32 imagelength;
        tdata_t buf;
        uint32 row;
    
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imagelength);
        buf = _TIFFmalloc(TIFFScanlineSize(tif));
        for (row = 0; row < imagelength; row++) {
            TIFFReadScanline(tif, buf, row);
            int imageSize = static_cast<int>(this->imgDimensions[0]);
            castToLowPrecision(this->getInternalDataType(), buf, result, imageSize, nbChannel);
        }
        _TIFFfree(buf);
    }
}

SimpleGrid::SimpleGrid(const std::string& filename, const glm::vec3& nbCube): grid(TIFFImage(filename)) {
    const glm::vec3 sizeCube = this->grid.imgDimensions / nbCube;
    this->tetmesh.buildGrid(nbCube, sizeCube, glm::vec3(0., 0., 0.));
}

glm::vec3 SimpleGrid::getCoordInInitial(const SimpleGrid& initial, glm::vec3 p) {
    int tetraIdx = this->tetmesh.inTetraIdx(p);
    if(tetraIdx != -1) {
        glm::vec4 baryCoordInDeformed = this->tetmesh.getTetra(tetraIdx).computeBaryCoord(p);
        glm::vec3 coordInInitial = initial.tetmesh.getTetra(tetraIdx).baryToWorldCoord(baryCoordInDeformed);
        return coordInInitial;
    } else {
        return p;
    }
}

uint16_t SimpleGrid::getValueFromPoint(const glm::vec3& p) const {
    if(isPtInBB(p, this->tetmesh.bbMin, this->tetmesh.bbMax)) {
        const glm::vec3 coord = p;
        return this->grid.getValue(coord);
    } else {
        // Background value
        return 0;
    }
}

void SimpleGrid::movePoint(const glm::vec3& indices, const glm::vec3& position) {
    this->tetmesh.movePoint(indices, position);
}

void SimpleGrid::replaceAllPoints(const std::vector<glm::vec3>& pts) {
    this->tetmesh.replaceAllPoints(pts);
}

void SimpleGrid::writeDeformedGrid(const SimpleGrid& initial) {
    if(this->tetmesh.isEmpty())
        throw std::runtime_error("Error: cannot write a grid without deformed mesh.");

    if(initial.tetmesh.isEmpty())
        throw std::runtime_error("Error: cannot write a grid without initial mesh.");

    // First we update the image size by adding some pixels
    glm::vec3 bboxMin = this->tetmesh.bbMin;
    glm::vec3 bboxMax = this->tetmesh.bbMax;

    glm::vec3 worldDimension = bboxMax - bboxMin;
    glm::vec3 imageDimension = this->grid.imgDimensions;

    glm::vec3 voxelDimension = worldDimension / imageDimension;

    glm::vec3 initialWorldDimension = initial.tetmesh.bbMax - initial.tetmesh.bbMin;
    glm::vec3 initialVoxelDimension = initialWorldDimension / imageDimension;

    glm::vec3 added = worldDimension - initialWorldDimension;
    added /= initialVoxelDimension;

    std::cout << "InitDim: " << initialWorldDimension << std::endl;
    std::cout << "DeformDim: " << worldDimension << std::endl;
    std::cout << "Added: " << added << std::endl;

    // TODO: CHECK WHEN ADDED HAVE POSITIV VALUES
    imageDimension[0] += std::ceil(added[0]);
    imageDimension[1] += std::ceil(added[1]);
    imageDimension[2] += std::ceil(added[2]);

    voxelDimension = worldDimension / imageDimension;
    std::cout << "For " << bboxMin << " to " << bboxMax << " per " << voxelDimension << std::endl;

    TinyTIFFWriterFile * tif = TinyTIFFWriter_open("../../../../Data/data_debug/img2.tif", 8, TinyTIFFWriter_UInt, 1, imageDimension[0], imageDimension[1], TinyTIFFWriter_Greyscale);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::vector<uint8_t> data;
    data.reserve(imageDimension[0] * imageDimension[1]);
    for(float k = bboxMin[2]; k < bboxMax[2]; k+=voxelDimension[2]) {
        std::cout << std::endl;
        end = std::chrono::steady_clock::now();
        std::cout << "Loading: " << ((k-bboxMin[2])/(bboxMax[2]-bboxMin[2])) * 100. << "%" << std::endl;
        std::cout << "Remain: "  << (((bboxMax[2]-bboxMin[2]) - (k-bboxMin[2])) * (std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() / (k-bboxMin[2])))/60. << "min" << std::endl << std::endl;
        data.clear();
        for(float j = bboxMin[1]; j < bboxMax[1]; j+=voxelDimension[1]) {
            //std::cout << (j/bboxMax[1]) * 100. << "% " << std::flush;
            for(float i = bboxMin[0]; i < bboxMax[0]; i+=voxelDimension[0]) {
                const glm::vec3 pt(i+voxelDimension[0]/2., j+voxelDimension[1]/2., k+voxelDimension[2]/2.);
                const glm::vec3 pt2 = this->getCoordInInitial(initial, pt);
                data.push_back(initial.getValueFromPoint(pt2));
            }
        }
        TinyTIFFWriter_writeImage(tif, data.data());
    }
    TinyTIFFWriter_close(tif);
}

std::pair<glm::vec3, glm::vec3> SimpleGrid::getBoundingBox() const {
    return std::pair(this->tetmesh.bbMin, this->tetmesh.bbMax);
}

glm::vec3 SimpleGrid::getResolution() const {
    return this->grid.imgDimensions;
}


Image::ImageDataType SimpleGrid::getInternalDataType() const {
    return this->grid.getInternalDataType();
}

void SimpleGrid::checkReadSlice() const {
    std::vector<uint16_t> res;
    this->grid.getImage(0, res, 1);
    for(int i = 4213; i < 4500; ++i)
        std::cout << "[" << i << "]" << unsigned(res[i]) << std::endl;
    throw std::runtime_error("END OF UT");
}

