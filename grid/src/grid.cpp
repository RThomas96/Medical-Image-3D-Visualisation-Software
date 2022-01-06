#include "../include/grid.hpp"
#include <chrono>

bool isPtInBB(const glm::vec3& p, const glm::vec3& bbmin, const glm::vec3& bbmax) {
    for(int i = 0; i < 3; ++i) {
        if(p[i] < bbmin[i] || p[i] > bbmax[i])
            return false;
    }
    return true;
}

Image::ImageDataType Sampler::getInternalDataType() const {
    return this->image.getInternalDataType();
}

SimpleGrid::SimpleGrid(const std::string& filename, const glm::vec3& nbCube): grid(Sampler(filename, 4)) {
    const glm::vec3 sizeCube = this->grid.samplerResolution / nbCube;
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
                // It depend if we want full res or no
                //data.push_back(initial.getValueFromPoint(pt2));
                data.push_back(initial.getFullResolutionValueFromPoint(pt2));
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
    return this->grid.samplerResolution;
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

Sampler::Sampler(const std::string& filename, int subsample): image(TIFFImage(filename)) {
    this->samplerResolution = this->image.imgResolution / static_cast<float>(subsample);
    this->resolutionRatio = this->image.imgResolution / this->samplerResolution;
    // If we naïvely divide the image dimensions for lowered its resolution we have problem is the case of a dimension is 1
    // In that case the voxelSizeRatio is still 2.f for example, but the dimension is 0.5
    // It is a problem as we will iterate until dimension with sizeRatio as an offset
    for(int i = 0; i < 3; ++i) {
        if(this->samplerResolution[i] < 1.) {
            this->samplerResolution[i] = 1;
            this->resolutionRatio[i] = 1;
        }
        this->samplerResolution[i] = std::ceil(this->samplerResolution[i]);
        this->resolutionRatio[i] = static_cast<int>(std::floor(this->resolutionRatio[i]));
    }
}

Sampler::Sampler(const std::string& filename): image(TIFFImage(filename)) {
    this->samplerResolution = this->image.imgResolution; 
    this->resolutionRatio = glm::vec3(1., 1., 1.); 
}

// This function do not use Grid::getValue as we do not want to open, copy and cast a whole image slice per value
void Sampler::getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const {

    int Zoffset = static_cast<int>(this->resolutionRatio[2]);
    if(sliceIdx % Zoffset != 0) {
        std::cout << "Error: wrong sliceIdx [" << sliceIdx << "] for n offset of [" << Zoffset << "]" << std::endl;
        throw std::runtime_error("Error: wrong slice Idx function getGridSlice");
    }

    std::pair<int, int> XYoffsets;
    XYoffsets.first = static_cast<int>(this->resolutionRatio[0]);
    XYoffsets.second = static_cast<int>(this->resolutionRatio[1]);

    this->image.getSlice(sliceIdx, result, nbChannel, XYoffsets);
}

uint16_t Sampler::getValue(const glm::vec3& coord) const {
    // Convert from grid coord to image coord
    const glm::vec3 coordImage = coord * this->resolutionRatio;
    return this->image.getValue(coordImage);
}

uint16_t Sampler::getFullResolutionValue(const glm::vec3& coord) const {
    // No conversion, we want directly values from the full resolution image
    return this->image.getValue(coord);
}

glm::vec3 Sampler::getImageDimensions() const {
    return this->image.imgResolution;
}
