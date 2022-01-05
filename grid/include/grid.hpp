#ifndef SIMPLEGRID_HPP_
#define SIMPLEGRID_HPP_

#include "../../image/utils/include/image_api_common.hpp"
#include "tetrahedral_mesh.hpp"
#include <tinytiffreader.h>
#include <tinytiffwriter.h>

struct TIFFImage {

    TIFF* tif;
    glm::vec3 imgDimensions;
    Image::ImageDataType imgDataType;

    TIFFImage(const std::string& filename);

    ~TIFFImage() {
        TIFFClose(this->tif);
    }

    // In theory floor aren't necessary cause coord are already integer
    uint16_t getValue(const glm::vec3& coord) const;

    //void getSlice(int imgIdx, int lineIdx, std::vector<uint16_t>& result, int nbChannel) const;

    // Here "image" refer to all pixels at the same z dimension
    //void getImage(int imgIdx, std::vector<std::uint16_t>& result, int nbChannel) const;

    Image::ImageDataType getInternalDataType() const;
};

struct Grid {
    TIFFImage image;
    glm::vec3 gridDimensions;

    glm::vec3 voxelSizeRatio;

    Grid(const std::string& filename, glm::vec3 gridDimensions);

    Grid(const std::string& filename);

    void getGridSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel) const;

    // In theory floor aren't necessary cause coord are already integer
    uint16_t getValue(const glm::vec3& coord) const;

    Image::ImageDataType getInternalDataType() const;

    glm::vec3 getImageDimensions() const;
};

// Struct able to make the link between the image and its 3D representation
// Associate 3D dimensions and origin to the image and thus allow to query values with a 3D point
// TODO: rename TIFFImage into image, and DeformableGrid into Grid
struct SimpleGrid {

    Grid grid;
    TetMesh tetmesh;

    SimpleGrid(const std::string& filename, const glm::vec3& nbCube);

    glm::vec3 getCoordInInitial(const SimpleGrid& initial, glm::vec3 p);

    // Here p is a 3D point, not like coord from TIFFImage's "getValue" function that is a set of 3 indices 
    uint16_t getValueFromPoint(const glm::vec3& p) const;

    void movePoint(const glm::vec3& indices, const glm::vec3& position);

    void replaceAllPoints(const std::vector<glm::vec3>& pts);

    void writeDeformedGrid(const SimpleGrid& initial);

    // TODO: to remove
    // Function added only for backward comptability with the old grid, used all along in the OpenGL interface
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;
    glm::vec3 getResolution() const;
    Image::ImageDataType getInternalDataType() const;
    int getVoxelDimensionality() const { return 2;}

    void checkReadSlice() const;
};


#endif
