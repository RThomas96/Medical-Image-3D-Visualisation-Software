#ifndef TIFFIMAGE_HPP_
#define TIFFIMAGE_HPP_

#include "../../image/utils/include/image_api_common.hpp"
#include <tinytiffreader.h>
#include <tinytiffwriter.h>
#include <tiff.h>
#include <tiffio.h>
#include <vector>

// Just a plain tiff image
// Access to data is made with plain coordinates and not 3D point
// No data are stored, this class is only a reader
struct TIFFImage {

    TIFF* tif;
    glm::vec3 imgResolution;
    Image::ImageDataType imgDataType;

    TIFFImage(const std::string& filename);

    ~TIFFImage() {
        TIFFClose(this->tif);
    }

    // In theory floor aren't necessary cause coord are already integer
    uint16_t getValue(const glm::vec3& coord) const;

    // The getSlice function can be used to retrieve slices at various resolutions, to do so, it can skip pixels.
    // With offsets = {1, 1}, no pixel are skipped and a slice at the original image resolution is returned.
    // With offsets = {2, 1}, all pixels with odd x coordinates will be skipped, which result with a slice with
    // half the resolution on the x axis.
    // With offsets = {3, 3}, the result image resolution will be divided per 3 on x and y axis.
    // NOTE: these offsets are managed by the grid class in "getGridClass" function. In theory this function
    // do not need to be used manually
    // NOTE: as the z axis is fixed (the sliceIdx parameter), you cannot change the z resolution
    void getSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel, std::pair<int, int>  offsets, std::pair<glm::vec3, glm::vec3> bboxes) const;

    Image::ImageDataType getInternalDataType() const;
};

#endif
