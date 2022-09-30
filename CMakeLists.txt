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
FIND_PACKAGE(OpenMP REQUIRED)

# Find locally-compiled libraries :
# If any of them aren't found, it stops CMake's generation process with an error message.
FIND_PACKAGE(TinyTIFF REQUIRED PATHS ${CMAKE_CURRENT_LIST_DIR}/third_party/compiled_libraries/lib/cmake PATH_SUFFIXES TinyTIFF)
FIND_PACKAGE(nanoflann REQUIRED PATHS ${CMAKE_CURRENT_LIST_DIR}/third_party/compiled_libraries/lib/cmake PATH_SUFFIXES nanoflann)

ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/third_party/glm)
SET(LOCAL_GLM_HEADER_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/glm/")
SET(LIBS_COMPILED_HEADERS "${CMAKE_CURRENT_LIST_DIR}/third_party/compiled_libraries/include")

# To remove
ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/legacy/image)

ADD_LIBRARY(VisualisationWidgets STATIC
    ./qt/main_widget.hpp
    ./qt/main_widget.cpp

    ./qt/image3D_viewer.hpp
    ./qt/image3D_viewer.cpp

    ./qt/UI/chooser.hpp
    ./qt/UI/chooser.cpp

    ./qt/UI/form.hpp
    ./qt/UI/form.cpp

    ./qt/UI/deformation_form.hpp
    ./qt/UI/deformation_form.cpp

    ./qt/UI/color_control.hpp
    ./qt/UI/color_control.cpp

    ./qt/UI/color_button.hpp
    ./qt/UI/color_button.cpp

    ./qt/UI/quicksave_mesh.hpp
    ./qt/UI/quicksave_mesh.cpp

    ./qt/UI/info_pannel.hpp
    ./qt/UI/info_pannel.cpp

    ./qt/UI/open_image_form.hpp
    ./qt/UI/open_image_form.cpp

    ./qt/UI/save_image_form.hpp
    ./qt/UI/save_image_form.cpp

    ./qt/helper/QActionManager.hpp

    ./qt/UI/tool_pannel.hpp

    ./qt/UI/display_pannel.hpp

    ./qt/scene_control.hpp
    ./qt/scene_control.cpp

    ./qt/double_slider.hpp
    ./qt/double_slider.cpp

    ./qt/range_slider.hpp
    ./qt/range_slider.cpp

    ./qt/legacy/applyCageWidget.hpp
    ./qt/legacy/applyCageWidget.cpp

    ./qt/legacy/openMeshWidget.hpp
    ./qt/legacy/openMeshWidget.cpp

    ./qt/cutplane_groupbox.hpp
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
	# Header files, for Qt MOCS compilation :
    ./qt/scene.hpp
    # ./qt/planar_viewer.hpp

    ./qt/3D_viewer.hpp
    ./qt/3D_viewer.cpp

    ./qt/legacy/viewer_structs.hpp
    ./qt/legacy/viewer_structs.cpp

    ./third_party/primitive/Sphere.h
    ./third_party/cimg/CImg.h

    ./grid/geometry/grid.hpp
    ./grid/geometry/base_mesh.hpp
    ./grid/geometry/tetrahedral_mesh.hpp
    ./grid/geometry/surface_mesh.hpp
    ./grid/geometry/graph_mesh.hpp
    ./grid/images/tiff_image.hpp
    ./grid/images/cache.hpp
    ./grid/ui/manipulator.hpp
    ./grid/ui/mesh_manipulator.hpp
    ./grid/ui/kid_manipulator.h
    ./grid/drawable/drawable_surface_mesh.hpp
    ./grid/drawable/drawable_graph.hpp
    ./grid/drawable/drawable_manipulator.hpp
    ./grid/drawable/drawable_selection.hpp
    ./grid/deformation/mesh_deformer.hpp
    ./grid/deformation/AsRigidAsPossible.h
    ./grid/deformation/CageCoordinates.h
    ./grid/deformation/cage_surface_mesh.hpp
    ./grid/deformation/CellInfo.h
    ./grid/deformation/CholmodLSStruct.h
    ./grid/utils/GLUtilityMethods.h
    ./grid/utils/Edge.h
    ./grid/utils/Vec3D.h
    ./grid/utils/Triangle.h
    ./grid/utils/BasicPoint.h
    ./grid/utils/PCATools.h
    ./grid/utils/apss.hpp

	# Main file :
	./neighbor_visu_main.cpp

	# Source files :
    ./qt/scene.cpp
    # ./qt/planar_viewer.cpp
    ./third_party/primitive/Sphere.cpp
    ./grid/geometry/grid.cpp
    ./grid/geometry/base_mesh.cpp
    ./grid/geometry/tetrahedral_mesh.cpp
    ./grid/geometry/surface_mesh.cpp
    ./grid/geometry/graph_mesh.cpp
    ./grid/images/tiff_image.cpp
    ./grid/images/cache.cpp
    ./grid/ui/manipulator.cpp
    ./grid/ui/mesh_manipulator.cpp
    ./grid/drawable/drawable_surface_mesh.cpp
    ./grid/drawable/drawable_graph.cpp
    ./grid/drawable/drawable_manipulator.cpp
    ./grid/drawable/drawable_selection.cpp
    ./grid/deformation/mesh_deformer.cpp
    ./grid/deformation/AsRigidAsPossible.cpp
    ./grid/deformation/cage_surface_mesh.cpp
    ./grid/utils/GLUtilityMethods.cpp
    ./grid/utils/apss.cpp

    #To remove
    ./legacy/image/utils/include/image_api_common.hpp
    ./legacy/image/utils/include/bounding_box.hpp
    ./legacy/image/utils/include/threaded_task.hpp
    ./legacy/image/utils/src/threaded_task.cpp
    ./legacy/meshes/drawable/shaders.hpp
    ./legacy/meshes/drawable/shaders.cpp

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
    PRIVATE ${LOCAL_GLM_HEADER_DIR}
)

if (UNIX)
TARGET_LINK_LIBRARIES(neighbor_visu
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC ${OPENGL_LIBRARIES}
	PUBLIC ${QGLViewer}
	PUBLIC VisualisationWidgets
	PUBLIC TinyTIFF
	PUBLIC ${libTIFF}
    PUBLIC glm::glm
    PUBLIC ${GSL_LIBRARIES}
    PUBLIC ${SUITESPARSE_LIBRARIES}
    PUBLIC SuiteSparse
    PUBLIC OpenMP::OpenMP_CXX
)
endif (UNIX)

if (WIN32)
TARGET_LINK_LIBRARIES(neighbor_visu
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC ${OPENGL_LIBRARIES}
	PUBLIC ${QGLViewer}
	PUBLIC VisualisationWidgets
	PUBLIC TinyTIFF
	PUBLIC ${libTIFF}
    PUBLIC glm::glm
    PUBLIC ${GSL_LIBRARIES}
    PUBLIC ${SUITESPARSE_LIBRARIES}
    #PUBLIC SuiteSparse
    PUBLIC SuiteSparse::cholmod
    PUBLIC OpenMP::OpenMP_CXX
)
endif (WIN32)

#set(CMAKE_CXX_FLAGS " -isystem /home/thomas/includes")

option(RELEASE_WITH_ASSERT "Build a release version with assert" OFF)
if(RELEASE_WITH_ASSERT)
    message(STATUS "WARNING: Build in release using assertion")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif(RELEASE_WITH_ASSERT)

# Unit testing
#Include(FetchContent)
#
#FetchContent_Declare(
#  Catch2
#  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#  GIT_TAG        v3.0.0-preview3
#)
#
#FetchContent_MakeAvailable(Catch2)
#
#ADD_EXECUTABLE(tests
#
#    #./tests/ICP_tests.cpp
#
#    ./grid/geometry/grid.hpp
#    ./grid/geometry/base_mesh.hpp
#    ./grid/geometry/tetrahedral_mesh.hpp
#    ./grid/images/tiff_image.hpp
#    ./grid/images/cache.hpp
#    ./grid/ui/manipulator.hpp
#    ./grid/ui/mesh_manipulator.hpp
#    ./grid/drawable/drawable_surface_mesh.hpp
#    ./grid/deformation/mesh_deformator.hpp
#    ./grid/deformation/AsRigidAsPossible.h
#    ./grid/utils/Edge.h
#    ./grid/utils/Vec3D.h
#    ./grid/utils/Triangle.h
#
#    ./grid/geometry/grid.cpp
#    ./grid/geometry/base_mesh.cpp
#    ./grid/geometry/tetrahedral_mesh.cpp
#    ./grid/images/tiff_image.cpp
#    ./grid/images/cache.cpp
#    ./grid/ui/manipulator.cpp
#    ./grid/ui/mesh_manipulator.cpp
#    ./grid/drawable/drawable_surface_mesh.cpp
#    ./grid/deformation/mesh_deformator.cpp
#    ./grid/deformation/AsRigidAsPossible.cpp
#
#    #To remove
#    ./legacy/image/utils/include/image_api_common.hpp
#    ./legacy/image/utils/include/bounding_box.hpp
#    ./legacy/image/utils/include/threaded_task.hpp
#    ./legacy/image/utils/src/threaded_task.cpp
#    ./legacy/meshes/drawable/shaders.hpp
#    ./legacy/meshes/drawable/shaders.cpp
#
#    ./third_party/cimg/CImg.h
#)
#TARGET_COMPILE_OPTIONS(tests
#	PUBLIC "-Wno-deprecated-declarations"
#)
#TARGET_PRECOMPILE_HEADERS(tests
#	PRIVATE ${QGLViewer_HEADER_DIR}/QGLViewer/qglviewer.h
#)
#TARGET_INCLUDE_DIRECTORIES(tests
#	PRIVATE ${OPENGL_INCLUDE_DIRS}
#	PRIVATE ${QGLViewer_HEADER_DIR}
#	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third_party/glm/
#	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include
#	# Nifti uses brace-include without the 'nifti/' prefix, leading
#	# to manually have to include the lib's include dir in targets:
#	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include/nifti
#)
#TARGET_LINK_LIBRARIES(tests
#    PRIVATE Catch2::Catch2WithMain
#	PUBLIC Qt5::Core
#	PUBLIC Qt5::Gui
#	PUBLIC Qt5::Xml
#	PUBLIC Qt5::Widgets
#	PUBLIC Qt5::OpenGL
#	PUBLIC ${OPENGL_LIBRARIES}
#	PUBLIC ${QGLViewer}
#	PUBLIC VisualisationWidgets
#	PUBLIC TinyTIFF
#	PUBLIC ${libTIFF}
#    PUBLIC ${GSL_LIBRARIES}
#    PUBLIC ${SUITESPARSE_LIBRARIES}
#    PUBLIC SuiteSparse
#)
