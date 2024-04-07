# Find the SuiteSparse libraries on Linux, and the GSL
# libraries as well :

FIND_PACKAGE(PkgConfig REQUIRED)

# GSL up first, easiest (has pkg-config file) :
pkg_search_module(GSL REQUIRED gsl)

SET(ROOT_SUITESPARSE_HEADERS "${CMAKE_CURRENT_LIST_DIR}/../third_party/SuiteSparse/include")
SET(ROOT_SUITESPARSE_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/../third_party/SuiteSparse/lib")

# Create imported target :
ADD_LIBRARY(SuiteSparse INTERFACE)
TARGET_INCLUDE_DIRECTORIES(SuiteSparse INTERFACE INTERFACE ${ROOT_SUITESPARSE_HEADERS})

MACRO(find_all_suitesparse_components library_names)
	MESSAGE(STATUS "Searching for SuiteSparse components ...")
    FOREACH(library_name IN LISTS ${library_names})
		# Find libs :
		STRING(TOLOWER ${library_name} library_name_lower)
		SET(library_name_lower lib${library_name_lower}.so)
        MESSAGE(STATUS "Searching for ${library_name_lower} returned the files : ${ROOT_SUITESPARSE_LIBRARIES}/${library_name_lower}")
        TARGET_LINK_LIBRARIES(SuiteSparse INTERFACE INTERFACE "${ROOT_SUITESPARSE_LIBRARIES}/${library_name_lower}")
	ENDFOREACH()
ENDMACRO()

SET(SUITESPARSE_NEEDED_LIBRARIES "")
LIST(APPEND SUITESPARSE_NEEDED_LIBRARIES AMD BTF CAMD CCOLAMD COLAMD CHOLMOD CXSparse KLU LDL UMFPACK SPQR)
MESSAGE(STATUS "Search for libraries in SuiteSparse : ${SUITESPARSE_NEEDED_LIBRARIES}")
find_all_suitesparse_components(SUITESPARSE_NEEDED_LIBRARIES)

