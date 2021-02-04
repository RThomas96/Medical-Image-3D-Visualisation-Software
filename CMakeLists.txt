CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
PROJECT(visualisation)

# Append current cmake/ directory to CMake file paths :
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
# CMake Setup :
INCLUDE(CMakeSetup)
# Find libQGLViewer :
INCLUDE(FindQGLViewer)

FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml OpenGL)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
# Find TinyTIFF compiled locally
FIND_PACKAGE(TinyTIFF REQUIRED PATHS ${CMAKE_CURRENT_LIST_DIR}/lib/lib/cmake PATH_SUFFIXES TinyTIFF)

ADD_SUBDIRECTORY(glm)

# Neighbor visualisation (Test variant) {{{
ADD_EXECUTABLE(neighbor_visu
	# Header files, for Qt MOCS compilation :
	./qt/include/grid_control.hpp
	./qt/include/grid_detailed_view.hpp
	./qt/include/grid_list_view.hpp
	./qt/include/neighbor_visu_main_widget.hpp
	./qt/include/scene_control.hpp
	./qt/include/qt_proxies.hpp
	./qt/include/viewer_header.hpp
	./qt/include/visu_box_controller.hpp
	./qt/include/opengl_debug_log.hpp
	./qt/include/user_settings_widget.hpp
	./qt/include/loader_widget.hpp

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
	./qt/src/grid_control.cpp
	./qt/src/grid_detailed_view.cpp
	./qt/src/grid_list_view.cpp
	./qt/src/neighbor_visu_main_widget.cpp
	./qt/src/scene_control.cpp
	./qt/src/qt_proxies.cpp
	./qt/src/viewer_header.cpp
	./qt/src/visu_box_controller.cpp
	./qt/src/opengl_debug_log.cpp
	./qt/src/user_settings_widget.cpp
	./qt/src/loader_widget.cpp

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
TARGET_INCLUDE_DIRECTORIES(neighbor_visu
	PRIVATE ${OPENGL_INCLUDE_DIRS}
	PRIVATE ${QGLViewer_HEADER_DIR}
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/glm/
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/include
)
TARGET_LINK_LIBRARIES(neighbor_visu
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC ${OPENGL_LIBRARIES}
	PUBLIC ${QGLViewer}
	PUBLIC TinyTIFF
)
# }}}

# vim: foldmethod=marker : foldmarker={{{,}}} : tabstop=8
