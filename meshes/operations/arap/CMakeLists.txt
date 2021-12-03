# ARAP Target, needs a lot of libraries like GSL, cholmod (...)

ADD_LIBRARY(${CMAKE_PROJECT_NAME}_meshop_arap
	AsRigidAsPossible.h
	AsRigidAsPossible.cpp
	Edge.h
)
TARGET_INCLUDE_DIRECTORIES(${CMAKE_PROJECT_NAME}_meshop_arap
	PRIVATE ${OPENGL_INCLUDE_DIRS}
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}_meshop_arap
	PRIVATE ${SuiteSparse_LIBRARIES}
	PRIVATE ${LOCAL_GSL}
	PRIVATE ${LOCAL_GSLCBLAS}
	PUBLIC Qt5::OpenGL
	PUBLIC glm::glm
)