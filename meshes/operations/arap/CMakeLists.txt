# ARAP Target, needs a lot of libraries like GSL, cholmod (...)

ADD_LIBRARY(${CMAKE_PROJECT_NAME}_meshop_arap
	AsRigidAsPossible.h
	AsRigidAsPossible.cpp
	Edge.h
	Manipulator.h
	manipulator.cpp
	mesh_manip_interface.h
	PCATools.h
	RectangleSelection.h
)
TARGET_INCLUDE_DIRECTORIES(${CMAKE_PROJECT_NAME}_meshop_arap
	PRIVATE ${OPENGL_INCLUDE_DIRS}
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
	PRIVATE ${GSL_INCLUDE_DIRS}
	PRIVATE ${QGLViewer_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}_meshop_arap
	PUBLIC Qt5::OpenGL
	PUBLIC glm::glm
	PUBLIC ${QGLViewer}
	PUBLIC Qt5::Xml
)

IF (WIN32 OR MINGW OR MSVC)
    TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}_meshop_arap
        PUBLIC ${SuiteSparse_LIBRARIES}
        PUBLIC GSL::gsl
        PUBLIC GSL::gslcblas
    )
ELSE()
    TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}_meshop_arap
        PUBLIC ${GSL_LIBRARIES}
        INTERFACE ${SUITESPARSE_LIBRARIES}
        INTERFACE SuiteSparse
    )
ENDIF()
