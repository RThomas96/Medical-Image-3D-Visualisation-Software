#ifndef CACHE_HPP_
#define CACHE_HPP_

#include "../../legacy/image/utils/include/image_api_common.hpp"
#define cimg_display 0
#include "../../third_party/cimg/CImg.h"
#include <vector>

//! \addtogroup img
//! @{

namespace Interpolation {

    enum class Method {
        NearestNeighbor,
        Linear,
        Cubic
    };

    Method fromString(const std::string& method);
    std::vector<std::string> toStringList();
}

using namespace cimg_library;

//! @brief The cache
struct Cache {
    CImg<uint16_t> img;

    Cache(glm::vec3 imageSize);

    void storeImage(int imageIdx, const std::vector<uint16_t>& data);

    void reset();

    uint16_t getValue(const glm::vec3& coord, Interpolation::Method interpolationMethod);
};

//! @brief The cache just STORE data
//!
//! Thus the only way to interact with it is to query the adress of a vector to store some data
//! The query and actual store process is manage by TIFFImage class
//! 
//! This class is unused for now
struct UnsortedCache {
    // Maximum number of slices to be stored
    int capacity;
    int nbInsertion;
    glm::vec3 imageSize;

    std::vector<int> indices;
    std::vector<std::vector<uint16_t>> data;

    UnsortedCache(glm::vec3 imageSize, int capacity);

    std::vector<uint16_t> * storeImage(int imageIdx);
    uint16_t getValue(const glm::vec3& coord);

    bool isCached(int imageIdx) const;
    int getCachedIdx(int imageIdx) const;
    int getNextCachedImageToReplace() const;

    void setCapacity(int capacity);
};

//! @}

#endif
