#ifndef SIMPLEGRID_HPP_
#define SIMPLEGRID_HPP_

#include "../../image/utils/include/image_api_common.hpp"
#include "tetrahedral_mesh.hpp"
#include <tinytiffreader.h>
#include <tinytiffwriter.h>

struct SimpleGrid {

    TIFF* tif;
    glm::vec3 imgDimensions;
    Image::ImageDataType imgDataType;

    SimpleGrid(const std::string& filename);

    ~SimpleGrid() {
        TIFFClose(this->tif);
    }

    // In theory floor aren't necessary cause coord are already integer
    uint8_t getValue(const glm::vec3& coord) const;

    void getSlice(int imgIdx, int lineIdx, std::vector<uint16_t>& result, int nbChannel) const;

    // Here "image" refer to all pixels at the same z dimension
    void getImage(int imgIdx, std::vector<std::uint16_t>& result, int nbChannel) const;

    Image::ImageDataType getInternalDataType() const;
};

// Struct able to make the link between the image and its 3D representation
// Associate 3D dimensions and origin to the image and thus allow to query values with a 3D point
// TODO: rename SimpleGrid into image, and DeformableGrid into Grid
struct DeformableGrid {

    SimpleGrid grid;
    TetMesh tetmesh;

    DeformableGrid(const std::string& filename, const glm::vec3& nbCube);

    glm::vec3 getCoordInInitial(const DeformableGrid& initial, glm::vec3 p);

    // Here p is a 3D point, not like coord from SimpleGrid's "getValue" function that is a set of 3 indices 
    uint8_t getValueFromPoint(const glm::vec3& p) const;

    void movePoint(const glm::vec3& indices, const glm::vec3& position);

    void replaceAllPoints(const std::vector<glm::vec3>& pts);

    void writeDeformedGrid(const DeformableGrid& initial);

    // TODO: to remove
    // Function added only for backward comptability with the old grid, used all along in the OpenGL interface
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;
    glm::vec3 getResolution() const;
    Image::ImageDataType getInternalDataType() const;
    int getVoxelDimensionality() const { return 2;}

    void checkReadSlice() const;
};


#endif
