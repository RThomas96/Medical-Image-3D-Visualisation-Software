#include "grid.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include <chrono>
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <vector>

#define USE_CACHE true

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

Grid::Grid(const std::vector<std::string>& filename, int subsample, const glm::vec3& sizeVoxel, const glm::vec3& nbCubeGridTransferMesh): sampler(Sampler(filename, subsample, sizeVoxel)), DrawableGrid(this) {
    this->buildTetmesh(nbCubeGridTransferMesh);
    this->history = new History(this->vertices, this->coordinate_system);
}

Grid::Grid(const std::vector<std::string>& filename, int subsample, const glm::vec3& sizeVoxel, const std::string& fileNameTransferMesh): sampler(Sampler(filename, subsample, sizeVoxel)), DrawableGrid(this) {
    this->loadMESH(fileNameTransferMesh);
}

// Only this one is used
void Grid::buildTetmesh(const glm::vec3& nbCube) {
    const glm::vec3 sizeCube = (this->sampler.getDimension() * this->getVoxelSize()) / nbCube;
    std::cout << "***" << std::endl;
    std::cout << "Build tetrahedral mesh grid..." << std::endl;
    std::cout << "Image dimension: " << this->sampler.getDimension() << std::endl;
    std::cout << "Nb of cubes: " << nbCube << std::endl;
    std::cout << "Cube size: " << sizeCube << std::endl;
    std::cout << "***" << std::endl;
    this->buildGrid(nbCube, sizeCube, glm::vec3(0., 0., 0.));
    // Here this initial mesh is built directly using the sampler dimension because we want it to match the sampler
    this->initialMesh.buildGrid(nbCube, this->sampler.getDimension() / nbCube, glm::vec3(0., 0., 0.));
}

uint16_t Grid::getValueFromPoint(const glm::vec3& p, Interpolation::Method interpolationMethod) const {
    glm::vec3 pSamplerRes = p;
    // Even if we want to query a point a full resolution res, the bbox is still based on the sampler
    // So the bbox check need to be in sampler space
    if(isPtInBB(pSamplerRes, this->sampler.bbMin, this->sampler.bbMax)) {
        return this->sampler.getValue(p, interpolationMethod);
    } else {
        // Background value
        return 0;
    }
}

uint16_t Grid::getDeformedValueFromPoint(const TetMesh& initial, const glm::vec3& p, Interpolation::Method interpolationMethod) const {
    glm::vec3 pt2(0., 0., 0.);
    bool ptIsInInitial = this->getCoordInInitial(initial, p, pt2);
    if(!ptIsInInitial)
        return 0.;
    return this->getValueFromPoint(pt2, interpolationMethod);
}

std::pair<glm::vec3, glm::vec3> Grid::getBoundingBox() const {
    //return std::pair(this->sampler.bbMin, this->sampler.bbMax);
    return std::pair(this->bbMin, this->bbMax);
}

bool Grid::getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos, glm::vec3& res) const {
    glm::vec3 nDirection = glm::normalize(direction);

    float maxDistance = glm::length(this->getOrigin() - origin)*2.;    
    float step = std::min(this->getVoxelSize()[0], std::min(this->getVoxelSize()[1], this->getVoxelSize()[2]));

    for(float i = 0; i < maxDistance; i+=step) {
        const glm::vec3 p = origin + i * nDirection;
        const uint16_t value = this->getDeformedValueFromPoint(this->initialMesh, p);
        //if(value > minValue && value < maxValue && (p[0]>planePos[0]-0.01 && p[1]>planePos[1]-0.01 && p[2]>planePos[2]-0.01)) {
        if(visibilityMap[value] && (p[0]>planePos[0]-0.01 && p[1]>planePos[1]-0.01 && p[2]>planePos[2]-0.01)) {
            res = p;
            return true;
        }
    }

    std::cout << "Warning: no point found" << std::endl;
    res = origin;
    return false;
}

glm::vec3 Grid::getVoxelSize() const {
    return this->sampler.getVoxelSize();
}

void Grid::loadMESH(std::string const &filename) {
    TetMesh::loadMESH(filename);
    this->initialMesh.loadMESH(filename);
    this->texCoord.clear();
    for(int i = 0; i < this->vertices.size(); ++i) {
        this->texCoord.push_back((this->vertices[i]/this->sampler.resolutionRatio)/this->sampler.getDimension());
    }    
}

void Grid::sampleSliceGridValues(const glm::vec3& slice, const std::pair<glm::vec3, glm::vec3>& areaToSample, const glm::vec3& imgSize, std::vector<uint16_t>& result, Interpolation::Method interpolationMethod) {
    auto start = std::chrono::steady_clock::now();

    auto convert = [&](glm::vec3& p) {
        if(slice.y == -1 && slice.z == -1)
            std::swap(p.x, p.z);
        if(slice.x == -1 && slice.z == -1)
            std::swap(p.y, p.z);
        if(slice.x == -1 && slice.y == -1)
            std::swap(p.z, p.z);
    };

    omp_set_nested(true);

    // Space to sample
    glm::vec3 bbMinScene = areaToSample.first;
    bbMinScene += glm::vec3(0.0001, 0.0001, 0.0001);
    glm::vec3 bbMaxScene = areaToSample.second;
    glm::vec3 sceneSize = (areaToSample.second - areaToSample.first);
    convert(sceneSize);
    glm::vec3 voxelSizeConvert = sceneSize / glm::vec3(imgSize);
    std::cout << "Img size: " << imgSize << std::endl;
    std::cout << "Scene size: " << sceneSize << std::endl;
    std::cout << "Voxel size: " << voxelSizeConvert << std::endl;

    auto isInScene = [&](glm::vec3& p) {
        return (p.x > bbMinScene.x && p.y > bbMinScene.y && p.z > bbMinScene.z && p.x < bbMaxScene.x && p.y < bbMaxScene.y && p.z < bbMaxScene.z);
    };

    auto fromWorldToImage = [&](glm::vec3& p) {
        p -= bbMinScene;
        convert(p);
        p /= voxelSizeConvert;
    };

    auto fromImageToWorld = [&](glm::vec3& p) {
        p *= voxelSizeConvert;
        convert(p);
        p += bbMinScene;
    };

    result.clear();
    result.resize(imgSize[0] * imgSize[1], 0);

    #pragma omp parallel for schedule(dynamic)
    for(int tetIdx = 0; tetIdx < this->mesh.size(); ++tetIdx) {
        const Tetrahedron& tet = this->mesh[tetIdx];
        glm::vec3 bbMin = tet.getBBMin();
        fromWorldToImage(bbMin);
        bbMin.x = std::ceil(bbMin.x) - 1;
        bbMin.y = std::ceil(bbMin.y) - 1;
        bbMin.z = std::ceil(bbMin.z) - 1;
        glm::vec3 bbMax = tet.getBBMax();
        fromWorldToImage(bbMax);
        bbMax.x = std::floor(bbMax.x) + 1;
        bbMax.y = std::floor(bbMax.y) + 1;
        bbMax.z = std::floor(bbMax.z) + 1;
        if(slice.y == -1 && slice.z == -1) {
            bbMin.z = slice.x;
            bbMax.z = slice.x+1;
        }
        if(slice.x == -1 && slice.y == -1) {
            bbMin.z = slice.z;
            bbMax.z = slice.z+1;
        }
        if(slice.x == -1 && slice.z == -1) {
            bbMin.z = slice.y;
            bbMax.z = slice.y+1;
        }
        for(int k = bbMin.z; k < int(bbMax.z); ++k) {
            for(int j = bbMin.y; j < int(bbMax.y); ++j) {
                for(int i = bbMin.x; i < int(bbMax.x); ++i) {
                    int insertIdx = i + j*imgSize[0];

                    glm::vec3 p(i, j, k);
                    p += glm::vec3(.5, .5, .5);
                    fromImageToWorld(p);

                    if(isInScene(p) && tet.isInTetrahedron(p)) {
                        if(this->getCoordInInitial(this->initialMesh, p, p, tetIdx)) {
                            result[insertIdx] = this->sampler.getValue(p, interpolationMethod);
                        }
                    }
                }
            }
        }
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Duration time: " << elapsed_seconds.count() << "s / " << elapsed_seconds.count()/60. << "m" << std::endl;
}

void Grid::sampleGridValues(const std::pair<glm::vec3, glm::vec3>& areaToSample, const glm::vec3& resolution, std::vector<std::vector<uint16_t>>& result, Interpolation::Method interpolationMethod) {
    auto start = std::chrono::steady_clock::now();

    omp_set_nested(true);

    // Space to sample
    glm::vec3 bbMinScene = areaToSample.first;
    glm::vec3 bbMaxScene = areaToSample.second;
    //glm::vec3 fromSamplerToSceneRatio = (bbMaxScene - bbMinScene) / resolution;
    glm::vec3 fromSamplerToSceneRatio = glm::vec3(1., 1., 1.);

    auto isInScene = [&](glm::vec3& p) {
        return (p.x > bbMinScene.x && p.y > bbMinScene.y && p.z > bbMinScene.z && p.x < bbMaxScene.x && p.y < bbMaxScene.y && p.z < bbMaxScene.z);
    };

    auto fromWorldToImage = [&](glm::vec3& p) {
        p -= bbMinScene;
        p /= fromSamplerToSceneRatio;
    };

    auto fromImageToWorld = [&](glm::vec3& p) {
        p *= fromSamplerToSceneRatio;
        p += bbMinScene;
    };

    result.clear();
    result.resize(resolution[2]);
    for(int i = 0; i < result.size(); ++i) {
        result[i].resize(resolution[0] * resolution[1]);
        std::fill(result[i].begin(), result[i].end(), 0);
    }

    int printOcc = 10;
    printOcc = this->mesh.size()/printOcc;

    //#pragma omp parallel for schedule(dynamic) num_threads(fromGrid->mesh.size()/10)
    #pragma omp parallel for schedule(dynamic)
    for(int tetIdx = 0; tetIdx < this->mesh.size(); ++tetIdx) {
        const Tetrahedron& tet = this->mesh[tetIdx];
        glm::vec3 bbMin = tet.getBBMin();
        fromWorldToImage(bbMin);
        bbMin.x = std::ceil(bbMin.x) - 1;
        bbMin.y = std::ceil(bbMin.y) - 1;
        bbMin.z = std::ceil(bbMin.z) - 1;
        glm::vec3 bbMax = tet.getBBMax();
        fromWorldToImage(bbMax);
        bbMax.x = std::floor(bbMax.x) + 1;
        bbMax.y = std::floor(bbMax.y) + 1;
        bbMax.z = std::floor(bbMax.z) + 1;
        //if((tetIdx%printOcc) == 0) {
        //    std::cout << "Loading: " << (float(tetIdx)/float(this->mesh.size())) * 100. << "%" << std::endl;
        //}
        for(int k = bbMin.z; k < int(bbMax.z); ++k) {
            for(int j = bbMin.y; j < int(bbMax.y); ++j) {
                for(int i = bbMin.x; i < int(bbMax.x); ++i) {
                    glm::vec3 p(i, j, k);
                    p += glm::vec3(.5, .5, .5);

                    fromImageToWorld(p);

                    if(isInScene(p) && tet.isInTetrahedron(p)) {
                        if(this->getCoordInInitial(this->initialMesh, p, p, tetIdx)) {

                            int insertIdx = i + k*resolution[0];

                            this->sampler.fromSamplerToImage(p);
                            result[j][insertIdx] = this->getValueFromPoint(p, interpolationMethod);
                        } 
                    }
                }
            }
        }
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Duration time: " << elapsed_seconds.count() << "s / " << elapsed_seconds.count()/60. << "m" << std::endl;
}

bool Grid::checkTransferMeshValidity() {
    glm::vec3 meshDimension = this->getDimensions();
    glm::vec3 imageDimension = this->sampler.getDimension();
    if(glm::abs(glm::length(meshDimension) - glm::length(imageDimension)) < glm::length(meshDimension)/10.)
        return true;
    return false;
}

/**************************/

Sampler::Sampler(const std::vector<std::string>& filename, int subsample, const glm::vec3& voxelSize): image(new tiff_image(filename)) {
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

    // Cache management
    this->useCache = USE_CACHE;
    if(this->useCache) {
        this->cache = new Cache(this->getDimension());
        this->fillCache();
    }

    bool useOriginalVoxelSize = true;
    if(voxelSize != glm::vec3(0., 0., 0.)) {
        this->voxelSize = voxelSize;
        useOriginalVoxelSize = false;
    } else {
        this->voxelSize = this->image->voxelSize;
    }

    std::cout << "Sampler initialized..." << std::endl;
    std::cout << "Sampler resolution: " << this->getDimension() << std::endl;
    std::cout << "Resolution ratio: " << this->resolutionRatio << std::endl;
    std::cout << "Voxel size: " << this->voxelSize;
    if(useOriginalVoxelSize)
        std::cout << " (original)" << std::endl; 
    else
        std::cout << " (manual)" << std::endl; 
}

glm::vec3 Sampler::getVoxelSize() const {
   return this->voxelSize;
}

glm::vec3 Sampler::getDimension() const {
   return this->bbMax - this->bbMin;
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

    std::pair<glm::vec3, glm::vec3> bboxes{this->bbMin, this->bbMax};
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
    for(int z = 0; z < this->getDimension()[2]; ++z) {
        slice.clear();
        this->getGridSlice(z, slice, 1);
        this->cache->storeImage(z, slice);
    }
}

uint16_t Sampler::getValue(const glm::vec3& coord, Interpolation::Method interpolationMethod) const {
    if(this->useCache) {
        return this->cache->getValue(coord, interpolationMethod);
    } else {
        return this->image->getValue(coord * this->resolutionRatio);
    }
}

void Sampler::fromSamplerToImage(glm::vec3& p) const {
    p = p * this->resolutionRatio;
}

void Sampler::fromImageToSampler(glm::vec3& p) const {
    p = p / this->resolutionRatio;
}

std::vector<int> Sampler::getHistogram() const {
   if(useCache)  {
       uint16_t minValue = this->image->minValue;
       uint16_t maxValue = this->image->maxValue;
       const CImg img = this->cache->img.get_histogram((maxValue - minValue)+1, minValue, maxValue);
       std::vector<int> result;
       for(int i = 0; i < minValue; ++i) {
           result.push_back(0);
       }
       for(auto value : img) {
           result.push_back(value);
       }
       return result;
   } else {
       return std::vector<int>{0};
   }
}

