# Building the project

----

This project is a Qt-based OpenGL viewer written in C++17. As such, here's what you'll need :

- A C++17-compliant compiler (`gcc` and `minGW` have been tested on Linux and Windows respectively)
- The Qt toolkit version 5.12 or higher
- `git` installed

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
Then copy includes in third\_party/compiled\_libraries/include/QGLViewer
And copy .so in compiled\_libraries/lib.

Change findLibQGLViewer to simply search in compiledLib.

---

### Suitsparse

```sh
$ sudo apt-get install libopenblas-dev liblapack-dev libgmp3-dev libgsl-dev
$ git clone suitsparse dans thirdparty
$ cd build
$ cmake ..
$ make local -j 4
```
Change findSuiteSPARE paths to simply search directly in the suitsparse/lib.

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
