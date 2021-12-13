#include "../include/grid.hpp"

#include "../../image/generic/include/generic_image_downsampler.hpp"
#include "../../image/generic/include/generic_image_reader_subregion.hpp"

namespace Image {

	Grid::Grid(GenericImageReader::Ptr _ptr) :
		pImpl(std::move(_ptr)), grid_transforms(std::make_shared<TransformStack>()) {
		this->parentGrid  = nullptr;
		this->voxelOffset = svec3(0, 0, 0);
		this->imageSize	  = svec3(0, 0, 0);
	}

	Grid::Grid(Grid::Ptr parent, svec3 size, ImageResamplingTechnique sampler) :
		Grid(Image::Downsampled::createBackend(parent, size, sampler)) {
		this->parentGrid  = parent;
		this->voxelOffset = svec3(0, 0, 0);
		this->imageSize	  = size;
	}

	Grid::Grid(Grid::Ptr parent, svec3 begin, svec3 end) :
		Grid(Image::SubRegion::createBackend(begin, end - begin, parent)) {
		this->parentGrid  = parent;
		svec3 imgsize	  = end - begin;
		this->voxelOffset = begin;
		this->imageSize	  = imgsize;
	}

	constexpr bool Grid::hasValidImplementation() const {
#warning TODO : add a isValid() function to the GenericImageReader
		return this->pImpl != nullptr;
	}

	ThreadedTask::Ptr Grid::updateInfoFromDisk(const std::vector<std::vector<std::string>>& filenames) {
		ThreadedTask::Ptr task = std::make_shared<ThreadedTask>();
		if (this->pImpl) {
			return this->pImpl->parseImageInfo(task->getPtr(), filenames);
		}
		return task;
	}

	void Grid::updateInfoFromGrid() {
		if (this->pImpl) {
#warning Invalid for subsampled regions ! Should not be updated if thats the case.
			this->imageSize = this->pImpl->getResolution();
			//this->gridName = this->pImpl->getImageName();
		}
	}

	ImageDataType Grid::getInternalDataType() const {
		if (this->pImpl) {
			return this->pImpl->getInternalDataType();
		}
		return ImageDataType::Unknown;
	}

	std::size_t Grid::getVoxelDimensionality() const {
		if (this->pImpl) {
			return this->pImpl->getVoxelDimensionality();
		}
		return 0;
	}

	glm::vec3 Grid::getVoxelDimensions() const {
		if (this->pImpl) {
			return this->pImpl->getVoxelDimensions();
		}
		return glm::vec3(-1.f, -1.f, -1.f);
	}

	svec3 Grid::getResolution() const {
		if (this->pImpl) {
			return this->pImpl->getResolution();
		}
		return svec3(0, 0, 0);
	}

	std::string Grid::getImageName() const {
		if (this->parentGrid != nullptr) {
			return this->parentGrid->getImageName() + "_copy";
		}
		return this->pImpl->getImageName();
	}

	Image::bbox_t Grid::getBoundingBox() const {
		if (this->pImpl) {
			MatrixTransform::Ptr grid_transform_pointer = std::dynamic_pointer_cast<MatrixTransform>(this->getPrecomputedMatrix());
			glm::mat4 transform							= grid_transform_pointer->matrix();
			return this->pImpl->getBoundingBox().transformTo(transform);
		} else {
			return Image::bbox_t();
		}
	}

	TransformStack::Ptr Grid::getTransformStack() const {
		// return a _copy_ of the smart pointer to the stack :
		return TransformStack::Ptr(this->grid_transforms);
	}

	MatrixTransform::Ptr Grid::getPrecomputedMatrix() const {
		// return a _copy_ of the matrix transform pointer
		return MatrixTransform::Ptr(this->grid_transforms->getPrecomputedMatrix());
	}

	void Grid::addTransform(ITransform::Ptr _transform_to_add) {
		this->grid_transforms->pushTransform(_transform_to_add);
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

	Grid::Ptr Grid::requestDownsampledVersion(svec3 target_size, ImageResamplingTechnique sampler) {
		return Grid::Ptr(new Grid(this->shared_from_this(), target_size, sampler));
	}

}	 // namespace Image
