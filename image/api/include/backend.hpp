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
			/// @b Simple tag which discriminates which function to call when called from a template.
			/// @details Allow to have multiple implementations of a function foo() (for different data types, in this
			/// case) which can thus be called from a function `template<> foo<>()`. This allows to leave the foo()
			/// implementation to derived classes, where the information actually is.
			template <typename T> struct restrict_tag {};

		protected:
			/// @b Default ctor of an image backend. Declared protected to not be instanciated alone.
			ImageBackendImpl(const std::vector<std::vector<std::string>>& fns);

		public:
			/// @b Default dtor of the class : frees up all allocated resources, and returns.
			virtual ~ImageBackendImpl(void) = default;

			/// @b Checks if the given image backend can read the file given in argument.
			/// @details Checks not only for the extension of the file, but could also for the file information once
			/// opened ! For example, if a TIFF file is given, but it has a tag we don't support, we can return false.
			static bool canReadImage(const std::string& image_name);

			/// @b Returns the internal data type represented in the input files.
			virtual ImageDataType getInternalDataType(void) const = 0;

			/// @b Simple call to parse images, the functionnality will be added in derived classes.
			virtual ThreadedTask::Ptr parseImageInfo(void) noexcept(false) = 0;

			/// @b Checks if the information present in this implementation is present on disk, or in memory.
			virtual bool presentOnDisk(void) const = 0;

			/// @b Returns the number of channels of the image
			virtual std::size_t getVoxelDimensionality(void) const;

			/// @b Returns the image's defined voxel resolutions, if applicable.
			virtual glm::vec3 getVoxelDimensions(void) const;

			/// @b Returns the dimensions of the image.
			virtual svec3 getResolution(void) const;

			/// @b If the file format allows and includes it, the name of the acquisition.
			/// @details Otherwise, this is the name of the first file in the stack, minus the  extension and possibly
			/// any string of identifiers for single slices (if filename = 'file_z000_c0.ext', this returns 'file').
			virtual std::string getImageName(void) const = 0;

			/// @b Returns the image bounding box, either as computed (voxel sizes x res), or defined in file.
			virtual BoundingBox_General<float> getBoundingBox(void) const = 0;

			/// @b Template to return the minimum and maximum values stored in the file, if given.
			/// @note By default, returns the internal data type's min and max values.
			/// @return True if the data could be accessed, and false if something went wrong.
			template <typename data_t>
			bool getRangeValues(glm::vec<2, data_t, glm::defaultp>& _range);

			/// @b Template to read a single pixel's value(s) in the image.
			/// @return True if the data could be accessed, and false if something went wrong.
			template <typename data_t>
			bool readPixel(svec3 index, std::vector<data_t>& values);

			/// @b Template to read a single line of voxels in ihe image.
			/// @return True if the data could be accessed, and false if something went wrong.
			template <typename data_t>
			bool readLine(svec2 line_idx, std::vector<data_t>& values) {
				svec3 read_origin = svec3(0, line_idx.x, line_idx.y);
				svec3 read_region_size(this->getResolution());
				read_region_size.y = 1;
				read_region_size.z = 1;
				return this->internal_readSubRegion(::Image::tag<data_t>{}, read_origin, read_region_size, values);
			}

			/// @b Template to read a whole slice of voxels in the image at once.
			/// @return True if the data could be accessed, and false if something went wrong.
			template <typename data_t>
			bool readSlice(std::size_t slice_idx, std::vector<data_t>& values) {
				svec3 read_origin = svec3(0, 0, slice_idx);
				svec3 read_region_size(this->getResolution());
				read_region_size.z = 1;
				return this->internal_readSubRegion(::Image::tag<data_t>{}, read_origin, read_region_size, values);
			}

			/// @b Template to read a sub-region of the voxels in the image at once.
			/// @return True if the data could be accessed, and false if something went wrong.
			template <typename data_t>
			bool readSubRegion(svec3 read_origin, svec3 read_size, std::vector<data_t>& values) {
				return this->internal_readSubRegion(read_origin, read_size, values);
			}

		protected:
			//////////////////////////////////////////////////////
			//													//
			//             SUB-REGION READ FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @b Read a sub-region of the image, implemented in the derived classes.  8-bit unsigned version.
			virtual bool internal_readSubRegion(::Image::tag<std::uint8_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint8_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit unsigned version.
			virtual bool internal_readSubRegion(::Image::tag<std::uint16_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint16_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit unsigned version.
			virtual bool internal_readSubRegion(::Image::tag<std::uint32_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint32_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit unsigned version.
			virtual bool internal_readSubRegion(::Image::tag<std::uint64_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint64_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes.  8-bit   signed version.
			virtual bool internal_readSubRegion(::Image::tag<std::int8_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int8_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit   signed version.
			virtual bool internal_readSubRegion(::Image::tag<std::int16_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int16_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit   signed version.
			virtual bool internal_readSubRegion(::Image::tag<std::int32_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int32_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit   signed version.
			virtual bool internal_readSubRegion(::Image::tag<std::int64_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int64_t>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes, single precision floating point.
			virtual bool internal_readSubRegion(::Image::tag<float> tag, svec3 origin, svec3 size,
									   std::vector<float>& data) = 0;

			/// @b Read a sub-region of the image, implemented in the derived classes, double precision floating point.
			virtual bool internal_readSubRegion(::Image::tag<double> tag, svec3 origin, svec3 size,
									   std::vector<double>& data) = 0;

		protected:
			/// @b The filenames of the implementation.
			std::vector<std::vector<std::string>> filenames;

			/// @b The internal data type representation, stored in the image.
			ImageDataType internal_data_type;

			/// @b The image resolution, as parsed by the discrete implementation in derived classes.
			svec3 imageResolution;

			/// @b The number of components per voxel. Its extraction depends on the derived implementation used.
			std::size_t dimensionality;

			/// @b The voxel dimensions, as parsed by the discrete implementation in derived classes.
			glm::vec3 voxelDimensions;
	};

} // namespace Image

#endif // VISUALIZATION_IMAGE_API_BACKEND_HPP_
