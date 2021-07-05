#include "../include/tiff_frame.hpp"

namespace Image {

namespace Tiff {

	Frame::Frame(const std::string& fname, const tdir_t dirOff) : sourceFile(fname), directoryOffset(dirOff) {
		this->rowsPerStrip = 0;
		this->samplesPerPixel = 0;
		this->stripsPerImage = 0;

		this->loadTIFFInfo(fname);
	}

	bool Frame::isCompatibleWith(uint32_t w, uint32_t h, uint16_t bps) {
		TIFF* libhandle = this->getLibraryHandle();
		bool width_compatible = this->width(libhandle) == w;
		bool height_compatible = this->height(libhandle) == h;
		bool bps_compatible	= this->bitsPerSample(libhandle) == bps;
		TIFFClose(libhandle);
		return width_compatible && height_compatible && bps_compatible;
	}

	bool Frame::isCompatibleWidth(const Frame &_frame) {

		TIFF* _self_handle = this->getLibraryHandle();
		TIFF* _other_handle = _frame.getLibraryHandle();

		// pretty thourough checks :
		bool w = (this->width(_self_handle) == _frame.width(_other_handle));
		bool h = (this->height(_self_handle) == _frame.height(_other_handle));
		bool b = (this->bitsPerSample(_self_handle) == _frame.bitsPerSample(_other_handle));
		bool i = (this->photometricInterpretation(_self_handle) == this->photometricInterpretation(_other_handle));
		bool p = (this->planarConfiguration(_self_handle) == _frame.planarConfiguration(_other_handle));
		bool t = (TIFFIsTiled(_self_handle) == TIFFIsTiled(_other_handle));

		TIFFClose(_self_handle);
		TIFFClose(_other_handle);

		return w && h && b && i && p && t;

	}

	bool Frame::hasSameEncoding(const Frame &_frame) {
		//
	}

	TIFF* Frame::getLibraryHandle(void) const {
		TIFF* fhandle = TIFFOpen(this->sourceFile.data(), "r");
		int result = TIFFSetDirectory(fhandle, this->directoryOffset);
		if (result != 1) { return nullptr; } // Something happened which cannot get us to the right IFD.
		return fhandle;
	}

	uint16_t Frame::planarConfiguration(TIFF *fhandle) const {
		if (fhandle == nullptr) {
			fhandle = this->getLibraryHandle();
		}
		uint16_t pc = 0;
		int result = TIFFGetField(fhandle, TIFFTAG_PLANARCONFIG, &pc);
		if (result != 1) { return PLANARCONFIG_CONTIG; }
		return pc;
	}

	uint16_t Frame::photometricInterpretation(TIFF *fhandle) const {
		if (fhandle == nullptr) {
			fhandle = this->getLibraryHandle();
		}
		uint16_t pi = 0;
		int result = TIFFGetField(fhandle, TIFFTAG_PHOTOMETRIC, &pi);
		if (result != 1) { return PHOTOMETRIC_MINISWHITE; }
		return pi;
	}

	uint32_t Frame::width(TIFF* fhandle) const {
		if (fhandle == nullptr) {
			fhandle = this->getLibraryHandle();
		}
		uint32_t width = 0;
		int result = TIFFGetField(fhandle, TIFFTAG_IMAGEWIDTH, &width);
		if (result != 1) { return 0; } // The return value of width is undef is error occured, fix this
		return width;
	}

	uint32_t Frame::height(TIFF* fhandle) const {
		if (fhandle == nullptr) { fhandle = this->getLibraryHandle(); }
		uint32_t height = 0;
		int result = TIFFGetField(fhandle, TIFFTAG_IMAGELENGTH, &height);
		if (result != 1) { return 0; } // The return value of height is undef is error occured, fix this
		return height;
	}

	uint16_t Frame::sampleFormat(TIFF *fhandle) const {
		if (fhandle == nullptr) { fhandle = this->getLibraryHandle(); }
		uint16_t sf = SAMPLEFORMAT_VOID;
		int result = TIFFGetField(fhandle, TIFFTAG_SAMPLEFORMAT, &sf);
		if (result != 1) {
			// Try to get the defaulted version of the field :
			result = TIFFGetFieldDefaulted(fhandle, TIFFTAG_SAMPLEFORMAT, &sf);
			// Some files might still not get the default info, in that case interpret as UINT
			if (result != 1) { sf = SAMPLEFORMAT_UINT; }
		}
		return sf;
	}

	uint16_t Frame::bitsPerSample(TIFF *fhandle) const {
		if (fhandle == nullptr) { fhandle = this->getLibraryHandle(); }
		uint16_t bps = 0;
		int result = TIFFGetField(fhandle, TIFFTAG_BITSPERSAMPLE, &bps);
		if (result != 1) { return 0; } // Malformed BitsPerSample do not allow to read the file.
		return bps;
	}

	void Frame::loadTIFFInfo(std::string_view fname) {
		UNUSED(fname);
		int result = 0;

		if (this->sourceFile.empty() == false) {
			// Nullify errors and warnings from the command line :
			TIFFSetErrorHandler(tiff_error_redirection);
			TIFFSetWarningHandler(tiff_warning_redirection);

			// Open the file :
			TIFF* file = this->getLibraryHandle();
			if (file == nullptr) {
				throw std::runtime_error(std::string("Could not open file ") + this->sourceFile.data());
			}

			// We don't currently support tiled images :
			if (TIFFIsTiled(file)) { throw std::runtime_error("Cannot read tiled images yet"); }

			// evaluate the # of samples per pixel :


			uint16_t pconfig = 0;
			result = TIFFGetField(file, TIFFTAG_PLANARCONFIG, &pconfig);
			if (result != 1) { throw std::runtime_error("Cannot get PlanarConfig. Image is malformed."); }
			if (pconfig != 1) {
				throw std::runtime_error("We currently do not support planar configurations other than 1.");
			}

			uint16_t photometric_interpretation = 0; // 0 is MinIsWhite
			result = TIFFGetField(file, TIFFTAG_PHOTOMETRIC, &photometric_interpretation);
			if (result != 1) { throw std::runtime_error("Cannot get PhotometricInterpretation. Image is malformed."); }
			if (photometric_interpretation == PHOTOMETRIC_PALETTE) {
				throw std::runtime_error("We currently do not support TIFF files with a color palette.");
			}
			if (photometric_interpretation == PHOTOMETRIC_MASK) {
				throw std::runtime_error("We currently do not support TIFF files which represent a holdout mask.");
			}
			if (photometric_interpretation == PHOTOMETRIC_SEPARATED) {
				throw std::runtime_error("We currently do not support TIFF files with separated components (CMYK).");
			}
			if (photometric_interpretation == PHOTOMETRIC_LOGL) {
				throw std::runtime_error("We currently do not support images encoded with Pixar's LogL encoding.");
			}
			if (photometric_interpretation == PHOTOMETRIC_LOGLUV) {
				throw std::runtime_error("We currently do not support images encoded with Pixar's LogLuv encoding.");
			}

			// Get the number of samples per-pixel :
			result = TIFFGetField(file, TIFFTAG_SAMPLESPERPIXEL, &this->samplesPerPixel);
			#warning to change, we support more than one sample now
			if (result != 1) { throw std::runtime_error("Cannot read SamplesPerPixel."); }
			if (this->samplesPerPixel > 1) { throw std::runtime_error("No support more than one sample per pixel."); }

			// Get the number of rows per strip :
			result = TIFFGetField(file, TIFFTAG_ROWSPERSTRIP, &this->rowsPerStrip);
			if (result != 1) { throw std::runtime_error("cannot read rowsperstrip. maybe the image was tiled ?"); }

			// Compute the number of stripoffsets to return :
			this->stripsPerImage = (this->height(file) + this->rowsPerStrip - 1)/this->rowsPerStrip;

			TIFFClose(file);
		}
	}

	bool Frame::fetchSamplesPerPixel(TIFF* lib_handle) const {
		uint16_t extra_samples = 0;
		uint16_t* extra_samples_type = nullptr;
		uint16_t* sample_counts = nullptr;
		int result = 1; // by default, success

		result = TIFFGetField(lib_handle, TIFFTAG_SAMPLESPERPIXEL, &this->samplesPerPixel);
		if (result != 1) { throw std::runtime_error("The SamplesPerPixel field is not present."); }

		result = TIFFGetField(lib_handle, TIFFTAG_EXTRASAMPLES, &extra_samples, &extra_samples_type);
		if(result != 1) { throw std::runtime_error("Cannot read extrasamples field."); }

		uint16_t total_samples = this->samplesPerPixel + extra_samples;
		result = TIFFGetField(lib_handle, TIFFTAG_BITSPERSAMPLE, &sample_counts);
		if (result != 1) { throw std::runtime_error("Cannot read bits per sample !"); }
		#warning malformed bits per sample do not allow to read the file !

		// get a reference to compare against
		uint16_t ref_bits_per_sample = sample_counts[0];

		for (uint16_t sample = 0; sample < this->samplesPerPixel; ++sample) {
			if (sample_counts[sample] != ref_bits_per_sample) { return false; }
		}

		return true;
	}

} // namespace Tiff

} // namespace Image
