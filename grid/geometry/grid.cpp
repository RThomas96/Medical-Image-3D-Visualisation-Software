#include "grid.hpp"
#include <chrono>
#include <algorithm>
#include <type_traits>

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

Grid::Grid(const std::vector<std::string>& filename, int subsample, const glm::vec3& sizeVoxel, const glm::vec3& nbCubeGridTransferMesh): sampler(Sampler(filename, subsample, sizeVoxel)), toSamplerMatrix(glm::mat4(1.f)) {
    this->buildTetmesh(nbCubeGridTransferMesh);
    this->history = new History(this->vertices);
}

Grid::Grid(const std::vector<std::string>& filename, int subsample, const glm::vec3& sizeVoxel, const std::string& fileNameTransferMesh): sampler(Sampler(filename, subsample, sizeVoxel)), toSamplerMatrix(glm::mat4(1.f)) {
    this->loadMESH(fileNameTransferMesh);
}

// Only this one is used
void Grid::buildTetmesh(const glm::vec3& nbCube) {
    const glm::vec3 sizeCube = (this->sampler.getSamplerDimension() * this->getVoxelSize()) / nbCube;
    std::cout << "***" << std::endl;
    std::cout << "Build tetrahedral mesh grid..." << std::endl;
    std::cout << "Image dimension: " << this->sampler.getSamplerDimension() << std::endl;
    std::cout << "Nb of cubes: " << nbCube << std::endl;
    std::cout << "Cube size: " << sizeCube << std::endl;
    std::cout << "***" << std::endl;
    this->buildGrid(nbCube, sizeCube, glm::vec3(0., 0., 0.));
    // Here this initial mesh is built directly using the sampler dimension because we want it to match the sampler
    this->initialMesh.buildGrid(nbCube, this->sampler.getSamplerDimension() / nbCube, glm::vec3(0., 0., 0.));
}

uint16_t Grid::getValueFromWorldPoint(const glm::vec3& p, Interpolation::Method interpolationMethod, ResolutionMode resolutionMode) const {
    glm::vec3 pSampler = p;
    this->toSampler(pSampler);
    return this->getValueFromPoint(pSampler, interpolationMethod, resolutionMode);
}

uint16_t Grid::getValueFromPoint(const glm::vec3& p, Interpolation::Method interpolationMethod, ResolutionMode resolutionMode) const {
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

uint16_t Grid::getDeformedValueFromPoint(const TetMesh& initial, const glm::vec3& p, Interpolation::Method interpolationMethod, ResolutionMode resolutionMode) const {
    glm::vec3 pt2(0., 0., 0.);
    bool ptIsInInitial = this->getCoordInInitial(initial, p, pt2);
    if(!ptIsInInitial)
        return 0.;
    if(resolutionMode == ResolutionMode::FULL_RESOLUTION)
        this->sampler.fromSamplerToImage(pt2);
    return this->getValueFromPoint(pt2, interpolationMethod, resolutionMode);
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
    return this->sampler.getVoxelSize();
}

void Grid::loadMESH(std::string const &filename) {
    TetMesh::loadMESH(filename);
    this->initialMesh.loadMESH(filename);
    this->texCoord.clear();
    for(int i = 0; i < this->vertices.size(); ++i) {
        this->texCoord.push_back(this->vertices[i]/this->sampler.getSamplerDimension());
    }    
}

void Grid::sampleSliceGridValues(const glm::vec3& slice, const std::pair<glm::vec3, glm::vec3>& areaToSample, const glm::vec3& resolution, std::vector<uint16_t>& result, Interpolation::Method interpolationMethod) {
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
    glm::vec3 bbMaxScene = areaToSample.second;
    glm::vec3 newResolution = resolution;
    convert(newResolution);
    glm::vec3 sizeVoxelInNewImage = (bbMaxScene - bbMinScene) / newResolution;

    std::cout << "Size voxel new: " << sizeVoxelInNewImage << std::endl;

    auto isInScene = [&](glm::vec3& p) {
        return (p.x > bbMinScene.x && p.y > bbMinScene.y && p.z > bbMinScene.z && p.x < bbMaxScene.x && p.y < bbMaxScene.y && p.z < bbMaxScene.z);
    };

    auto fromWorldToNewImage = [&](glm::vec3& p) {
        p -= bbMinScene;
        p /= sizeVoxelInNewImage;
    };

    auto fromNewImageToWorld = [&](glm::vec3& p) {
        p *= sizeVoxelInNewImage;
        p += bbMinScene;
    };

    result.clear();
    result.resize(resolution[0] * resolution[1], 0);

    int printOcc = 10;
    printOcc = this->mesh.size()/printOcc;

    //#pragma omp parallel for schedule(dynamic) num_threads(fromGrid->mesh.size()/10)
    #pragma omp parallel for schedule(dynamic)
    for(int tetIdx = 0; tetIdx < this->mesh.size(); ++tetIdx) {
        const Tetrahedron& tet = this->mesh[tetIdx];
        glm::vec3 bbMin = tet.getBBMin();
        fromWorldToNewImage(bbMin);
        bbMin.x = std::ceil(bbMin.x) - 1;
        bbMin.y = std::ceil(bbMin.y) - 1;
        bbMin.z = std::ceil(bbMin.z) - 1;
        glm::vec3 bbMax = tet.getBBMax();
        fromWorldToNewImage(bbMax);
        bbMax.x = std::floor(bbMax.x) + 1;
        bbMax.y = std::floor(bbMax.y) + 1;
        bbMax.z = std::floor(bbMax.z) + 1;
        if((tetIdx%printOcc) == 0) {
            std::cout << "Loading: " << (float(tetIdx)/float(this->mesh.size())) * 100. << "%" << std::endl;
        }
        if(slice.y == -1 && slice.z == -1) {
            bbMin.x = slice.x;
            bbMax.x = slice.x+1;
        }
        if(slice.x == -1 && slice.y == -1) {
            bbMin.z = slice.z;
            bbMax.z = slice.z+1;
        }
        if(slice.x == -1 && slice.z == -1) {
            bbMin.y = slice.y;
            bbMax.y = slice.y+1;
        }
        for(int k = bbMin.z; k < int(bbMax.z); ++k) {
            for(int j = bbMin.y; j < int(bbMax.y); ++j) {
                for(int i = bbMin.x; i < int(bbMax.x); ++i) {
                    glm::vec3 p(i, j, k);

                    p += glm::vec3(.5, .5, .5);
                    fromNewImageToWorld(p);

                    if(isInScene(p) && tet.isInTetrahedron(p)) {
                        if(this->getCoordInInitial(this->initialMesh, p, p, tetIdx)) {

                            glm::vec3 pImg(i, j, k);
                            convert(pImg);
                            int insertIdx = pImg.x + pImg.y*resolution[0];

                            this->sampler.fromSamplerToImage(p);
                            result[insertIdx] = this->getValueFromPoint(p, interpolationMethod);
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


//void Grid::sampleSliceGridValues(const glm::vec3& slice, const std::pair<glm::vec3, glm::vec3>& areaToSample, const glm::vec3& resolution, std::vector<uint16_t>& result, Interpolation::Method interpolationMethod) {
//    auto start = std::chrono::steady_clock::now();
//
//    omp_set_nested(true);
//
//    // Space to sample
//    glm::vec3 bbMinScene = areaToSample.first;
//    glm::vec3 bbMaxScene = areaToSample.second;
//    glm::vec3 sizeVoxelInNewImage = (bbMaxScene - bbMinScene) / resolution;
//
//    auto isInScene = [&](glm::vec3& p) {
//        return (p.x > bbMinScene.x && p.y > bbMinScene.y && p.z > bbMinScene.z && p.x < bbMaxScene.x && p.y < bbMaxScene.y && p.z < bbMaxScene.z);
//    };
//
//    auto fromWorldToNewImage = [&](glm::vec3& p) {
//        p -= bbMinScene;
//        p /= sizeVoxelInNewImage;
//    };
//
//    auto fromNewImageToWorld = [&](glm::vec3& p) {
//        p *= sizeVoxelInNewImage;
//        p += bbMinScene;
//    };
//
//    result.clear();
//    result.resize(resolution[0] * resolution[1], 0);
//
//    int printOcc = 10;
//    printOcc = this->mesh.size()/printOcc;
//
//    //#pragma omp parallel for schedule(dynamic) num_threads(fromGrid->mesh.size()/10)
//    #pragma omp parallel for schedule(dynamic)
//    for(int tetIdx = 0; tetIdx < this->mesh.size(); ++tetIdx) {
//        const Tetrahedron& tet = this->mesh[tetIdx];
//        glm::vec3 bbMin = tet.getBBMin();
//        fromWorldToNewImage(bbMin);
//        bbMin.x = std::ceil(bbMin.x) - 1;
//        bbMin.y = std::ceil(bbMin.y) - 1;
//        bbMin.z = std::ceil(bbMin.z) - 1;
//        glm::vec3 bbMax = tet.getBBMax();
//        fromWorldToNewImage(bbMax);
//        bbMax.x = std::floor(bbMax.x) + 1;
//        bbMax.y = std::floor(bbMax.y) + 1;
//        bbMax.z = std::floor(bbMax.z) + 1;
//        if((tetIdx%printOcc) == 0) {
//            std::cout << "Loading: " << (float(tetIdx)/float(this->mesh.size())) * 100. << "%" << std::endl;
//        }
//        for(int k = bbMin.z; k < int(bbMax.z); ++k) {
//            for(int j = bbMin.y; j < int(bbMax.y); ++j) {
//                for(int i = bbMin.x; i < int(bbMax.x); ++i) {
//                    glm::vec3 p(i, j, k);
//                    if(p.x != slice.x && p.y != slice.y && p.z != slice.z)
//                        break;
//                    p += glm::vec3(.5, .5, .5);
//
//                    fromNewImageToWorld(p);
//
//                    if(isInScene(p) && tet.isInTetrahedron(p)) {
//                        if(this->getCoordInInitial(this->initialMesh, p, p, tetIdx)) {
//
//                            glm::vec3 pImg(i, j, k);
//                            int insertIdx = pImg.x + pImg.y*resolution[0];
//
//                            this->sampler.fromSamplerToImage(p);
//                            if(insertIdx > result.size())
//                                break;
//                            result[insertIdx] = this->getValueFromPoint(p, interpolationMethod);
//                        }
//                    }
//                }
//            }
//        }
//    }
//
//    auto end = std::chrono::steady_clock::now();
//    std::chrono::duration<double> elapsed_seconds = end-start;
//    std::cout << "Duration time: " << elapsed_seconds.count() << "s / " << elapsed_seconds.count()/60. << "m" << std::endl;
//}

//void Grid::sampleSliceGridValues(const glm::vec3& slice, const std::pair<glm::vec3, glm::vec3>& areaToSample, const glm::vec3& resolution, std::vector<uint16_t>& result, Interpolation::Method interpolationMethod) {
//    auto start = std::chrono::steady_clock::now();
//
//    omp_set_nested(true);
//
//    // Space to sample
//    glm::vec3 bbMinScene = areaToSample.first;
//    glm::vec3 bbMaxScene = areaToSample.second;
//    glm::vec3 sizeVoxelInNewImage = (bbMaxScene - bbMinScene) / resolution;
//    glm::vec3 sizeVoxelInOriginalImage = (bbMaxScene - bbMinScene) / this->getResolution();
//
//    auto isInScene = [&](glm::vec3& p) {
//        return (p.x > bbMinScene.x && p.y > bbMinScene.y && p.z > bbMinScene.z && p.x < bbMaxScene.x && p.y < bbMaxScene.y && p.z < bbMaxScene.z);
//    };
//
//    //auto closestDivisible = [&](const glm::vec3& p, const glm::vec3& div) {
//    //    glm::vec3 res(0., 0., 0.);
//    //    for(int i = 0; i < 3; ++i) {
//    //       int divVal = std::ceil(div[i]);
//    //       int value = std::ceil(p[i]);
//
//    //       int c1 = a - (a % b);
//    //       int c2 = (a + b) - (a % b);
//    //    }
//    //}
//
//    auto fromWorldToNewImage = [&](glm::vec3& p) {
//        p -= bbMinScene;
//        p /= sizeVoxelInNewImage;
//    };
//
//    auto fromNewImageToWorld = [&](glm::vec3& p) {
//        p *= sizeVoxelInNewImage;
//        p += bbMinScene;
//    };
//
//    result.clear();
//    result.resize(resolution[0] * resolution[1], 0);
//
//    int printOcc = 10;
//    printOcc = this->mesh.size()/printOcc;
//
//    //#pragma omp parallel for schedule(dynamic) num_threads(fromGrid->mesh.size()/10)
//    //#pragma omp parallel for schedule(dynamic)
//    for(int tetIdx = 0; tetIdx < this->mesh.size(); ++tetIdx) {
//        const Tetrahedron& tet = this->mesh[tetIdx];
//        glm::vec3 bbMin = tet.getBBMin();
//        bbMin.x = std::ceil(bbMin.x) - 1;
//        bbMin.y = std::ceil(bbMin.y) - 1;
//        bbMin.z = std::ceil(bbMin.z) - 1;
//        glm::vec3 bbMax = tet.getBBMax();
//        bbMax.x = std::floor(bbMax.x) + 1;
//        bbMax.y = std::floor(bbMax.y) + 1;
//        bbMax.z = std::floor(bbMax.z) + 1;
//        if((tetIdx%printOcc) == 0) {
//            std::cout << "Loading: " << (float(tetIdx)/float(this->mesh.size())) * 100. << "%" << std::endl;
//        }
//        for(float k = bbMin.z; k < bbMax.z; k+=sizeVoxelInOriginalImage.z) {
//            for(float j = bbMin.y; j < bbMax.y; j+=sizeVoxelInOriginalImage.y) {
//                for(float i = bbMin.x; i < bbMax.x; i+=sizeVoxelInOriginalImage.x) {
//                    glm::vec3 pScene(i, j, k);
//                    pScene += sizeVoxelInNewImage/2.f;
//
//                    glm::vec3 pNewImg = pScene;
//                    fromWorldToNewImage(pNewImg);
//
//                    if(pNewImg.x != slice.x && pNewImg.y != slice.y && pNewImg.z != slice.z)
//                        break;
//
//                    if(isInScene(pScene) && tet.isInTetrahedron(pScene)) {
//                        glm::vec3 pValueSampler = pScene;
//                        if(this->getCoordInInitial(this->initialMesh, pValueSampler, pValueSampler, tetIdx)) {
//
//                            glm::vec3 pValue = pValueSampler;
//                            this->sampler.fromSamplerToImage(pValue);
//                            int insertIdx = pNewImg.x + pNewImg.y * resolution[0];
//                            if(insertIdx > result.size())
//                                break;
//                            result[insertIdx] = this->getValueFromPoint(pValue, interpolationMethod);
//                        }
//                    }
//                }
//            }
//        }
//    }
//
//    auto end = std::chrono::steady_clock::now();
//    std::chrono::duration<double> elapsed_seconds = end-start;
//    std::cout << "Duration time: " << elapsed_seconds.count() << "s / " << elapsed_seconds.count()/60. << "m" << std::endl;
//}

void Grid::fromImageToWorld(glm::vec3& p) const {
    this->sampler.fromImageToSampler(p);
    p *= this->getVoxelSize();
    p += this->bbMin;
}

void Grid::sampleGridValues(const std::pair<glm::vec3, glm::vec3>& areaToSample, const glm::vec3& resolution, std::vector<std::vector<uint16_t>>& result, Interpolation::Method interpolationMethod) {
    auto start = std::chrono::steady_clock::now();

    omp_set_nested(true);

    // Space to sample
    glm::vec3 bbMinScene = areaToSample.first;
    glm::vec3 bbMaxScene = areaToSample.second;
    glm::vec3 fromSamplerToSceneRatio = (bbMaxScene - bbMinScene) / resolution;

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
        if((tetIdx%printOcc) == 0) {
            std::cout << "Loading: " << (float(tetIdx)/float(this->mesh.size())) * 100. << "%" << std::endl;
        }
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

/**************************/

Sampler::Sampler(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox): image(new SimpleImage(filename)) {
    this->init(filename, subsample, bbox, glm::vec3(0., 0., 0.));
}

Sampler::Sampler(const std::vector<std::string>& filename, int subsample): image(new SimpleImage(filename)) {
    this->init(filename, subsample, std::pair<glm::vec3, glm::vec3>(glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.)), glm::vec3(0., 0., 0.));
}

Sampler::Sampler(const std::vector<std::string>& filename, int subsample, const glm::vec3& voxelSize): image(new SimpleImage(filename)) {
    this->init(filename, subsample, std::pair<glm::vec3, glm::vec3>(glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.)), voxelSize);
}

Sampler::Sampler(const std::vector<std::string>& filename): image(new SimpleImage(filename)) {
    this->init(filename, 1, std::pair<glm::vec3, glm::vec3>(glm::vec3(0., 0., 0.), glm::vec3(0., 0., 0.)), glm::vec3(0., 0., 0.));
}

Sampler::Sampler(glm::vec3 size): image(nullptr) {
    glm::vec3 samplerResolution = size; 
    this->resolutionRatio = glm::vec3(1., 1., 1.); 

    this->bbMin = glm::vec3(0., 0., 0.);
    this->bbMax = samplerResolution;

    this->subregionMin = this->bbMin;
    this->subregionMax = this->bbMax;

    // Cache management
    this->useCache = USE_CACHE;
    if(this->useCache) {
        this->cache = new Cache(this->getSamplerDimension());
    }
    // We do not fill the cache as we do not have associated image
    //this->fillCache();
}

void Sampler::init(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox, const glm::vec3& voxelSize) {
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

    if(glm::distance(bbox.first, bbox.second) > 0.0000001) {
        this->subregionMin = bbox.first;
        this->subregionMax = bbox.second;
    } else {
        this->subregionMin = this->bbMin;
        this->subregionMax = this->bbMax;
    }

    // Cache management
    this->useCache = USE_CACHE;
    if(this->useCache) {
        this->cache = new Cache(this->getSamplerDimension());
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
    std::cout << "Sampler resolution: " << this->getSamplerDimension() << std::endl;
    std::cout << "Resolution ratio: " << this->resolutionRatio << std::endl;
    std::cout << "Voxel size: " << this->voxelSize;
    if(useOriginalVoxelSize)
        std::cout << " (original)" << std::endl; 
    else
        std::cout << " (manual)" << std::endl; 
    std::cout << "Subregion selected: " << this->subregionMin << " | " << this->subregionMax << std::endl;
}


void Sampler::setVoxelSize(const glm::vec3& voxelSize) {
    this->voxelSize = voxelSize;
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

uint16_t Sampler::getValue(const glm::vec3& coord, Interpolation::Method interpolationMethod, ResolutionMode resolutionMode) const {
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

glm::vec3 Sampler::getVoxelSize() const {
    return this->voxelSize;
}
