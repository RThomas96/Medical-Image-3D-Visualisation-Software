#!/bin/bash

# Set variables by default :
CMAKE_PATH=
QMAKE_PATH=
MAKE_PATH=

# Check we have the right programs installed
function check_needed_programs {
	VALID_CONFIG=1 # By default, system config is OK.
	# We need to have make, CMake, QMake, a C/C++ compiler and possibly more.
	# Check one by one the needed programs.

	# make :
	MAKE_PATH=$(which make)
	if [ $? -ne 0 ]; then echo "ERROR : GNU Make not found in your current path !"; VALID_CONFIG=0;
	else echo "Found GNU Make : \"${MAKE_PATH}\"."; fi

	# cmake :
	CMAKE_PATH=$(which cmake)
	if [ $? -ne 0 ]; then echo "ERROR : CMake not found in your current path !"; VALID_CONFIG=0;
	else echo "Found CMake : \"${CMAKE_PATH}\"."; fi

	# qmake :
	QMAKE_PATH=$(which qmake)
	if [ $? -ne 0 ]; then echo "ERROR : QMake not found in your current path !"; VALID_CONFIG=0;
	else echo "Found QMake : \"${QMAKE_PATH}\"."; fi

	return $VALID_CONFIG
}

# Check the compiled directory exists, and create it if not :
function check_compiled_directory {
	if [ ! -d ./compiled_libraries/ ]; then
		mkdir compiled_libraries
		if [ $? -ne 0 ]; then
			echo "ERROR : something went wrong with mkdir."
			exit 1
		fi
	fi
}

# Cleanup of compiled files/libs :
function cleanup_compiled_libs {
	echo "Cleaning compiled libraries ..."
	rm -rf ./compiled_libraries/*
	echo "Cleaning up TinyTIFF ..."
	cd TinyTIFF/
	rm -rf release
	cd ..
	echo "Cleaning up libTIFF ..."
	cd libtiff/
	rm -rf release
	cd ..
	echo "Cleaning up NIFTI's C-lib ..."
	cd nifticlib/
	rm -rf release
	cd ..
	echo "Cleaning up libQGLViewer ..."
	cd libQGLViewer/
	# Removes all compiled files, _and_ shared libs (more than regular clean) :
	${MAKE_PATH} distclean
	cd ..
	# Remove compiled lib directory :
	rm -rf ./compiled_libraries
}

# Configure libQGLViewer :
function configure_qglviewer {
	echo "Configuring libQGLViewer ..."
	cd libQGLViewer/
	if [ -f Makefile ]; then
		# Makefile was generated before, and probably
		# compiled by this script beforehand :
		echo "libQGLViewer was already configured."
		cd ..
		return
	fi
	# Create stash file, and makefile for the lib :
	${QMAKE_PATH} PREFIX=$(pwd)/../compiled_libraries
	# Compile and install in the PREFIX directory :
	${MAKE_PATH} install -j
	cd ../
	echo "Configuration of libQGLViewer done."
}

# Configure TinyTIFF :
function configure_tinytiff {
	echo "Configuring TinyTIFF ..."
	cd TinyTIFF/
	if [ -d release ]; then echo "TinyTIFF was already compiled."; cd ..; return; fi
	mkdir release
	${CMAKE_PATH} -S. -Brelease -DCMAKE_BUILD_TYPE=Release \
		-DTinyTIFF_BUILD_TESTS=OFF \
		-DTinyTIFF_BUILD_DECORATE_LIBNAMES_WITH_BUILDTYPE=OFF \
		-DCMAKE_INSTALL_PREFIX=$(pwd)/../compiled_libraries \
		-DTinyTIFF_BUILD_STATIC_LIBS=ON
	${CMAKE_PATH} --build release --target install --parallel
	cd ../
	echo "Configuration of TinyTIFF done."
}

# Configure libTIFF :
function configure_libtiff {
	echo "Configuring libTIFF ..."
	cd libtiff
	if [ -d release ]; then echo "libTIFF was already compiled."; cd ..; return; fi
	mkdir release
	${CMAKE_PATH} -S. -Brelease -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$(pwd)/../compiled_libraries
	${CMAKE_PATH} --build release --target install --parallel
	cd ../
	echo "Configuration of libTIFF done."
}

# Configure NIFTI's C library :
function configure_nifti {
	echo "Configuring NIFTI's C library ..."
	cd nifticlib
	if [ -d release ]; then echo "NIFTI was already configured."; cd ..; return; fi
	mkdir release
	${CMAKE_PATH} -S. -Brelease -DCMAKE_BUILD_TYPE=Release \
		-DNIFTI_BUILD_TESTING=OFF \
		-DNIFTI_SHELL_SCRIPT_TESTS=OFF \
		-DNIFTI_INSTALL_NO_DOCS=TRUE \
		-DCMAKE_INSTALL_PREFIX=$(pwd)/../compiled_libraries
	${CMAKE_PATH} --build release --target install --parallel
	cd ..
	echo "Configuration of NIFTI's C library done."
}

# Configure nanoflann :
function configure_nanoflann {
	#
	echo "Configuring nanoflann ..."
	cd nanoflann
	if [ -d release ]; then echo "NIFTI was already configured."; cd ..; return; fi
	mkdir release
	${CMAKE_PATH} -S. -Brelease -DCMAKE_BUILD_TYPE=Release \
		-DNANOFLANN_BUILD_EXAMPLES=OFF \
		-DNANOFLANN_BUILD_BENCHMARKS=OFF \
		-DNANOFLANN_BUILD_TESTS=OFF \
		-DCMAKE_INSTALL_PREFIX=$(pwd)/../compiled_libraries
	${CMAKE_PATH} --build release --target install --parallel
	cd ..
	echo "Configuration of nanoflann done."
}

# Check if system config is valid :
check_needed_programs
if [ $? -ne 1 ]; then
	echo "System configuration was not valid."
	exit 1
fi

# If the first user argument was "clean", clean the compiled libraries :
if [ $# -eq 1 ] && [ $1 == "clean" ]; then
	cleanup_compiled_libs
	exit 0
fi

if [ $# -gt 0 ]; then
	for arg in $@
	do
		lowercasearg=$(echo $arg | awk '{print tolower($0)}')
		case $lowercasearg in
			("qglviewer") configure_qglviewer;;
			("qglviewer/") configure_qglviewer;;
			("libtiff") configure_libtiff;;
			("libtiff/") configure_libtiff;;
			("tinytiff") configure_tinytiff;;
			("tinytiff/") configure_tinytiff;;
			("nifti") configure_nifti;;
			("nifti/") configure_nifti;;
			("nanoflann") configure_nanoflann;;
			("nanoflann/") configure_nanoflann;;
			(*) echo "Argument ${lowercasearg} not recognized";;
		esac
	done
fi

# Create needed directory :
check_compiled_directory

# Configure all libs :
configure_libtiff
configure_qglviewer
configure_tinytiff
configure_nifti
configure_nanoflann

