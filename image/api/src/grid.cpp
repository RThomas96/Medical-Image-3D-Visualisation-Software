#include "../include/grid.hpp"

namespace Image {

	Grid::Grid(ImageBackendImpl::Ptr _ptr) : pImpl(std::move(_ptr)) {
		this->parentGrid = nullptr;
		this->voxelOffset = svec3(0,0,0);
		this->imageSize = svec3(0,0,0);
		this->gridName = "<unknown grid>";
	}

	Grid::Grid(Grid::Ptr parent, svec3 size) {
		this->pImpl = nullptr;
		this->parentGrid = parent;
		this->voxelOffset = svec3(0,0,0);
		this->imageSize = size;
	}

	Grid::Grid(Grid::Ptr parent, svec3 begin, svec3 end) {
		this->pImpl = nullptr;
		this->parentGrid = parent;
		svec3 imgsize = end-begin;
		this->voxelOffset = begin;
		this->imageSize = imgsize;
	}

	ImageDataType Grid::getInternalDataType() const {
		if (this->pImpl) {
			return this->pImpl->getInternalDataType();
		}
		return ImageDataType::Unknown;
	}

	std::size_t Grid::getVoxelDimensionality() const {
		return this->pImpl->getVoxelDimensionality();
	}

	std::string Grid::getImageName() const {
		if (this->parentGrid != nullptr) { return this->parentGrid->getImageName()+"_copy"; }
		return this->pImpl->getImageName();
	}

	bool Grid::isRootGrid() const {
		return this->parentGrid == nullptr;
	}

	Grid::Ptr Grid::getParentGrid() const {
		return this->parentGrid;
	}

	Grid::Ptr Grid::requestSubRegion(svec3 begin, svec3 end) {
		return Grid::Ptr(new Grid(this->shared_from_this(), begin, end));
	}

	Grid::Ptr Grid::requestDownsampledVersion(svec3 target_size) {
		return Grid::Ptr(new Grid(this->shared_from_this(), target_size));
	}
}
