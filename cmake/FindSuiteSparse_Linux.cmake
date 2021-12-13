# Find the SuiteSparse libraries on Linux, and the GSL
# libraries as well :

FIND_PACKAGE(PkgConfig REQUIRED)

# GSL up first, easiest (has pkg-config file) :
pkg_search_module(GSL REQUIRED)
# Export it as a library to link it later :
ADD_LIBRARY(GSL::gsl INTERFACE)
TARGET_LINK_LIBRARIES(GSL::gsl INTERFACE PUBLIC ${GSL_LINK_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(GSL::gsl INTERFACE PUBLIC ${GSL_INCLUDE_DIRS})
ADD_LIBRARY(GSL::gslcblas ALIAS GSL::gsl)

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
TARGET_INCLUDE_DIRECTORIES(SuiteSparse INTERFACE PUBLIC ${ROOT_SUITESPARSE_HEADERS})

MACRO(find_all_suitesparse_components library_names)
	MESSAGE(STATUS "Searching for SuiteSparse components ...")
	FOREACH(library_name IN LISTS ${library_names})
		# Find libs :
		SET(target_name_libs suitesparse_${library_name}_libs)
		MESSAGE(STATUS "Searching for ${library_name} and putting it into ${target_name_libs} ...")
		# Search all files named ${SUITESPARSE_LIBRARIES}/lib<name>.so
		STRING(TOLOWER ${library_name} library_name_lower)
		FILE(GLOB ${target_name_libs} "${ROOT_SUITESPARSE_LIBRARIES}/*lib${library_name_lower}*.so")
		# Print them :
		FOREACH(library_lib IN ${target_name_libs})
			MESSAGE(STATUS "\tFor target ${target_name_libs}, found library ${library_lib}")
		ENDFOREACH()
		# Create an interface library with the name of the component
		ADD_LIBRARY(SuiteSparse::${library_name} INTERFACE)
		TARGET_LINK_LIBRARIES(SuiteSparse::${library_name} INTERFACE PUBLIC ${target_name_libs})
		TARGET_INCLUDE_DIRECTORIES(SuiteSparse::${library_name} INTERFACE PUBLIC ${ROOT_SUITESPARSE_HEADERS})
		# Add that to the general suitesparse target :
		TARGET_LINK_LIBRARIES(SuiteSparse INTERFACE PUBLIC SuiteSparse::${library_name})
	ENDFOREACH()
ENDMACRO()

SET(SUITESPARSE_NEEDED_LIBRARIES "")
LIST(APPEND ${SUITESPARSE_NEEDED_LIBRARIES} "AMD" "BTF" "CAMD" "CCOLAMD" "COLAMD" "CHOLMOD" "CXSparse" "KLU" "LDL" "UMFPACK" "SPQR")
find_all_suitesparse_components(SUITESPARSE_NEEDED_LIBRARIES)

