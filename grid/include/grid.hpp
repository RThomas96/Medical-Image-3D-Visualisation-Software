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

    Sampler(const std::vector<std::string>& filename);
    Sampler(const std::vector<std::string>& filename, int subsample);
    Sampler(const std::vector<std::string>& filename, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox);

    void getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const;

    // In theory floor aren't necessary cause coord are already integer
    uint16_t getValue(const glm::vec3& coord, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    Image::ImageDataType getInternalDataType() const;

    glm::vec3 getSamplerDimension() const;

    void fromSamplerToImage(glm::vec3& p) const;
    void fromImageToSampler(glm::vec3& p) const;

    void setUseCache(bool useCache);
    void setCacheCapacity(int capacity);

private:
    //Nobody should access to the original image size everthing need to pass by the Sampler
    glm::vec3 getImageDimensions() const;
    SimpleImage image;
};

// Struct to make link between the grid and a 3D shape used to deform the space of its representation
// Struct able to make the link between the grid and its 3D representation
struct Grid {

    Sampler sampler;

    Grid(const std::string& filename, const glm::vec3& nbCube, int subsample);
    Grid(const std::vector<std::string>& filename, const glm::vec3& nbCube, int subsample);
    Grid(const std::vector<std::string>& filename, const glm::vec3& nbCube, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox);

    // Here p is a 3D point, not like coord from TIFFImage's "getValue" function that is a set of 3 indices 
    uint16_t getValueFromPoint(const glm::vec3& p, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    glm::vec3 getCoordInInitial(const Grid& initial, glm::vec3 p);

    void movePoint(const glm::vec3& indices, const glm::vec3& position);
    void writeDeformedGrid(const Grid& initial, ResolutionMode resolutionMode = ResolutionMode::FULL_RESOLUTION);

    void replaceAllPoints(const std::vector<glm::vec3>& pts);

    glm::vec3 getDimension() const;
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;

    // TODO: temproray
    TetMesh tetmesh;
private:
    //TetMesh tetmesh;

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
