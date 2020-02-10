CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
PROJECT(visualisation)

# CMake Setup (compile_commands, debug mode check, and build directory check) {{{
IF(WIN32 OR MINGW OR MSVC)
        MESSAGE(FATAL_ERROR "This project cannot be compiled on a Windows platform.")
ENDIF()

IF(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
	MESSAGE(FATAL_ERROR "Please select another Build Directory !")
ENDIF()

SET(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)
SET(CMAKE_USE_RELATIVE_PATHS TRUE)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
        MESSAGE("Currently compiling in Debug mode.")
	SET(GCC_COMPILE_FLAGS "-ggdb --pedantic")
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

# libQGLViewer Setup {{{
IF (EXISTS "/home/thibault/.config/awesome/rc.lua")
	SET(QGLViewer_MAIN_DIR "/usr/local")
	SET(QGLViewer_INCLUDE_DIR "${QGLViewer_MAIN_DIR}/include/QGLViewer")
	SET(QGLViewer_LIB_DIR "${QGLViewer_MAIN_DIR}/lib")
	FIND_LIBRARY(QGLViewer_LIBRARY
		NAMES libQGLViewer-qt5.so
		PATHS ${QGLViewer_MAIN_DIR} ${QGLViewer_LIB_DIR}
		NO_DEFAULT_PATH)
	IF(NOT QGLViewer_LIBRARY OR NOT EXISTS ${QGLViewer_LIBRARY})
		MESSAGE(FATAL "Could not find QGLViewer library, set the path to a *.so file")
	ENDIF()
ELSE()
	MESSAGE(FATAL "Warning : did not detect we were on the default system. As such, we could not infer
		the location of the libQGLViewer file, needed to compile the project. Please contact me at:
		thibault.de-villele@etu.umontpellier.fr and we'll try to sort your problem out.
		ERROR MESSAGE : C_GLV_NOT_FOUND")
ENDIF()

# }}}

SET(Qt5_DIR "/usr/lib/cmake/Qt5/")

FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets Multimedia Xml OpenGL)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(glm REQUIRED)
# }}}

SET(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${OPENGL_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES})

ADD_EXECUTABLE(visualisation
	./qt/include/main_window.hpp
	./viewer/include/simple_viewer.hpp

	./main.cpp

	./qt/src/main_window.cpp
	./viewer/src/simple_viewer.cpp
)
TARGET_LINK_LIBRARIES(visualisation
	PUBLIC Qt5::Core
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::Gui
	# The following are __ALL__ needed by libQGLViewer. For some f*ing reason. Don't touch them.
	PUBLIC Qt5::Xml
	PUBLIC Qt5::OpenGL
	PUBLIC OpenGL
	PUBLIC ${QGLViewer_LIBRARY}
)

# vim: foldmethod=marker : foldmarker={{{,}}} : tabstop=8
