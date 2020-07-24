#ifndef IMAGE_INCLUDE_WRITER_HPP_
#define IMAGE_INCLUDE_WRITER_HPP_

#include "../../TinyTIFF/tinytiffwriter.h" // To write to TIF files

#include <iostream>
#include <fstream>
#include <vector>

class DiscreteGrid; // Fwd-declaration

/// @brief A namespace for file inputs and outputs for the program.
namespace IO {

	/// @brief A generic class to implement writing a voxel grid to disk. [<F2> : See more.]
	/// @details This is a generic-enough class to write voxel grids to disk : it will not be so
	/// specific it can only write grid of a certain type, but it will not be so generic you can
	/// write arbitrary to a file. As such, this base class doesn't contain any structures or
	/// pointers to structures like std::ofstream, as those might be different based on the file
	/// type you're trying to write to disk.
	/// A generic writer should allow for a few basic operations :
	///     - opening file(s)
	///     - get the size written on disk
	///     - reading a voxel grid
	///     - writing it to a file at once
	///     - writing it to multiple files, in this case one per depth 'level'
	///     - allow to put user-generated comments, for additionnal info about the grid
	///     - provide empty virtual functions for derived/daughter classes to interface with (for
	///       additional filetype-specific functionnalities)
	/// This generic writer allows for all. It currently has a few derived classes that can write
	/// any grid in the following formats :
	///     - BrainVISA DIM/IMA files,
	///     - TIFF stacked file (1 stack = 1 file)
	///     - TIFF single-frame files (1 stack = n images)
	///     - (Soon) NIFTI files // TODO : Implement a NIFTIWriter
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
			virtual GenericGridWriter& write(const DiscreteGrid* const _vg);

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
			virtual std::size_t write_Once(const DiscreteGrid* const _vg);

			/// @brief Writes the entire grid as `depth` files, according to the basename
			/// and extension set beforehand.
			/// @details This method
			/// @note If multiple files are written, this represents the toal amount of
			/// bytes written by this object.
			/// @param data The data to write.
			/// @param width The width of the voxel grid to write.
			/// @param height The height of the voxel grid to write.
			/// @param depth The depth of the voxel grid to write. Also, the number of
			/// images to write.
			/// @return The number of bytes written to disk.
			virtual std::size_t write_Depthwise(const DiscreteGrid* const _vg, std::size_t depthChosen);

		protected:
			std::string baseName; ///< Base name of the file. If multiple files are written, they will be written as `baseName_XX.extension`.
			std::string comment; ///< Comment to be written to the file, if allowed.
			std::size_t bytesWritten; ///< The total number of bytes written to disk, by this writer.
			std::size_t depthReached; ///< The depth reached by the last write() call
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
			virtual DIMWriter& write(const DiscreteGrid* const _vg) override;

		protected:
			/// @brief Opens the DIM/IMA file combo based on the basename provided.
			virtual void openFile(bool _binaryMode = false) override;

			/// @brief Writes the entire file at once.
			virtual std::size_t write_Once(const DiscreteGrid* const _vg) override;

			/// @brief Writes the info about the grid to the DIM file.
			void writeDIMInfo(const DiscreteGrid* const _vg);

		protected:
			std::ofstream* outputDIM; ///< A pointer to a file handle for the DIM file
			std::ofstream* outputIMA; ///< A pointer to a file handle for the IMA file
	};

	/// @brief Writes the voxel grid as a single TIF file, with multiple frames.
	/// @details The format accepts comments, but unfortunately the lib we're using (TinyTIFF) does not
	/// allow to put user-generated comments, and only allows for a generic comment to be made.
	/// @note This class can only write a stack of images of the same width, height, and bitdepth.
	class SingleTIFFWriter : public GenericGridWriter {
		public:
			/// @brief Default constructor. Opens the TIFF image, and nothing else.
			SingleTIFFWriter(const std::string _baseName);

			/// @brief Default destructor. Closes the TIFF image.
			~SingleTIFFWriter(void);

			/// @brief Write the whole voxel grid to file,
			virtual SingleTIFFWriter& write(const DiscreteGrid* const _vg) override;

		protected:
			/// @brief Opens the TIFF file to consequently write to.
			void openTIFFFile(const DiscreteGrid* const _vg);

			/// @brief Writes the whole file at once.
			virtual std::size_t write_Once(const DiscreteGrid* const _vg) override;
		protected:
			TinyTIFFFile* tiffFile; ///< The tiff file to write to.
	};

	/// @brief Writes the voxel grid as a multitude of TIFF files, one for each depth level.
	/// @details Unlike the SingleTIFFWriter class, this class allows for an incremental writing of the
	/// voxel grid. As in, for each level generated, you could write the TIFF image associated (maybe
	/// on a separate thread, since we don't backtrack to another depth level whilst generating the grid
	/// ?) and have the whole grid written by the end of its generation.
	class StaggeredTIFFWriter : public GenericGridWriter {

		public:
			/// @brief Default constructor. Initializes a default name and suffix for the files.
			StaggeredTIFFWriter(const std::string _baseName);

			/// @brief Default destructor. Closes the last file, and destroys he resources
			/// associated with it.
			~StaggeredTIFFWriter(void);

			/// @brief Writes a single image, the latest generated by the voxel reconstruction
			/// algorithm.
			/// @details Each call to this function writes the latest image generated in the
			/// grid, and then increments `depthReached` by one, signifying we will then (at
			/// the next call) need to write the image at depth `depthReached`.
			virtual StaggeredTIFFWriter& write(const DiscreteGrid* const _vg) override;

		protected:
			/// @brief Opens the file corresponding to the depth level 'n', to be the next one
			/// written when called with `write()`.
			void openVersionnedTIFFFile(const DiscreteGrid* const _vg);

			/// @brief Computes the length of the suffix to append, equal to log10(maxDepth).
			/// @details Computes the total number of digits to have in the suffix, in order to
			/// have a naming hierarchy that goes 0001, 0002 ... 0010, 0011, ... 0100 [...] and
			/// not 1, 2, ..., 10, 11, ..., 100 [...] in order for the files to be displayed in
			/// the right order when sorted alphabetically in most file explorers (and in `ls`
			/// or `dir` too !)
			void computeSuffixLength(void);

			/// @brief Computes the filename to be given to the image to be written next.
			/// @details Reads the `depthReached` vairable, and creates the suffix to append to
			/// the writer's base name in order to write the versionned file, at iteration 'n'.
			/// Needs info from computed in `computeSuffixLength()` in order to write a cohesive
			/// suffix.
			std::string createSuffixedFilename(void);

			TinyTIFFFile* tifFiles; ///< A pointer to a TIFF file, which will change through the program's execution.
			std::size_t totalFiles; ///< The total number of files to be written. Gathered from the voxel grid info.
	};

	/// @brief Namespace for writer classes, giving shortcuts of the form IO::Writer::<Format/Filetype>
	namespace Writer {
		/// @brief Writer for a voxel grid, writing it to a DIM/IMA BrainVisa file combo.
		/// @see ::IO::DIMWriter
		typedef ::IO::DIMWriter DIM;

		/// @brief Writer for a voxel grid, writing it to a single TIFF file.
		/// @see ::IO::SingleTIFFWriter
		typedef ::IO::SingleTIFFWriter SingleTIFF;

		/// @brief Writer for a voxel grid, writing it to a stack of TIFF files.
		/// @see ::IO::StaggeredTIFFWriter
		typedef ::IO::StaggeredTIFFWriter MultiTIFF;
	}

}

#endif // IMAGE_INCLUDE_WRITER_HPP_
