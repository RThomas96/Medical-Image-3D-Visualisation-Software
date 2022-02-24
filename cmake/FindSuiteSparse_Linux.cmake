# Find the SuiteSparse libraries on Linux, and the GSL
# libraries as well :

FIND_PACKAGE(PkgConfig REQUIRED)

# GSL up first, easiest (has pkg-config file) :
pkg_search_module(GSL REQUIRED gsl)

# Next up, suitesparse. First, find the path where the suitesparse headers
# and library files are located, and then create a target with the compound
# targets :
FIND_FILE(SUITESPARSE_CHOLMOD_HEADER
	NAMES cholmod.h
	PATHS ${CMAKE_INCLUDE_PATH}
	PATH_SUFFIXES suitesparse
	DOC "The SuiteSparse CHOLMOD header, used to find the SuiteSparse library."
	REQUIRED
)
IF (NOT ${SUITESPARSE_CHOLMOD_HEADER_FOUND})
	MESSAGE(FATAL_ERROR "The SuiteSparse headers could not be found !")
ENDIF()

FIND_FILE(SUITESPARSE_CHOLMOD_LIBRARY
	NAMES libcholmod.so
	PATH_SUFFIXES lib
	DOC "The SuiteSpase CHOLMOD shared library file, used to find the SuiteSparse library."
	REQUIRED
)
IF (NOT ${SUITESPARSE_CHOLMOD_LIBRARY_FOUND})
	MESSAGE(FATAL_ERROR "The SuiteSparse libraries could not be found !")
ENDIF()

# Find their parent folder :
GET_FILENAME_COMPONENT(ROOT_SUITESPARSE_HEADERS ${SUITESPARSE_CHOLMOD_HEADER} DIRECTORY)
GET_FILENAME_COMPONENT(ROOT_SUITESPARSE_LIBRARIES ${SUITESPARSE_CHOLMOD_LIBRARY} DIRECTORY)

# Create imported target :
ADD_LIBRARY(SuiteSparse INTERFACE)
TARGET_INCLUDE_DIRECTORIES(SuiteSparse INTERFACE INTERFACE ${ROOT_SUITESPARSE_HEADERS})

MACRO(find_all_suitesparse_components library_names)
	MESSAGE(STATUS "Searching for SuiteSparse components ...")
    FOREACH(library_name IN LISTS ${library_names})
		# Find libs :
		SET(target_name_libs suitesparse_${library_name}_libs)
		STRING(TOLOWER ${library_name} library_name_lower)
        FIND_LIBRARY(${target_name_libs}
            NAMES ${library_name_lower}
            PATHS ${ROOT_SUITESPARSE_LIBRARIES}
            REQUIRED)
        MESSAGE(STATUS "Searching for ${library_name} returned the files : ${${target_name_libs}}")
        TARGET_LINK_LIBRARIES(SuiteSparse INTERFACE INTERFACE ${${target_name_libs}})
	ENDFOREACH()
ENDMACRO()

SET(SUITESPARSE_NEEDED_LIBRARIES "")
LIST(APPEND SUITESPARSE_NEEDED_LIBRARIES AMD BTF CAMD CCOLAMD COLAMD CHOLMOD CXSparse KLU LDL UMFPACK SPQR)
MESSAGE(STATUS "Search for libraries in SuiteSparse : ${SUITESPARSE_NEEDED_LIBRARIES}")
find_all_suitesparse_components(SUITESPARSE_NEEDED_LIBRARIES)

