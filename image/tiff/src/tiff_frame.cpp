#include "../include/tiff_frame.hpp"

#include <tiff.h>
#include <tiffio.h>

#include <iostream>

namespace Image {

namespace Tiff {

	Frame::Frame(const std::string& fname, const tdir_t dirOff) : sourceFile(fname), directoryOffset(dirOff) {
		this->width = 0;
		this->height = 0;
		this->rowsPerStrip = 0;
		this->samplesPerPixel = 0;
		this->stripsPerImage = 0;
		this->bitsPerSample.clear();
	}

	void Frame::loadTIFFInfo() {
		int result = 0;

		if (this->sourceFile.empty() == false) {
			// Nullify errors and warnings from the command line :
			TIFFSetErrorHandler(tiff_error_redirection);
			TIFFSetWarningHandler(tiff_warning_redirection);

			// Open the file :
			TIFF* file = TIFFOpen(this->sourceFile.c_str(), "r");
			// We don't currently support tiled images :
			if (TIFFIsTiled(file)) { throw std::runtime_error("Cannot read tiled images"); }

			// Set the current directory (should already be there, but just to be sure) :
			result = TIFFSetDirectory(file, this->directoryOffset);
			// for some reason, we might read past-the-end on some directories. Handle the case :
			if (result != 1) {
				throw std::runtime_error("cannot set file to directory "+std::to_string(this->directoryOffset));
			}

			// Check the sample format of the directory :
			uint16_t sampleformat = SAMPLEFORMAT_UINT;
			result = TIFFGetField(file, TIFFTAG_SAMPLEFORMAT, &sampleformat);
			if (result != 1) {
				// Some images do not have a default-given field for the sample format, so
				// assign one now in order to parse the file regardless :
				result = TIFFGetFieldDefaulted(file, TIFFTAG_SAMPLEFORMAT, &sampleformat);
				if (result != 1) {
					std::cerr << "[MAJOR_WARNING] The file \"" << this->sourceFile << "\" has no SAMPLEFORMAT.\n";
					std::cerr << "[MAJOR_WARNING] Interpreting the data as 'uint' by default.\n";
					sampleformat = SAMPLEFORMAT_UINT;
				}
			}

			uint16_t pconfig = 0;
			result = TIFFGetField(file, TIFFTAG_PLANARCONFIG, &pconfig);
			if (result != 1) { throw std::runtime_error("Cannot get PlanarConfig."); }
			if (pconfig != 1) {
				throw std::runtime_error("We currently do not support planar configurations other than 1.");
			}

			// Get the number of samples per-pixel :
			result = TIFFGetField(file, TIFFTAG_SAMPLESPERPIXEL, &this->samplesPerPixel);
			if (result != 1) { throw std::runtime_error("Cannot read SamplesPerPixel."); }

			// Get the number of bits per sample :
			this->bitsPerSample.resize(this->samplesPerPixel);
			result = TIFFGetField(file, TIFFTAG_BITSPERSAMPLE, this->bitsPerSample.data());
			if (result != 1) { throw std::runtime_error("Cannot read BitsPerPixel."); }

			#warning Support for more than 3 samples per pixel is limited.
			// If more than 3 samples per pixel are present, we throw an error.
			// Would be nice if we could integrate them into the image data.

			// If there are more than 3 samples per pixel, then extra samples are used.
			// We don't support this currently.
			if (this->samplesPerPixel > 3u) { throw std::runtime_error("cant parse images with more than 3 samples."); }

			// Get image width and height :
			result = TIFFGetField(file, TIFFTAG_IMAGEWIDTH, &this->width);
			if (result != 1) { throw std::runtime_error("cannot read width field in this directory"); }
			result = TIFFGetField(file, TIFFTAG_IMAGELENGTH, &this->height);
			if (result != 1) { throw std::runtime_error("cannot read height field in this directory"); }

			// Get the number of rows per strip :
			result = TIFFGetField(file, TIFFTAG_ROWSPERSTRIP, &this->rowsPerStrip);
			if (result != 1) { throw std::runtime_error("cannot read rowsperstrip. maybe the image was tiled ?"); }

			// Compute the number of stripoffsets to return :
			this->stripsPerImage = (this->height + this->rowsPerStrip - 1)/this->rowsPerStrip;

			TIFFClose(file);
			std::cerr.flush();
		}
	}

}

}
