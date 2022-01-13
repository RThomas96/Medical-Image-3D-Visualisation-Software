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

SimpleGrid::SimpleGrid(const std::string& filename, const glm::vec3& nbCube, int subsample): grid(Sampler(std::vector<std::string>{filename}, subsample)) {
    const glm::vec3 sizeCube = this->grid.getSamplerDimension() / nbCube;
    this->tetmesh.buildGrid(nbCube, sizeCube, this->grid.subregionMin);
}

SimpleGrid::SimpleGrid(const std::vector<std::string>& filename, const glm::vec3& nbCube, int subsample): grid(Sampler(filename, subsample)) {
    const glm::vec3 sizeCube = this->grid.getSamplerDimension() / nbCube;
    this->tetmesh.buildGrid(nbCube, sizeCube, this->grid.subregionMin);
}

SimpleGrid::SimpleGrid(const std::vector<std::string>& filename, const glm::vec3& nbCube, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox): grid(Sampler(filename, subsample, bbox)) {
    const glm::vec3 sizeCube = this->grid.getSamplerDimension() / nbCube;
    this->tetmesh.buildGrid(nbCube, sizeCube, this->grid.subregionMin);
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

uint16_t SimpleGrid::getValueFromPoint(const glm::vec3& p, ResolutionMode resolutionMode) const {
    glm::vec3 pSamplerRes = p;
    // Even if we want to query a point a full resolution res, the bbox is still based on the sampler
    // So the bbox check need to be in sampler space
    if(resolutionMode == ResolutionMode::FULL_RESOLUTION) {
        pSamplerRes = p / this->grid.resolutionRatio;
    }
    if(isPtInBB(pSamplerRes, this->grid.bbMin, this->grid.bbMax)) {
        return this->grid.getValue(p, resolutionMode);
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

void SimpleGrid::writeDeformedGrid(const SimpleGrid& initial, ResolutionMode resolutionMode) {
    if(this->tetmesh.isEmpty())
        throw std::runtime_error("Error: cannot write a grid without deformed mesh.");

    if(initial.tetmesh.isEmpty())
        throw std::runtime_error("Error: cannot write a grid without initial mesh.");

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    glm::vec3 bboxMin = this->tetmesh.bbMin;
    glm::vec3 bboxMax = this->tetmesh.bbMax;

    for(int i = 0; i < 3; ++i) {
        bboxMin[i] = std::ceil(bboxMin[i]);
        bboxMax[i] = std::ceil(bboxMax[i]);
    }

    glm::vec3 voxelDimension = glm::vec3(1., 1., 1.);
    glm::vec3 imageDimension = bboxMax - bboxMin; 

    if(resolutionMode == ResolutionMode::FULL_RESOLUTION) {
        this->grid.fromSamplerToImage(imageDimension);

        // The save algorithm will generate points between bbmax and bbmin and save each of them after a deformation
        // Those generated points are in sampler space, as bboxMin and Max as well as the deformation computation provided by tetmesh are all in sampler space
        // Thus the saving at sampler resolution is simple
        // However, for saving at image resolution, we need to generate points in image space
        // We could convert bboxMin and max as well as all tetmesh points to the image space
        // Instead we keep our computation on sampler space but we change the offset between two points aka the voxelDimension
        // By reducing the voxelDimension we keep the same offset between two points as if we were in image space
        // At the end, we just have to convert the deformed point to the image space for the query
        this->grid.fromImageToSampler(voxelDimension);
    }

    std::cout << "Original image dimensions: " << initial.tetmesh.bbMax - initial.tetmesh.bbMin << std::endl;
    std::cout << "Image dimensions: " << imageDimension << std::endl;
    std::cout << "For " << bboxMin << " to " << bboxMax << " per " << voxelDimension << std::endl;
    TinyTIFFWriterFile * tif = TinyTIFFWriter_open("../../../../Data/data_debug/img2.tif", 16, TinyTIFFWriter_UInt, 1, imageDimension[0], imageDimension[1], TinyTIFFWriter_Greyscale);
    std::cout << "For " << bboxMin << " to " << bboxMax << " per " << voxelDimension << std::endl;

    std::vector<uint16_t> data;
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
                glm::vec3 pt2 = this->getCoordInInitial(initial, pt);

                if(resolutionMode == ResolutionMode::FULL_RESOLUTION)
                    this->grid.fromSamplerToImage(pt2);// Here we do a convertion because the final point is on image space 
                data.push_back(initial.getValueFromPoint(pt2, resolutionMode));// Here we stay at sampler resolution because bbox are aligned on the sampler
            }
        }
        TinyTIFFWriter_writeImage(tif, data.data());
    }
    TinyTIFFWriter_close(tif);

    std::cout << "Save sucessfull" << std::endl;
}

std::pair<glm::vec3, glm::vec3> SimpleGrid::getBoundingBox() const {
    return std::pair(this->tetmesh.bbMin, this->tetmesh.bbMax);
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

glm::vec3 SimpleGrid::getResolution() const {
    return this->grid.getSamplerDimension();
}

/**************************/

Sampler::Sampler(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox): image(TIFFImage(filename)) {
    glm::vec3 samplerResolution = this->image.imgResolution / static_cast<float>(subsample);
    this->resolutionRatio = this->image.imgResolution / samplerResolution;
    // If we naïvely divide the image dimensions for lowered its resolution we have problem is the case of a dimension is 1
    // In that case the voxelSizeRatio is still 2.f for example, but the dimension is 0.5
    // It is a problem as we will iterate until dimension with sizeRatio as an offset
    for(int i = 0; i < 3; ++i) {
        if(samplerResolution[i] < 1.) {
            samplerResolution[i] = 1;
            this->resolutionRatio[i] = 1;
        }
        samplerResolution[i] = std::ceil(samplerResolution[i]);
        this->resolutionRatio[i] = static_cast<int>(std::floor(this->resolutionRatio[i]));
    }

    this->bbMin = glm::vec3(0., 0., 0.);
    this->bbMax = samplerResolution;

    this->subregionMin = bbox.first;
    this->subregionMax = bbox.second;
}

Sampler::Sampler(const std::vector<std::string>& filename, int subsample): image(TIFFImage(filename)) {
    glm::vec3 samplerResolution = this->image.imgResolution / static_cast<float>(subsample);
    this->resolutionRatio = this->image.imgResolution / samplerResolution;
    // If we naïvely divide the image dimensions for lowered its resolution we have problem is the case of a dimension is 1
    // In that case the voxelSizeRatio is still 2.f for example, but the dimension is 0.5
    // It is a problem as we will iterate until dimension with sizeRatio as an offset
    for(int i = 0; i < 3; ++i) {
        if(samplerResolution[i] < 1.) {
            samplerResolution[i] = 1;
            this->resolutionRatio[i] = 1;
        }
        samplerResolution[i] = std::ceil(samplerResolution[i]);
        this->resolutionRatio[i] = static_cast<int>(std::floor(this->resolutionRatio[i]));
    }

    this->bbMin = glm::vec3(0., 0., 0.);
    this->bbMax = samplerResolution;

    this->subregionMin = this->bbMin;
    this->subregionMax = this->bbMax;
}

Sampler::Sampler(const std::vector<std::string>& filename): image(TIFFImage(filename)) {
    glm::vec3 samplerResolution = this->image.imgResolution; 
    this->resolutionRatio = glm::vec3(1., 1., 1.); 

    this->bbMin = glm::vec3(0., 0., 0.);
    this->bbMax = samplerResolution;

    this->subregionMin = this->bbMin;
    this->subregionMax = this->bbMax;
}

glm::vec3 Sampler::getSamplerDimension() const {
   return this->subregionMax - this->subregionMin; 
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

    std::pair<glm::vec3, glm::vec3> bboxes{this->subregionMin, this->subregionMax};
    this->fromSamplerToImage(bboxes.first);
    this->fromSamplerToImage(bboxes.second);

    for(int i = 0; i < 3; ++i) {
        if(static_cast<int>(bboxes.first[i]) % static_cast<int>(this->resolutionRatio[i]) != 0)
            throw std::runtime_error("Error in getGridSlice: bboxes not aligned with resolution ratio !");

        if(static_cast<int>(bboxes.second[i]) % static_cast<int>(this->resolutionRatio[i]) != 0)
            throw std::runtime_error("Error in getGridSlice: bboxes not aligned with resolution ratio !");
    }

    this->image.getSlice(sliceIdx, result, nbChannel, XYoffsets, bboxes);
}

uint16_t Sampler::getValue(const glm::vec3& coord, ResolutionMode resolutionMode) const {
    // Convert from grid coord to image coord
    if(resolutionMode == ResolutionMode::SAMPLER_RESOLUTION)
        return this->image.getValue(coord * this->resolutionRatio);
    else
        return this->image.getValue(coord);
}

glm::vec3 Sampler::getImageDimensions() const {
    return this->image.imgResolution;
}

void Sampler::fromSamplerToImage(glm::vec3& p) const {
    p = p * this->resolutionRatio;
}

void Sampler::fromImageToSampler(glm::vec3& p) const {
    p = p / this->resolutionRatio;
}
