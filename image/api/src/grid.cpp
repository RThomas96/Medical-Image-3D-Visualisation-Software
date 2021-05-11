#include "../include/grid.hpp"

namespace Image {

	Grid::Grid(ImageBackendImpl::Ptr _ptr) : pImpl(std::move(_ptr)) {
		this->parentGrid = nullptr;
		this->voxelOffset = svec3(0,0,0);
		this->imageSize = svec3(0,0,0);
		//this->gridName = "<unknown grid>";
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

	constexpr bool Grid::hasValidImplementation() const {
		#warning TODO : add a isValid() function to the ImageBackendImpl
		return this->pImpl != nullptr;
	}

	ThreadedTask::Ptr Grid::updateInfoFromDisk() {
		if (this->pImpl) { return this->pImpl->parseImageInfo(); }
		ThreadedTask::Ptr task = std::make_shared<ThreadedTask>();
		task->end();
		return task;
	}

	void Grid::updateInfoFromGrid() {
		if (this->pImpl) {
			this->imageSize = this->pImpl->getResolution();
			//this->gridName = this->pImpl->getImageName();
		}
	}

	ImageDataType Grid::getInternalDataType() const {
		if (this->pImpl) { return this->pImpl->getInternalDataType(); }
		return ImageDataType::Unknown;
	}

	std::size_t Grid::getVoxelDimensionality() const {
		if (this->pImpl) { return this->pImpl->getVoxelDimensionality(); }
		return 0;
	}

	glm::vec3 Grid::getVoxelDimensions() const {
		if (this->pImpl) { return this->pImpl->getVoxelDimensions(); }
		return glm::vec3(-1.f, -1.f, -1.f);
	}

	svec3 Grid::getResolution() const {
		if (this->pImpl) { return this->pImpl->getResolution(); }
		return svec3(0, 0, 0);
	}

	std::string Grid::getImageName() const {
		if (this->parentGrid != nullptr) { return this->parentGrid->getImageName()+"_copy"; }
		return this->pImpl->getImageName();
	}

	Image::bbox_t Grid::getBoundingBox() const {
		if (this->pImpl) {
			return this->pImpl->getBoundingBox();
		} else { return Image::bbox_t(); }
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
