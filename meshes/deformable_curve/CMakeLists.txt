# Curve class : the deformable type (with a Mesh cage)

ADD_LIBRARY(${CMAKE_PROJECT_NAME}_deformable_curve
	curve.hpp
	curve.cpp
	CageCoordinates.h
	obj_loader.hpp
)
TARGET_INCLUDE_DIRECTORIES(${CMAKE_PROJECT_NAME}_deformable_curve
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}_deformable_curve
	PUBLIC glm::glm
	PUBLIC ${CMAKE_PROJECT_NAME}_mesh
)
