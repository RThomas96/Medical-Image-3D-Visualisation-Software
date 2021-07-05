#include "../include/tiff_private.hpp"

namespace Image {
namespace Tiff {

	TIFFPrivate::TIFFPrivate(uint32_t w, uint32_t h) {
		UNUSED(w)
		UNUSED(h)
		this->internal_data_type = ImageDataType::Unknown;
	}

	TIFFPrivate& TIFFPrivate::resizeImages(std::size_t imgcount) {
		this->images.resize(imgcount);
		return *this;
	}

	TIFFPrivate& TIFFPrivate::setImage(std::size_t idx, TIFFImage img) {
		if (idx >= this->images.size()) {
			throw std::runtime_error("Tried to set a non-existant TIFF image.");
		}

		this->images[idx] = img;
		return *this;
	}

	TIFFPrivate& TIFFPrivate::appendFrame(std::size_t idx, Frame::Ptr fr) {
		if (idx >= this->images.size()) {
			throw std::runtime_error("Tried to append to a non-existant TIFF image.");
		}

		this->images[idx].push_back(fr);
		return *this;
	}

	TIFFPrivate& TIFFPrivate::addImage(TIFFImage &img) {
		if (img.empty()) { return *this; }
		this->images.push_back(img);
		return *this;
	}

	TIFFPrivate& TIFFPrivate::setDimensionality(std::size_t dim) {
		this->voxel_dimensionalty = dim;
		return *this;
	}

	ImageDataType TIFFPrivate::getInternalType() const { return this->internal_data_type; }

}
}
