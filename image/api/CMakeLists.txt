#
# CMake file compiling the grid API and the image backend interface
#

ADD_LIBRARY(imagegrid STATIC
	include/backend.hpp
	include/image_api_common.hpp
	include/read_cache.hpp
	include/threaded_task.hpp

	src/backend.cpp
)
TARGET_INCLUDE_DIRECTORIES(imagegrid
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
SET_TARGET_PROPERTIES(imagegrid PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
	POSITION_INDEPENDENT_CODE ON
)

ADD_LIBRARY(grid_api SHARED
	include/grid.hpp
	src/grid.cpp
	src/grid_creator.cpp
)
TARGET_LINK_LIBRARIES(grid_api
	PUBLIC grid_tiff_backend
	PUBLIC imagegrid
	PUBLIC transform_types
)
TARGET_INCLUDE_DIRECTORIES(grid_api
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
SET_TARGET_PROPERTIES(grid_api PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
)
