CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(visualisation)

# CMake Setup :
INCLUDE(./cmake/CMakeSetup.cmake)
# Find libQGLViewer :
INCLUDE(./cmake/FindQGLViewer.cmake)

FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml OpenGL)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(glm REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

SET(INCLUDE_DIRECTORIES
	${INCLUDE_DIRECTORIES}
	${OPENGL_INCLUDE_DIRS}
)
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES})

SET(GLHANDLER_USE_QT ON CACHE BOOL "Compile GLHandler with Qt")
ADD_SUBDIRECTORY(gl)

# TinyTIFF Library {{{
ADD_LIBRARY(libtiff STATIC
	./TinyTIFF/tinytiffreader.cpp
	./TinyTIFF/tinytiffwriter.cpp
	./TinyTIFF/tinytiffhighrestimer.cpp
)
SET_TARGET_PROPERTIES(libtiff PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../lib/
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../lib/
)
# TinyTIFF doesn't need to be compiled with Qt :
SET_TARGET_PROPERTIES(libtiff PROPERTIES AUTOMOC OFF AUTORCC OFF AUTOUIC OFF)
# Handy variable to refer to the target :
SET(TIF_LIB libtiff)
# }}}

# Neighbor visualisation (Test variant) {{{
ADD_EXECUTABLE(neighbor_visu
	# Header files, for Qt MOCS compilation :
	./qt/include/control_panel.hpp
	./qt/include/grid_control.hpp
	./qt/include/grid_detailed_view.hpp
	./qt/include/grid_list_view.hpp
	./qt/include/neighbor_visu_main_widget.hpp
	./qt/include/scene_control.hpp
	./qt/include/qt_proxies.hpp
	./qt/include/viewer_header.hpp

	./viewer/include/scene.hpp
	./viewer/include/planar_viewer.hpp
	./viewer/include/neighbor_visu_viewer.hpp
	./viewer/include/viewer_structs.hpp

	./grid/include/tetmesh.hpp
	./grid/include/discrete_grid.hpp
	./grid/include/input_discrete_grid.hpp
	./grid/include/output_discrete_grid.hpp

	./image/include/reader.hpp
	./image/include/writer.hpp

	# Main file :
	./neighbor_visu_main.cpp

	# Source files :
	./qt/src/control_panel.cpp
	./qt/src/grid_control.cpp
	./qt/src/grid_detailed_view.cpp
	./qt/src/grid_list_view.cpp
	./qt/src/neighbor_visu_main_widget.cpp
	./qt/src/scene_control.cpp
	./qt/src/qt_proxies.cpp
	./qt/src/viewer_header.cpp

	./viewer/src/scene.cpp
	./viewer/src/planar_viewer.cpp
	./viewer/src/neighbor_visu_viewer.cpp
	./viewer/src/viewer_structs.cpp

	./grid/src/tetmesh.cpp
	./grid/src/discrete_grid.cpp
	./grid/src/input_discrete_grid.cpp
	./grid/src/output_discrete_grid.cpp

	./image/src/reader.cpp
	./image/src/writer.cpp
)
TARGET_LINK_LIBRARIES(neighbor_visu
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC OpenGL
	PUBLIC ${QGLViewer_LIBRARY}
	PUBLIC GLEW
	PUBLIC glHandler
	PUBLIC ${TIF_LIB}
)
# }}}

# vim: foldmethod=marker : foldmarker={{{,}}} : tabstop=8
