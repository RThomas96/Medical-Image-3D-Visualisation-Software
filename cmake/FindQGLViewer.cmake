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

FIND_LIBRARY(QGLViewer
	NAMES ${QGLVIEWER_LIBRARY_NAME}
	PATHS ${LOCAL_COMPILED_LIBS_PATH}/lib
	DOC "QGLViewer library location on disk."
	REQUIRED
	NO_DEFAULT_PATH)
IF(NOT EXISTS ${QGLViewer})
	MESSAGE(FATAL_ERROR "The QGLViewer library cannot be found. Have you configured third-party libraries in ./third_party ?")
ELSE()
	MESSAGE(STATUS "Found libQGLViewer : ${QGLViewer}")
ENDIF()

FIND_PATH(QGLViewer_HEADER_DIR
	NAMES QGLViewer
	PATHS ${LOCAL_COMPILED_LIBS_PATH}/include
	DOC "QGLViewer header location on disk."
	NO_DEFAULT_PATH)
IF(NOT EXISTS ${QGLViewer_HEADER_DIR})
	MESSAGE(FATAL_ERROR "The QGLViewer headers cannot be found. Have you configured third-party libraries in ./third_party ?")
ELSE()
	MESSAGE(STATUS "Found libQGLViewer : ${QGLViewer_HEADER_DIR}")
ENDIF()
