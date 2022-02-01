# ShaderDependency.cmake :
# 	Allows the GLSL shaders in the shaders folder
# 	to be copied over to the build directory whenever
# 	their contents change.

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/bounding_box.frag
	${SHADER_OUTPUT_DIR}/bounding_box.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/bounding_box.vert
	${SHADER_OUTPUT_DIR}/bounding_box.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/coloring.glsl
	${SHADER_OUTPUT_DIR}/coloring.glsl
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/colorize_new_flow.glsl
	${SHADER_OUTPUT_DIR}/colorize_new_flow.glsl
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/plane.frag
	${SHADER_OUTPUT_DIR}/plane.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/plane.geom
	${SHADER_OUTPUT_DIR}/plane.geom
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/plane.vert
	${SHADER_OUTPUT_DIR}/plane.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/texture_explorer.frag
	${SHADER_OUTPUT_DIR}/texture_explorer.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/texture_explorer.vert
	${SHADER_OUTPUT_DIR}/texture_explorer.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/transfer_mesh.frag
	${SHADER_OUTPUT_DIR}/transfer_mesh.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/transfer_mesh.geom
	${SHADER_OUTPUT_DIR}/transfer_mesh.geom
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/transfer_mesh.vert
	${SHADER_OUTPUT_DIR}/transfer_mesh.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/voxelgrid.frag
	${SHADER_OUTPUT_DIR}/voxelgrid.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/voxelgrid.geom
	${SHADER_OUTPUT_DIR}/voxelgrid.geom
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/voxelgrid.vert
	${SHADER_OUTPUT_DIR}/voxelgrid.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/sphere.vert
	${SHADER_OUTPUT_DIR}/sphere.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/sphere.frag
	${SHADER_OUTPUT_DIR}/sphere.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/base_mesh.vert
	${SHADER_OUTPUT_DIR}/base_mesh.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/base_mesh.frag
	${SHADER_OUTPUT_DIR}/base_mesh.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/base_sphere.vert
	${SHADER_OUTPUT_DIR}/base_sphere.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/base_sphere.frag
	${SHADER_OUTPUT_DIR}/base_sphere.frag
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/base_curve.vert
	${SHADER_OUTPUT_DIR}/base_curve.vert
	COPYONLY)

CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/shaders/base_curve.frag
	${SHADER_OUTPUT_DIR}/base_curve.frag
	COPYONLY)

