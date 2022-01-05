#include "../include/grid.hpp"
#include <chrono>

bool isPtInBB(const glm::vec3& p, const glm::vec3& bbmin, const glm::vec3& bbmax) {
    for(int i = 0; i < 3; ++i) {
        if(p[i] < bbmin[i] || p[i] > bbmax[i])
            return false;
    }
    return true;
}

Image::ImageDataType Grid::getInternalDataType() const {
    return this->image.getInternalDataType();
}

SimpleGrid::SimpleGrid(const std::string& filename, const glm::vec3& nbCube): grid(filename) {
    const glm::vec3 sizeCube = this->grid.gridDimensions / nbCube;
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

uint16_t SimpleGrid::getFullResolutionValueFromPoint(const glm::vec3& p) const {
    if(isPtInBB(p, this->tetmesh.bbMin, this->tetmesh.bbMax)) {
        return this->grid.getFullResolutionValue(p);
    } else {
        // Background value
        return 0;
    }
}

uint16_t SimpleGrid::getValueFromPoint(const glm::vec3& p) const {
    if(isPtInBB(p, this->tetmesh.bbMin, this->tetmesh.bbMax)) {
        return this->grid.getValue(p);
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
    glm::vec3 imageDimension = this->grid.getImageDimensions();// Directly get image dimension because we use full resolution

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
    return this->grid.gridDimensions;
}


Image::ImageDataType SimpleGrid::getInternalDataType() const {
    return this->grid.getInternalDataType();
}

void SimpleGrid::checkReadSlice() const {
    std::vector<uint16_t> res;
    this->grid.getGridSlice(0, res, 1);
    for(int i = 4213; i < 4500; ++i)
        std::cout << "[" << i << "]" << unsigned(res[i]) << std::endl;
    throw std::runtime_error("END OF UT");
}

/**************************/

template <typename data_t>
void castToUintAndInsert(data_t * values, std::vector<uint16_t>& res, size_t size, int duplicate, int offset) {
    for(int i = 0; i < size; i+=offset) {
        for(int j = 0; j < duplicate; ++j)
            res.push_back(static_cast<std::uint16_t>(values[i])); 
    }
}

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

Grid::Grid(const std::string& filename, glm::vec3 gridDimensions): image(TIFFImage(filename)), gridDimensions(gridDimensions) {
    this->voxelSizeRatio = this->image.imgDimensions / this->gridDimensions;
}

Grid::Grid(const std::string& filename): image(TIFFImage(filename)) {
    this->gridDimensions = this->image.imgDimensions / 4.f;
    this->voxelSizeRatio = this->image.imgDimensions / this->gridDimensions;
    // If we naïvely divide the image dimensions for lowered its resolution we have problem is the case of a dimension is 1
    // In that case the voxelSizeRatio is still 2.f for example, but the dimension is 0.5
    // It is a problem as we will iterate until dimension with sizeRatio as an offset
    for(int i = 0; i < 3; ++i) {
        if(this->gridDimensions[i] < 1.) {
            this->gridDimensions[i] = 1;
            this->voxelSizeRatio[i] = 1;
        }
        this->gridDimensions[i] = std::ceil(this->gridDimensions[i]);
    }
}

// This function do not use Grid::getValue as we do not want to open, copy and cast a whole image slice per value
// Thus 
void Grid::getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const {
    int offsetOnZ = static_cast<int>(std::floor(this->voxelSizeRatio[2]));
    if(sliceIdx % offsetOnZ != 0) {
        std::cout << "Error: wrong sliceIdx [" << sliceIdx << "] for n offset of [" << offsetOnZ << "]" << std::endl;
        throw std::runtime_error("Error: wrong slice Idx function getGridSlice");
    }
    if (this->image.tif) {
        TIFFSetDirectory(this->image.tif, sliceIdx);
        uint32 imagelength;
        tdata_t buf;
        uint32 row;
    
        TIFFGetField(this->image.tif, TIFFTAG_IMAGELENGTH, &imagelength);
        buf = _TIFFmalloc(TIFFScanlineSize(this->image.tif));

        int offset = static_cast<int>(std::floor(this->voxelSizeRatio[1]));
        for (row = 0; row < imagelength; row+=offset) {
            TIFFReadScanline(this->image.tif, buf, row);
            int imageSize = static_cast<int>(this->image.imgDimensions[0]);
            castToLowPrecision(this->getInternalDataType(), buf, result, imageSize, nbChannel, static_cast<int>(std::floor(this->voxelSizeRatio[0])));
        }
        _TIFFfree(buf);
    }
}

uint16_t Grid::getValue(const glm::vec3& coord) const {
    // Convert from grid coord to image coord
    const glm::vec3 coordImage = coord * this->voxelSizeRatio;
    return this->image.getValue(coordImage);
}

uint16_t Grid::getFullResolutionValue(const glm::vec3& coord) const {
    // No conversion, we want directly values from the full resolution image
    return this->image.getValue(coord);
}

glm::vec3 Grid::getImageDimensions() const {
    return this->image.imgDimensions;
}
