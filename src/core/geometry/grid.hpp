#ifndef SIMPLEGRID_HPP_
#define SIMPLEGRID_HPP_

#include "../drawable/drawable_grid.hpp"
#include "tetrahedral_mesh.hpp"
#include "../images/image.hpp"

//! \defgroup grid Grid
//! \addtogroup grid
//! @{

// Wrapper around an Image in order to access its data
// This class allow to have a resolution different from the original image
// This class also allow to access data as if we have a subregion of the original image
// The subregion is defined as a pair of triplet of INDICES and or not 3D data, position are whatever
// Moreover subregions indices are in Sampler space
// NOTE: the sampler DO NOT have any 3D data like position or 3D vectors, or size. It only provides functions
// to access to the image data.
struct Sampler {

private:
    glm::vec3 voxelSize;
public:
    glm::vec3 resolutionRatio;

    glm::vec3 bbMin;
    glm::vec3 bbMax;

    bool useCache;
    Cache * cache;

    Sampler(const std::vector<std::string>& filename, int subsample, const glm::vec3& voxelSize);

    uint16_t getValue(const glm::vec3& coord, Interpolation::Method interpolationMethod = Interpolation::Method::NearestNeighbor) const;
    template<typename DataType>
    DataType getValue(const glm::vec3& coord) const {
        return this->image->getValue<DataType>(coord * this->resolutionRatio);
    }

    glm::vec3 getDimension() const;

    void fromSamplerToImage(glm::vec3& p) const;
    void fromImageToSampler(glm::vec3& p) const;

    Tiff_image * image;

    // Fraude
    void getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const;
    glm::vec3 getVoxelSize() const;
    Image::ImageDataType getInternalDataType() const;
    std::vector<int> getHistogram() const;
private:
    void fillCache();
};

//! @brief Struct able to make the link between the grid and its 3D representation
struct Grid : public TetMesh, public DrawableGrid {

    TetMesh initialMesh;
    Sampler sampler;

    Grid(const std::vector<std::string>& filename, int subsample, const glm::vec3& sizeVoxel, const glm::vec3& nbCubeGridTransferMesh);
    Grid(const std::vector<std::string>& filename, int subsample, const glm::vec3& sizeVoxel, const std::string& fileNameTransferMesh);

    void buildTetmesh(const glm::vec3& nbCube);

    void loadMESH(std::string const &filename) override;

    glm::vec3 getVoxelSize() const;

    void sampleSliceGridValues(const glm::vec3 &slice, const std::pair<glm::vec3, glm::vec3> &areaToSample, const glm::vec3 &resolution, std::vector<uint16_t> &result, Interpolation::Method interpolationMethod);

    uint16_t getDeformedValueFromPoint(const TetMesh& initial, const glm::vec3& p, Interpolation::Method interpolationMethod = Interpolation::Method::NearestNeighbor) const;
    uint16_t getValueFromPoint(const glm::vec3& coord, Interpolation::Method interpolationMethod = Interpolation::Method::NearestNeighbor) const;

    uint16_t getMaxValue() {return this->sampler.image->maxValue;}
    uint16_t getMinValue() {return this->sampler.image->minValue;}

    template<typename DataType>
    DataType getValueFromPointGeneric(const glm::vec3& coord) const {
        bool isInBBox = true;
        for(int i = 0; i < 3; ++i) {
            if(coord[i] < this->sampler.bbMin[i] || coord[i] > this->sampler.bbMax[i])
                isInBBox = false;
        }
        if(isInBBox) {
            return this->sampler.getValue<DataType>(coord);
        } else {
            // Background value
            return static_cast<DataType>(0);
        }
    }

    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos, glm::vec3& res) const override;

    void updateTextureCoordinates() {
        this->texCoord.clear();
        for(int i = 0; i < this->vertices.size(); ++i) {
            this->texCoord.push_back(this->vertices[i]/this->sampler.getDimension());
        }    
    }

    Image::ImageDataType getInternalDataType() const {
        return this->sampler.getInternalDataType();
    }
    
    glm::vec3 getResolution() const {
        return this->sampler.getDimension();
    }
    
    int getNbSlice() const {
        return this->sampler.getDimension()[2];
    }
    
    void getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const {
        this->sampler.getGridSlice(sliceIdx, result, nbChannel);
    }

    void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) override {
        BaseMesh::movePoints(origins, targets);
        DrawableGrid::sendTetmeshToGPU(Grid::InfoToSend(Grid::InfoToSend::VERTICES | Grid::InfoToSend::NORMALS));
    }
};

//! @}

#endif
