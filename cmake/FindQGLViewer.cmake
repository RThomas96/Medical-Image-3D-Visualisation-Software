############################################################
############################################################
### libQGLViewer setup : Finding the shared library file ###
############################################################
############################################################

# TODO : change this check once windows is supported !
IF (EXISTS "/usr")
	SET(UBUNTU_INSTALL_DIR "/usr/lib/x86_64-linux-gnu")
	SET(QGLViewer_MAIN_DIR "/usr/local")
	SET(QGLViewer_INCLUDE_DIR "${QGLViewer_MAIN_DIR}/include/QGLViewer")
	SET(QGLViewer_LIB_DIR "${QGLViewer_MAIN_DIR}/lib")
	SET(QGLVIEWER_PARENT_HINT "")
	IF(${QGLVIEWER_HINT})
		GET_FILENAME_COMPONENT(QGLVIEWER_PARENT_HINT ${QGLVIEWER_HINT} DIRECTORY)
		MESSAGE(STATUS "Looking for libQGLViewer in the following locations : ${QGLVIEWER_PARENT_HINT},
	${QGLVIEWER_HINT},
	${QGLViewer_MAIN_DIR},
	${QGLViewer_LIB_DIR},
	${UBUNTU_INSTALL_DIR}")
	ELSE()
		MESSAGE(STATUS "Looking for libQGLViewer in the following locations : ${QGLViewer_MAIN_DIR}, ${QGLViewer_LIB_DIR}, ${UBUNTU_INSTALL_DIR}")
	ENDIF()
	FIND_LIBRARY(QGLViewer_LIBRARY
		NAMES libQGLViewer-qt5.so
		PATHS ${QGLVIEWER_HINT} ${QGLVIEWER_PARENT_HINT} ${QGLViewer_MAIN_DIR} ${QGLViewer_LIB_DIR} ${UBUNTU_INSTALL_DIR}
		NO_DEFAULT_PATH)
	IF(NOT QGLViewer_LIBRARY OR NOT EXISTS ${QGLViewer_LIBRARY})
		MESSAGE(FATAL_ERROR "\tThe QGLViewer library cannot be found. It either means the library has not been installed, or it has been installed to a non-standard location.
	If this is your case, then re-run this cmake command with the variable QGLVIEWER_HINT set to where libQGLViewer is installed.
	If you have not installed the library, go to http://www.libqglviewer.com and follow the installation instructions here.")
	ELSE()
		MESSAGE(STATUS "Found libQGLViewer : ${QGLViewer_LIBRARY}")
	ENDIF()
ELSE()
	MESSAGE(FATAL_ERROR "\tThe /usr directory cannot be found. It either means the library has not been installed, or it has been installed to a non-standard location.
	If this is your case, then re-run this cmake command with the variable QGLVIEWER_HINT set to where you installed libQGLViewer.
	If you have not installed the library, go to http://www.libqglviewer.com and follow the installation instructions here.")
ENDIF()
