# ResourceDependency.cmake :
# 	Like the ShaderDependency.cmake file, but for the icons
# 	used in the program.

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/resources/cube.png
	${RESOURCE_OUTPUT_DIR}/cube.png
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/resources/eye_close.png
	${RESOURCE_OUTPUT_DIR}/eye_close.png
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/resources/eye_open.png
	${RESOURCE_OUTPUT_DIR}/eye_open.png
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/resources/invert.png
	${RESOURCE_OUTPUT_DIR}/invert.png
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/resources/label_2D.png
	${RESOURCE_OUTPUT_DIR}/label_2D.png
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/resources/label_3D.png
	${RESOURCE_OUTPUT_DIR}/label_3D.png
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/resources/label_3D_box.png
	${RESOURCE_OUTPUT_DIR}/label_3D_box.png
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/resources/rotate.png
	${RESOURCE_OUTPUT_DIR}/rotate.png
	COPYONLY)

