#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TEMPLATED_WRITER_BACKEND_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TEMPLATED_WRITER_BACKEND_HPP_

#include "./tiff_writer.hpp"

namespace Image {

	namespace Tiff {

		/// @ingroup tiff newgrid
		/// @brief The implementation template of the TIFFWriterBackend class.
		/// @details Allows to write TIFF files of a given type, directly to disk. Can be used to write slices of a grid
		/// or the whole grid completely. However, do note that this implementation will always write single-channel
		/// (greyscale) images. So if a grid has more than 1 channel, then there will be as many 'file stacks' as there
		/// are channels.
		template <typename element_t>
		class TIFFWriterDetail : public TIFFWriterBackend {
			public:
				/// @brief Public typedef to the internal type of the backend implementation.
				typedef element_t pixel_t;

				/// @brief Unique pointer type to an object of this class.
				typedef std::unique_ptr<TIFFWriterDetail<pixel_t>> Ptr;

			public:
				/// @brief A default ctor which defines a basename for the files, with the base path being the root.
				TIFFWriterDetail(std::string bname);

				/// @brief A 'full' ctor which defines a basename for the file, as well as a base path to save them.
				TIFFWriterDetail(std::string bname, std::string bpath);

				/// @brief The writer's default ctor, de-allocating any resources it holds.
				virtual ~TIFFWriterDetail(void);

				/// @brief Creates a suitable backend of the current type (TIFFWriterDetail<element_t>)
				static Ptr createBackend(std::string bname) {
					return std::make_unique<TIFFWriterDetail<pixel_t>>(bname);
				}

				/// @brief Creates a suitable backend of the current type (TIFFWriterDetail<element_t>)
				static Ptr createBackend(std::string bname, std::string bpath) {
					return std::make_unique<TIFFWriterDetail<pixel_t>>(bname, bpath);
				}

			public:
				/// @brief Virtual function call to write a grid's slice to disk. TIFF specialization.
				/// @details Takes a pointer to a grid and a slice index in parameter. It will then write the contents of
				/// this slice to disk, according to the capabilities of the TIFF format.
				/// @param src_grid The grid to get the contents of the image from
				/// @param slice The slice index to take from the grid and write to disk.
				/// @param task A placeholder (for now) pointer to a ThreadedTask object, in case the write op is queued.
				/// @returns true if the write operation encountered no errors, and false otherwise.
				/// @note For the moment, the write is synchronous. But a ThreadedTask object is specified here in order to
				/// be able to queue up the tasks later.
				virtual bool writeSlice(Grid::Ptr src_grid, std::size_t slice, ThreadedTask::Ptr task) noexcept(false) override;

				/// @brief Virtual function call to write a whole grid to disk. TIFF specialization.
				/// @details Takes a pointer to a grid to write to disk in parameter. It will then write the contents of
				/// the whole grid to the disk, according to the capabilities of the TIFF file format.
				/// @param src_grid The grid to get the contents of the image from
				/// @param task A placeholder (for now) pointer to a ThreadedTask object, in case the write op is queued.
				/// @returns true if the write operation encountered no errors, and false otherwise.
				/// @note For the moment, the write is synchronous. But a ThreadedTask object is specified here in order to
				/// be able to queue up the tasks later.
				virtual bool writeGrid(Grid::Ptr src_grid, ThreadedTask::Ptr task) noexcept(false) override;

			protected:
				/// @brief Writes the necessary TIFF tags to the currently opened TIFF file, based on the grid info.
				/// @param file The tiff file to write to
				/// @param grid The grid to get information from
				/// @param slice_idx The index for the slice to write to disk
				/// @param start If more than 3 samples are present, specifies the index of the first sample to write.
				/// @returns The set strip size, according to all of the other parameters set beforehand.
				std::uint32_t write_tiff_tags(TIFF* file, Grid::Ptr grid, std::size_t slice_idx, std::size_t start);

				/// @brief Reads the specified subpixel in all voxels from the source vector into the final vector.
				/// @param src The source data vector
				/// @param samples_in_src The nb of samples from the source vector. Helps with the nb pixels to copy
				/// @param idx The sample to read from every pixel
				/// @return The vector comprising of every sample in [beg, end] extracted from the source
				std::vector<pixel_t> read_subpixels_from_slice(std::vector<pixel_t>& src, std::size_t samples_in_src,
															   std::size_t idx);

				/// @brief Writes the data in strips to the TIFF file.
				/// @param file The TIFF file to write to
				/// @param data The entire data to write.
				/// @param width The width of the image
				/// @param height The number of rows to write to the file.
				/// @param estimated_strip_size The strip size, returned by TIFFDefaultStripSize() for this file
				/// @returns Returns true if the write could happen without hurdles, and false if some error occured.
				/// @details The recommended buffer size for TIFF strips is around 8KB of information (from the TIFF
				/// specification at https://www.adobe.io/content/dam/udp/en/open/standards/tiff/TIFF6.pdf , page 39.
				/// However, we alreay set the number of rows per strip in write_tiff_tags() so we only write row per
				/// row (or scanline by scanline using TIFF terminology).
				bool write_tiff_strips(TIFF* file, std::vector<pixel_t>& data, std::size_t width, std::size_t height,
									   std::uint32_t estimated_strip_size);

			protected:
				uint16_t bits_per_sample;	///< The number of bits per sample, defined by the template instanciation.
				uint16_t sample_format;		///< The sample format, defined by the template instanciation.
				uint16_t planar_config;		///< The planar configuration (config of bitplanes in file, always PACKED)
		};

	}

} // namespace Image

#include "./templated_writer_backend.impl.hpp"

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TEMPLATED_WRITER_BACKEND_HPP_
