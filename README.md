# Medical Image Visualizer

*N.B.* : This project is the result of the work done for the end of my Masters degree for the year 2019/2020. You can find the original project
proposal [here](http://www.lirmm.fr/~nfaraj/files/positions/sujet_stage-prostate3D.pdf).

====

### Compilation and installation of the project

To install the project, you must clone this repository with __one__ of the following commands, depending on how you want to download it :

```sh
$ git clone git@gitlab.com:thibaulltt/visualisation.git visualisation # To clone via SSH
$ git clone https://gitlab.com/thibaulltt/visualisation.git visualisation # To clone via HTTPS
```

Then, once in the `visualisation` folder, you must update and clone the repositories needed with :

```
$ git submodule init && git submodule update
```

To compile the project, you'll need to have `cmake`, `Qt 5`, `libQGLViewer`, the `glm` library and an implementation of OpenGL (usually bundled
with your graphic card's video driver, or with the `mesa` library for the integrated Intel GPUs for example). Once those pre-requisites are
installed, you can launch the compilation with the following command :

```sh
$ cmake -S ./ -B buildDir
$ cmake --build buildDir --parallel
```

Then, you can launch the program with :

```sh
$ ./buildDir/visualisation
```

