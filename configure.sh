# Start by greeting the user
echo "Configuring TinyTIFF ..."
#make lib directory :
mkdir lib 2> /dev/null
cd TinyTIFF/
mkdir release 2> /dev/null
cmake -S. -Brelease -DCMAKE_BUILD_TYPE=Release -DTinyTIFF_BUILD_TESTS=OFF -DTinyTIFF_BUILD_DECORATE_LIBNAMES_WITH_BUILDTYPE=OFF -DCMAKE_INSTALL_PREFIX=/home/thibault/git/stage/TinyTIFF/../lib -DTinyTIFF_BUILD_STATIC_LIBS=ON
cmake --build release --target install
cd ../
echo "Configuration of TinyTIFF done."
