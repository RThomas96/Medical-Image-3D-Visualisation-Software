#include "../include/tiff_private.hpp"

namespace Image {
namespace Tiff {

	TIFFPrivate::TIFFPrivate() {
		this->internal_data_type = ImageDataType::Unknown;
		this->voxel_dimensionalty = 0;
		this->images.clear();
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
		this->images.push_back(img);
		return *this;
	}

	TIFFPrivate& TIFFPrivate::setVoxelDimension(std::size_t dim) {
		this->voxel_dimensionalty = dim;
	}

}
}
