#ifndef SIMPLEGRID_HPP_
#define SIMPLEGRID_HPP_

#include "tetrahedral_mesh.hpp"
#include "tiff_image.hpp"

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

// Struct to make link between the grid and a 3D shape used to deform the space of its representation
// Struct able to make the link between the grid and its 3D representation
struct Grid : public TetMesh {

    TetMesh initialMesh;
    Sampler sampler;

    Grid(glm::vec3 gridSize);
    Grid(const std::string& filename, int subsample);
    Grid(const std::vector<std::string>& filename, int subsample);
    Grid(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox);

    void buildTetmesh(const glm::vec3& nbCube);
    void buildTetmesh(const glm::vec3& nbCube, const glm::vec3& origin);
    void buildTetmesh(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin);

    // In mesh interface
    glm::vec3 getDimension() const;
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;
    void writeDeformedGrid(ResolutionMode resolutionMode = ResolutionMode::FULL_RESOLUTION);

    uint16_t getDeformedValueFromPoint(const TetMesh& initial, const glm::vec3& p, InterpolationMethod interpolationMethod = InterpolationMethod::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;
    uint16_t getValueFromPoint(const glm::vec3& coord, InterpolationMethod interpolationMethod = InterpolationMethod::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;
    uint16_t getValueFromWorldPoint(const glm::vec3& coord, InterpolationMethod interpolationMethod = InterpolationMethod::NearestNeighbor, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, glm::vec3& res) const override;
};

// This is the only class that interact with the openGL head
// ALL data are transiting by these functions
// This function serve as an interface between backend and openGl display
// It also serve to make to ensure that all points from world space as been converted to grid space
// It is mandatory as all computations are performed in grid space
// It allow to move the grid arround without affecting the computation
struct GridGL {

    glm::mat4 transform;

    GridGL(const std::string& filename, const glm::vec3& nbCube, int subsample);
    GridGL(const std::vector<std::string>& filename, const glm::vec3& nbCube, int subsample);
    GridGL(const std::vector<std::string>& filename, const glm::vec3& nbCube, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox);

    Image::ImageDataType getInternalDataType() const;

    //void fromGridToWorld(glm::vec3& p);
    //void fromWorldToGrid(glm::vec3& p);

    // Interface with the texture transfert to OpenGL
    int getNbSlice() const;
    void getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const;

    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;
    glm::vec3 getResolution() const;
    int getVoxelDimensionality() const { return 2;}

    Grid * grid;
    //TODO: temporary
private:
    //Grid * grid;
};

#endif
