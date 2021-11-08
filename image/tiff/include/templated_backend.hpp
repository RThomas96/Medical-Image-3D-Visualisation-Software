#ifndef VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_HPP_
#define VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_HPP_

#include "./TIFFReaderInterface.hpp"

namespace Image {

namespace Tiff {

	/// @ingroup tiff newgrid
	/// @brief The implementation template of the TIFFReader class.
	/// @details This is the templated version of the TIFFReader class, that actually interacts with the TIFF files
	/// on disk. This allows to read files of any internal data types and provide access to them in a Grid object.
	/// @tparam element_type The fundamental type used for reading. Supports `std::[u]int{8|16|32|64}_t|double|float`.
	template <typename element_type>
	class TIFFReaderTemplated : public TIFFReaderInterface {

		public:
			/// @brief The type of the data loaded in.
			typedef element_type pixel_t;

			/// @brief Redefinition of the pointer type for this particular class.
			typedef std::unique_ptr<TIFFReaderTemplated<element_type>> Ptr;

			/// @brief Typedef for the cache structure which holds recently accessed data.
			typedef ReadCache<std::size_t, std::vector<element_type>> cache_t;

		protected:
			/// @brief Default ctor for the class, initializing the right variables.
			TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim);

		public:
			/// @brief Default dtor for the class, removing all allocated data from the program's memory pool.
			virtual ~TIFFReaderTemplated(void);

			/// @brief Static function, creating a backend with the given specs.
			static Ptr createBackend(uint32_t width, uint32_t height, std::size_t dim);

		protected:
			/// @brief Cleans up the allocated resources after an error has occured.
			/// @note Can also be called in the dtor.
			virtual void cleanResources(void) override;

			/// @brief Parse the filename's information into a separate thread.
			/// @note The ThreadedTask pointer is a shared resource that allows to know the progress of the task.
			virtual void parse_info_in_separate_thread(ThreadedTask::Ptr _task, const std::vector<std::vector<std::string>>& _f) override;

			/// @brief Checks the currently generated object is compatible with the given TIFF frame as reference.
			bool is_frame_compatible_with_backend(ThreadedTask::Ptr& _task, Frame::Ptr& reference_frame) const;

			/// @brief Computes the basename of the stack, based on the filenames.
			/// @details Defined as the first image's file name, without the path, extension and (possibly) numerical
			/// suffixe(s) (can be separated by a dash, underscore or something else). If the name is fully numerical,
			/// can also be computed by using similarities between the names of the different filenames.
			/// @todo For now, only returns 'default_grid_name' and should be implemented more thoroughly.
			void compute_stack_basename(void);

			/// @brief Asks the cache for a given slice 's'. If already cached, return the data but load it otherwise.
			/// @note Asking to load slices this way loads the _whole slice_. If it's made up of 10 samples across
			/// multiple images, it's gonna load that and cache the result.
			std::size_t loadSlice(std::size_t s);

			/// @brief Reads data, converted in the right type into the given vector from a specific subregion of the image.
			/// @returns True if the region was successfully read, and false otherwise.
			template <typename out_t>
			bool template_tiff_read_sub_region(svec3 origin, svec3 size, std::vector<out_t>& values);

			/// @brief Reads the min and max ranges of the image, if available.
			/// @note If not available in the metadata, returns the internal type's min and max values.
			/// @warning For now, sets the range values implicitely, without explicit or safe casting.
			/// @todo Implement a 'safe' version of glm::convert_to<>() that clips to the smallest type's limits.
			template <typename range_t>
			bool template_tiff_get_sub_range_values(std::size_t channel, glm::vec<2, range_t, glm::defaultp>& _values);

			//////////////////////////////////////////////////////
			//													//
			//             SUB-REGION READ FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @brief Read a sub-region of the image, implemented in the derived classes.  8-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint8_t> tag, svec3 origin, svec3 size,
									std::vector<std::uint8_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @brief Read a sub-region of the image, implemented in the derived classes. 16-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint16_t> tag, svec3 origin, svec3 size,
									std::vector<std::uint16_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @brief Read a sub-region of the image, implemented in the derived classes. 32-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint32_t> tag, svec3 origin, svec3 size,
									std::vector<std::uint32_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @brief Read a sub-region of the image, implemented in the derived classes. 64-bit unsigned version.
			virtual bool internal_readSubRegion(tag<std::uint64_t> tag, svec3 origin, svec3 size,
									std::vector<std::uint64_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @brief Read a sub-region of the image, implemented in the derived classes.  8-bit   signed version.
			virtual bool internal_readSubRegion(tag<std::int8_t> tag, svec3 origin, svec3 size,
									std::vector<std::int8_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @brief Read a sub-region of the image, implemented in the derived classes. 16-bit   signed version.
			virtual bool internal_readSubRegion(tag<std::int16_t> tag, svec3 origin, svec3 size,
									std::vector<std::int16_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @brief Read a sub-region of the image, implemented in the derived classes. 32-bit   signed version.
			virtual bool internal_readSubRegion(tag<std::int32_t> tag, svec3 origin, svec3 size,
									std::vector<std::int32_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @brief Read a sub-region of the image, implemented in the derived classes. 64-bit   signed version.
			virtual bool internal_readSubRegion(tag<std::int64_t> tag, svec3 origin, svec3 size,
									std::vector<std::int64_t>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @brief Read a sub-region of the image, implemented in the derived classes, single precision floating point.
			virtual bool internal_readSubRegion(tag<float> tag, svec3 origin, svec3 size,
									std::vector<float>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}
			/// @brief Read a sub-region of the image, implemented in the derived classes, double precision floating point.
			virtual bool internal_readSubRegion(tag<double> tag, svec3 origin, svec3 size,
									std::vector<double>& data) override {
				UNUSED(tag);
				return this->template_tiff_read_sub_region(origin, size, data);
			}

			//////////////////////////////////////////////////////
			//													//
			//          VALUE RANGE GETTER FUNCTIONS            //
			//													//
			//////////////////////////////////////////////////////

			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int8_t> tag, std::size_t channel,
												glm::tvec2<std::int8_t>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int16_t> tag, std::size_t channel,
												glm::tvec2<std::int16_t>& _values) override	{
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int32_t> tag, std::size_t channel,
												glm::tvec2<std::int32_t>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::int64_t> tag, std::size_t channel,
												glm::tvec2<std::int64_t>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint8_t> tag, std::size_t channel,
												glm::tvec2<std::uint8_t>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint16_t> tag, std::size_t channel,
												glm::tvec2<std::uint16_t>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint32_t> tag, std::size_t channel,
												glm::tvec2<std::uint32_t>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<std::uint64_t> tag, std::size_t channel,
												glm::tvec2<std::uint64_t>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<float> tag, std::size_t channel,
												glm::tvec2<float>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}
			/// @brief Reads the range of the loaded data, if specified by the file format or a subsequent image analysis.
			virtual bool internal_getRangeValues(tag<double> tag, std::size_t channel,
												glm::tvec2<double>& _values) override {
				UNUSED(tag);
				return this->template_tiff_get_sub_range_values(channel, _values);
			}

		protected:
			/// @brief The cached slices, in order to speed up offline computation.
			cache_t cachedSlices;

			/// @brief The range of values contained in each channel, if defined or computed.
			/// @details By default, contains the min and max value of the represented types, but can be set
			/// by an external operation (like histogram extraction) or by the user directly.
			std::vector<glm::tvec2<pixel_t>> value_ranges;
	};

}

}

#include "./templated_backend.impl.hpp"

#endif // VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_HPP_
