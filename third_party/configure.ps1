param([switch]$clean)

# Executable paths :
[string]$CMakeCmdPath = ""
[string]$QMakeCmdPath = ""
[string]$GNUMakeCmdPath = ""
[int]$IsSystemValid = 1;

# The Powershell path is already in the third_party directory, no
# need to append it once more to the library path :
[string]$ProjectRootPath = (Split-Path -Parent $PSCommandPath)
[string]$qglviewerPath = ($ProjectRootPath)+"\libQGLViewer"
[string]$TinyTIFFPath = ($ProjectRootPath)+"\TinyTIFF"
[string]$libTIFFPath = ($ProjectRootPath)+"\libtiff"
[string]$niftiPath = ($ProjectRootPath)+"\nifticlib"
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
	$Global:CMakeCmdPath = (Get-Command -Name cmake.exe).Source
	$Global:QMakeCmdPath = "C:\Qt\5.15.2\mingw81_64\bin\qmake.exe"
	$Global:GNUMakeCmdPath = "C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe"
	
	if (Test-Path -Path $Global:CMakeCmdPath -PathType Leaf) {
		Write-Host "CMake was found at : "${Global:CMakeCmdPath}
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
		Start-Process cmake -NoNewWindow -Wait -ArgumentList "--build release --target install"
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
			"--no-print-directory install"
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

		# Start the CMake generation process :
		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList `
			"-S. -Brelease -G `"MinGW Makefiles`" -DCMAKE_BUILD_TYPE=Release -DNIFTI_BUILD_TESTING=OFF -DNIFTI_SHELL_SCRIPT_TESTS=OFF -DNIFTI_INSTALL_NO_DOCS=TRUE -DCMAKE_INSTALL_PREFIX=$CompiledLibPath "
		# Call CMake to compile the project :
		Start-Process -FilePath $Global:CMakeCmdPath -NoNewWindow -Wait -ArgumentList `
			"--build release --target install"
	}
	# Finish the process :
	Write-Host "Configuration of NIFTI CLib done."
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
}

# Clean, if that's waht the user wants :
if ($clean) {
	Clear-GitAll
	exit
}

# Otherwise, create the compiling directory and compile in it :
if ($IsSystemValid) {
	# Create new folder :
	[void](New-Item -Force -Path . -Name "compiled_libraries" -ItemType "directory")
}

Publish-GitlibTIFF
Publish-GitQGLViewer
Publish-GitNifti
Publish-GitTinyTIFF
