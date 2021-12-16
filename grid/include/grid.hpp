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

    uint8_t getValueFromPoint(const glm::vec3& p) const;

    void movePoint(const glm::vec3& indices, const glm::vec3& position);

    void replaceAllPoints(const std::vector<glm::vec3>& pts);

    void writeDeformedGrid(const DeformableGrid& initial);
};

#endif
