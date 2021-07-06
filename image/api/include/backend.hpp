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
			ImageBackendImpl() : internal_data_type(ImageDataType::Unknown) {}

		public:
			/// @b Default dtor of the class : frees up all allocated resources, and returns.
			virtual ~ImageBackendImpl(void) = default;

			/// @b Returns the internal data type represented in the input files.
			virtual ImageDataType getInternalDataType(void) const = 0;

			/// @b Simple call to parse images, the functionnality will be added in derived classes.
			virtual ThreadedTask::Ptr parseImageInfo(ThreadedTask::Ptr pre_existing_task,
													 const std::vector<std::vector<std::string>>& _filenames
													 ) noexcept(false) = 0;

			/// @b Checks if the information present in this implementation is present on disk, or in memory.
			virtual bool presentOnDisk(void) const = 0;

			/// @b Returns the number of channels of the image
			virtual std::size_t getVoxelDimensionality(void) const = 0;

			/// @b Returns the image's defined voxel resolutions, if applicable.
			virtual glm::vec3 getVoxelDimensions(void) const = 0;

			/// @b Returns the dimensions of the image.
			virtual svec3 getResolution(void) const = 0;

			/// @b Allows to get the name of the loaded image(s).
			/// @details If the file format does not support defining the name of the grid in its files or metadata
			/// (like the TIFF format for example), then the name returned is either a previously user-defined name, or
			/// the name of the first image/file loaded.
			virtual std::string getImageName(void) const = 0;

			/// @b Allows for the user to specify a custom name for the grid.
			virtual void setImageName(std::string& _user_defined_name_) = 0;

			/// @b Returns the image bounding box, either as computed (voxel sizes x res), or defined in file.
			virtual BoundingBox_General<float> getBoundingBox(void) const = 0;

			/// @b Template to return the minimum and maximum values stored in the file, if given.
			/// @note By default, returns the internal data type's min and max values.
			/// @return True if the data could be accessed, and false if something went wrong.
			template <typename data_t>
			bool getRangeValues(std::size_t channel, glm::tvec2<data_t>& _range) {
				return this->internal_getRangeValues(tag<data_t>{}, channel, _range);
			}

			/// @b Template to read a single pixel's value(s) in the image.
			/// @return True if the data could be accessed, and false if something went wrong.
			template <typename data_t>
			bool readPixel(svec3 index, std::vector<data_t>& values) {
				svec3 read_region_size(this->getResolution());
				read_region_size.x = 1;
				read_region_size.y = 1;
				read_region_size.z = 1;
				return this->internal_readSubRegion(tag<data_t>{}, index, read_region_size, values);
			}

			/// @b Template to read a single line of voxels in ihe image.
			/// @return True if the data could be accessed, and false if something went wrong.
			template <typename data_t>
			bool readLine(svec2 line_idx, std::vector<data_t>& values) {
				svec3 read_origin = svec3(0, line_idx.x, line_idx.y);
				svec3 read_region_size(this->getResolution());
				read_region_size.y = 1;
				read_region_size.z = 1;
				return this->internal_readSubRegion(tag<data_t>{}, read_origin, read_region_size, values);
			}

			/// @b Template to read a whole slice of voxels in the image at once.
			/// @return True if the data could be accessed, and false if something went wrong.
			template <typename data_t>
			bool readSlice(std::size_t slice_idx, std::vector<data_t>& values) {
				svec3 read_origin = svec3(0, 0, slice_idx);
				svec3 read_region_size(this->getResolution());
				read_region_size.z = 1;
				return this->internal_readSubRegion(tag<data_t>{}, read_origin, read_region_size, values);
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
			virtual bool internal_readSubRegion(tag<std::uint8_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint8_t>& data) = 0;
			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint16_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint16_t>& data) = 0;
			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint32_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint32_t>& data) = 0;
			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint64_t> tag, svec3 origin, svec3 size,
									   std::vector<std::uint64_t>& data) = 0;
			/// @b Read a sub-region of the image, implemented in the derived classes.  8-bit signed version.
			virtual bool internal_readSubRegion(tag<std::int8_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int8_t>& data) = 0;
			/// @b Read a sub-region of the image, implemented in the derived classes. 16-bit signed version.
			virtual bool internal_readSubRegion(tag<std::int16_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int16_t>& data) = 0;
			/// @b Read a sub-region of the image, implemented in the derived classes. 32-bit signed version.
			virtual bool internal_readSubRegion(tag<std::int32_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int32_t>& data) = 0;
			/// @b Read a sub-region of the image, implemented in the derived classes. 64-bit signed version.
			virtual bool internal_readSubRegion(tag<std::int64_t> tag, svec3 origin, svec3 size,
									   std::vector<std::int64_t>& data) = 0;
			/// @b Read a sub-region of the image, implemented in the derived classes, single precision floating point.
			virtual bool internal_readSubRegion(tag<float> tag, svec3 origin, svec3 size,
									   std::vector<float>& data) = 0;
			/// @b Read a sub-region of the image, implemented in the derived classes, double precision floating point.
			virtual bool internal_readSubRegion(tag<double> tag, svec3 origin, svec3 size,
									   std::vector<double>& data) = 0;

			//////////////////////////////////////////////////////
			//													//
			//          VALUE RANGE GETTER FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int8_t> tag, std::size_t channel,
												 glm::tvec2<std::int8_t>& _values) = 0;
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int16_t> tag, std::size_t channel,
												 glm::tvec2<std::int16_t>& _values) = 0;
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int32_t> tag, std::size_t channel,
												 glm::tvec2<std::int32_t>& _values) = 0;
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int64_t> tag, std::size_t channel,
												 glm::tvec2<std::int64_t>& _values) = 0;
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint8_t> tag, std::size_t channel,
												 glm::tvec2<std::uint8_t>& _values) = 0;
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint16_t> tag, std::size_t channel,
												 glm::tvec2<std::uint16_t>& _values) = 0;
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint32_t> tag, std::size_t channel,
												 glm::tvec2<std::uint32_t>& _values) = 0;
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint64_t> tag, std::size_t channel,
												 glm::tvec2<std::uint64_t>& _values) = 0;
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<float> tag, std::size_t channel,
												 glm::tvec2<float>& _values) = 0;
			/// @b Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<double> tag, std::size_t channel,
												 glm::tvec2<double>& _values) = 0;

#if ENABLE_RANGE_SETTING_FUNCTIONS
			//////////////////////////////////////////////////////
			//													//
			//          VALUE RANGE SETTER FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<std::int8_t> tag, std::size_t channel,
												 glm::tvec2<std::int8_t> _values) = 0;
			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<std::int16_t> tag, std::size_t channel,
												 glm::tvec2<std::int16_t> _values) = 0;
			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<std::int32_t> tag, std::size_t channel,
												 glm::tvec2<std::int32_t> _values) = 0;
			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<std::int64_t> tag, std::size_t channel,
												 glm::tvec2<std::int64_t> _values) = 0;
			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<std::uint8_t> tag, std::size_t channel,
												 glm::tvec2<std::uint8_t> _values) = 0;
			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<std::uint16_t> tag, std::size_t channel,
												 glm::tvec2<std::uint16_t> _values) = 0;
			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<std::uint32_t> tag, std::size_t channel,
												 glm::tvec2<std::uint32_t> _values) = 0;
			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<std::uint64_t> tag, std::size_t channel,
												 glm::tvec2<std::uint64_t> _values) = 0;
			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<float> tag, std::size_t channel,
												 glm::tvec2<float> _values) = 0;
			/// @b Sets the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_setRangeValues(tag<double> tag, std::size_t channel,
												 glm::tvec2<double> _values) = 0;
#endif
		protected:
			/// @b The internal data type representation, stored in the image.
			ImageDataType internal_data_type;
	};

} // namespace Image

#endif // VISUALIZATION_IMAGE_API_BACKEND_HPP_
