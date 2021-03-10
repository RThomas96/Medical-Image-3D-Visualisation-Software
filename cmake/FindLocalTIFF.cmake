############################################################
############################################################
######  libTIFF setup : Finds the shared library file ######
############################################################
############################################################

SET(libTIFF_LIBRARY_NAME "libtiff.so")
IF(WIN32 OR MINGW OR MSVC)
		MESSAGE(FATAL_ERROR "check the filenames produced for libTIFF on windows.")
		SET(libTIFF_LIBRARY_NAME "libtiff.a" "TIFF.dll")
ENDIF()

SET(LOCAL_COMPILED_LIBS "${CMAKE_SOURCE_DIR}/lib")

MESSAGE(STATUS "Looking for libTIFF in ${LOCAL_COMPILED_LIBS} ...")

FIND_LIBRARY(libTIFF
	NAMES ${libTIFF_LIBRARY_NAME}
	PATHS ${LOCAL_COMPILED_LIBS}
	PATH_SUFFIXES lib
	DOC "Locally compiled libTIFF location on disk."
	REQUIRED
	NO_DEFAULT_PATH)
IF(NOT EXISTS ${libTIFF})
		MESSAGE(FATAL_ERROR "The libTIFF locally compiled has not been found. Have you run the configuration script ?")
ELSE()
		MESSAGE(STATUS "Found libTIFF : ${libTIFF}")
ENDIF()

FIND_PATH(libTIFF_HEADER_DIR
	NAMES include
	PATHS ${LOCAL_COMPILED_LIBS}
	DOC "libTIFF header location on disk."
	NO_DEFAULT_PATH)
IF(NOT EXISTS ${libTIFF_HEADER_DIR})
		MESSAGE(FATAL_ERROR "The libTIFF headers cannot be found.")
ELSE()
		MESSAGE(STATUS "Found libTIFF headers in : ${libTIFF_HEADER_DIR}")
ENDIF()
