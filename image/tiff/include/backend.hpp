#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HPP_

#include "./tiff_frame.hpp"
#include "./tiff_private.hpp"

#include "../../api/include/image_api_common.hpp"
#include "../../api/include/backend.hpp"
#include "../../api/include/read_cache.hpp"
#include "../../api/include/threaded_task.hpp"

namespace Image {

	/**
	 * @brief The TIFFBackend class allows to read information from one or more TIFF (virtual) stacks directly.
	 * @details This class will parse the TIFF files, generate adequate frame descrptions for each of the detected
	 * frames and allow random access to data, no matter its dimensionality, number of slices or resolution.
	 * The image resolution, voxel dimensions, and other metadata (such as the internal filetype) will be decided by
	 * the metadata of the _first frame_ of the _first file_ given. This will be the reference frame, from which all
	 * other frames will be compared.
	 * @warning This class supports only reading data from single-channel TIFF frames. If more channels are requested,
	 * those additional channels will have to be provided using another TIFF frames (multiple _frames_ can live inside
	 * a single TIFF _file_).
	 */
	class TIFFBackend : public ImageBackendImpl {
		protected:
			/// @b Default ctor for the implementation, simply calling its superclass ctor.
			TIFFBackend(const std::vector<std::vector<std::string>>& fns);

		public:
			/// @b Default dtor for the class
			virtual ~TIFFBackend(void) = default;

			/// @b Returns true if the TIFF file provided can be read by this image implementation.
			/// @note This static function only works for _basic TIFF files_. The OME-TIFF backend implementation which
			/// inherits from this class has its own implementation of this.
			static bool canReadImage(const std::string& image_name);

			/// @b Creates a backend implementation which can read the data, if possible. Returns nullptr otherwise.
			static ImageBackendImpl::Ptr createBackend(std::vector<std::vector<std::string>> fns);

			/// @b Simple call to parse images given in the ctor.
			virtual ThreadedTask::Ptr parseImageInfo(void) noexcept(false) override;

			/// @b Get the number of elements present in each voxel.
			virtual std::size_t getVoxelDimensionality(void) const override;

			/// @b Returns the image's defined voxel resolutions, if applicable.
			virtual glm::vec3 getVoxelDimensions(void) const override;

			/// @b Returns the dimensions of the image.
			virtual svec3 getResolution(void) const override;

			/// @b Returns the image bounding box, either as computed (voxel sizes x res), or defined in file.
			virtual BoundingBox_General<float> getBoundingBox(void) const override;

			/// @b Template to return the minimum and maximum values stored in the file, if given.
			/// @note For the regular TIFF files, returns the internal data type's min and max values.
			/// @note This will call `pImpl->getRangeValues<>();`.
			template <typename data_t> void getRangeValues(glm::vec<2, data_t, glm::defaultp>& _range);

			/// @b Template to read a single pixel's value(s) in the image.
			/// @note This will call `pImpl->readPixel<>();`.
			template <typename data_t> void readPixel(svec3 index, std::vector<data_t>& values);

			/// @b Template to read a single line of voxels in ihe image.
			/// @note This will call `pImpl->readLine<>();`.
			template <typename data_t> void readLine(svec2 line_idx, std::vector<data_t>& values);

			/// @b Template to read a whole slice of voxels in the image at once.
			/// @note This will call `pImpl->readSlice<>();`.
			template <typename data_t> void readSlice(std::size_t slice_idx, std::vector<data_t>& values);
		protected:
			/// @b Parses the information from the images in a separate thread.
			/// @warning Can only work for this TIFF implementation, not any derived classes ! (OME-TIFF)
			virtual void parseImageInfo_thread(ThreadedTask::Ptr& task) override;

		protected:
#ifdef PIMPL_USE_EXPERIMENTAL_PROPAGATE_CONST
			/// @b The pointer which can interface directly with the files on disk.
			std::experimental::propagate_const<Tiff::TIFFPrivate::Ptr> pImpl;
#else
			/// @b The pointer which can interface directly with the files on disk.
			Tiff::TIFFPrivate::Ptr pImpl
#endif
	};

	/// @b Returns the data read by the pImpl pointer in this class.
	template <typename data_t>
	void TIFFBackend::getRangeValues(glm::vec<2, data_t, glm::defaultp>& _range) {
		if (this->pImpl) { this->pImpl->getRangeValues<data_t>(_range); }
		return;
	}

	/// @b Returns the data read by the pImpl pointer in this class.
	template <typename data_t>
	void TIFFBackend::readPixel(svec3 index, std::vector<data_t>& values) {
		if (this->pImpl) { this->pImpl->readPixel<data_t>(index, values); }
		return;
	}

	/// @b Returns the data read by the pImpl pointer in this class.
	template <typename data_t>
	void TIFFBackend::readLine(svec2 index, std::vector<data_t>& values) {
		if (this->pImpl) { this->pImpl->readLine<data_t>(index, values); }
		return;
	}

	/// @b Returns the data read by the pImpl pointer in this class.
	template <typename data_t>
	void TIFFBackend::readSlice(std::size_t index, std::vector<data_t>& values) {
		if (this->pImpl) { this->pImpl->readSlice<data_t>(index, values); }
		return;
	}

} // namespace Image

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_HPP_
