#
# CMake file compiling the grid API and the image backend interface
#

ADD_LIBRARY(imagegrid SHARED
	include/backend.hpp
	include/grid.hpp
	include/image_api_common.hpp
	include/read_cache.hpp
	include/threaded_task.hpp

	src/backend.cpp
	src/grid.cpp
	src/grid_creator.cpp
)
#TARGET_LINK_LIBRARIES(imagegrid)
TARGET_INCLUDE_DIRECTORIES(imagegrid
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
