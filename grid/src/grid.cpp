#include "../include/grid.hpp"

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

uint8_t TIFFImage::getValue(const glm::vec3& coord) const {
    if (this->tif) {
        TIFFSetDirectory(tif, static_cast<int>(std::floor(coord[2])));
        tdata_t buf;
        uint32 row; 
        buf = _TIFFmalloc(TIFFScanlineSize(tif));
        TIFFReadScanline(tif, buf, static_cast<int>(std::floor(coord[1])));
        uint8_t res = static_cast<uint8_t*>(buf)[static_cast<int>(std::floor(coord[0]))];
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
        for(int i = 0; i < sliceSize; ++i) {
           result.push_back(static_cast<uint16_t>(res[i])); 
        }
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
            //std::uint16_t * res = static_cast<std::uint16_t*>(buf);
            std::uint8_t * res = static_cast<std::uint8_t*>(buf);
            int imageSize = static_cast<int>(this->imgDimensions[0]);
            for(int i = 0; i < imageSize; ++i) {
                for(int j = 0; j < nbChannel; ++j)
                    result.push_back(static_cast<std::uint16_t>(res[i])); 
            }
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

uint8_t SimpleGrid::getValueFromPoint(const glm::vec3& p) const {
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
        std::cout << "Loading: " << (k/bboxMax[2]) * 100. << "%" << std::endl;
        std::cout << "Remain: "  << ((bboxMax[2] - k) * (std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() / k))/60. << "min" << std::endl << std::endl;
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

/**************************/
// UNIT TEST
/**************************/

void SimpleGrid::checkReadSlice() const {
    std::vector<uint16_t> res;
    this->grid.getImage(0, res, 1);
    for(int i = 4213; i < 4500; ++i)
        std::cout << "[" << i << "]" << unsigned(res[i]) << std::endl;
    throw std::runtime_error("END OF UT");
}

void checkPointQuery() {
    glm::vec3 origin = glm::vec3(0., 0., 0.);
    glm::vec3 size = glm::vec3(4930., 512., 51.);
    glm::vec3 nb = glm::vec3(1., 1., 1.);
    SimpleGrid grid("../../../../../../../data/datasets/tulane/v3/registration_subset/v3_a5_100_150_8bit_normalized_25.tif", nb);

    glm::vec3 originalPosition = glm::vec3(1194., 20., 4.);

    std::cout << "Point at 1194. 20. 4.: " << unsigned(grid.getValueFromPoint(originalPosition))  << " == 112" << std::endl;

    origin = glm::vec3(2., 0., 0.);
    size = glm::vec3(2465., 256., 25.);
    nb = glm::vec3(1., 1., 1.);
    SimpleGrid grid2("../../../../../../../data/datasets/tulane/v3/registration_subset/v3_a5_100_150_8bit_normalized_25.tif", nb);

    //std::cout << "Grid voxel size: " << grid2.voxelDimensions << " == (0.5, 0.5, 0.5)" << std::endl;
    glm::vec3 newPosition = (originalPosition/glm::vec3(2., 2., 2.))+origin;
    std::cout << "Same point: " << unsigned(grid2.getValueFromPoint(newPosition)) << std::endl;
}

void checkMeshMove() {
    // TODO: move this test to tetmesh scope
    //glm::vec3 origin = glm::vec3(0., 0., 0.);
    //glm::vec3 size = glm::vec3(1., 1., 1.);
    //glm::vec3 nb = glm::vec3(10., 10., 1.);
    //SimpleGrid grid(origin, size, nb);
    ////grid.tetmesh.ptGrid[5][5][0][2] = -1;
    //grid.tetmesh.ptGrid[1][0][1][2] = -1;

    //for(int i = 0; i < grid.tetmesh.mesh.size(); ++i) {
    //    for(int j = 0; j < 4; ++j) {
    //        //std::cout << (*(grid.tetmesh.mesh[i].points[j]))[2] << std::endl;
    //        if((*(grid.tetmesh.mesh[i].points[j]))[2] < 0) {
    //            std::cout << "Modified: " << i << " - " << j << std::endl;
    //        }
    //    }
    //}
}

void checkDeformable() {
    // For same ratio
    glm::vec3 origin = glm::vec3(0., 0., 0.);
    glm::vec3 size = glm::vec3(940, 510, 20);
    //std::cout << "New size: " << size << std::endl;
    //std::cout << "Old size: " << size * glm::length(glm::vec3(940, 510, 20)) << std::endl;
    glm::vec3 nb = glm::vec3(1., 1., 1.);

    std::string filename = "../../../../Data/myTiff.tif";
    SimpleGrid deformableGrid(filename, nb);
    SimpleGrid initialGrid(filename, nb);

    deformableGrid.movePoint(glm::vec3(1, 1, 1), glm::vec3(600, 0., 0.));
    deformableGrid.writeDeformedGrid(initialGrid);
}

void checkReadSimpleImage () {
    //"../../../../Data/v3_a5_100_150_8bit_normalized.tif"
    //"../../../../Data/myTiff.tif"
    //"../../../../../../../data/datasets/tulane/v3/registration_subset/v3_a5_100_150_8bit_normalized_25.tif"
    //"../../../../../../../data/datasets/tulane/v3/registration_subset/v3_a5_100_150_8bit_normalized.tif"

    TIFFImage gridTiff("../../../../Data/v3_a5_100_150_8bit_normalized_25.tif");
    std::cout << "Get simple value: " << unsigned(gridTiff.getValue(glm::vec3(1182, 8, 0))) << " == 181" << std::endl;

    TIFFImage gridTiffFiji("../../../../Data/myTiff.tif");
    std::cout << "Get simple value: " << unsigned(gridTiffFiji.getValue(glm::vec3(50, 29, 0))) << " == 162" << std::endl;
}
