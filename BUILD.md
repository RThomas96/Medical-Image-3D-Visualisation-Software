# Building the project

----

This project is a Qt-based OpenGL viewer written in C++17. As such, here's what you'll need :

- A C++17-compliant compiler (`gcc` and `minGW` have been tested on Linux and Windows respectively)
- The Qt toolkit version 5.12 or higher
- `git` installed

----

### Compilation of the project

To install the project, you must clone this repository with __one__ of the following commands, depending on how you want to download it :

```sh
$ git clone git@gitlab.com:thibaulltt/visualisation.git visualisation # To clone via SSH
$ git clone https://gitlab.com/thibaulltt/visualisation.git visualisation # To clone via HTTPS
```

Then, once in the `visualisation` folder, you must update and clone the repositories needed with :

```
$ git submodule init && git submodule update
```

To compile the project, you'll need to have `cmake`, `Qt 5`, and an implementation of OpenGL (usually bundled with your graphic card's video driver, or with the `mesa` library for the integrated Intel GPUs for example). Once those pre-requisites are installed, you have to first configure the project with those commands :

```sh
$ cd third_party/
$ ./configure.ps1 # For Windows
$ ./configure.sh  # For Linux
$ cd ..
```

N.B. : On Windows, you can also open an explorer window in the project's folder, and double-click on the "configure.ps1" file.

---

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
