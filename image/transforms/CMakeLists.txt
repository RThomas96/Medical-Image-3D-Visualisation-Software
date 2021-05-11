#
# CMake target(s) for the transform types library
#

ADD_LIBRARY(transform_types STATIC
	# Headers (included to have them as dependencies in make/ninja) :
	./include/transform_interface.hpp
	./include/affine_transform.hpp
	./include/trs_transform.hpp

	./include/transform_stack.hpp

	# Sources :
	./src/affine_transform.cpp
	./src/trs_transform.cpp

	./src/transform_stack.cpp
)
TARGET_INCLUDE_DIRECTORIES(transform_types
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
SET_TARGET_PROPERTIES(transform_types PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
	POSITION_INDEPENDENT_CODE ON
)
