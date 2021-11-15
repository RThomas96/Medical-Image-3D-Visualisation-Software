# ShaderDependency.cmake :
# 	Allows the GLSL shaders in the new_shaders folder
# 	to be copied over to the build directory whenever
# 	their contents change.

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/bounding_box.frag
	${SHADER_OUTPUT_DIR}/bounding_box.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/bounding_box.vert
	${SHADER_OUTPUT_DIR}/bounding_box.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/coloring.glsl
	${SHADER_OUTPUT_DIR}/coloring.glsl
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/colorize_new_flow.glsl
	${SHADER_OUTPUT_DIR}/colorize_new_flow.glsl
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/plane.frag
	${SHADER_OUTPUT_DIR}/plane.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/plane.geom
	${SHADER_OUTPUT_DIR}/plane.geom
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/plane.vert
	${SHADER_OUTPUT_DIR}/plane.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/texture_explorer.frag
	${SHADER_OUTPUT_DIR}/texture_explorer.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/texture_explorer.vert
	${SHADER_OUTPUT_DIR}/texture_explorer.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/transfer_mesh.frag
	${SHADER_OUTPUT_DIR}/transfer_mesh.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/transfer_mesh.geom
	${SHADER_OUTPUT_DIR}/transfer_mesh.geom
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/transfer_mesh.vert
	${SHADER_OUTPUT_DIR}/transfer_mesh.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/voxelgrid.frag
	${SHADER_OUTPUT_DIR}/voxelgrid.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/voxelgrid.geom
	${SHADER_OUTPUT_DIR}/voxelgrid.geom
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/new_shaders/voxelgrid.vert
	${SHADER_OUTPUT_DIR}/voxelgrid.vert
	COPYONLY)

