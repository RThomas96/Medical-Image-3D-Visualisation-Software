# libTIFF implementation of GenericImageReader

## What files are supported ?

The following files are supported :

- Fully compliant TIFF files (no malformed or missing tags, for the exception of `SampleFormat` since ImageJ sometimes doesn't write it)
- TIFF files with the following extensions :
	- `.tif`, `.tiff` (case sensitive)
- TIFF files with a `PlanarConfiguration` of 1 (striped data, not planar)
- TIFF stacks with all components :
	- containing the same number of TIFF frames
	- containing only _one sample per pixel_ (for RGB data, provide each color as a separate frame/file)
	- containing _no extra samples_
	- containing data encoded as :
		- 8, 16, 32, 64 bit signed integers,
		- 8, 16, 32, 64 bit unsigned integers,
		- 32, 64 bit floating point

Any attempt to load files/stacks that do not conform to this set of requirements will result in an error. This error will usually be handled in a graceful way, but might require you to remove/change some files.

