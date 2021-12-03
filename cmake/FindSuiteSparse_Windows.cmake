# Find the locally-compiled versions of suitesparse, choldmod (...)

IF(WIN32 OR MINGW OR MSVC)
	SET(BUILDTYPE_PATH "release")
	IF(CMAKE_BUILD_TYPE MATCHES Debug)
		SET(BUILDTYPE_PATH "debug")
	ENDIF()

	SET(CODEBASES_BUILD_DIR "$ENV{HOMEDRIVE}$ENV{HOMEPATH}\\Documents\\codebases\\library_build\\${BUILDTYPE_PATH}")
	SET(CODEBASES_SOURCE_DIR "$ENV{HOMEDRIVE}$ENV{HOMEPATH}\\Documents\\codebases\\library_sources")
	SET(CODEBASES_INSTALL_DIR "$ENV{HOMEDRIVE}$ENV{HOMEPATH}\\Documents\\codebases\\library_install\\${BUILDTYPE_PATH}")

	SET(LOCAL_GSL_LIB_DIR "${CODEBASES_INSTALL_DIR}\\lib")
	FIND_LIBRARY(LOCAL_GSL
		NAMES "gsl"
		PATHS ${LOCAL_GSL_LIB_DIR}
		DOC "GSL library location on disk."
		REQUIRED
		NO_DEFAULT_PATH)
	FIND_LIBRARY(LOCAL_GSLCBLAS
		NAMES "gslcblas"
		PATHS ${LOCAL_GSL_LIB_DIR}
		DOC "GSL/Cblas library location on disk."
		REQUIRED
		NO_DEFAULT_PATH)

	MESSAGE(STATUS "Found GSL at : ${LOCAL_GSL}")
	MESSAGE(STATUS "Found cBLAS at : ${LOCAL_GSLCBLAS}")

	SET(LOCAL_DLL_ROOTS_GSL "${CODEBASES_SOURCE_DIR}\\suitesparse-metis-for-windows\\lapack_windows\\x64\\")
	SET(ENV{LAPACK_DIR} ${LOCAL_DLL_ROOTS_GSL})
	IF (EXISTS ${LOCAL_DLL_ROOTS_GSL})
		MESSAGE(STATUS "Copying lib{blas|lapack}.dll ...")
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/libblas.dll" DESTINATION ${CMAKE_BINARY_DIR})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/liblapack.dll" DESTINATION ${CMAKE_BINARY_DIR})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/libgcc_s_sjlj-1.dll" DESTINATION ${CMAKE_BINARY_DIR})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/libgfortran-3.dll" DESTINATION ${CMAKE_BINARY_DIR})
	ENDIF() # lapack dlls exist

	# SuiteSparse compiled library :
	SET(LOCAL_SUITESPARSE_LIBRARY_DIR "${CODEBASES_INSTALL_DIR}\\lib")
	# SuiteSparse (cholmod, specifically) header directory :
	SET(LOCAL_SUITESPARSE_HEADER_DIR "${CODEBASES_SOURCE_DIR}\\suitesparse-metis-for-windows\\SuiteSparse\\CHOLMOD\\SourceWrappers")
	SET(LOCAL_SUITESPARSE_HEADER_ROOT_DIR "${CODEBASES_SOURCE_DIR}\\suitesparse-metis-for-windows\\SuiteSparse" PATH)

	# All the different libraries in SuiteSparse (header dirs) :
	SET(LOCAL_SUITESPARSE_INSTALL_DIR "${CODEBASES_INSTALL_DIR}\\lib" PATH)

	SET(SuiteSparse_DIR "${LOCAL_SUITESPARSE_INSTALL_DIR}\\cmake\\suitesparse-5.4.0")
	FIND_PACKAGE(SuiteSparse CONFIG PATHS ${SuiteSparse_DIR} PATH_SUFFIXES "suitesparse-5.4.0" REQUIRED)


ENDIF()