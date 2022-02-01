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

FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml OpenGL)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

# Find locally-compiled libraries :
# If any of them aren't found, it stops CMake's generation process with an error message.
FIND_PACKAGE(TinyTIFF REQUIRED PATHS ${CMAKE_CURRENT_LIST_DIR}/third_party/compiled_libraries/lib/cmake PATH_SUFFIXES TinyTIFF)
FIND_PACKAGE(nanoflann REQUIRED PATHS ${CMAKE_CURRENT_LIST_DIR}/third_party/compiled_libraries/lib/cmake PATH_SUFFIXES nanoflann)

ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/third_party/glm)
SET(LOCAL_GLM_HEADER_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/glm/")
SET(LIBS_COMPILED_HEADERS "${CMAKE_CURRENT_LIST_DIR}/third_party/compiled_libraries/include")

ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/image)
ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/meshes)

ADD_LIBRARY(VisualisationWidgets STATIC
	./qt/include/neighbor_visu_main_widget.hpp
	./qt/include/scene_control.hpp
	./qt/include/viewer_header.hpp
	./qt/include/visu_box_controller.hpp
	./qt/include/opengl_debug_log.hpp
	./qt/include/double_slider.hpp
	./qt/include/range_slider.hpp
	./qt/include/user_settings_widget.hpp
	./qt/include/loader_widget.hpp
	./qt/include/deformation_widget.hpp

	./qt/src/neighbor_visu_main_widget.cpp
	./qt/src/scene_control.cpp
	./qt/src/viewer_header.cpp
	./qt/src/visu_box_controller.cpp
	./qt/src/opengl_debug_log.cpp
	./qt/src/double_slider.cpp
	./qt/src/range_slider.cpp
	./qt/src/user_settings_widget.cpp
	./qt/src/loader_widget.cpp
	./qt/src/deformation_widget.cpp
)
TARGET_INCLUDE_DIRECTORIES(VisualisationWidgets
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party/glm/
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include
	# Nifti uses brace-include without the 'nifti/' prefix, leading
	# to manually have to include the lib's include dir in targets:
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include/nifti
	PRIVATE ${QGLViewer_HEADER_DIR}
)
TARGET_LINK_LIBRARIES(VisualisationWidgets
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC Threads::Threads
)

ADD_EXECUTABLE(neighbor_visu
	./qt/include/dialog_pick_grids_from_scene.hpp
	# Header files, for Qt MOCS compilation :
	./viewer/include/scene.hpp
	./viewer/include/planar_viewer.hpp
	./viewer/include/neighbor_visu_viewer.hpp
	./viewer/include/viewer_structs.hpp
    ./third_party/primitive/Sphere.h
    ./qt/include/GLmanipulator.hpp
    ./qt/include/manipulator.hpp
    ./grid/include/grid.hpp
    ./grid/include/base_mesh.hpp
    ./grid/include/tetrahedral_mesh.hpp
    ./grid/include/mesh_deformator.hpp
    ./grid/include/tiff_image.hpp

    #Probably to remove
    ./image/transforms/include/affine_transform.hpp
    ./image/transforms/src/affine_transform.cpp
    ./image/transforms/include/transform_interface.hpp
    ./image/transforms/include/transform_stack.hpp
    ./image/transforms/src/transform_stack.cpp
    ./image/transforms/include/trs_transform.hpp
    ./image/transforms/src/trs_transform.cpp
    ./image/utils/include/image_api_common.hpp
    ./image/utils/include/bounding_box.hpp
    ./image/utils/include/threaded_task.hpp
    ./image/utils/src/threaded_task.cpp

	# Main file :
	./neighbor_visu_main.cpp

	# Source files :
	./viewer/src/scene.cpp
	./viewer/src/planar_viewer.cpp
	./viewer/src/neighbor_visu_viewer.cpp
	./viewer/src/viewer_structs.cpp
    ./third_party/primitive/Sphere.cpp
    ./qt/src/GLmanipulator.cpp
    ./qt/src/manipulator.cpp
    ./grid/src/grid.cpp
    ./grid/src/base_mesh.cpp
    ./grid/src/tetrahedral_mesh.cpp
    ./grid/src/mesh_deformator.cpp
    ./grid/src/tiff_image.cpp
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
)
TARGET_LINK_LIBRARIES(neighbor_visu
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC ${OPENGL_LIBRARIES}
	PUBLIC ${QGLViewer}
	PUBLIC VisualisationWidgets
	PUBLIC ${CMAKE_PROJECT_NAME}_shaders
	PUBLIC ${CMAKE_PROJECT_NAME}_mesh
	PUBLIC ${CMAKE_PROJECT_NAME}_deformable_curve
	PUBLIC ${CMAKE_PROJECT_NAME}_meshop_arap
	PUBLIC ${CMAKE_PROJECT_NAME}_drawable_meshes
	PUBLIC TinyTIFF
	PUBLIC ${libTIFF}
)

option(RELEASE_WITH_ASSERT "Build a release version with assert" OFF)
if(RELEASE_WITH_ASSERT)
    message(STATUS "WARNING: Build in release using assertion")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif(RELEASE_WITH_ASSERT)

# Unit testing
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.0-preview3
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests tests/grid_tests.cpp
                     tests/tetmesh_tests.cpp

                     grid/include/grid.hpp
                     grid/include/base_mesh.hpp
                     grid/include/tetrahedral_mesh.hpp
                     grid/include/mesh_deformator.hpp
                     grid/include/tiff_image.hpp

                     grid/src/grid.cpp
                     grid/src/base_mesh.cpp
                     grid/src/tetrahedral_mesh.cpp
                     grid/src/mesh_deformator.cpp
                     grid/src/tiff_image.cpp)

TARGET_INCLUDE_DIRECTORIES(tests
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party/glm/
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include/nifti
	PRIVATE ${QGLViewer_HEADER_DIR}
	PRIVATE ${libTIFF_HEADER_DIR}
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain
	                        PUBLIC TinyTIFF
	                        PUBLIC ${libTIFF}
                            )
