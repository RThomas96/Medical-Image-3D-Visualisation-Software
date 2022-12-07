# Medical Image Visualizer

https://github.com/RThomas96/Medical-Image-3D-Visualisation-Software/raw/master

*N.B.* : This project has two contributors, @RThomas96 for the year 2022 and @thibaulltt for the year 2021. For more information about you can find the resulting scientific publication [here](https://diglib.eg.org/bitstream/handle/10.2312/vcbm20221191/093-097.pdf?sequence=1&isAllowed=y).

----

This project aims to provide an easy way to visualize and deform high-resolution 3D images.

You can load, visualize and deform TIFF and OME-TIFF images in real-time. 
You can choose from a variety of color scales to apply to all or a part of the displayed values on screen.

This is a C++ project using modern OpenGL and Qt.

|      3D rendering         |    Cutting planes                   |    Alpha blending                 |
|:-------------------------:|:-----------------------------------:|:---------------------------------:|
| <img src="https://github.com/RThomas96/Medical-Image-3D-Visualisation-Software/raw/master/doc/images/rendering.gif" width="270"> |<img src="https://github.com/RThomas96/Medical-Image-3D-Visualisation-Software/raw/master/doc/images/cutting_planes.gif" width="230"> |<img src="https://github.com/RThomas96/Medical-Image-3D-Visualisation-Software/raw/master/doc/images/alpha_blending.gif" width="270"> |

|      ARAP deformation     |    Global deformation               |    2D view                        |
|:-------------------------:|:-----------------------------------:|:---------------------------------:|
| <img src="https://github.com/RThomas96/Medical-Image-3D-Visualisation-Software/raw/master/doc/images/arap_deformation.gif" width="270"> |<img src="https://github.com/RThomas96/Medical-Image-3D-Visualisation-Software/raw/master/doc/images/global.gif" width="230"> |<img src="https://github.com/RThomas96/Medical-Image-3D-Visualisation-Software/raw/master/doc/images/2Dview.gif" width="270"> |

----

#### Building the project

Want to build the project ? See [BUILD.md](./BUILD.md).

#### User manual

To learn how to use the software, see [MANUAL.md](./MANUAL.md).

#### Code documentation

To generate the technical documentation install `doxygen` and `graphviz` packages, and run `doxygen` with the "doc/doxygen_config" file, and open "doc/html/index.html" with your favorite web browser.
Commands to run on Linux:
```sh
$ sudo apt-get install doxygen graphviz
$ cd doc/
$ doxygen doxygen_config
$ # The documentation is generated in html and latex
$ # For a complete documentation website
$ firefox html/index.html
```

#### Issues

To check what are the remaining bugs, see [TODO.md](./TODO.md).
