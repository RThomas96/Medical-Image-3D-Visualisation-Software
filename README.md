# Tasks to accomplish

- [ ] Add the ability to convert a stack of images into one file
- [ ] Implement a voxel grid
- [ ] Implement a viewer and loader for the grid

### Warnings

Currently, the TIF loader (TinyTIFF) only supports a few datatypes in TIFF images :
- INT
- UINT
- FLOAT

It also only handles uncompressed TIF[F] files. It can handle multiple samples per
frame, but it remains to be seen how it will be handled in code (TODO)

### Questions for Noura

In the `VoxelGrid.h` file :

- What is `GRID_TYPE` ? Is it the data stored inside ?
