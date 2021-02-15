############################################################
############################################################
### libQGLViewer setup : Finding the shared library file ###
############################################################
############################################################

SET(QGLVIEWER_HEADER_NAME "qglviewer.h")
SET(QGLVIEWER_LIBRARY_NAME "libQGLViewer-qt5.so")
IF(WIN32 OR MINGW OR MSVC)
        SET(QGLVIEWER_LIBRARY_NAME "libQGLViewer2.a" "QGLViewer2.dll")
ENDIF()

SET(QGLViewer_LOCAL_DIR "${CMAKE_SOURCE_DIR}/libQGLViewer/QGLViewer")
# If installed as a system-wide shared library on Windows
SET(QGLViewer_WINDOWS_SEARCH_DIR "C:/Windows/System32")
# Ubuntu default library path :
SET(QGLViewer_UBUNTU_INSTALL_DIR "/usr/lib/x86_64-linux-gnu")
# When installed on Linux :
SET(QGLViewer_MAIN_DIR "/usr/local")
SET(QGLViewer_INCLUDE_DIR "${QGLViewer_MAIN_DIR}/include/QGLViewer")
SET(QGLViewer_LIB_DIR "${QGLViewer_MAIN_DIR}/lib")
MESSAGE(STATUS "Hint for QGLViewer is : ${QGLVIEWER_HINT}")

SET(QLGViewer_ALL_SEARCH_DIRS ${QGLViewer_LOCAL_DIR} ${QGLViewer_MAIN_DIR} ${QGLViewer_LIB_DIR} ${QGLViewer_UBUNTU_INSTALL_DIR} ${QGLViewer_WINDOWS_SEARCH_DIR})

SET(QGLViewer_DIR "${CMAKE_SOURCE_DIR}/libQGLViewer/QGLViewer")

IF(${QGLVIEWER_HINT})
        SET(QGLVIEWER_PARENT_HINT "")
	GET_FILENAME_COMPONENT(QGLVIEWER_PARENT_HINT ${QGLVIEWER_HINT} DIRECTORY)
	LIST(APPEND QLGViewer_ALL_SEARCH_DIRS ${QGLVIEWER_PARENT_HINT} ${QGLVIEWER_HINT})
	MESSAGE(STATUS "Looking for ${QGLVIEWER_LIBRARY_NAME} in the following locations :\n\t${QLGViewer_ALL_SEARCH_DIRS}")
ELSE()
        MESSAGE(STATUS "Looking for ${QGLVIEWER_LIBRARY_NAME} in the following locations :\n\t${QLGViewer_ALL_SEARCH_DIRS}")
ENDIF()

FIND_LIBRARY(QGLViewer
	NAMES ${QGLVIEWER_LIBRARY_NAME}
	PATHS ${CMAKE_SOURCE_DIR}/libQGLViewer
	PATH_SUFFIXES QGLViewer
	DOC "QGLViewer library location on disk."
	REQUIRED
	NO_DEFAULT_PATH)
IF(NOT EXISTS ${QGLViewer})
        MESSAGE(FATAL_ERROR "The QGLViewer library cannot be found. It either means the library has not been installed, or it has been installed to a non-standard location.\n"
	"If this is your case, then re-run this cmake command with the variable QGLVIEWER_HINT set to where libQGLViewer is installed.\n"
	"If you have not installed the library, go to http://www.libqglviewer.com and follow the installation instructions here.")
ELSE()
        MESSAGE(STATUS "Found libQGLViewer : ${QGLViewer}")
ENDIF()

FIND_PATH(QGLViewer_HEADER_DIR
	NAMES QGLViewer
	PATHS ${CMAKE_SOURCE_DIR}/libQGLViewer
	DOC "QGLViewer header location on disk."
	NO_DEFAULT_PATH)
IF(NOT EXISTS ${QGLViewer_HEADER_DIR})
        MESSAGE(FATAL_ERROR "The QGLViewer headers cannot be found. It either means the library has not been installed, or it has been installed to a non-standard location.\n"
	"If this is your case, then re-run this cmake command with the variable QGLVIEWER_HINT set to where libQGLViewer is installed.\n"
	"If you have not installed the library, go to http://www.libqglviewer.com and follow the installation instructions here.")
ELSE()
        MESSAGE(STATUS "Found libQGLViewer : ${QGLViewer_HEADER_DIR}")
ENDIF()
