# Builds the TIFF backend of the image component

# Once the project is known to compile, make this part of the library optional
IF(${BUILD_TIFF_SUPPORT})
ENDIF()

# Defined symbols :
# libTIFF : libtiff's path to the compiled lib
# libTIFF_HEADER_DIR : path to libtiff headers

ADD_LIBRARY(grid_tiff_backend
	src/backend.cpp
	src/tiff_frame.cpp
	src/tiff_private.cpp

	include/backend.hpp
	include/tiff_frame.hpp
	include/tiff_private.hpp
	include/tiff_template.hpp
	include/tiff_template.impl.hpp
)
TARGET_INCLUDE_DIRECTORIES(grid_tiff_backend
	PUBLIC ${libTIFF_HEADER_DIR}
	// Also add GLM headers here !
)
TARGET_LINK_LIBRARIES(grid_tiff_backend
	PUBLIC ${libTIFF}
)

# Once the compilation is conditional, maybe add that lib to a list of compilable libs which
# then get linked by the top CMakeLists.txt ??? Just a random thought.
