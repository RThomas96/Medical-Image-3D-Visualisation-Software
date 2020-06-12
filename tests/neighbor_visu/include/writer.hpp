#ifndef TESTS_NEIGHBOR_VISU_INCLUDE_WRITER_HPP_
#define TESTS_NEIGHBOR_VISU_INCLUDE_WRITER_HPP_

#include "TinyTIFF/tinytiffwriter.h" // To write to TIF files

#include <iostream>
#include <fstream>
#include <vector>

class VoxelGrid; // Fwd-declaration

namespace IO {

	/// @brief A generic class to implement writing a voxel grid to disk.
	/// @details This is a generic-enough class to write voxel grids to disk : it will not be so
	/// specific it can only write grid of a certain type, but it will not be so generic you can
	/// write arbitrary to a file. As such, this base class doesn't contain any structures or
	/// pointers to structures like std::ofstream, as those might be different based on the file
	/// type you're trying to write to disk.
	class GenericGridWriter {
		protected:
			/// @brief The base constructor of a grid writer, with the base name of the
			/// file (without extensions).
			/// @param _baseName The base name of the file(s) to be written to disk.
			/// @param _binaryMode If the file should be open as binary.
			GenericGridWriter(const std::string _baseName, bool _binaryMode = false);

		public:
			/// @brief Destructor of the class. Writes file on destruction.
			/// @details Closes the ofstream, writing the file to disk upon destruction.
			~GenericGridWriter(void);

			/// @brief Writes a sequence of bytes representing the grid to the ofstream.
			/// @details This is just a default function. In IGridWriter, it does nothing.
			/// Classes that inherit from this class should override it. However, please
			/// do note this triggers a basic wrinting behaviour : if called multiple
			/// times on a file with a header, it will most probably re-write the header
			/// according to the rules defined by the overloaded function in the inherited
			/// class. Triggers an immediate writing of the file, with the comment (if
			/// specified and allowed).
			/// @param _vg The voxel grid to write the contents of.
			/// @returns A reference to *this, to chain function calls.
			virtual GenericGridWriter& write(const VoxelGrid* const _vg);

			/// @brief Sets the 'comment' to be written to the file, overwriting any previous value.
			/// @details Most file formats will not allow a comment, but if the writer you
			/// are using allows for comments (such as PNG, JPEG, TIF ...) this will be
			/// written as a comment in the correct field.
			/// @param _comment The comment to be potentially written.
			/// @return A reference to *this, to chain function calls.
			GenericGridWriter& setComment(const std::string _comment);

			/// @brief Appends 'comment' to the comment field, to be written to the file.
			/// @details Most file formats will not allow a comment, but if the writer you
			/// are using allows for comments (such as PNG, JPEG, TIF ...) this will be
			/// written as a comment in the correct field.
			/// @param _comment The comment to be potentially written.
			/// @return A reference to *this, to chain function calls.
			GenericGridWriter& appendComment(const std::string _comment);

			/// @brief Returns the size, in bytes, of data that was sent to disk.
			/// @details Most of the time : 0 before writing data, and <vectorsize> after
			/// writing. Doesn't do any difference between bytes written per file or
			/// overall, since it returns the number of bytes written at this point in
			/// time. The user has to be careful *when* he's calling this function.
			/// @return The number of bytes written as of this call.
			std::size_t sizeWritten(void) const;

		protected:
			///@brief Opens the file, according to the basename, extension and binary mode requested.
			virtual void openFile(bool _binaryMode = false);

			///@brief Opens the file, with a version or sequence number, according to the basename, extension and binary mode requested.
			virtual void openFileVersioned(std::size_t version, bool _binaryMode = false);

			/// @brief Writes the entire grid at once, as a block.
			/// @param _vg The voxel grid to write to file.
			/// @return The number of bytes written to disk.
			virtual std::size_t write_Once(const VoxelGrid* const _vg);

			/// @brief Writes the entire grid as `depth` files, according to the basename
			/// and extension set beforehand.
			/// @note If multiple files are written, this represents the toal amount of
			/// bytes written by this object.
			/// @param data The data to write.
			/// @param width The width of the voxel grid to write.
			/// @param height The height of the voxel grid to write.
			/// @param depth The depth of the voxel grid to write. Also, the number of
			/// images to write.
			/// @return The number of bytes written to disk.
			virtual std::size_t write_Depthwise(const VoxelGrid* const _vg);

		protected:
			std::string baseName; ///< Base name of the file. If multiple files are written, they will be written as `baseName_XX.extension`.
			std::string comment; ///< Comment to be written to the file, if allowed.
			std::size_t bytesWritten; ///< The total number of bytes written to disk, by this writer.
	};

	/// @brief Voxel grid writer for DIM/IMA files.
	/// @details This file format does not accept comments. This class writes the DIM/IMA combo at once, with
	/// the data provided by the voxel grid.
	class DIMWriter : public GenericGridWriter{
		public:
			/// @brief Default constructor. Opens DIM/IMA files with the basename provided.
			DIMWriter(const std::string baseName);

			/// @brief Default destructor. Closes the DIM/IMA file pair, writing it to disk.
			~DIMWriter(void);

			/// @brief Writes a sequence of bytes representing the grid to the ofstream.
			/// @details Writes the entire grid at once, as the DIM/IMA filetype represents a whole grid at once.
			/// @param _vg The voxel grid to write the contents of.
			/// @returns A reference to *this, to chain function calls.
			virtual DIMWriter& write(const VoxelGrid* const _vg) override;

		protected:
			/// @brief Opens the DIM/IMA file combo based on the basename provided.
			virtual void openFile(bool _binaryMode = false) override;

			/// @brief Writes the entire file at once.
			virtual std::size_t write_Once(const VoxelGrid* const _vg) override;

			/// @brief Writes the info about the grid to the DIM file.
			void writeDIMInfo(const VoxelGrid* const _vg);

		protected:
			std::ofstream* outputDIM; ///< A pointer to a file handle for the DIM file
			std::ofstream* outputIMA; ///< A pointer to a file handle for the IMA file
	};

	/// @brief Writes the voxel grid as a single TIF file, with multiple frames.
	/// @details The format accepts comments, but unfortunately the lib we're using (TinyTIFF) does not
	/// allow to put user-generated comments, and only allows for a generic comment to be made. See
	class SingleTIFFWriter : public GenericGridWriter {
		public:
			/// @brief Default constructor. Opens the TIFF image, and nothing else.
			SingleTIFFWriter(const std::string _baseName);

			/// @brief Default destructor. Closes the TIFF image.
			~SingleTIFFWriter(void);

			/// @brief Write the whole voxel grid to file,
			virtual SingleTIFFWriter& write(const VoxelGrid* const _vg) override;

		protected:
			void openTIFFFile(const VoxelGrid* const _vg);
		protected:
			TinyTIFFFile* tiffFile;
	};

	/// @brief Writes the voxel grid as a multitude of TIFF files, one for each depth level.
	/// @details Unlike the SingleTIFFWriter class, this class allows for an incremental writing of the
	/// voxel grid. As in, for each level generated, you could write the TIFF image associated (maybe
	/// on a separate thread ?) and have the whole grid written by the end of its generation.
	class StaggeredTIFFWriter : public GenericGridWriter {
		public:
			StaggeredTIFFWriter(const std::string _baseName);
			~StaggeredTIFFWriter(void);
		protected:
			std::vector<TinyTIFFFile*> tifFiles;
	};

}

#endif // TESTS_NEIGHBOR_VISU_INCLUDE_WRITER_HPP_
