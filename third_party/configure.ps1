#param(
#	[Switch]$clean = $false,
#	[Switch]$nifti = $false,
#	[Switch]$libtiff = $false,
#	[Switch]$tinytiff = $false,
#	[Switch]$qglviewer = $false
#)

# Executable paths :
[string]$CMakeCmdPath = ""
[string]$QMakeCmdPath = ""
[string]$GitPath = ""
[string]$GNUMakeCmdPath = ""
[int]$IsSystemValid = 1;

# The Powershell path is already in the third_party directory, no
# need to append it once more to the library path :
[string]$ProjectRootPath = (Split-Path -Parent $PSCommandPath)
[string]$qglviewerPath = ($ProjectRootPath)+"\libQGLViewer"
[string]$TinyTIFFPath = ($ProjectRootPath)+"\TinyTIFF"
[string]$libTIFFPath = ($ProjectRootPath)+"\libtiff"
[string]$niftiPath = ($ProjectRootPath)+"\nifticlib"
[string]$zlibPath = ($ProjectRootPath)+"\zlib"
[string]$CompiledLibPath = ($ProjectRootPath)+"\compiled_libraries"

function Clear-GitAll {
	Write-Host "Removing previously compiled libraries ..."

	# TinyTIFF :
	Set-Location $TinyTIFFPath
	if ( Test-Path -Path "release" ) {
		Remove-Item -Path "release" -Recurse
		Write-Host "TinyTIFF's CMake folder was removed."
	} else {
		Write-Host "TinyTIFF was not compiled by this script, or at all."
	}
	Set-Location $ProjectRootPath

	# libTIFF :
	Set-Location $libTIFFPath
	if ( Test-Path -Path "release" ) {
		Remove-Item -Path "release" -Recurse
		Write-Host "libTIFF's CMake folder was removed."
	} else {
		Write-Host "libTIFF was not compiled by this script, or at all."
	}

	# NIFTI :
	Set-Location $niftiPath
	if ( Test-Path -Path "release" ) {
		Remove-Item -Path "release" -Recurse
		Write-Host "NIFTI's CMake folder was removed."
	} else {
		Write-Host "NIFTI was not compiled by this script, or at all."
	}

	# Zlib :
	Set-Location $zlibPath
	if ( Test-Path -Path "release" ) {
		Remove-Item -Path "release" -Recurse
		Write-Host "Zlib's CMake folder was removed"
	} else {
		Write-Host "Zlib was not compiled by this script, or at all."
	}

	# QGLViewer :
	Set-Location $qglviewerPath
	if ( Test-Path -Path "Makefile" -PathType Leaf ) {
		# Execute make's clean dist command :
		Start-Process -FilePath $Global:GNUMakeCmdPath -NoNewWindow -Wait -ArgumentList "distclean"
	} else {
		Write-Host "QGLViewer was not compiled by this script, or at all."
	}

	# Main compiled directory :
	Set-Location $ProjectRootPath
	if ( Test-Path -Path "compiled_libraries" ) {
		Remove-Item -Path "compiled_libraries" -Recurse
	} else {
		Write-host "There is no compiled directories folder."
	}

	Write-Host "Removed all references to previously compiled libraries."
}

function Start-GitCompile {
	# Checks the right programs are available, then creates the compiled_libraries folder :
	$Global:GitPath = (Get-Command -Name git.exe).Source
	$Global:CMakeCmdPath = "C:\Qt\Tools\CMake_64\bin\cmake.exe"
	$Global:QMakeCmdPath = "C:\Qt\5.15.2\mingw81_64\bin\qmake.exe"
	$Global:GNUMakeCmdPath = "C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe"

	if (Test-Path -Path $Global:CMakeCmdPath -PathType Leaf) {
		Write-Host "CMake was found at : "${Global:CMakeCmdPath}
	} else {
		$Global:IsSystemValid = 0;
	}

	if (Test-Path -Path $Global:GitPath -PathType Leaf) {
		Write-Host "git was found at : "${Global:GitPath}
	} else {
		$Global:IsSystemValid = 0;
	}

	if (Test-Path -Path $Global:QMakeCmdPath -PathType Leaf) {
		Write-Host "QMake was found at : "${Global:QMakeCmdPath}
	} else {
		$Global:IsSystemValid = 0;
	}

	if (Test-Path -Path $Global:GNUMakeCmdPath -PathType Leaf) {
		Write-Host "GNU Make was found at : "${Global:GNUMakeCmdPath}
	} else {
		$Global:IsSystemValid = 0;
	}
}

function Publish-GitTinyTIFF {
	Write-Host "Configuring TinyTIFF ..."
	Set-Location $TinyTIFFPath
	if ( Test-Path -Path $TinyTIFFPath+"\release" ) {
		Write-Host "TinyTIFF was already compiled by this script."
	} else {
		# Here we cast the result to void in order to have no output on the CMD/PS :
		[void](New-Item -Force -Path . -Name "release" -ItemType "directory")

		# Start the CMake generation process :
		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList `
			"-S. -Brelease -G `"MinGW Makefiles`" -DCMAKE_BUILD_TYPE=Release -DTinyTIFF_BUILD_TESTS=OFF -DTinyTIFF_BUILD_DECORATE_LIBNAMES_WITH_BUILDTYPE=OFF -DCMAKE_INSTALL_PREFIX=$CompiledLibPath -DTinyTIFF_BUILD_STATIC_LIBS=ON"
		# Call CMake to compile the project :
		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList "--build release --target install"
	}
	# Finish the process :
	Write-Host "Configuration of TinyTIFF done."
	Set-Location $ProjectRootPath
}

function Publish-GitlibTIFF {
	Write-Host "Configuring libTIFF ..."
	Set-Location $libTIFFPath
	if ( Test-Path -Path $libTIFFPath+"\release" ) {
		Write-Host "libTIFF was already compiled by this script."
	} else {
		# Here we cast the result to void in order to have no output on the CMD/PS :
		[void](New-Item -Force -Path $libTIFFPath -Name "release" -ItemType "directory")

		# Start the CMake generation process :
		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList `
			"-S. -Brelease -G `"MinGW Makefiles`" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CompiledLibPath "
		# Call CMake to compile the project :
		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList `
			"--build release --target install "
	}
	# Finish the process :
	Write-Host "Configuration of libTIFF done."
	Set-Location $ProjectRootPath
}

function Publish-GitQGLViewer {
	Write-Host "Configuring LibQGLViewer  ..."
	Set-Location $qglviewerPath
	if ( Test-Path -Path $qglviewerPath+"\Makefile" -PathType Leaf) {
		Write-Host "QGLViewer was already compiled by this script."
	} else {
		# Call QMake on the project :
		Start-Process -FilePath $Global:QMakeCmdPath -NoNewWindow -Wait -ArgumentList " PREFIX=$CompiledLibPath"

		# Add QGLViewer's paths to the LIBRARY_PATH for MinGW's `ld` utility. If not set, causes
		# errors in the compilation of QGLViewer ...
		$env:LIBRARY_PATH+=";$qglviewerPath;$qglviewerPath\QGLViewer;$env:PATH";
		# Call GNU's make :
		Start-Process -FilePath $Global:GNUMakeCmdPath -NoNewWindow -Wait -ArgumentList `
			"--no-print-directory --silent -j sub-QGLViewer"
	}
	# Finish the process :
	Write-Host "Configuration of QGLViewer done."
	Set-Location $ProjectRootPath
}

function Publish-GitNifti {
	Write-Host "Configuring NIFTI Clib ..."
	Set-Location $niftiPath
	if ( Test-Path -Path $niftiPath+"\release" ) {
		Write-Host "NIFTI CLib was already compiled by this script."
	} else {
		# Here we cast the result to void in order to have no output on the CMD/PS :
		[void](New-Item -Force -Path $niftiPath -Name "release" -ItemType "directory")

		[string]$cmakeBuildArgs="-S. -Brelease -G `"MinGW Makefiles`" -DCMAKE_BUILD_TYPE=Release -DZLIB_ROOT=`"$CompiledLibPath`" -DNIFTI_BUILD_TESTING=OFF -DNIFTI_SHELL_SCRIPT_TESTS=OFF -DNIFTI_BUILD_APPLICATIONS=OFF -DNIFTI_INSTALL_NO_DOCS=TRUE -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=$CompiledLibPath "

		# Patch the znzlib cmake file to include the local/global zlib include dirs :
		Start-Process -FilePath $Global:GitPath -NoNewWindow -Wait -ArgumentList " apply ../znz_include_zlib.patch "
		# Start the CMake generation process :
		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList $cmakeBuildArgs
		# Call CMake to compile the project :
		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList `
			"--build release --target install"
	}
	# Finish the process :
	Write-Host "Configuration of NIFTI CLib done."
	Set-Location $ProjectRootPath
}

function Publish-GitZlib {
	Write-Host "Configuring Zlib ..."
	Set-Location $zlibPath
	if ( Test-Path -Path "release" ) {
		Write-Host "Zlib was already compiled by this script."
	} else {
		[void](New-Item -Force -Path $zlibPath -Name "release" -ItemType "directory")

		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList `
			"-S. -Brelease -G `"MinGW Makefiles`" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CompiledLibPath "
		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList `
			"--build release --target install"
	}
	# Finish the process :
	Write-Host "Configuration of Zlib done."
	Set-Location $ProjectRootPath
}

Write-Verbose "Project root is located at ${ProjectThirdParty_lib_Path}"
Write-Host "Warning : Most paths for Qt executables are set to my computer's paths."
Write-Host "Warning : Please change them as necessary."

Start-GitCompile

if (!$IsSystemValid) {
	Write-Host "Error : The system configuration was not valid."
	exit 0;
} else {
	Write-Host "System configuration is valid !"
	# Create new folder, if already exists do nothing :
	[void](New-Item -Force -Path . -Name "compiled_libraries" -ItemType "directory")
}

if ($args.Count -gt 0) {
	# Clean, if that's what the user wants :
	if ($args.Contains("clean")) {
		Clear-GitAll
		exit
	}

	if ($args.Contains("zlib")) {
		Publish-GitZlib
	}
	# If NIFTI as passed in argument, compile it :
	if ($args.Contains("nifti")) {
		Publish-GitNifti
	}
	# Do the same for libtiff :
	if ($args.Contains("libtiff")) {
		Publish-GitlibTIFF
	}
	# And TinyTIFF :
	if ($args.Contains("tinytiff")) {
		Publish-GitTinyTIFF
	}
	# And finally with QGLViewer :
	if ($args.Contains("qglviewer")) {
		Publish-GitQGLViewer
	}
	exit
}

Publish-GitlibTIFF
Publish-GitQGLViewer
Publish-GitZlib
Publish-GitNifti
Publish-GitTinyTIFF
