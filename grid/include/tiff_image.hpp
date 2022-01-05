#ifndef TIFFIMAGE_HPP_
#define TIFFIMAGE_HPP_

#include "../../image/utils/include/image_api_common.hpp"
#include <tinytiffreader.h>
#include <tinytiffwriter.h>
#include <tiff.h>
#include <tiffio.h>

// Just a plain tiff image
// Access to data is made with plain coordinates and not 3D point
// No data are stored, this class is only a reader
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

    Image::ImageDataType getInternalDataType() const;
};

#endif
