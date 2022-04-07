#include "grid.hpp"
#include <chrono>
#include <algorithm>

bool isPtInBB(const glm::vec3& p, const glm::vec3& bbmin, const glm::vec3& bbmax) {
    for(int i = 0; i < 3; ++i) {
        if(p[i] < bbmin[i] || p[i] > bbmax[i])
            return false;
    }
    return true;
}

Image::ImageDataType Sampler::getInternalDataType() const {
    return this->image->getInternalDataType();
}

Grid::Grid(glm::vec3 gridSize): sampler(Sampler(gridSize)), toSamplerMatrix(glm::mat4(1.f)) {
}

Grid::Grid(const std::string& filename, int subsample): sampler(Sampler(std::vector<std::string>{filename}, subsample)), toSamplerMatrix(glm::mat4(1.f)) {
}

Grid::Grid(const std::vector<std::string>& filename, int subsample): sampler(Sampler(filename, subsample)), toSamplerMatrix(glm::mat4(1.f)) {
}

Grid::Grid(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox): sampler(Sampler(filename, subsample, bbox)), toSamplerMatrix(glm::mat4(1.f)) {
}

void Grid::buildTetmesh(const glm::vec3& nbCube) {
    const glm::vec3 sizeCube = this->sampler.getSamplerDimension() / nbCube;
    const glm::vec3& origin = this->sampler.subregionMin;
    this->buildTetmesh(nbCube, sizeCube, origin);
}

// Only this one is used
void Grid::buildTetmesh(const glm::vec3& nbCube, const glm::vec3& sizeVoxel) {
    //const glm::vec3 sizeCube = this->sampler.getSamplerDimension() / nbCube;
    const glm::vec3 sizeCube = (this->sampler.getSamplerDimension() * sizeVoxel) / nbCube;
    std::cout << "***" << std::endl;
    std::cout << "Build tetrahedral mesh grid..." << std::endl;
    std::cout << "Image dimension: " << this->sampler.getSamplerDimension() << std::endl;
    std::cout << "Nb of cubes: " << nbCube << std::endl;
    std::cout << "Cube size: " << sizeCube << std::endl;
    std::cout << "***" << std::endl;
    this->buildTetmesh(nbCube, sizeCube, glm::vec3(0., 0., 0.));
    // Here this initial mesh is built directly using the sampler dimension because we want it to match the sampler
    this->initialMesh.buildGrid(nbCube, this->sampler.getSamplerDimension() / nbCube, glm::vec3(0., 0., 0.));
}

void Grid::buildTetmesh(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin) {
    this->buildGrid(nbCube, sizeCube, origin);
}

uint16_t Grid::getValueFromWorldPoint(const glm::vec3& p, InterpolationMethod interpolationMethod, ResolutionMode resolutionMode) const {
    glm::vec3 pSampler = p;
    this->toSampler(pSampler);
    return this->getValueFromPoint(pSampler, interpolationMethod, resolutionMode);
}

uint16_t Grid::getValueFromPoint(const glm::vec3& p, InterpolationMethod interpolationMethod, ResolutionMode resolutionMode) const {
    glm::vec3 pSamplerRes = p;
    // Even if we want to query a point a full resolution res, the bbox is still based on the sampler
    // So the bbox check need to be in sampler space
    if(resolutionMode == ResolutionMode::FULL_RESOLUTION) {
        pSamplerRes = p / this->sampler.resolutionRatio;
    }
    if(isPtInBB(pSamplerRes, this->sampler.bbMin, this->sampler.bbMax)) {
        return this->sampler.getValue(p, interpolationMethod, resolutionMode);
    } else {
        // Background value
        return 0;
    }
}

uint16_t Grid::getDeformedValueFromPoint(const TetMesh& initial, const glm::vec3& p, InterpolationMethod interpolationMethod, ResolutionMode resolutionMode) const {
    glm::vec3 pt2 = this->getCoordInInitial(initial, p);
    if(resolutionMode == ResolutionMode::FULL_RESOLUTION)
        this->sampler.fromSamplerToImage(pt2);
    return this->getValueFromPoint(pt2, interpolationMethod, resolutionMode);
}

void Grid::writeDeformedGrid(ResolutionMode resolutionMode) {
    if(this->isEmpty())
        throw std::runtime_error("Error: cannot write a grid without deformed mesh.");

    if(this->initialMesh.isEmpty())
        throw std::runtime_error("Error: cannot write a grid without initialMesh mesh.");

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    this->updatebbox();
    glm::vec3 bboxMin = this->bbMin;
    glm::vec3 bboxMax = this->bbMax;

    for(int i = 0; i < 3; ++i) {
        bboxMin[i] = std::ceil(bboxMin[i]);
        bboxMax[i] = std::ceil(bboxMax[i]);
    }

    glm::vec3 voxelDimension = glm::vec3(1., 1., 1.);
    glm::vec3 imageDimension = bboxMax - bboxMin; 

    if(resolutionMode == ResolutionMode::FULL_RESOLUTION) {
        this->sampler.fromSamplerToImage(imageDimension);

        // The save algorithm will generate points between bbmax and bbmin and save each of them after a deformation
        // Those generated points are in sampler space, as bboxMin and Max as well as the deformation computation provided by tetmesh are all in sampler space
        // Thus the saving at sampler resolution is simple
        // However, for saving at image resolution, we need to generate points in image space
        // We could convert bboxMin and max as well as all tetmesh points to the image space
        // Instead we keep our computation on sampler space but we change the offset between two points aka the voxelDimension
        // By reducing the voxelDimension we keep the same offset between two points as if we were in image space
        // At the end, we just have to convert the deformed point to the image space for the query
        this->sampler.fromImageToSampler(voxelDimension);
    }

    std::cout << "Original image dimensions: " << this->initialMesh.bbMax - this->initialMesh.bbMin << std::endl;
    std::cout << "Image dimensions: " << imageDimension << std::endl;
    std::cout << "For " << bboxMin << " to " << bboxMax << " per " << voxelDimension << std::endl;
    TinyTIFFWriterFile * tif = TinyTIFFWriter_open("save_deformed_image.tif", 16, TinyTIFFWriter_UInt, 1, imageDimension[0], imageDimension[1], TinyTIFFWriter_Greyscale);
    std::cout << "For " << bboxMin << " to " << bboxMax << " per " << voxelDimension << std::endl;

    std::vector<uint16_t> data;
    data.reserve(imageDimension[0] * imageDimension[1]);
    for(float k = bboxMin[2]; k < bboxMax[2]; k+=voxelDimension[2]) {
        std::cout << std::endl;
        end = std::chrono::steady_clock::now();
        std::cout << "Loading: " << ((k-bboxMin[2])/(bboxMax[2]-bboxMin[2])) * 100. << "%" << std::endl;
        //std::cout << "Remain: "  << (((bboxMax[2]-bboxMin[2]) - (k-bboxMin[2])) * (std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() / (k-bboxMin[2])))/60. << "min" << std::endl << std::endl;
        data.clear();
        for(float j = bboxMin[1]; j < bboxMax[1]; j+=voxelDimension[1]) {
            //std::cout << (j/bboxMax[1]) * 100. << "% " << std::flush;
            for(float i = bboxMin[0]; i < bboxMax[0]; i+=voxelDimension[0]) {
                const glm::vec3 pt(i+voxelDimension[0]/2., j+voxelDimension[1]/2., k+voxelDimension[2]/2.);
                glm::vec3 pt2 = this->getCoordInInitial(this->initialMesh, pt);

                if(resolutionMode == ResolutionMode::FULL_RESOLUTION)
                    this->sampler.fromSamplerToImage(pt2);// Here we do a convertion because the final point is on image space 
                data.push_back(this->getValueFromPoint(pt2, InterpolationMethod::NearestNeighbor, resolutionMode));// Here we stay at sampler resolution because bbox are aligned on the sampler
            }
        }
        TinyTIFFWriter_writeImage(tif, data.data());
    }
    TinyTIFFWriter_close(tif);

    std::cout << "Save sucessfull" << std::endl;
}

std::pair<glm::vec3, glm::vec3> Grid::getBoundingBox() const {
    return std::pair(this->bbMin, this->bbMax);
}

bool Grid::getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos, glm::vec3& res) const {
    glm::vec3 nDirection = glm::normalize(direction);

    float maxDistance = glm::length(this->getOrigin() - origin)*2.;    
    float step = std::min(this->getVoxelSize()[0], std::min(this->getVoxelSize()[1], this->getVoxelSize()[2]));

    for(float i = 0; i < maxDistance; i+=step) {
        const glm::vec3 p = origin + i * nDirection;
        const uint16_t value = this->getDeformedValueFromPoint(this->initialMesh, p);
        if(value > minValue && value < maxValue && (p[0]>planePos[0]-0.01 && p[1]>planePos[1]-0.01 && p[2]>planePos[2]-0.01)) {
            res = p;
            return true;
        }
    }

    std::cout << "Warning: no point found" << std::endl;
    res = origin;
    return false;
}

glm::mat4 Grid::getTransformationMatrix() const {
    glm::mat4 transf(1.f);
    for(int i = 0; i < this->transformations.size(); ++i)
        transf *= this->transformations[i];
    return transf;
}

void Grid::translate(const glm::vec3& vec) {
    TetMesh::translate(vec);
    this->transformations.push_back(glm::translate(glm::mat4(1.f), vec));
    this->toSamplerMatrix = glm::inverse(this->getTransformationMatrix());
}

void Grid::rotate(const glm::mat3& transf) {
    TetMesh::rotate(transf);
    this->transformations.push_back(transf);
    this->toSamplerMatrix = glm::inverse(this->getTransformationMatrix());
}

void Grid::scale(const glm::vec3& scale) {
    TetMesh::scale(scale);
    this->transformations.push_back(glm::scale(glm::mat4(1.f), scale));
    this->toSamplerMatrix = glm::inverse(this->getTransformationMatrix());
}

void Grid::setOrigin(const glm::vec3& origin) {
    TetMesh::setOrigin(origin);
    this->toSamplerMatrix = glm::inverse(this->getTransformationMatrix());
}

void Grid::toSampler(glm::vec3& p) const {
    p = glm::vec3(this->toSamplerMatrix * glm::vec4(p[0], p[1], p[2], 1.));
}

glm::vec3 Grid::getVoxelSize() const {
    //return this->sampler.getSamplerDimension() / this->getResolution();
    return this->getDimensions() / this->sampler.getSamplerDimension();
}

void Grid::loadMESH(std::string const &filename) {
    TetMesh::loadMESH(filename);
    this->texCoord.clear();
    for(int i = 0; i < this->vertices.size(); ++i) {
        this->texCoord.push_back(this->vertices[i]/this->sampler.getSamplerDimension());
    }    
}

/**************************/

Sampler::Sampler(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox): image(new SimpleImage(filename)) {
    glm::vec3 samplerResolution = this->image->imgResolution / static_cast<float>(subsample);
    this->resolutionRatio = this->image->imgResolution / samplerResolution;
    // If we naïvely divide the image dimensions for lowered its resolution we have problem is the case of a dimension is 1
    // In that case the voxelSizeRatio is still 2.f for example, but the dimension is 0.5
    // It is a problem as we will iterate until dimension with sizeRatio as an offset
    for(int i = 0; i < 3; ++i) {
        samplerResolution[i] = std::floor(samplerResolution[i]);
        if(samplerResolution[i] <= 1.) {
            samplerResolution[i] = 1;
            this->resolutionRatio[i] = 1;
        }
        this->resolutionRatio[i] = static_cast<int>(std::floor(this->resolutionRatio[i]));
    }

    this->bbMin = glm::vec3(0., 0., 0.);
    this->bbMax = samplerResolution;

    this->subregionMin = bbox.first;
    this->subregionMax = bbox.second;

    // Cache management
    this->useCache = false;
    if(this->useCache) {
        this->cache = new Cache(this->getSamplerDimension());
        this->fillCache();
    }

    std::cout << "Sampler initialized..." << std::endl;
    std::cout << "Sampler resolution: " << this->getSamplerDimension() << std::endl;
    std::cout << "Subregion selected: " << this->subregionMin << " | " << this->subregionMax << std::endl;
}

Sampler::Sampler(const std::vector<std::string>& filename, int subsample): image(new SimpleImage(filename)) {
    glm::vec3 samplerResolution = this->image->imgResolution / static_cast<float>(subsample);
    this->resolutionRatio = this->image->imgResolution / samplerResolution;
    // If we naïvely divide the image dimensions for lowered its resolution we have problem is the case of a dimension is 1
    // In that case the voxelSizeRatio is still 2.f for example, but the dimension is 0.5
    // It is a problem as we will iterate until dimension with sizeRatio as an offset
    for(int i = 0; i < 3; ++i) {
        samplerResolution[i] = std::floor(samplerResolution[i]);
        if(samplerResolution[i] <= 1.) {
            samplerResolution[i] = 1;
            this->resolutionRatio[i] = 1;
        }
        this->resolutionRatio[i] = static_cast<int>(std::floor(this->resolutionRatio[i]));
    }

    this->bbMin = glm::vec3(0., 0., 0.);
    this->bbMax = samplerResolution;

    this->subregionMin = this->bbMin;
    this->subregionMax = this->bbMax;

    // Cache management
    this->useCache = false;
    if(this->useCache) {
        this->cache = new Cache(this->getSamplerDimension());
        this->fillCache();
    }

    std::cout << "Sampler initialized..." << std::endl;
    std::cout << "Sampler resolution: " << this->getSamplerDimension() << std::endl;
    std::cout << "Resolution ratio: " << this->resolutionRatio << std::endl;
    std::cout << "No subregion selected" << std::endl;
}

Sampler::Sampler(const std::vector<std::string>& filename): image(new SimpleImage(filename)) {
    glm::vec3 samplerResolution = this->image->imgResolution; 
    this->resolutionRatio = glm::vec3(1., 1., 1.); 

    this->bbMin = glm::vec3(0., 0., 0.);
    this->bbMax = samplerResolution;

    this->subregionMin = this->bbMin;
    this->subregionMax = this->bbMax;

    // Cache management
    this->useCache = false;
    if(this->useCache) {
        this->cache = new Cache(this->getSamplerDimension());
        this->fillCache();
    }
}

Sampler::Sampler(glm::vec3 size): image(nullptr) {
    glm::vec3 samplerResolution = size; 
    this->resolutionRatio = glm::vec3(1., 1., 1.); 

    this->bbMin = glm::vec3(0., 0., 0.);
    this->bbMax = samplerResolution;

    this->subregionMin = this->bbMin;
    this->subregionMax = this->bbMax;

    // Cache management
    this->useCache = false;
    if(this->useCache) {
        this->cache = new Cache(this->getSamplerDimension());
    }
    // We do not fill the cache as we do not have associated image
    //this->fillCache();
}


glm::vec3 Sampler::getSamplerDimension() const {
   return this->subregionMax - this->subregionMin; 
}

// This function do not use Grid::getValue as we do not want to open, copy and cast a whole image slice per value
void Sampler::getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const {
    if(!this->image) {
        std::cerr << "[4001] ERROR: Try to [getGridSlice()] on a grid without attached image" << std::endl;
    }

    int Zoffset = static_cast<int>(this->resolutionRatio[2]);
    sliceIdx *= Zoffset;// Because sliceIdx isn't in grid space
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

    this->image->getSlice(sliceIdx, result, nbChannel, XYoffsets, bboxes);
}

void Sampler::fillCache() {
    if(!this->image) {
        std::cerr << "[4001] ERROR: Try to [fillCache()] on a grid without attached image" << std::endl;
    }
    std::vector<uint16_t> slice;
    std::cout << "Filling the cache" << std::endl;
    for(int z = 0; z < this->getSamplerDimension()[2]; ++z) {
        slice.clear();
        this->getGridSlice(z, slice, 1);
        this->cache->storeImage(z, slice);
    }
}

uint16_t Sampler::getValue(const glm::vec3& coord, InterpolationMethod interpolationMethod, ResolutionMode resolutionMode) const {
    // Convert from grid coord to image coord
    if(resolutionMode == ResolutionMode::SAMPLER_RESOLUTION) {
        if(this->useCache) {
            return this->cache->getValue(coord, interpolationMethod);
        } else {
            return this->image->getValue(coord * this->resolutionRatio);
        }
    } else {
        if(!this->image) {
            std::cerr << "[4001] ERROR: Try to [getValue()] at [ResolutionMode::FULL_RESOLUTION] on a grid without attached image" << std::endl;
            return 0;
        }
        return this->image->getValue(coord);
    }
}

glm::vec3 Sampler::getImageDimensions() const {
    if(!this->image) {
        std::cerr << "[4001] ERROR: Try to [getImageDimensions()] on a grid without attached image" << std::endl;
        return this->getSamplerDimension();
    }
    return this->image->imgResolution;
}

void Sampler::fromSamplerToImage(glm::vec3& p) const {
    p = p * this->resolutionRatio;
}

void Sampler::fromImageToSampler(glm::vec3& p) const {
    p = p / this->resolutionRatio;
}

