CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
PROJECT(visualisation)

ADD_COMPILE_DEFINITIONS(NEED_ARAP)

# Append current cmake/ directory to CMake file paths :
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
# CMake Setup :
INCLUDE(CMakeSetup)
# Find libQGLViewer :
INCLUDE(FindQGLViewer)
# Find the local libTIFF version :
INCLUDE(FindLocalTIFF)
# Add the GLSL and icons dependencies :
INCLUDE(ShaderDependency)
INCLUDE(ResourceDependency)
# Find the local SuiteSparse and GSL libraries : (windows-only for now)
IF (WIN32 OR MINGW OR MSVC)
	INCLUDE(FindSuiteSparse_Windows)
ELSE()
	INCLUDE(FindSuiteSparse_Linux)
ENDIF()

# Required libraries {{{
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml OpenGL)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

# Find locally-compiled libraries :
# If any of them aren't found, it stops CMake's generation process with an error message.
FIND_PACKAGE(TinyTIFF REQUIRED PATHS ${CMAKE_CURRENT_LIST_DIR}/third_party/compiled_libraries/lib/cmake PATH_SUFFIXES TinyTIFF)
FIND_PACKAGE(nanoflann REQUIRED PATHS ${CMAKE_CURRENT_LIST_DIR}/third_party/compiled_libraries/lib/cmake PATH_SUFFIXES nanoflann)
# }}}

ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/third_party/glm)
SET(LOCAL_GLM_HEADER_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/glm/")
SET(LIBS_COMPILED_HEADERS "${CMAKE_CURRENT_LIST_DIR}/third_party/compiled_libraries/include")

ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/image)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/new_grid)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/colorscale)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/meshes)

# Library for image reading/writing {{{
ADD_LIBRARY(ImageIO STATIC
	./grid/include/discrete_grid_reader.hpp
	./grid/include/discrete_grid_writer.hpp

	./grid/src/discrete_grid_reader.cpp
	./grid/src/discrete_grid_writer.cpp
)
TARGET_INCLUDE_DIRECTORIES(ImageIO
	PRIVATE ${LOCAL_GLM_HEADER_DIR}
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include
	# Nifti uses brace-include without the 'nifti/' prefix, leading
	# to manually have to include the lib's include dir in targets:
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include/nifti
	PRIVATE ${libTIFF_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(ImageIO
	PUBLIC TinyTIFF
	PUBLIC grid_api
	PUBLIC ${libTIFF}
	PUBLIC Qt5::Xml
)
# }}}

# Library for DiscreteGrid and TetMesh {{{
ADD_LIBRARY(DiscreteGrid STATIC
	# Headers :
	./grid/include/tetmesh.hpp
	./grid/include/discrete_grid.hpp
	./grid/include/input_discrete_grid.hpp
	./grid/include/output_discrete_grid.hpp
	# Sources :
	./grid/src/tetmesh.cpp
	./grid/src/discrete_grid.cpp
	./grid/src/input_discrete_grid.cpp
	./grid/src/output_discrete_grid.cpp
	meshes/deformable_curve/curve.hpp)
TARGET_INCLUDE_DIRECTORIES(DiscreteGrid
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party/glm/
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include
	# Nifti uses brace-include without the 'nifti/' prefix, leading
	# to manually have to include the lib's include dir in targets:
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include/nifti
)
TARGET_LINK_LIBRARIES(DiscreteGrid
	PUBLIC ImageIO
)
SET_TARGET_PROPERTIES(DiscreteGrid PROPERTIES AUTOMOC OFF AUTORCC OFF AUTOUIC OFF)
# }}}

# All the needed widgets for this project, under one target {{{
ADD_LIBRARY(VisualisationWidgets STATIC
	./qt/include/grid_control.hpp
	./qt/include/viewer_helper.hpp
	./qt/include/grid_detailed_view.hpp
	./qt/include/grid_list_view.hpp
	./qt/include/neighbor_visu_main_widget.hpp
	./qt/include/qt_proxies.hpp
	./qt/include/visu_box_controller.hpp
	./qt/include/opengl_debug_log.hpp
	./qt/include/user_settings_widget.hpp

	./qt/src/grid_control.cpp
	./qt/src/grid_detailed_view.cpp
	./qt/src/grid_list_view.cpp
	./qt/src/neighbor_visu_main_widget.cpp
	./qt/src/qt_proxies.cpp
	./qt/src/visu_box_controller.cpp
	./qt/src/opengl_debug_log.cpp
	./qt/src/user_settings_widget.cpp
	./qt/src/viewer_helper.cpp
)
TARGET_INCLUDE_DIRECTORIES(VisualisationWidgets
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party/glm/
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include
	# Nifti uses brace-include without the 'nifti/' prefix, leading
	# to manually have to include the lib's include dir in targets:
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include/nifti
	PRIVATE ${QGLViewer_HEADER_DIR}
	PRIVATE ${GSL_INCLUDE_DIRS}
)
TARGET_LINK_LIBRARIES(VisualisationWidgets
	PUBLIC ImageIO
	PUBLIC DiscreteGrid
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC Threads::Threads
)
# }}}

# Neighbor visualisation {{{
ADD_EXECUTABLE(neighbor_visu
	./qt/include/dialog_pick_grids_from_scene.hpp
	# Header files, for Qt MOCS compilation :
	./viewer/include/scene.hpp
	./viewer/include/planar_viewer.hpp
	./viewer/include/neighbor_visu_viewer.hpp
	./viewer/include/viewer_structs.hpp
	./third_party/primitive/Sphere.h
	./qt/include/GLmanipulator.hpp
	./qt/include/scene_control.hpp
	./qt/include/manipulator.hpp
	./qt/include/loader_widget.hpp
	./qt/include/double_slider.hpp
	./qt/include/range_slider.hpp
	./qt/include/viewer_header.hpp

	# Main file :
	./neighbor_visu_main.cpp

	# Source files :
	./viewer/src/scene.cpp
	./viewer/src/planar_viewer.cpp
	./viewer/src/neighbor_visu_viewer.cpp
	./viewer/src/viewer_structs.cpp
	./qt/src/scene_control.cpp
	./third_party/primitive/Sphere.cpp
	./qt/src/GLmanipulator.cpp
	./qt/src/manipulator.cpp
	./qt/src/loader_widget.cpp
	./qt/src/double_slider.cpp
	./qt/src/range_slider.cpp
	./qt/src/viewer_header.cpp
)
TARGET_COMPILE_OPTIONS(neighbor_visu
	# Set so CMake (more precisely clang/gcc) will not print 30-80 warnings about QGLViewer's
	# use of QString::null, a deprecated feature (causes long compilation times, and tons of
	# warnings which make it harder to filter through our own warnings/errors).
	PUBLIC "-Wno-deprecated-declarations"
)
TARGET_PRECOMPILE_HEADERS(neighbor_visu
	# Made so that QtCreator will precompile this header and
	# shut up about the deprecated functions used within :
	PRIVATE ${QGLViewer_HEADER_DIR}/QGLViewer/qglviewer.h
	# Also, dramatically reduces compilation time.
)
TARGET_INCLUDE_DIRECTORIES(neighbor_visu
	PRIVATE ${OPENGL_INCLUDE_DIRS}
	PRIVATE ${QGLViewer_HEADER_DIR}
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party/glm/
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include
	# Nifti uses brace-include without the 'nifti/' prefix, leading
	# to manually have to include the lib's include dir in targets:
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include/nifti
	PRIVATE ${GSL_INCLUDE_DIRS}
)
TARGET_LINK_LIBRARIES(neighbor_visu
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC ${OPENGL_LIBRARIES}
	PUBLIC ${QGLViewer}
	PUBLIC ImageIO
	PUBLIC DiscreteGrid
	PUBLIC VisualisationWidgets
	PUBLIC writergrid
	PUBLIC writergrid_tiff
	PUBLIC ${CMAKE_PROJECT_NAME}_shaders
	PUBLIC ${CMAKE_PROJECT_NAME}_mesh
	PUBLIC ${CMAKE_PROJECT_NAME}_deformable_curve
	PUBLIC ${CMAKE_PROJECT_NAME}_meshop_arap
	PUBLIC ${CMAKE_PROJECT_NAME}_drawable_meshes
)
# }}}

# ARAP deformation {{{
ADD_SUBDIRECTORY(executables/arap_deformation)
# }}}

# vim: foldmethod=marker : foldmarker={{{,}}} : tabstop=8

option(RELEASE_WITH_ASSERT "Build a release version with assert" OFF)
if(RELEASE_WITH_ASSERT)
	message(STATUS "WARNING: Build in release using assertion")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif(RELEASE_WITH_ASSERT)
