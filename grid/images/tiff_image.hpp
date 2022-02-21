#ifndef TIFFIMAGE_HPP_
#define TIFFIMAGE_HPP_

#include <tinytiffreader.h>
#include <tinytiffwriter.h>
#include <tiff.h>
#include <tiffio.h>
#include "cache.hpp"

//! \defgroup img Image
//! \addtogroup img
//! @{

//! @brief The TIFFReader class is a simple libtiff overlay to read a full tiff image row with an easier API than the raw libtiff.
//!
//! This class can handle multiple tiff images.
//! When reading a tiff image with the libtiff there is no notion of pixel, slice or datatype, it just read from
//! an image and copy the data into a buffer of undetermined datatype.
//! Thus this class can only read a full image row and get its raw data.
struct TIFFReader {

    TIFF* tif;
    int openedImage;
    std::vector<std::string> filenames;

    TIFFReader(const std::vector<std::string>& filename);

    glm::vec3 getImageResolution() const;
    Image::ImageDataType getImageInternalDataType() const;

    //! Get how many values are contained in a single row of the image
    tsize_t getScanLineSize() const;

    int readScanline(tdata_t buf, uint32 row) const;

    //! The TIFFReader class can handle multiple tiff images, this function set which image has to be read.
    void openImage(int imageIdx);

    //! A single tiff image file can contain an entire stack of images, this function set which image has to be read in the current tiff image.
    void setImageToRead(int sliceIdx);

    void closeImage();
};

//! @brief SimpleImage read data from an image using a TIFFReader, and store them in a cache.
//!
//! TIFFReader is a very low level class that can only read a full image row, SimpleImage is a class that provide a more convenient access to the data.
//! For example, as TIFFReader return data of type (void*), TIFFImage take in charge to cast the data to the right type.
//! It also use the cache as a storage to speed up the query process.
//! The cache is also used to benefit from the features of the CImg class, like the interpolation.
//! The getSlice function as numerous of options as nbChannel, offsets or bboxes to query respectively multiple channels, 
//! to skip voxels or to query only a subregion of the image.
//! These options are managed by the Sampler class, that is in charge to call TIFFImage the right way to query data.
struct SimpleImage {

    glm::vec3 imgResolution;
    Image::ImageDataType imgDataType;

    TIFFReader * tiffReader;

    SimpleImage(const std::vector<std::string>& filename);

    ~SimpleImage() {
        this->tiffReader->closeImage();
    }

    uint16_t getValue(const glm::vec3& coord) const;

    Image::ImageDataType getInternalDataType() const;

    //! @brief The getSlice function retrieve images at various resolutions, to do so, it can skip pixels.
    //! These function parameters are managed by the grid class. This function do not need to be used manually.
    //! @param offsets With offsets = {1, 1}, no pixel are skipped and a slice at the original image resolution is returned.
    //! With offsets = {2, 1}, all pixels with odd x coordinates will be skipped, which result with a slice with
    //! half the resolution on the x axis.
    //! With offsets = {3, 3}, the result image resolution will be divided per 3 on x and y axis.
    //! @param bboxes Allow to query only a subregion of the image using this bbox.
    //! @param nbChannel WARNING: this parameter isn't fully supported yet, for now it just duplicate the data to simulate multiple channels.
    //! @note As the z axis is fixed (the sliceIdx parameter), you cannot change the z resolution
    void getSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel, std::pair<int, int>  offsets, std::pair<glm::vec3, glm::vec3> bboxes) const;
};

//! @}

#endif
