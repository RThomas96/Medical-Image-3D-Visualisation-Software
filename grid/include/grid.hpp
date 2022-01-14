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
    TIFFImage image;

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
    glm::vec3 getImageDimensions() const;

    glm::vec3 getSamplerDimension() const;

    void fromSamplerToImage(glm::vec3& p) const;
    void fromImageToSampler(glm::vec3& p) const;
};

// Struct to make link between the grid and a 3D shape used to deform the space of its representation
// Struct able to make the link between the grid and its 3D representation
struct SimpleGrid {

    Sampler sampler;
    TetMesh tetmesh;

    SimpleGrid(const std::string& filename, const glm::vec3& nbCube, int subsample);
    SimpleGrid(const std::vector<std::string>& filename, const glm::vec3& nbCube, int subsample);
    SimpleGrid(const std::vector<std::string>& filename, const glm::vec3& nbCube, int subsample, const std::pair<glm::vec3, glm::vec3>& bbox);

    // Here p is a 3D point, not like coord from TIFFImage's "getValue" function that is a set of 3 indices 
    uint16_t getValueFromPoint(const glm::vec3& p, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    glm::vec3 getCoordInInitial(const SimpleGrid& initial, glm::vec3 p);

    void movePoint(const glm::vec3& indices, const glm::vec3& position);
    void writeDeformedGrid(const SimpleGrid& initial, ResolutionMode resolutionMode = ResolutionMode::FULL_RESOLUTION);

    void replaceAllPoints(const std::vector<glm::vec3>& pts);

    glm::vec3 getDimension() const;

    // TODO: to remove
    // Function added only for backward comptability with the old grid, used all along in the OpenGL interface
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;
    glm::vec3 getResolution() const;
    Image::ImageDataType getInternalDataType() const;
    int getVoxelDimensionality() const { return 2;}
    void checkReadSlice() const;
};


#endif
