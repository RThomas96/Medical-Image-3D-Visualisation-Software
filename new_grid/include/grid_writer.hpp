#ifndef VISUALIZATION_IMAGE_API_INCLUDE_WRITER_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_WRITER_HPP_

#include "../../new_grid/include/grid.hpp"
#include "../../image/interface/generic_image_reader.hpp"
#include "./generic_grid_writer.hpp"

#include <memory>
#include <string>

namespace Image {

	/// @brief The Writer class allows to write images from a grid to disk.
	/// @details Akin to the Grid class, this class uses the pImpl idiom to have multiple possible implementations that
	/// depend on the file type desired, and on the data type required. As such, this structure can write whatever kind
	/// of image stack the user desires, as long as it has a valid implementation as a backend.
	class GridWriter {
		public:
			/// @brief Shared pointer type to an object of this class.
			typedef std::shared_ptr<GridWriter> Ptr;

		public:
			/// @brief Default ctor, which moves the given pointer to implementation to its member field.
			GridWriter(GenericGridWriter::Ptr _backend);

			/// @brief default dtor for the class. De-allocates any ressources it took.
			~GridWriter(void);

		public:
			/// @brief Sets the base name of the files to a user-defined constant.
			/// @note By default, it is defined as 'grid'.
			void setBaseName(std::string basename);

			/// @brief Returns the base name of the files this writer will produce.
			/// @note By default, it is defined as 'grid'.
			std::string getBaseName(void) const;

			/// @brief Sets the base path where this writer will write files to.
			/// @note By default, it is the user's HOME directory.
			void setBasePath(std::string basepath);

			/// @brief Returns the base path where this writer will write files to.
			/// @note By default, it is the user's HOME directory.
			std::string getBasePath(void) const;

			/// @brief Replaces the backend of the writer object with a new backend type.
			void setBackend(GenericGridWriter::Ptr _new_backend);

			/// @brief Virtual function call to write a grid's slice to disk.
			/// @details Takes a pointer to a grid and a slice index in parameter. It will then write the contents of
			/// this slice to disk, according to the capabilities of the pImpl object.
			/// @param src_grid The grid to get the contents of the image from
			/// @param slice The slice index to take from the grid and write to disk.
			/// @param task A placeholder (for now) pointer to a ThreadedTask object, in case the write op is queued.
			/// @returns true if the write operation encountered no errors, and false otherwise.
			/// @note For the moment, the write is synchronous. But a ThreadedTask object is specified here in order to
			/// be able to queue up the tasks later.
			bool writeSlice(Grid::Ptr src_grid, std::size_t slice, ThreadedTask::Ptr& task) noexcept(false);

			/// @brief Virtual function call to write a whole grid to disk.
			/// @details Takes a pointer to a grid to write to disk in parameter. It will then write the contents of
			/// the whole grid to the disk, according to the capabilities of the pImpl object.
			/// @param src_grid The grid to get the contents of the image from
			/// @param task A placeholder (for now) pointer to a ThreadedTask object, in case the write op is queued.
			/// @returns true if the write operation encountered no errors, and false otherwise.
			/// @note For the moment, the write is synchronous. But a ThreadedTask object is specified here in order to
			/// be able to queue up the tasks later.
			bool writeGrid(Grid::Ptr src_grid, ThreadedTask::Ptr& task) noexcept(false);

		protected:
			/// @brief The pointer type which holds the logic for writing files to disk.
			std::experimental::propagate_const<GenericGridWriter::Ptr> pImpl;
	};

} // namespace Image

#endif // VISUALIZATION_IMAGE_API_INCLUDE_WRITER_HPP_
