#ifndef TESTS_TIFF_TESTS_INCLUDE_TIFF_DISPLAYER_HPP_
#define TESTS_TIFF_TESTS_INCLUDE_TIFF_DISPLAYER_HPP_

#include <QLabel>
#include <QPixmap>

#include "../../../TinyTIFF/tinytiffreader.h"

class tiff_display : public QLabel {
		Q_OBJECT
	public:
		/**
		 * @brief Default constructor for a pixmap
		 * @param tiff_file file to display, or nullptr if not.
		 * @warning The program does not check if the tiff image is valid. Only if it's not nullptr.
		 */
		tiff_display(TinyTIFFReaderFile* tiff_file = nullptr);
		/**
		 * @brief Destructor of the tiff display
		 */
		~tiff_display();
		/**
		 * @brief Loads the TIFF image and displays it.
		 * @param tiff_file the file to load
		 */
		void load(TinyTIFFReaderFile* tiff_file = nullptr);
	protected:
		/**
		 * @brief Loaded image
		 */
		QImage* loaded_image;
		/**
		 * @brief Image data from TinyTIFF
		 */
		std::vector<uchar> data;
};

template <typename tin, typename tout>
void read_file_from_TinyTIFF(TinyTIFFReaderFile* tif, std::vector<tout>& out_buffer, uint32_t& w, uint32_t& h) {
	// clear buffer
	out_buffer.clear();
	// get image width/height :
	uint32_t wwidth = TinyTIFFReader_getWidth(tif);		w = wwidth;
	uint32_t hheight = TinyTIFFReader_getHeight(tif);	h = hheight;
	// allocate for image data for in and out buffers :
	tin* tmp = static_cast<tin*>(calloc(wwidth*hheight, sizeof(tin)));
	out_buffer.resize(wwidth*hheight);
	// read frame :
	TinyTIFFReader_getSampleData(tif, tmp, 0);
	// copy into out buffer
	for (uint32_t i=0; i<wwidth*hheight; i++) {
		out_buffer[i] = static_cast<tout>(tmp[i]);
	}
	free(tmp);
}

#endif // TESTS_TIFF_TESTS_INCLUDE_TIFF_DISPLAYER_HPP_
