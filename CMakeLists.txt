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
ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/src/legacy/image)

ADD_LIBRARY(VisualisationWidgets STATIC
    ./src/qt/main_widget.hpp
    ./src/qt/main_widget.cpp

    ./src/qt/image3D_viewer.hpp
    ./src/qt/image3D_viewer.cpp

    ./src/qt/UI/chooser.hpp
    ./src/qt/UI/chooser.cpp

    ./src/qt/UI/form.hpp
    ./src/qt/UI/form.cpp

    ./src/qt/UI/deformation_form.hpp
    ./src/qt/UI/deformation_form.cpp

    ./src/qt/UI/color_control.hpp
    ./src/qt/UI/color_control.cpp

    ./src/qt/UI/color_button.hpp
    ./src/qt/UI/color_button.cpp

    ./src/qt/UI/quicksave_mesh.hpp
    ./src/qt/UI/quicksave_mesh.cpp

    ./src/qt/UI/info_pannel.hpp
    ./src/qt/UI/info_pannel.cpp

    ./src/qt/UI/open_image_form.hpp
    ./src/qt/UI/open_image_form.cpp

    ./src/qt/UI/save_image_form.hpp
    ./src/qt/UI/save_image_form.cpp

    ./src/qt/helper/QActionManager.hpp

    ./src/qt/UI/tool_pannel.hpp

    ./src/qt/UI/display_pannel.hpp

    ./src/qt/scene_control.hpp
    ./src/qt/scene_control.cpp

    ./src/qt/double_slider.hpp
    ./src/qt/double_slider.cpp

    ./src/qt/range_slider.hpp
    ./src/qt/range_slider.cpp

    ./src/qt/legacy/applyCageWidget.hpp
    ./src/qt/legacy/applyCageWidget.cpp

    ./src/qt/legacy/openMeshWidget.hpp
    ./src/qt/legacy/openMeshWidget.cpp

    ./src/qt/cutplane_groupbox.hpp
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
    ./src/qt/scene.hpp
    # src/./qt/planar_viewer.hpp

    ./src/qt/3D_viewer.hpp
    ./src/qt/3D_viewer.cpp

    ./src/qt/legacy/viewer_structs.hpp
    ./src/qt/legacy/viewer_structs.cpp

    ./third_party/primitive/Sphere.h
    ./third_party/cimg/CImg.h

    ./src/core/geometry/grid.hpp
    ./src/core/geometry/base_mesh.hpp
    ./src/core/geometry/tetrahedral_mesh.hpp
    ./src/core/geometry/surface_mesh.hpp
    ./src/core/geometry/graph_mesh.hpp
    ./src/core/images/tiff_image.hpp
    ./src/core/images/cache.hpp
    ./src/core/interaction/manipulator.hpp
    ./src/core/interaction/mesh_manipulator.hpp
    ./src/core/interaction/kid_manipulator.h
    ./src/core/drawable/drawable_surface_mesh.hpp
    ./src/core/drawable/drawable.hpp
    #.src//core/drawable/drawable_manipulator.hpp
    ./src/core/drawable/drawable_selection.hpp
    ./src/core/drawable/drawable_grid.hpp
    #.src//core/deformation/mesh_deformer.hpp
    ./src/core/deformation/AsRigidAsPossible.h
    ./src/core/deformation/CageCoordinates.h
    ./src/core/deformation/cage_surface_mesh.hpp
    ./src/core/deformation/CellInfo.h
    ./src/core/deformation/CholmodLSStruct.h
    ./src/core/utils/GLUtilityMethods.h
    ./src/core/utils/Edge.h
    ./src/core/utils/Vec3D.h
    ./src/core/utils/Triangle.h
    ./src/core/utils/BasicPoint.h
    ./src/core/utils/PCATools.h
    ./src/core/utils/apss.hpp

    # Maisrc/n file :
    ./neighbor_visu_main.cpp

    # Sousrc/rce files :
    ./src/qt/scene.cpp
    # src/./qt/planar_viewer.cpp
    ./third_party/primitive/Sphere.cpp
    ./src/core/geometry/grid.cpp
    ./src/core/geometry/base_mesh.cpp
    ./src/core/geometry/tetrahedral_mesh.cpp
    ./src/core/geometry/surface_mesh.cpp
    ./src/core/geometry/graph_mesh.cpp
    ./src/core/images/tiff_image.cpp
    ./src/core/images/cache.cpp
    ./src/core/interaction/manipulator.cpp
    ./src/core/interaction/mesh_manipulator.cpp
    ./src/core/drawable/drawable_surface_mesh.cpp
    ./src/core/drawable/drawable.cpp
    #.src//core/drawable/drawable_manipulator.cpp
    ./src/core/drawable/drawable_selection.cpp
    ./src/core/drawable/drawable_grid.cpp
    #.src//core/deformation/mesh_deformer.cpp
    ./src/core/deformation/AsRigidAsPossible.cpp
    ./src/core/deformation/cage_surface_mesh.cpp
    ./src/core/utils/GLUtilityMethods.cpp
    ./src/core/utils/apss.cpp

    #To remove
    ./src/legacy/image/utils/include/image_api_common.hpp
    ./src/legacy/image/utils/include/bounding_box.hpp
    ./src/legacy/image/utils/include/threaded_task.hpp
    ./src/legacy/image/utils/src/threaded_task.cpp
    ./src/legacy/meshes/drawable/shaders.hpp
    ./src/legacy/meshes/drawable/shaders.cpp

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
