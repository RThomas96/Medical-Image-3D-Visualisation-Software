#ifndef SIMPLEGRID_HPP_
#define SIMPLEGRID_HPP_

#include "tetrahedral_mesh.hpp"
#include "../images/tiff_image.hpp"

//! \defgroup grid Grid
//! \addtogroup grid
//! @{

// This enum allow to choose at which resolution we want to query points
// FULL_RESOLUTION allow to bypass the Sampler class and query directly from the image
enum ResolutionMode {
    SAMPLER_RESOLUTION,
    FULL_RESOLUTION
};

// Wrapper around an Image in order to access its data
// This class allow to have a resolution different from the original image
// This class also allow to access data as if we have a subregion of the original image
// The subregion is defined as a pair of triplet of INDICES and or not 3D data, position are whatever
// Moreover subregions indices are in Sampler space
// NOTE: the grid DO NOT have any 3D data like position or 3D vectors, or size. It only provides functions 
// to access to the image data.
struct Sampler {

    glm::vec3 voxelSize;
    glm::vec3 resolutionRatio;

    glm::vec3 bbMin;
    glm::vec3 bbMax;

    glm::vec3 subregionMin;
    glm::vec3 subregionMax;

    bool useCache;
    Cache * cache;

    Sampler(glm::vec3 size);
    Sampler(const std::vector<std::string>& filename);
    Sampler(const std::vector<std::string>& filename, int subsample);
    Sampler(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox);
    Sampler(const std::vector<std::string>& filename, int subsample, const glm::vec3& voxelSize);

    void init(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox, const glm::vec3& voxelSize);

    void setVoxelSize(const glm::vec3& voxelSize);
    glm::vec3 getVoxelSize() const;

    void getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const;

    uint16_t getValue(const glm::vec3& coord, Interpolation::Method interpolationMethod = Interpolation::Method::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    Image::ImageDataType getInternalDataType() const;

    glm::vec3 getSamplerDimension() const;

    void fromSamplerToImage(glm::vec3& p) const;
    void fromImageToSampler(glm::vec3& p) const;

    //Nobody should access to the original image size everthing need to pass by the Sampler
    // Used to compute voxel size
    glm::vec3 getImageDimensions() const;
private:
    void fillCache();
    SimpleImage * image;
};

//! @brief Struct able to make the link between the grid and its 3D representation
struct Grid : public TetMesh {

    //This matrix keep track of modification of the TetMesh in order to get back to the sampler space which have 0 as origin
    std::vector<glm::mat4> transformations;
    glm::mat4 toSamplerMatrix;

    TetMesh initialMesh;
    Sampler sampler;

    Grid(const std::vector<std::string>& filename, int subsample, const glm::vec3& sizeVoxel, const glm::vec3& nbCubeGridTransferMesh);
    Grid(const std::vector<std::string>& filename, int subsample, const glm::vec3& sizeVoxel, const std::string& fileNameTransferMesh);

    //Removed because unused but still working
    //Grid(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox);

    void buildTetmesh(const glm::vec3& nbCube);

    void loadMESH(std::string const &filename) override;

    glm::vec3 getVoxelSize() const;
    glm::mat4 getTransformationMatrix() const;
    void toSampler(glm::vec3& p) const;
    void toWorld(glm::vec3& p) const;

    // In mesh interface
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;
    //void writeDeformedGrid(ResolutionMode resolutionMode = ResolutionMode::FULL_RESOLUTION);
    void writeDeformedGrid(ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION);

    void sampleGridValues(const std::pair<glm::vec3, glm::vec3>& areaToSample, const glm::vec3& resolution, std::vector<std::vector<uint16_t>>& result, Interpolation::Method interpolationMethod = Interpolation::Method::NearestNeighbor);
    void sampleSliceGridValues(const glm::vec3& slice, const std::pair<glm::vec3, glm::vec3>& areaToSample, const glm::vec3& resolution, int& idx, std::vector<uint16_t>& result, Interpolation::Method interpolationMethod = Interpolation::Method::NearestNeighbor);

    uint16_t getDeformedValueFromPoint(const TetMesh& initial, const glm::vec3& p, Interpolation::Method interpolationMethod = Interpolation::Method::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;
    uint16_t getValueFromPoint(const glm::vec3& coord, Interpolation::Method interpolationMethod = Interpolation::Method::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;
    uint16_t getValueFromWorldPoint(const glm::vec3& coord, Interpolation::Method interpolationMethod = Interpolation::Method::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos, glm::vec3& res) const override;

    Image::ImageDataType getInternalDataType() const {
        return this->sampler.getInternalDataType();
    }
    
    glm::vec3 getResolution() const {
        return this->sampler.getSamplerDimension();
    }
    
    int getNbSlice() const {
        return this->sampler.getSamplerDimension()[2];
    }
    
    void getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const {
        this->sampler.getGridSlice(sliceIdx, result, nbChannel);
    }

    void translate(const glm::vec3& vec) override;
    void rotate(const glm::mat3& transf) override;
    void scale(const glm::vec3& scale) override;
    void setOrigin(const glm::vec3& origin) override;

};

//! @}

#endif
