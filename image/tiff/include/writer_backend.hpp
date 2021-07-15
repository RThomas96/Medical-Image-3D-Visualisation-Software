#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_WRITER_BACKEND_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_WRITER_BACKEND_HPP_

#include "./backend.hpp"

#include "../../api/include/grid.hpp"
#include "../../api/include/backend.hpp"
#include "../../api/include/writer_backend.hpp"

namespace Image {

	namespace Tiff {

		/// @b Re-implementation of the writer backend class for the TIFF type.
		/// @note For now, does not re-implement any of the functionnalities of the WriterBackendImpl class, as the
		/// important parts of the code happen in the templated classes behind this code.
		class TIFFWriterBackend : public WriterBackendImpl {
			public:
				/// @b Unique pointer type to an object of this class.
				typedef std::unique_ptr<TIFFWriterBackend> Ptr;

			protected:
				/// @b A default ctor which defines a basename for the files, with the base path being the user's HOME.
				TIFFWriterBackend(std::string basename);

				/// @b A 'full' ctor which defines a basename for the file, as well as a base path to save them.
				TIFFWriterBackend(std::string basename, std::string basepath);

			public:
				/// @b The writer's default ctor, de-allocating any resources it holds.
				virtual ~TIFFWriterBackend(void);

			protected:
				/// @b Builds a filename composed of the base name, channel index and slice index (in that order).
				/// @details For example, if the basename is "tiff_grid" and the slice and channel indices are 75 and 1
				/// respectively, then the returned filename will be : "tiff_grid_1_75.tiff".
				std::string build_iterative_filename(std::size_t slice_idx, std::size_t channel);

				/// @b Attempts to open a TIFF file with the given '_name', along with the given 'permissions' (default is
				/// wb, write binary because windows requires binary).
				/// @returns A valid handle to a TIFF file with the given permissions, or nullptr.
				TIFF* open_file(std::string _name, std::string permissions = "wb");

				/// @b Closes the given file, previously opened by libTIFF's TIFFOpen()
				void close_file(TIFF* handle);
		};

	} // namespace Tiff

} // namespace Image

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_WRITER_BACKEND_HPP_
