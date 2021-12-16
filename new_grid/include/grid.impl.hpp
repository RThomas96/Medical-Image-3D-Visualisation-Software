#ifndef VISUALISATION_GRID_INCLUDE_GRID_IMPL_HPP_
#define VISUALISATION_GRID_INCLUDE_GRID_IMPL_HPP_

#ifndef VISUALIZATION_IMAGE_API_GRID_HPP_
#include "./grid.hpp"
#endif

namespace Image {

	template <typename data_t>
	bool Grid::getRangeValues(std::size_t channel, glm::vec<2, data_t, glm::defaultp>& values) {
		using return_vec_t = glm::vec<2, data_t, glm::defaultp>;
		// Checks the implementation pointer is valid, if it is return the data
		// from there, and if not return an invalid range of values :
		if (this->pImpl) {
			return this->pImpl->getRangeValues(channel, values);
		}
		// If the pointer to implementation is not here, we should probably throw or return erroneous values ...
		values = return_vec_t(std::numeric_limits<data_t>::min(), std::numeric_limits<data_t>::max());
		return false;
	}

	template <typename data_t>
	bool Grid::readPixel(svec3 index, std::vector<data_t>& values) {
		// Checks the position is valid, the backend implementation is valid and returns the value
		//if (index.x < this->imageSize.x && index.y < this->imageSize.y && index.z < this->imageSize.z) {
			if (this->pImpl) {
				return this->pImpl->readPixel(index, values);
			}
			return false;
		//}
		// position is out of bounds :
		return false;
	}

	template <typename data_t>
	bool Grid::readLine(svec2 index, std::vector<data_t>& values) {
		// Checks the position requested is valid, then calls the implementation's function if valid.
		if (index.x < this->imageSize.y && index.y < this->imageSize.z) {
			if (this->pImpl) {
				return this->pImpl->readLine<data_t>(index, values);
			}
			return false;
		}
		return false;
	}

	template <typename data_t>
	bool Grid::readSlice(std::size_t slice_idx, std::vector<data_t>& values) {
		// Checks the slice index is valid, then reads it if the implementation is valid :
		if (slice_idx < this->imageSize.z) {
			if (this->pImpl) {
				return this->pImpl->readSlice<data_t>(slice_idx, values);
				;
			}
			return false;
		}
		return false;
	}

	template <typename data_t>
	bool Grid::readSubRegion(svec3 origin, svec3 size, std::vector<data_t>& values) {
		if (this->pImpl) {
			return this->pImpl->readSubRegion<data_t>(origin, size, values);
		}
		return false;
	}

}	 // namespace Image

#endif	  // VISUALISATION_GRID_INCLUDE_GRID_IMPL_HPP_
