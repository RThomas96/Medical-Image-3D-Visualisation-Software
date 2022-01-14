#ifndef TIFFIMAGE_HPP_
#define TIFFIMAGE_HPP_

#include "../../image/utils/include/image_api_common.hpp"
#include <tinytiffreader.h>
#include <tinytiffwriter.h>
#include <tiff.h>
#include <tiffio.h>
#include <vector>

// The cache just STORE data
// Thus the only way to interact with it is to query the adress of a vector to store some data
// The query and actual store process is manage by TIFFImage class
struct Cache {
    // Maximum number of slices to be stored
    int capacity;
    int nbInsertion;
    glm::vec3 imageSize;

    std::vector<int> indices;
    std::vector<std::vector<uint16_t>> data;

    Cache(glm::vec3 imageSize, int capacity);

    std::vector<uint16_t> * storeImage(int imageIdx);
    uint16_t getValue(const glm::vec3& coord);

    bool isCached(int imageIdx) const;
    int getCachedIdx(int imageIdx) const;
    int getNextCachedImageToReplace() const;

    void setCapacity(int capacity);
};

// TIFFReader read a tiff image using only and only the libtiff
// When reading with libtiff there is no notion of point, slice or even datatype, it just read from
// an image and copy the data into a buffer of indetermined datatype
// This is why we haven't any function like get point or get slice, but only readScanLine
struct TIFFReader {

    TIFF* tif;
    int openedImage;
    std::vector<std::string> filenames;

    TIFFReader(const std::vector<std::string>& filename);

    glm::vec3 getImageResolution() const;
    Image::ImageDataType getImageInternalDataType() const;

    void setImageToRead(int sliceIdx);

    tsize_t getScanLineSize() const;
    int readScanline(tdata_t buf, uint32 row) const;

    void openImage(int imageIdx);
    void closeImage();
};

// TIFFImage query data from an image, by storing data in a cache (if toggled) using a TIFFReader to query the data 
// As TIFFReader return void data, TIFFImage take in charge to cast the data to the right type
// It also use the cache as a storage to speed up the query process
// The getSlice function as numerous of options as nbChannel, offsets or bboxes to query respectively multiple channels, 
// to skip voxels or to query only a subregion of the slice
// These options are managed by the Sampler class, that is in charge to call TIFFImage the right way to query data
struct SimpleImage {

    glm::vec3 imgResolution;
    Image::ImageDataType imgDataType;

    TIFFReader * tiffReader;
    bool useCache;
    Cache * cache;

    SimpleImage(const std::vector<std::string>& filename);

    ~SimpleImage() {
        this->tiffReader->closeImage();
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

    void getFullSlice(int sliceIdx, std::vector<std::uint16_t>& result) const;

    Image::ImageDataType getInternalDataType() const;
};

#endif
