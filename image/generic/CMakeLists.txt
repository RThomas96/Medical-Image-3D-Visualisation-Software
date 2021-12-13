#
# CMake targets for the generic image readers.
#

##############################################################
# Generic :
ADD_LIBRARY(imagegrid STATIC
	include/generic_image_reader.hpp
	#src/generic_image_reader.cpp
)
TARGET_INCLUDE_DIRECTORIES(imagegrid
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(imagegrid
	PUBLIC imagegrid_utils
)
SET_TARGET_PROPERTIES(imagegrid PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
	POSITION_INDEPENDENT_CODE ON
)

##############################################################
# Sub-regions :
ADD_LIBRARY(image_subregion
	include/generic_image_reader_subregion.hpp
	include/generic_image_reader_subregion_templated.hpp
	include/generic_image_reader_subregion_templated.impl.hpp
	src/generic_image_reader_subregion.cpp
)
TARGET_INCLUDE_DIRECTORIES(image_subregion
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(image_subregion
	PUBLIC imagegrid
	PUBLIC imagegrid_utils
)
SET_TARGET_PROPERTIES(image_subregion PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
	POSITION_INDEPENDENT_CODE ON
)

##############################################################
# Downsamplers :
ADD_LIBRARY(image_downsampler
	include/generic_image_downsampler.hpp
	include/generic_image_interpolators.hpp
	include/generic_image_interpolators.impl.hpp
	include/generic_image_downsampler_templated.hpp
	include/generic_image_downsampler_templated.impl.hpp
	src/generic_image_downsampler.cpp
	#include/generic_image_downsampler_templated.hpp
	#include/generic_image_downsampler_templated.impl.hpp
	#src/generic_image_reader_subregion.cpp
)
TARGET_INCLUDE_DIRECTORIES(image_downsampler
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(image_downsampler
	PUBLIC imagegrid
	PUBLIC imagegrid_utils
)
SET_TARGET_PROPERTIES(image_downsampler PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
	POSITION_INDEPENDENT_CODE ON
)
