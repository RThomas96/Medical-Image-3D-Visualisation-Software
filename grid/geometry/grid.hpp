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

    void getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const;

    uint16_t getValue(const glm::vec3& coord, InterpolationMethod interpolationMethod = InterpolationMethod::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    Image::ImageDataType getInternalDataType() const;

    glm::vec3 getSamplerDimension() const;

    void fromSamplerToImage(glm::vec3& p) const;
    void fromImageToSampler(glm::vec3& p) const;

private:
    void fillCache();
    //Nobody should access to the original image size everthing need to pass by the Sampler
    glm::vec3 getImageDimensions() const;
    SimpleImage * image;
};

//! @brief Struct able to make the link between the grid and its 3D representation
struct Grid : public TetMesh {

    //This matrix keep track of modification of the TetMesh in order to get back to the sampler space which have 0 as origin
    std::vector<glm::mat4> transformations;
    glm::mat4 toSamplerMatrix;

    TetMesh initialMesh;
    Sampler sampler;

    Grid(glm::vec3 gridSize);
    Grid(const std::string& filename, int subsample);
    Grid(const std::vector<std::string>& filename, int subsample);
    Grid(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox);

    void buildTetmesh(const glm::vec3& nbCube);
    void buildTetmesh(const glm::vec3& nbCube, const glm::vec3& origin);
    void buildTetmesh(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin);

    glm::mat4 getTransformationMatrix();
    void toSampler(glm::vec3& p) const;

    // In mesh interface
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;
    void writeDeformedGrid(ResolutionMode resolutionMode = ResolutionMode::FULL_RESOLUTION);

    uint16_t getDeformedValueFromPoint(const TetMesh& initial, const glm::vec3& p, InterpolationMethod interpolationMethod = InterpolationMethod::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;
    uint16_t getValueFromPoint(const glm::vec3& coord, InterpolationMethod interpolationMethod = InterpolationMethod::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;
    uint16_t getValueFromWorldPoint(const glm::vec3& coord, InterpolationMethod interpolationMethod = InterpolationMethod::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, glm::vec3& res) const override;

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
