#
# CMake file compiling the grid API and the image backend interface
#

ADD_LIBRARY(grid_api SHARED
	include/grid.hpp
	src/grid.cpp
	src/grid_creator.cpp
)
TARGET_LINK_LIBRARIES(grid_api
	PUBLIC grid_tiff_backend
	PUBLIC imagegrid
	PUBLIC transform_types
	PUBLIC writergrid
	PUBLIC writergrid_tiff
)
TARGET_INCLUDE_DIRECTORIES(grid_api
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
SET_TARGET_PROPERTIES(grid_api PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
)
