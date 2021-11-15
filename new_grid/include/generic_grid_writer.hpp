#ifndef VISUALIZATION_IMAGE_API_INCLUDE_WRITER_BACKEND_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_WRITER_BACKEND_HPP_

#include "../../new_grid/include/grid.hpp"
#include "../../image/generic/include/generic_image_reader.hpp"

#include <string>
#include <memory>

namespace Image {

	/// @brief This is a generic function interface for a class that writes images to disk.
	/// @details Akin to the GenericImageReader class, the GenericGenericGridWriter class is a function interface that allows to
	/// write many different kinds of image stacks to disk.
	/// @note Due to the highly variable nature of the grid it might be 'referencing', the particular behaviour of each
	/// filetype's writing procedures will take place in derived classes, like the GenericImageReader class.
	class GenericGridWriter {
		public:
			/// @brief Unique pointer type to an object of this class.
			typedef std::unique_ptr<GenericGridWriter> Ptr;

		protected:
			/// @brief A default ctor which defines a basename for the files, with the base path being the user's HOME.
			GenericGridWriter(std::string basename);

			/// @brief A 'full' ctor which defines a basename for the file, as well as a base path to save them.
			GenericGridWriter(std::string basename, std::string basepath);

		public:
			/// @brief The writer's default ctor, de-allocating any resources it holds.
			virtual ~GenericGridWriter(void) = default;

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

			/// @brief Virtual function call to write a grid's slice to disk. Specialized in derived classes.
			/// @details Takes a pointer to a grid and a slice index in parameter. It will then write the contents of
			/// this slice to disk, according to the capabilities of the paritcular writer backend implemented.
			/// @param src_grid The grid to get the contents of the image from
			/// @param slice The slice index to take from the grid and write to disk.
			/// @param task A placeholder (for now) pointer to a ThreadedTask object, in case the write op is queued.
			/// @returns true if the write operation encountered no errors, and false otherwise.
			/// @note For the moment, the write is synchronous. But a ThreadedTask object is specified here in order to
			/// be able to queue up the tasks later.
			virtual bool writeSlice(Grid::Ptr src_grid, std::size_t slice, ThreadedTask::Ptr task) noexcept(false) = 0;

			/// @brief Virtual function call to write a whole grid to disk. Specialized in derived classes.
			/// @details Takes a pointer to a grid to write to disk in parameter. It will then write the contents of
			/// the whole grid to the disk, according to the capabilities of the paritcular writer backend implemented.
			/// @param src_grid The grid to get the contents of the image from
			/// @param task A placeholder (for now) pointer to a ThreadedTask object, in case the write op is queued.
			/// @returns true if the write operation encountered no errors, and false otherwise.
			/// @note For the moment, the write is synchronous. But a ThreadedTask object is specified here in order to
			/// be able to queue up the tasks later.
			virtual bool writeGrid(Grid::Ptr src_grid, ThreadedTask::Ptr task) noexcept(false) = 0;

		protected:
			/// @brief The basename of the grids to write
			std::string file_base_name;

			/// @brief The base path of the files to write to disk.
			std::string file_base_path;
	};

} // namespace Image

#endif // VISUALIZATION_IMAGE_API_INCLUDE_WRITER_BACKEND_HPP_
