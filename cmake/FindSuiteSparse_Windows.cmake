# Find the locally-compiled versions of suitesparse, choldmod (...)

IF(WIN32 OR MINGW OR MSVC)
	SET(BUILDTYPE_PATH "release")
	IF(CMAKE_BUILD_TYPE MATCHES Debug)
		SET(BUILDTYPE_PATH "debug")
	ENDIF()

    #SET(CODEBASES_SOURCE_DIR "$ENV{HOMEDRIVE}$ENV{HOMEPATH}\\Documents\\codebases\\library_sources")
	#SET(CODEBASES_INSTALL_DIR "$ENV{HOMEDRIVE}$ENV{HOMEPATH}\\Documents\\codebases\\library_install\\${BUILDTYPE_PATH}")

    SET(CODEBASES_SOURCE_DIR "${CODEBASES_INSTALL_DIR}\\..")
    SET(CODEBASES_INSTALL_DIR "${CODEBASES_INSTALL_DIR}\\${BUILDTYPE_PATH}")

	SET(GSL_ROOT ${CODEBASES_INSTALL_DIR})
	SET(SuiteSparse_ROOT ${CODEBASES_INSTALL_DIR})
	SET(LOCAL_DLL_ROOTS_GSL "${CODEBASES_SOURCE_DIR}\\suitesparse-metis-for-windows\\lapack_windows\\x64\\")
	SET(ENV{LAPACK_DIR} ${LOCAL_DLL_ROOTS_GSL})
	FIND_PACKAGE(GSL REQUIRED COMPONENTS gsl gslcblas)
	FIND_PACKAGE(SuiteSparse REQUIRED)

	SET(MSG_GSL_BEG "Found GSL")
	IF (NOT GSL_FOUND)
		SET(MSG_GSL_BEG "Not found GSL")
		MESSAGE(STATUS "GSL wasn't found !!!")
	ENDIF()
	MESSAGE(STATUS "${MSG_GSL_BEG} at : ${GSL_LIBRARY}")
	MESSAGE(STATUS "${MSG_GSL_BEG} headers at : ${GSL_INCLUDE_DIRS}")

	SET(MSG_SS_BEG "Found SuiteSparse")
	IF (NOT SuiteSparse_FOUND)
		SET(MSG_SS_BEG "Not found SuiteSparse")
		MESSAGE(STATUS "SuiteSparse wasn't found !!!")
	ENDIF()
	MESSAGE(STATUS "${MSG_SS_BEG} at : ${SuiteSparse_LIBRARIES}")
	MESSAGE(STATUS "${MSG_SS_BEG} headers at : ${SuiteSparse_INCLUDE_DIRS}")

	IF (EXISTS ${LOCAL_DLL_ROOTS_GSL})
		MESSAGE(STATUS "Copying lib{blas|lapack}.dll ...")
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/libblas.dll" DESTINATION ${CMAKE_BINARY_DIR})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/liblapack.dll" DESTINATION ${CMAKE_BINARY_DIR})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/libgcc_s_sjlj-1.dll" DESTINATION ${CMAKE_BINARY_DIR})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/libgfortran-3.dll" DESTINATION ${CMAKE_BINARY_DIR})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/libblas.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/liblapack.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/libgcc_s_sjlj-1.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		FILE(COPY "${LOCAL_DLL_ROOTS_GSL}/libgfortran-3.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	ELSE()
		MESSAGE(FATAL_ERROR "Lapack DLLs cannot be found !!!")
	ENDIF() # lapack dlls exist

	# SuiteSparse compiled library :
	SET(LOCAL_SUITESPARSE_LIBRARY_DIR "${CODEBASES_INSTALL_DIR}\\lib")
	# SuiteSparse (cholmod, specifically) header directory :
	SET(LOCAL_SUITESPARSE_HEADER_DIR "${CODEBASES_SOURCE_DIR}\\suitesparse-metis-for-windows\\SuiteSparse\\CHOLMOD\\SourceWrappers")
	SET(LOCAL_SUITESPARSE_HEADER_ROOT_DIR "${CODEBASES_SOURCE_DIR}\\suitesparse-metis-for-windows\\SuiteSparse" PATH)

ENDIF()
