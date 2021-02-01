$ProjectRootPath = (Split-Path -Parent $PSCommandPath)
$TinyTIFF_Path = ($ProjectRootPath)+"\TinyTIFF"
$CompiledLibPath = ($ProjectRootPath)+"\lib"
Write-Host "Configuring TinyTIFF ..."
# Here we cast the results to void in order to have no output on the CMD/PS :
[void](New-Item -Force -Path $ProjectRootPath -Name "lib" -ItemType "directory")
[void](New-Item -Force -Path $TinyTIFF_Path -Name "release" -ItemType "directory")
Set-Location $TinyTIFF_Path
Start-Process cmake -NoNewWindow -Wait -ArgumentList "-S. -Brelease -G `"MinGW Makefiles`" -DCMAKE_BUILD_TYPE=Release -DTinyTIFF_BUILD_TESTS=OFF -DTinyTIFF_BUILD_DECORATE_LIBNAMES_WITH_BUILDTYPE=OFF -DCMAKE_INSTALL_PREFIX=$CompiledLibPath -DTinyTIFF_BUILD_STATIC_LIBS=ON"
Start-Process cmake -NoNewWindow -Wait -ArgumentList "--build release --target install --parallel"
Set-Location ($ProjectRootPath)
Write-Host "Configuration of TinyTIFF done."