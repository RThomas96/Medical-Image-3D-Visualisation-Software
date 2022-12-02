#ifndef TIFFIMAGE_HPP_
#define TIFFIMAGE_HPP_

#include <tinytiffreader.h>
#include <tinytiffwriter.h>
#include <tiff.h>
#include <tiffio.h>
#include "cache.hpp"
#include <fstream>
#include <bitset>
//#include <sys/stat.h>
//#include <filesystem>

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <string>
#include <QXmlStreamReader>

//! \defgroup img Image
//! @brief Modules to read images from multiple formats. 
//! The main class is ImageReader .
//! These classes do not implement any writing functions.
//! The only functions to write images are Scene::writeDeformedImageTemplated() and Scene::writeGreyscaleTIFFImage() .
//
//! \addtogroup img
//! @{

//! @brief A set of functions to simplify the libtiff API.
//!
//! When reading a tiff image with the libtiff there is no notion of pixel, slice or datatype, it just read from
//! an image and copy the data into a buffer of undetermined datatype.
//! This class can handle multiple tiff images.
struct TIFFReaderLibtiff {

    TIFF* tif;
    int openedImage;
    std::vector<std::string> filenames;

    TIFFReaderLibtiff(const std::vector<std::string>& filename);

    glm::vec3 getImageResolution() const;
    glm::vec3 getVoxelSize() const;
    Image::ImageDataType getImageInternalDataType() const;

    //! @brief Get how many values are contained in a single row of the image
    tsize_t getScanLineSize() const;

    int readScanline(tdata_t buf, uint32 row) const;

    //! @brief The TIFFReader class can handle multiple tiff images, this function set which image has to be read.
    void openImage(int imageIdx);

    //! @brief A single tiff image file can contain an entire stack of images, this function set which image has to be read in the current tiff image.
    void setImageToRead(int sliceIdx);

    void closeImage();
};

enum class ImageFormat {
    TIFF,
    OME_TIFF,
    DIM_IMA
};

//! @brief Get a single value from a buffer casted from imgDataType type to DataType type
template<typename DataType>
DataType getToLowPrecision(Image::ImageDataType imgDataType, const tdata_t& buf, int idx) {
    if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8)) {
        uint8_t * data = static_cast<uint8_t*>(buf);
        return static_cast<DataType>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16)) {
        uint16_t * data = static_cast<uint16_t*>(buf);
        return static_cast<DataType>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32)) {
        uint32_t * data = static_cast<uint32_t*>(buf);
        return static_cast<DataType>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64)) {
        uint64_t * data = static_cast<uint64_t*>(buf);
        return static_cast<DataType>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_8)) {
        int8_t * data = static_cast<int8_t*>(buf);
        return static_cast<DataType>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_16)) {
        int16_t * data = static_cast<int16_t*>(buf);
        return static_cast<DataType>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_32)) {
        int32_t * data = static_cast<int32_t*>(buf);
        return static_cast<DataType>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_64)) {
        int64_t * data = static_cast<int64_t*>(buf);
        return static_cast<DataType>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_32)) {
        float * data = static_cast<float*>(buf);
        return static_cast<DataType>(data[idx]);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_64)) {
        double * data = static_cast<double*>(buf);
        return static_cast<DataType>(data[idx]);
    }
}

//! @brief Get an entire buffer casted from imgDataType type to DataType type
template<typename DataType>
DataType getImageToLowPrecision(Image::ImageDataType imgDataType, const tdata_t& buf) {
    if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8)) {
        uint8_t * data = static_cast<uint8_t*>(buf);
        return static_cast<DataType>(data);
        //return data;
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16)) {
        uint16_t * data = static_cast<uint16_t*>(buf);
        return static_cast<DataType>(data);
        //return data;
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32)) {
        uint32_t * data = static_cast<uint32_t*>(buf);
        return static_cast<DataType>(data);
        //return data;
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64)) {
        uint64_t * data = static_cast<uint64_t*>(buf);
        return static_cast<DataType>(data);
        //return data;
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_8)) {
        int8_t * data = static_cast<int8_t*>(buf);
        return static_cast<DataType>(data);
        //return data;
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_16)) {
        int16_t * data = static_cast<int16_t*>(buf);
        return static_cast<DataType>(data);
        //return data;
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_32)) {
        int32_t * data = static_cast<int32_t*>(buf);
        return static_cast<DataType>(data);
        //return data;
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_64)) {
        int64_t * data = static_cast<int64_t*>(buf);
        return static_cast<DataType>(data);
        //return data;
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_32)) {
        float * data = static_cast<float*>(buf);
        return static_cast<DataType>(data);
        //return data;
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_64)) {
        double * data = static_cast<double*>(buf);
        return static_cast<DataType>(data);
        //return data;
    }
}
//
//Function to cast and insert for the get slice
// Important: a cast do not change a value, so if we want to cast from an int we need to make a conversion, this is the usage of needInversion
template <typename data_t, typename out_data_t>
void castToUintAndInsert2(data_t * values, std::vector<out_data_t>& res, int duplicate, int offset, std::pair<glm::vec3, glm::vec3> bboxes, bool needInversion = false) {
    for(int i = bboxes.first[0]; i < bboxes.second[0]; i+=offset) {
        for(int j = 0; j < duplicate; ++j)
            if(needInversion)
                res.push_back(static_cast<out_data_t>((std::bitset<sizeof(data_t) * CHAR_BIT>(values[i]).flip(std::bitset<sizeof(data_t)*CHAR_BIT>().size()-1)).to_ulong())); 
            else
                res.push_back(static_cast<out_data_t>(values[i])); 
    }
}

//Function to cast and insert for the get slice
template <typename data_t>
void castToLowPrecision2(Image::ImageDataType imgDataType, const tdata_t& buf, std::vector<data_t>& res, int duplicate, int offset, std::pair<glm::vec3, glm::vec3> bboxes) {
    if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_8)) {
        uint8_t * data = static_cast<uint8_t*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16)) {
        uint16_t * data = static_cast<uint16_t*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_32)) {
        uint32_t * data = static_cast<uint32_t*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_64)) {
        uint64_t * data = static_cast<uint64_t*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_8)) {
        int8_t * data = static_cast<int8_t*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes, true);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_16)) {
        int16_t * data = static_cast<int16_t*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes, true);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_32)) {
        int32_t * data = static_cast<int32_t*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes, true);
    } else if(imgDataType == (Image::ImageDataType::Signed | Image::ImageDataType::Bit_64)) {
        int64_t * data = static_cast<int64_t*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes, true);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_32)) {
        float * data = static_cast<float*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes);
    } else if(imgDataType == (Image::ImageDataType::Floating | Image::ImageDataType::Bit_64)) {
        double * data = static_cast<double*>(buf);
        castToUintAndInsert2(data, res, duplicate, offset, bboxes);
    } 
}


struct TIFFReader {

    glm::vec3 voxelSize; // Read from the image, not necessarily the one used in the software
    glm::vec3 imgResolution;
    Image::ImageDataType imgDataType;

    TIFFReaderLibtiff * tiffReader;

    TIFFReader(const std::vector<std::string>& filename);

    ~TIFFReader() {
        this->tiffReader->closeImage();
    }

    uint16_t getValue(const glm::vec3& coord) const;

    template<typename DataType>
    DataType getValue(const glm::vec3& coord) const {
        // If we read directly from the raw image we use Nearest Neighbor interpolation
        const glm::ivec3 newCoord{std::floor(coord[0]), std::floor(coord[1]), std::floor(coord[2])};
        int imageIdx = newCoord[2];
        this->tiffReader->setImageToRead(imageIdx);
        tdata_t buf = _TIFFmalloc(this->tiffReader->getScanLineSize());
        this->tiffReader->readScanline(buf, newCoord[1]);
        DataType res = getToLowPrecision<DataType>(this->getInternalDataType(), buf, newCoord[0]);
        _TIFFfree(buf);
        return res;
    }

    template <typename data_t>
    void getImage(int sliceIdx, std::vector<data_t>& result, std::pair<glm::vec3, glm::vec3> bboxes) const {
        this->tiffReader->setImageToRead(sliceIdx);

        tdata_t buf;
        buf = _TIFFmalloc(this->tiffReader->getScanLineSize());

        uint32 row;
        for (row = bboxes.first[1]; row < bboxes.second[1]; row+=1) {
            this->tiffReader->readScanline(buf, row);
            castToLowPrecision2(this->getInternalDataType(), buf, result, 1, 1, bboxes);
        }
        _TIFFfree(buf);
    }

    Image::ImageDataType getInternalDataType() const;

    //! @brief See ImageReader::getSlice() .
    void getSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel, std::pair<int, int>  offsets, std::pair<glm::vec3, glm::vec3> bboxes) const;
};

inline bool fileExist (const std::string& name) {
  if (FILE *file = fopen(name.c_str(), "r")) {
      fclose(file);
      return true;
  } else {
      return false;
  }
}

struct OMETIFFReader : public TIFFReader {
    OMETIFFReader(const std::vector<std::string>& filename) : TIFFReader(filename) {
        std::cout << "Start of the OME-TIFF format parsing..." << std::endl;
        QDir path = QDir(QFileInfo(filename[0].c_str()).absolutePath());
        this->tiffReader->filenames.clear();
        char * xmlData; 
        TIFFGetField(this->tiffReader->tif, TIFFTAG_IMAGEDESCRIPTION, &xmlData);
        std::string strData(xmlData);
        if(!strData.empty()) {
            QXmlStreamReader xmlReader(xmlData); 
            while (!xmlReader.atEnd()) {
                xmlReader.readNextStartElement();
                std::cout << xmlReader.name().toString().toStdString() << std::endl;
                if(xmlReader.name().toString() == QString("UUID")) {
                    if(xmlReader.attributes().hasAttribute("FileName")) {
                        QString finalFileName = path.filePath(xmlReader.attributes().value("FileName").toString());
                        if(fileExist(finalFileName.toStdString())) {
                            this->tiffReader->filenames.push_back(finalFileName.toStdString());
                        } else {
                            //std::cout << "WARNING: [" << finalFileName.toStdString() << "] file doesn't exist but is present in the XML." << std::endl;
                        }
                    }
                }
            }
            std::cout << "[" << this->tiffReader->filenames.size() << "] files found" << std::endl;
            if (xmlReader.hasError()) {
                std::cout << "WARNING: the XML file contained in the first ome tiff file's comment has errors." << std::endl;
            }
            this->imgResolution[2] = this->tiffReader->filenames.size();
        } else {
            std::cout << "WARNING: no XML data has been found in the first ome.tiff file. Those files will be parse as regular tiff files." << std::endl;
        }
    }
};


struct DIMReader {

    glm::vec3 voxelSize; // Read from the image, not necessarily the one used in the software
    glm::vec3 imgResolution;
    Image::ImageDataType imgDataType;

    std::vector<uint16_t> data;

    DIMReader(const std::vector<std::string>& filename) {
        QString imaName = QString(filename[0].c_str());

        imaName.replace(".dim", ".ima" );
        std::ifstream imaFile (imaName.toUtf8());
        if (!imaFile.is_open())
            return;

        std::ifstream dimFile (imaName.toUtf8());
        if (!dimFile.is_open())
            return;

        int n[3];
        double d[3];

        dimFile >> n[0]; dimFile >> n[1]; dimFile >> n[2];

        std::string dummy, type;

        dimFile >> dummy;
        while (dummy.find("-type")==std::string::npos)
            dimFile >> dummy;

        //dimFile >> type;

        while (dummy.find("-dx")==std::string::npos)
            dimFile >> dummy;

        dimFile >> d[0];

        dimFile >> dummy;
        while (dummy.find("-dy")==std::string::npos)
            dimFile >> dummy;

        dimFile >> d[1];

        dimFile >> dummy;
        while (dummy.find("-dz")==std::string::npos)
            dimFile >> dummy;

        dimFile >> d[2];


        std::cout << "(nx,dx) = ( " << n[0] << " ; " << d[0] << " ) "<< std::endl;
        std::cout << "(ny,dy) = ( " << n[1] << " ; " << d[1] << " ) "<< std::endl;
        std::cout << "(nz,dz) = ( " << n[2] << " ; " << d[2] << " ) "<< std::endl;

        unsigned int size = n[0]*n[1]*n[2];
        unsigned int sizeIn = size;

        if( type.find("S16")!=std::string::npos )
            sizeIn = size*2;
        if( type.find("FLOAT")!=std::string::npos )
            sizeIn = size*4;

        unsigned char * data = new unsigned char[sizeIn];

        imaFile.read((char*)data, sizeIn);

        std::vector<uint16_t> _data(size);

        if( type.find("S16")!=std::string::npos ){
            for(unsigned int i = 0, j=0 ; i < size ; i ++, j+=2)
                _data[i] = (uint16_t)data[j];
        } else if( type.find("FLOAT")!=std::string::npos ){
            float * floatArray = (float*) data;

            for(unsigned int i = 0 ; i < size ; i ++)
                _data[i] = (uint16_t)floatArray[i];

            delete [] floatArray;
        } else {
            for(unsigned int i = 0 ; i < size ; i ++)
                _data[i] = (uint16_t)data[i];
        }

        this->imgResolution[0] = n[0];
        this->imgResolution[1] = n[1];
        this->imgResolution[2] = n[2];
        this->voxelSize[0] = d[0];
        this->voxelSize[1] = d[1];
        this->voxelSize[2] = d[2];
        this->imgDataType = (Image::ImageDataType::Unsigned | Image::ImageDataType::Bit_16);

        delete [] data;
        this->data = _data;
    }

    ~DIMReader() {
    }

    int from3DTo1D(const glm::vec3& p) const {
        return p[0] + this->imgResolution[0] * p[1] + this->imgResolution[0] * this->imgResolution[1] * p[2]; 
    }

    uint16_t getValue(const glm::vec3& coord) const {
        return this->data[this->from3DTo1D(coord)];
    }

    Image::ImageDataType getInternalDataType() const {
        return this->imgDataType;
    }

    //! @brief See ImageReader::getSlice() .
    void getSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel, std::pair<int, int>  offsets, std::pair<glm::vec3, glm::vec3> bboxes) const {
        float k = sliceIdx;
        result.clear();
        for(int j = bboxes.first[1]; j < bboxes.second[1]; j+=offsets.second) {
            for(int i = bboxes.first[0]; i < bboxes.second[0]; i+=offsets.first) {
                for(int l = 0; l < nbChannel; ++l) {
                    result.push_back(this->from3DTo1D(glm::vec3(i, j, k)));
                }
            }
        }
    }
};

//! @brief Provides functions to read values from a TIFF, OME-TIFF or DIM-IMA image.
//! \warning DIM-IMA reader has not been maintained from a long time and is not available for the user.
//! \note
//! This class do not implement any writing functions.
//! The only functions to write images are Scene::writeDeformedImageTemplated() and Scene::writeGreyscaleTIFFImage() .
struct ImageReader {

    ImageFormat imageFormat;

    TIFFReader * tiffImageReader;
    OMETIFFReader * omeTiffImageReader;
    DIMReader * dimImageReader;

    glm::vec3 voxelSize; // Read from the image, not necessarily the one used in the software
    glm::vec3 imgResolution;
    Image::ImageDataType imgDataType;

    uint16_t maxValue;
    uint16_t minValue;

    ImageReader(const std::vector<std::string>& filename) {
        std::string extension = filename[0].substr(filename[0].find_last_of(".") + 1);
        if(filename.size() > 0 || extension == "tif" || extension == "tiff") {
            if(filename[0].substr(filename[0].find_first_of(".") + 1).find("ome")!=std::string::npos) {
                this->imageFormat = ImageFormat::OME_TIFF;
                this->omeTiffImageReader = new OMETIFFReader(filename);
                this->tiffImageReader = nullptr;
                this->dimImageReader = nullptr;
                this->voxelSize = this->omeTiffImageReader->voxelSize;
                this->imgResolution = this->omeTiffImageReader->imgResolution;
                this->imgDataType = this->omeTiffImageReader->imgDataType;
                return;
            } else {
                this->imageFormat = ImageFormat::TIFF;
                this->tiffImageReader = new TIFFReader(filename);
                this->omeTiffImageReader = nullptr;
                this->dimImageReader = nullptr;
                this->voxelSize = this->tiffImageReader->voxelSize;
                this->imgResolution = this->tiffImageReader->imgResolution;
                this->imgDataType = this->tiffImageReader->imgDataType;
                return;
            }
        }

        if(extension == "dim" || extension == "ima") {
            this->imageFormat = ImageFormat::DIM_IMA;
            this->tiffImageReader = nullptr;
            this->omeTiffImageReader = nullptr;
            this->dimImageReader = new DIMReader(filename);
            this->voxelSize = this->dimImageReader->voxelSize;
            this->imgResolution = this->dimImageReader->imgResolution;
            this->imgDataType = this->dimImageReader->imgDataType;
            return;
        }
    }

    ~ImageReader() {
        delete this->dimImageReader;
        delete this->tiffImageReader;
    }

    uint16_t getValue(const glm::vec3& coord) const {
        switch(this->imageFormat) {
            case ImageFormat::TIFF :
                return this->tiffImageReader->getValue(coord);
                break;
            case ImageFormat::DIM_IMA :
                return this->dimImageReader->getValue(coord);
                break;
            case ImageFormat::OME_TIFF :
                return this->omeTiffImageReader->getValue(coord);
                break;
        }
    }

    template<typename DataType>
    DataType getValue(const glm::vec3& coord) const {
        switch(this->imageFormat) {
            case ImageFormat::TIFF :
                return this->tiffImageReader->getValue<DataType>(coord);
                break;
            case ImageFormat::DIM_IMA :
                return 0.; 
                break;
            case ImageFormat::OME_TIFF :
                return 0.;
                break;
        }
    }

    Image::ImageDataType getInternalDataType() const {
        return this->imgDataType;
    }

    //! @brief Get one image of an image stack.
    //! @param sliceIdx Image index to get.
    //! @param nbChannel WARNING: this parameter isn't fully supported yet, for now it just duplicate the data to simulate multiple channels.
    //! @param offsets With offsets = {1, 1}, no pixel are skipped and a slice at the original image resolution is returned.
    //! With offsets = {2, 1}, all pixels with odd x coordinates will be skipped, which result with a slice with
    //! half the resolution on the x axis.
    //! With offsets = {3, 3}, the result image resolution will be divided per 3 on x and y axis.
    //! @param bboxes Allow to query only a subregion of the image using this bbox.
    //! @note As the z axis is fixed (the sliceIdx parameter), you cannot change the z resolution
    void getSlice(int sliceIdx, std::vector<std::uint16_t>& result, int nbChannel, std::pair<int, int>  offsets, std::pair<glm::vec3, glm::vec3> bboxes) const {
        switch(this->imageFormat) {
            case ImageFormat::TIFF :
                this->tiffImageReader->getSlice(sliceIdx, result, nbChannel, offsets, bboxes);
                break;
            case ImageFormat::DIM_IMA :
                this->dimImageReader->getSlice(sliceIdx, result, nbChannel, offsets, bboxes);
                break;
            case ImageFormat::OME_TIFF :
                this->omeTiffImageReader->getSlice(sliceIdx, result, nbChannel, offsets, bboxes);
                break;
        }
    }
};


//! @}

#endif
