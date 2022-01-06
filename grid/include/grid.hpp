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
// It can be deactivate in order to operate on the original image, aka at full resolution
// NOTE: the grid DO NOT have any 3D data like position, or size. It only provides functions 
// to access to the image data.
struct Sampler {
    TIFFImage image;
    glm::vec3 samplerResolution;

    glm::vec3 resolutionRatio;

    Sampler(const std::string& filename);
    Sampler(const std::string& filename, int subsample);

    void getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const;

    // In theory floor aren't necessary cause coord are already integer
    uint16_t getValue(const glm::vec3& coord, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    Image::ImageDataType getInternalDataType() const;
    glm::vec3 getImageDimensions() const;
};

// Struct to make link between the grid and a 3D shape used to deform the space of its representation
// Struct able to make the link between the grid and its 3D representation
struct SimpleGrid {

    Sampler grid;
    TetMesh tetmesh;

    SimpleGrid(const std::string& filename, const glm::vec3& nbCube, int subsample);

    // Here p is a 3D point, not like coord from TIFFImage's "getValue" function that is a set of 3 indices 
    uint16_t getValueFromPoint(const glm::vec3& p, ResolutionMode resolutionMode = ResolutionMode::SAMPLER_RESOLUTION) const;

    glm::vec3 getCoordInInitial(const SimpleGrid& initial, glm::vec3 p);

    void movePoint(const glm::vec3& indices, const glm::vec3& position);
    void writeDeformedGrid(const SimpleGrid& initial, ResolutionMode resolutionMode = ResolutionMode::FULL_RESOLUTION);

    void replaceAllPoints(const std::vector<glm::vec3>& pts);

    // TODO: to remove
    // Function added only for backward comptability with the old grid, used all along in the OpenGL interface
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;
    glm::vec3 getResolution() const;
    Image::ImageDataType getInternalDataType() const;
    int getVoxelDimensionality() const { return 2;}
    void checkReadSlice() const;
};


#endif
