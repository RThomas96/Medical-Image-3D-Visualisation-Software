#include "../include/tiff_displayer.hpp"

#include <iostream>

#define WIN_SIZE 400

tiff_display::tiff_display(TinyTIFFReaderFile* tiff_file) {
	this->setMinimumSize(WIN_SIZE,WIN_SIZE);
	this->setMaximumSize(WIN_SIZE,WIN_SIZE);

	this->load(tiff_file);
}

tiff_display::~tiff_display() {}

 void tiff_display::load(TinyTIFFReaderFile* tiff_file) {
	uint32_t w = WIN_SIZE;
	uint32_t h = WIN_SIZE;
	if (tiff_file != nullptr) {
		w = TinyTIFFReader_getWidth(tiff_file);
		h = TinyTIFFReader_getHeight(tiff_file);
		this->data.resize(w*h);
		/**
		 * For now, the images coming from the scanner only contain values from 0-255 (a char) so we don't
		 * need to let TinyTIFF guess the type. The images are also only in greyscale for now, so we don't
		 * need to have data be resized to w*h*{number of color channels} (if we need to, uncomment the
		 * line below this if/else block which dictates the number of bytes per pixel)
		 *
		 * TODO [Improvement/Future] = Be a bit more flexible when reading image data, to allow for more
		 *				pixel formats, and a color mode. When we do, comment the line just
		 *				below, and un-comment the switch block.
		 */
		TinyTIFFReader_readFrame<uchar>(tiff_file, this->data.data());
		/*
		uint16_t type = TinyTIFFReader_getSampleFormat(tiff_file);
		switch (type) {
			case TINYTIFFREADER_SAMPLEFORMAT_INT:
				std::clog << "Image was stored with signed integers" << std::endl;
				TinyTIFFReader_readFrame<int32_t>(tiff_file, this->data.data());
				break;
			case TINYTIFFREADER_SAMPLEFORMAT_UINT:
				std::clog << "Image was stored with unsigned integers" << std::endl;
				TinyTIFFReader_readFrame<uint32_t>(tiff_file, this->data.data());
				break;
			case TINYTIFFREADER_SAMPLEFORMAT_FLOAT:
				std::clog << "Image was stored with floating point" << std::endl;
				TinyTIFFReader_readFrame<float>(tiff_file, this->data.data());
				break;
			default:
				std::cerr << "tiff_display::read_file_from_TinyTIFF() :" << std::endl;
				std::cerr << "Could not infer the type of the image." << std::endl;
				std::cerr << "No image will be shown." << std::endl;
				break;
		}
		*/
	} else {
		// Fill with median grey :
		data.resize(w*h, static_cast<uchar>(56)); // 56 is about median grey for uchar
	}

	// int bpp = 1;
	QImage image(this->data.data(), w, h, w, QImage::Format_Grayscale8, nullptr, nullptr);

	QPixmap display_area = QPixmap::fromImage(image, Qt::ImageConversionFlag::MonoOnly);
	this->setPixmap(display_area.scaled(this->width(), this->height(), Qt::KeepAspectRatio));
}
