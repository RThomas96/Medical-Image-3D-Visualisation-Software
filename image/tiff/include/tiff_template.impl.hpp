#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_IMPL_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_IMPL_HPP_

#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_HPP_
// For the linters and syntax highlighters:
#include "tiff_template.hpp"
#endif

#include <algorithm>
#include <iterator>
#include <iomanip>

namespace Image {
namespace Tiff {

	template <> TIFFReader<std::uint8_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_8;
	}

	template <> TIFFReader<std::uint16_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_16;
	}

	template <> TIFFReader<std::uint32_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_32;
	}

	template <> TIFFReader<std::uint64_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_64;
	}

	template <> TIFFReader<std::int8_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_8;
	}

	template <> TIFFReader<std::int16_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_16;
	}

	template <> TIFFReader<std::int32_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_32;
	}

	template <> TIFFReader<std::int64_t>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_64;
	}

	template <> TIFFReader<float>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_32;
	}

	template <> TIFFReader<double>::TIFFReader() : Tiff::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_64;
	}

	// Default templated ctor of the TIFFReader class, throwing an error for unsupported (not specialized) types.
	template <typename unsupported_t> TIFFReader<unsupported_t>::TIFFReader() {
		throw std::runtime_error("Type given to private TIFF implementation is not supported.");
	}

	template <typename img_t>
	template <typename target_t>
	void TIFFReader<img_t>::getRangeValues(glm::vec<2, target_t, glm::defaultp>& _r) const {
		#warning a static cast is done for range values, without any checks or proper conversions
		_r.x = static_cast<target_t>(this->dataRange.x);
		_r.y = static_cast<target_t>(this->dataRange.y);
		return;
	}

	template <typename img_t>
	template <typename target_t>
	void TIFFReader<img_t>::readPixel(svec3 pixel_index, std::vector<target_t>& values) {
		// Load and cache (or retrieve) the image data :
		std::size_t sliceidx = this->loadAndCacheSlice(pixel_index.z);
		std::vector<img_t>& imgdata = this->cachedSlices.getData(sliceidx);

		// All frames are the same dimensions, get the width and height from there :
		std::size_t width = this->images[0][0]->width;

		// Get the right length of one line :
		std::size_t line_length = this->voxel_dimensionalty * width;
		// The offset within the requested line of the pixel values :
		std::size_t line_offset = this->voxel_dimensionalty * pixel_index.x;
		// The offset within the entire vector, once loaded.
		std::size_t pos_in_vector = pixel_index.y * line_length + line_offset;

		// Resize to fit only the values of one pixel :
		values.resize(this->voxel_dimensionalty);
		// Copy data and cast it to the right type :
		for (std::size_t i = 0; i < this->voxel_dimensionalty; ++i) {
			values[i] = static_cast<target_t>(imgdata[pos_in_vector + i]);
		}
		auto begin = imgdata.cbegin() + pos_in_vector;
		// Transform data into the right type, in the destination buffer :
		std::transform(begin, begin + this->voxel_dimensionalty, values.begin(), [](const img_t from) -> target_t {
			return static_cast<target_t>(from);
		});

		return;
	}

	template <typename img_t>
	template <typename target_t>
	void TIFFReader<img_t>::readLine(svec2 line_index, std::vector<target_t>& values) {
		// Load and cache (or retrieve) the image data :
		std::size_t sliceidx = this->loadAndCacheSlice(line_index.y);
		std::vector<img_t>& imgdata = this->cachedSlices.getData(sliceidx);

		// All frames are the same dimensions, get the width and height from there :
		std::size_t width = this->images[0][0]->width;

		// Get the right length of one line :
		std::size_t line_length = this->voxel_dimensionalty * width;
		// offset of the line within the image data :
		std::size_t line_offset = line_index.x * line_length;

		// Resize to fit only the values of one pixel :
		values.resize(line_length);
		// Transform data into the right type, in the destination buffer :
		std::transform(imgdata.cbegin() + line_offset, imgdata.cbegin() + line_offset + line_length, values.begin(),
		[](const img_t from) -> target_t {
			return static_cast<target_t>(from);
		});

		return;
	}

	template <typename img_t>
	template <typename target_t>
	void TIFFReader<img_t>::readSlice(std::size_t slice, std::vector<target_t>& values) {
		// Load and cache (or retrieve) the image data :
		std::size_t sliceidx = this->loadAndCacheSlice(slice);
		std::vector<img_t>& imgdata = this->cachedSlices.getData(sliceidx);
		// Resize to fit only the values of one pixel :
		values.resize(imgdata.size());
		// Transform data into the right type, in the destination buffer :
		std::transform(imgdata.cbegin(), imgdata.cend(), values.begin(), [](const img_t from) -> target_t {
			return static_cast<target_t>(from);
		});

		return;
	}

	template <typename img_t>
	std::size_t TIFFReader<img_t>::loadAndCacheSlice(std::size_t slice_idx) {
		if (slice_idx >= this->images.size()) { throw std::runtime_error("Tried to load past-the-end image."); }

		// If the image is already loaded, return its index :
		if (this->cachedSlices.hasData(slice_idx)) { return this->cachedSlices.findIndex(slice_idx); }

		// Image dimensions :
		std::size_t width = this->images[0][0]->width;
		std::size_t height = this->images[0][0]->height;
		std::size_t imgsize = width * this->voxel_dimensionalty * height;

		// Read all values for all frames at index slice_idx :
		std::vector<img_t> framedata(width*height);
		// The target vector to combine into :
		std::shared_ptr<std::vector<img_t>> full_image = std::make_shared<std::vector<img_t>>(framedata.size()*this->voxel_dimensionalty);

		// Result for libTIFF operations. For most ops, returns 1 on success.
		int result = 1;

		// Iterate on all planes :
		for (std::size_t i = 0; i < this->voxel_dimensionalty; ++i) {
			// Get the right frame :
			const Frame::Ptr& frame = this->images[slice_idx][i];
			// Open the file, and set it to the right directory :
			TIFF* file = TIFFOpen(frame->sourceFile.c_str(), "r");
			result = TIFFSetDirectory(file, frame->directoryOffset);
			if (result != 1) { throw std::runtime_error("Could not set the directory of file "+frame->sourceFile); }

			// Read all strips :
			for (uint64_t i = 0; i < frame->stripsPerImage; ++i) {
				tsize_t readPixelSize = 0; // bytes to read from file
				if (i == frame->stripsPerImage-1) {
					// The last strip is handled differently than the rest. Fewer bytes should be read.
					// compute the remaining rows, to get the number of bytes :
					tsize_t last_strip_row_count = frame->height - (frame->stripsPerImage - 1)*frame->rowsPerStrip;
					if (last_strip_row_count == 0) {
						throw std::runtime_error("Last strip was 0 rows tall ! Something went wrong beforehand.");
					}
					readPixelSize = last_strip_row_count * frame->width;
				} else {
					readPixelSize = frame->width * frame->rowsPerStrip;	// strip size, in pixels
				}

				tsize_t readBytesSize = readPixelSize*(frame->bitsPerSample/8);	// strip size, in bytes
				tsize_t stripPixelSize = frame->width * frame->rowsPerStrip;	// The size of a strip, in rows

				tmsize_t read = TIFFReadEncodedStrip(file, i, framedata.data()+i*stripPixelSize, readBytesSize);
				if (read < 0) {
					// print width for numbers :
					std::size_t pwidth = static_cast<std::size_t>(std::ceil(std::log10(static_cast<double>(frame->stripsPerImage))));
					std::cerr << "[DEBUG]\t\t(" << std::setw(pwidth) << i << "/" << frame->stripsPerImage << ") ERROR:";
					std::cerr << "Could not read strip " << i << " from the frame.\n";
				}
			}

			// Close the tiff file (mostly to free up one file descriptor) :
			TIFFClose(file);

			// Copy the strips' data to the full image :
			typename std::vector<img_t>::iterator full_img_pos = full_image->begin() + i; // begin + current color plane queried
			std::for_each(framedata.cbegin(), framedata.cend(), [this, &full_img_pos](const img_t pixval) -> void {
				*full_img_pos = pixval;
				full_img_pos += this->voxel_dimensionalty;
			});
		}

		return this->cachedSlices.loadData(slice_idx, full_image);
	}

}
}

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_TEMPLATE_IMPL_HPP_
