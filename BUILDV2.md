# Building the project

----

This project is a Qt-based OpenGL viewer written in C++17. As such, here's what you'll need :

- A C++17-compliant compiler (`gcc` and `minGW` have been tested on Linux and Windows respectively)
- The Qt toolkit version 5.12 or higher
- `git` installed
- An implementation of OpenGL v3.2 or higher (usually bundled with your graphic card's video driver, or with the mesa library for the integrated Intel GPUs for example). If you have troubles installing OpenGL see the `Troubleshooting` section.

This guide has been tested on Ubuntu 22.04.4 LTS. Some modifications can be needed on other versions.

----

### Libraries

To compile this project multiple libraries are required:
```sh
$ sudo apt-get install gcc git cmake build-essential libglu1-mesa-dev libgl-dev
```

----

### QT installation

This project needs Qt5, and NOT Qt4 or Qt6.
```sh
$ sudo apt-get install qtbase5-dev qt5-qmake
```

----

### Compilation of the project

To install the project, you must clone this repository with __one__ of the following commands, depending on how you want to download it :

```sh
$ sudo apt-get install gh
$ gh auth login
$ git clone https://github.com/RThomas96/Medical-Image-3D-Visualisation-Software.git
```

Then, once in the `visualisation` folder, you must update and clone the repositories needed with :

```sh
$ git submodule init && git submodule update
```

If the access is denied during the init this is because git try to clone with ssh instead of https. Change URLs in .git/config from ssh to https.

To compile the project, you'll need to have `cmake`, `Qt 5`, and an implementation of OpenGL (usually bundled with your graphic card's video driver, or with the `mesa` library for the integrated Intel GPUs for example). Once those pre-requisites are installed, you have to first configure the project with those commands :

```sh
$ cd third_party/
$ ./configure.ps1 # For Windows
$ # For Windows, if the script fails check the path to
$ # QMake is right (line 88). Modify it if necessary
$ # since it depends on the Qt version installed.
$ ./configure.sh  # For Linux
$ cd ..
```

N.B. : On Windows, you can also open an explorer window in the project's folder, and double-click on the "configure.ps1" file.

---

### LibQGLViewer compilation

In libgqlViewer.pro remove plugin and examples.
```sh
$ cd libQGLViewer
$ qmake
$ make -j
```
Then copy all .h to third\_party/compiled\_libraries/include/QGLViewer
And copy .so in compiled\_libraries/lib and rename it .so.2.

---

### Suitsparse

```sh
$ sudo apt-get install libopenblas-dev liblapack-dev libgmp3-dev libgsl-dev libmpfr-dev pkg-config
$ git clone suitsparse dans thirdparty
$ cmake
$ make local -j 4
$ make install
```
Copy choldmod.h and Suitsparseconfig.h to compiled\_libraries/include/suitesparse

---

### Project compilation

Once the configuration part is done, we can build the project :

```sh
# In the root directory of the project :
$ cmake -S ./ -B <a build path>
$ cmake --build <the same build path> --parallel
```

Then, you can launch the program with :

```sh
$ ./<your build path>/neighbor_visu.exe # For Windows
$ ./<your build path>/neighbor_visu     # For Linux
```

---

## Troubleshooting

### OpenGL installation
Depending of your Ubuntu version, when installing opengl by `apt-get install libgl1-mesa-dev` the default version of OpenGL could be different than v3.2, but the project **needs OpenGL-v3.2**. 
This article could help you updating the right version of OpenGL [[Updating OpenGL from version 3.0 to latest 4.5](https://askubuntu.com/questions/928538/updating-opengl-from-version-3-0-to-latest-4-5)](https://askubuntu.com/questions/928538/updating-opengl-from-version-3-0-to-latest-4-5), as
```
glxinfo|grep OpenGL
export MESA_GL_VERSION_OVERRIDE=3.2
glxinfo | grep "OpenGL version"
echo 'export MESA_GL_VERSION_OVERRIDE=3.2' >> ~/.bashrc
```

---

## Using vcpkg

Vcpkg is a free and open-source C++ package manager that allows you to install all dependencies automatically.

### Install vcpkg

```
$ sudo apt-get install ninja-build
$ git clone https://github.com/microsoft/vcpkg.git
$ cd vcpkg; ./bootstrap-vcpkg.bat
$ export VCPKG_ROOT=/path/to/vcpkg
$ export PATH=$PATH:$VCPKG_ROOT
$ cmake --preset=default
```

You need to export the variable everytime you restart your shell.
Or add it to your `~/.profile` file.

You need some mistery dependencies too:

```
# For another vcpkg
# Maybe not usefull
sudo apt-get install bison pkg-config autoconf automake libtool libsystemd-dev

# For qt5 package, missing jinja2
sudo apt-get install python3-pip
pip install jinja2
sudo apt-get install libx11-*
sudo apt-get install libx11*

sudo apt-get install libxcb-*
sudo apt-get install libxcb*

sudo apt-get install libxkbcommon-dev
sudo apt-get install libxkbcommon-x11-dev
sudo apt-get install libgles2-mesa-dev

sudo apt-get install libxi-dev libxtst-dev gfortran flex
sudo apt-get install autoconf automake autoconf-archive
sudo apt-get install libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libegl1-mesa-dev libxcb.*-dev
```
