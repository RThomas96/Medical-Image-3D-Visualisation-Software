#
# CMake file compiling the grid API and the image backend interface
#

ADD_LIBRARY(imagegrid_utils STATIC
	include/image_api_common.hpp
	include/read_cache.hpp
	include/threaded_task.hpp

	src/threaded_task.cpp
)
TARGET_INCLUDE_DIRECTORIES(imagegrid_utils
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
SET_TARGET_PROPERTIES(imagegrid_utils PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
	POSITION_INDEPENDENT_CODE ON
)
