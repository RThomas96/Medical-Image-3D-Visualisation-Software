###########################################
###########################################
### CMake Setup : Setting all variables ###
###########################################
###########################################

# We cannot currently compile on Windows (coming at a later date)
IF(WIN32 OR MINGW OR MSVC)
        LIST(APPEND CMAKE_PREFIX_PATH "${GLM_HINT}")
ENDIF()

# Warning : CMAKE_CURRENT_LIST_DIR is set here to ./cmake !
# (since list dir is the dir of the current cmake file, not
# necessarily the top CMakeLists.txt) :
IF(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/../lib)
	MESSAGE(FATAL_ERROR "TinyTIFF has not yet been compiled ! Run ./configure.sh or ./configure.bat first.")
ELSE()
	MESSAGE(STATUS "TinyTIFF has previously been installed.")
ENDIF()

# Requiring an out-of-source build
IF(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
	MESSAGE(FATAL_ERROR "Please select another Build Directory !\nWe recommend doing an out-of-source build. Create a build folder\nhere, or in the parent root, but don't build the project here !")
ENDIF()

# Setting the default build type to Release, or whatever the user requested :
SET(DEFAULT_BUILD_TYPE "Release")
IF(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	MESSAGE(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
	SET(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
		STRING "Choose the type of build." FORCE)
	# Set the possible values of build type for cmake-gui
	SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
ENDIF()

MESSAGE(STATUS "Currently compiling in ${CMAKE_BUILD_TYPE} mode.")

# Additionnal flags for Debug mode :
IF(CMAKE_BUILD_TYPE MATCHES Debug)
	SET(GCC_COMPILE_FLAGS "-gdwarf-2 --pedantic")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined -fsanitize=null -fsanitize=return -fsanitize=bounds")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

MESSAGE(STATUS "Current flags in use : ${CMAKE_CXX_FLAGS}")

SET(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)
SET(CMAKE_CXX_STANDARD 17) # Needs C++17
SET(CMAKE_USE_RELATIVE_PATHS TRUE)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt5 stuff :
SET(CMAKE_AUTORCC ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)

# Check if the library directory exists :
IF(NOT EXISTS ${PROJECT_BINARY_DIR}/../lib/)
	FILE(MAKE_DIRECTORY "../lib")
ENDIF()

