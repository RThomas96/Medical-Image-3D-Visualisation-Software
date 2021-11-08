#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_INCLUDE_COMMON_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_INCLUDE_COMMON_HPP_

/**
* tiff_include_common.hpp :
*	Includes all necessary files for the TIFF library in one place.
*	Also includes the necessary headers for the grid and image APIs.
*/

// Program-wide macros :
#include "../../macros.hpp"

// Grid and Image APIs :
#include "../../utils/include/image_api_common.hpp"
#include "../../interface/image_reader_interface.hpp"
#include "../../utils/include/read_cache.hpp"
#include "../../utils/include/threaded_task.hpp"

// libTIFF headers :
#include <tiff.h>
#include <tiffio.h>

// STD headers :
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <string_view>

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_INCLUDE_COMMON_HPP_
