CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(visualisation)

# CMake Setup :
INCLUDE(./cmake/CMakeSetup.cmake)
# Find libQGLViewer :
INCLUDE(./cmake/FindQGLViewer.cmake)

FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets Multimedia Xml OpenGL)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(glm REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

SET(INCLUDE_DIRECTORIES
	${INCLUDE_DIRECTORIES}
	${OPENGL_INCLUDE_DIRS}
)
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES})

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

# TinyTIFF library tests {{{
ADD_EXECUTABLE(tiff_loader_tests
	# Header files (needed for Qt's MOCS compilation) :
	./tests/tiff_tests/include/main_window.hpp
	./tests/tiff_tests/include/tiff_inspector.hpp
	./tests/tiff_tests/include/tiff_displayer.hpp

	# Main file :
	./tests/tiff_tests/main.cpp

	# Implementation files :
	./tests/tiff_tests/src/main_window.cpp
	./tests/tiff_tests/src/tiff_inspector.cpp
	./tests/tiff_tests/src/tiff_displayer.cpp
)
TARGET_LINK_LIBRARIES(tiff_loader_tests
	PUBLIC ${TIF_LIB}
	PUBLIC Qt5::Core
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::Gui
)
# }}}

# 3D Texture tests for viewer {{{
ADD_EXECUTABLE(3d_texture_test
	# Header files (needed for Qt's MOCS compilation) :
	./tests/3D_texture_test/include/image_stack_loader.hpp
	./tests/3D_texture_test/include/3D_texture_viewer.hpp
	./tests/3D_texture_test/include/container_widget.hpp
	./tests/3D_texture_test/include/slider_widget.hpp

	# Main file :
	./tests/3D_texture_test/main.cpp

	# Implementation files :
	./tests/3D_texture_test/src/image_stack_loader.cpp
	./tests/3D_texture_test/src/3D_texture_viewer.cpp
	./tests/3D_texture_test/src/slider_widget.cpp
)
TARGET_LINK_LIBRARIES(3d_texture_test
	PUBLIC ${TIF_LIB}
	PUBLIC Qt5::Core
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::Gui
	# The following are __ALL__ needed by libQGLViewer. For some f*ing reason. Don't touch them.
	PUBLIC Qt5::Xml
	PUBLIC Qt5::OpenGL
	PUBLIC OpenGL
	PUBLIC ${QGLViewer_LIBRARY}
)
# }}}

# Main executable {{{
ADD_EXECUTABLE(visualisation
	# Header files (needed for Qt's MOCS compilation) :
	./qt/include/main_widget.hpp
	./viewer/include/texture_viewer.hpp
	./image/include/bulk_texture_loader.hpp
	./image/include/image_loader.hpp
	./qt/include/texture_sliders.hpp
	./qt/include/texture_control.hpp

	# Implementation files :
	./qt/src/main_widget.cpp
	./viewer/src/texture_viewer.cpp
	./image/src/bulk_texture_loader.cpp
	./image/src/image_loader.cpp
	./qt/src/texture_sliders.cpp
	./qt/src/texture_control.cpp

	# Main file :
	./main.cpp
)
TARGET_LINK_LIBRARIES(visualisation
	PUBLIC ${TIF_LIB}
	PUBLIC Threads::Threads
	PUBLIC Qt5::Core
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::Gui
	# The following are __ALL__ needed by libQGLViewer. For some f*ing reason. Don't touch them.
	PUBLIC Qt5::Xml
	PUBLIC Qt5::OpenGL
	PUBLIC OpenGL
	PUBLIC ${QGLViewer_LIBRARY}
)
# }}}

ADD_EXECUTABLE(neighbor_visu
	# Header files, for Qt MOCS compilation :
	./tests/neighbor_visu/include/control_panel.hpp

	# Main file :
	./tests/neighbor_visu/main.cpp

	# Source files :
	./tests/neighbor_visu/src/control_panel.cpp
)
TARGET_LINK_LIBRARIES(neighbor_visu
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC OpenGL
	PUBLIC ${QGLViewer_LIBRARY}
)

# vim: foldmethod=marker : foldmarker={{{,}}} : tabstop=8
