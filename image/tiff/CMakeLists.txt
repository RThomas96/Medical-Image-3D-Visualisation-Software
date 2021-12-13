# Builds the TIFF backend of the image component

# Once the project is known to compile, make this part of the library optional
IF(${BUILD_TIFF_SUPPORT})
ENDIF()

# Defined symbols :
# libTIFF : libtiff's path to the compiled lib
# libTIFF_HEADER_DIR : path to libtiff headers

ADD_LIBRARY(grid_tiff_backend STATIC
	src/tiff_frame.cpp
	src/tiff_helpers.cpp
    src/tiff_reader.cpp

	include/tiff_include_common.hpp

    include/tiff_reader.hpp
	include/tiff_reader_templated.hpp
	include/tiff_reader_templated.impl.hpp
	include/tiff_frame.hpp
	include/tiff_helpers.hpp
)
TARGET_INCLUDE_DIRECTORIES(grid_tiff_backend
	PRIVATE ${libTIFF_HEADER_DIR}
	PUBLIC ${LIBS_COMPILED_HEADERS}
	# Also add GLM headers here !
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(grid_tiff_backend
	PUBLIC imagegrid # Parent lib
	PUBLIC ${libTIFF}
    PUBLIC Qt5::Xml
)
SET_TARGET_PROPERTIES(grid_tiff_backend PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
	POSITION_INDEPENDENT_CODE ON
)

ADD_LIBRARY(writergrid_tiff STATIC
	include/tiff_writer.hpp
	src/tiff_writer.cpp
	include/tiff_reader_templated.hpp
	include/tiff_reader_templated.impl.hpp
)
TARGET_INCLUDE_DIRECTORIES(writergrid_tiff
	PRIVATE ${libTIFF_HEADER_DIR}
	PUBLIC ${LIBS_COMPILED_HEADERS}
	# Also add GLM headers here !
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(writergrid_tiff
	PUBLIC writergrid # Parent lib
	PUBLIC ${libTIFF}
)
SET_TARGET_PROPERTIES(writergrid_tiff PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
	POSITION_INDEPENDENT_CODE ON
)


# Once the compilation is conditional, maybe add that lib to a list of compilable libs which
# then get linked by the top CMakeLists.txt ??? Just a random thought.
