#ifndef VISUALIZATION_IMAGE_API_BACKEND_HPP_
#define VISUALIZATION_IMAGE_API_BACKEND_HPP_

#include "image_api_common.hpp"
#include "threaded_task.hpp"
// Needed for bounding box :
#include "../../../grid/include/bounding_box.hpp"

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace Image {

	/**
	 * @brief The ImageBackendImpl class aims to provide a simple, yet usable API to query data from a set of files.
	 * @details This is only a base class, all its functionnality will be implemented in derived classes. As such, we
	 * can provide support for a wide range of data types, while having a stable API.
	 */
	class ImageBackendImpl {
		public:
			/// @b Simple typedef for a unique_ptr of an image backend.
			typedef std::unique_ptr<ImageBackendImpl> Ptr;

		protected:
			/// @b Default ctor of an image backend. Declared protected to not be instanciated alone.
			ImageBackendImpl(std::vector<std::vector<std::string>> fns);

		public:
			/// @b Default dtor of the class : frees up all allocated resources, and returns.
			virtual ~ImageBackendImpl(void) = default;

			/// @b Checks if the given image backend can read the file given in argument.
			/// @details Checks not only for the extension of the file, but could also for the file information once
			/// opened ! For example, if a TIFF file is given, but it has a tag we don't support, we can return false.
			static bool canReadImage(const std::string& image_name);

			virtual ImageDataType getInternalDataType(void) const;

			/// @b Simple call to parse images, the functionnality will be added in derived classes.
			virtual ThreadedTask::Ptr parseImageInfo(void) noexcept(false) = 0;

			/// @b Returns the number of channels of the image
			virtual std::size_t getVoxelDimensionality(void) = 0;

			/// @b Returns the image's internal representation for data.
			virtual ImageDataType getVoxelDataType(void) = 0;

			/// @b Returns the image's defined voxel resolutions, if applicable.
			virtual glm::vec3 getVoxelDimensions(void) = 0;

			/// @b Returns the dimensions of the image.
			virtual svec3 getResolution(void) = 0;

			/// @b If the file format allows and includes it, the name of the acquisition.
			/// @details Otherwise, this is the name of the first file in the stack, minus the  extension and possibly
			/// any string of identifiers for single slices (if filename = 'file_z000_c0.ext', this returns 'file').
			virtual std::string getImageName(void) const = 0;

			/// @b Returns the image bounding box, either as computed (voxel sizes x res), or defined in file.
			virtual BoundingBox_General<float> getBoundingBox(void) = 0;

			/// @b Template to return the minimum and maximum values stored in the file, if given.
			/// @note By default, returns the internal data type's min and max values.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t> void getRangeValues(glm::vec<2, data_t, glm::defaultp>& _range);

			/// @b Template to read a single pixel's value(s) in the image.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t, size_t dim> void readPixel(svec3 index, std::array<data_t, dim>& values);

			/// @b Template to read a single line of voxels in ihe image.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t, size_t dim> void readLine(svec2 line_idx, std::array<data_t, dim>& values);

			/// @b Template to read a whole slice of voxels in the image at once.
			/// @warning This function is left undefined here : it is implemented in derived classes, and
			/// trying to call it directly will lead to linker errors !
			template <typename data_t, size_t dim> void readSlice(std::size_t slice_idx, std::array<data_t, dim>& values);

		protected:
			/// @b Parse image info in a single thread. Useful for things that are _not_ parallelizable.
			virtual void parseImageInfo_sequential(ThreadedTask::Ptr&) noexcept(false) = 0;

			/// @b Simple call to parse images, the functionnality will be added in derived classes.
			/// @note This version launches the image parsing in separate thread(s), which can be implemented
			/// differently depending on the derived class' implementation.
			virtual void parseImageInfo_thread(ThreadedTask::Ptr&) noexcept(false) = 0;

		protected:
			/// @b The filenames which the backends must parse in order to load the image.
			std::vector<std::vector<std::string>> filenames;

			/// @b The internal data type representation, stored in the image.
			ImageDataType internal_data_type;
	};

} // namespace Image

#endif // VISUALIZATION_IMAGE_API_BACKEND_HPP_
