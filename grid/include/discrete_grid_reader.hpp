#ifndef IMAGE_INCLUDE_READER_HPP_
#define IMAGE_INCLUDE_READER_HPP_
// Program wide features and macros :
#include "../../macros.hpp"
#include "../../features.hpp"
// Grid bounding box and reader interpolator structs :
#include "../../grid/include/bounding_box.hpp"
#include "../include/interpolator.hpp"
// TinyTIFF header :
#include <tinytiffreader.h>
// libTIFF header :
#include <tiffio.h>
// NIFTI header(s) :
#include <nifti/nifti1_io.h>
//#include <nifti/nifti2_io.h>
// GLM header :
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <memory>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>

namespace IO {

	/// @ingroup discreteGrid
	/// @brief The Threaded task class is a simple progress tracker for a task in a different thread.
	/// @warning This class has been superseeded by the Image::ThreadedTask class.
	/// @details This class allows to know how many steps the task has left, and if it's finished or not. Here, a task
	/// is defined as any callable in a spearate thread. Requires manual intervention (does not track progress
	/// automatically, see reference functions).
	class ThreadedTask {
		public:
			using Ptr = std::shared_ptr<ThreadedTask>;
		public:
			/// @brief Ctor for a threaded task.
			ThreadedTask(std::size_t _maxSteps = 0) : m_lock() {
				this->maxSteps = _maxSteps;
				this->currentStep = 0;
			}
			/// @brief Default dtor for the class.
			~ThreadedTask(void) = default;
			/// @brief Checks if the task is complete.
			bool isComplete(void) {
				bool retval = false;
				if (this->m_lock.try_lock_for(std::chrono::milliseconds(100))) {
					retval = (this->maxSteps > std::size_t(0)) && (this->currentStep >= this->maxSteps-1);
					this->m_lock.unlock();
				}
				return retval;
			}
			/// @brief Allows to immediately end a task.
			void end(void) {
				if (this->m_lock.try_lock_for(std::chrono::milliseconds(100))) {
					if (this->maxSteps == 0) {
						this->maxSteps = 1;
						this->currentStep = 2;
					} else {
						this->currentStep = this->maxSteps+1;
					}
					this->m_lock.unlock();
				}
				return;
			}
			/// @brief Check if the task has steps.
			bool hasSteps(void) {
				bool retval = false;
				if (this->m_lock.try_lock_for(std::chrono::milliseconds(100))) {
					retval = this->maxSteps > std::size_t(0);
					this->m_lock.unlock();
				}
				return retval;
			}
			/// @brief Get the maximum number of steps possible
			std::size_t getMaxSteps(void) {
				std::size_t retval = 0;
				if (this->m_lock.try_lock_for(std::chrono::milliseconds(100))) {
					retval = this->maxSteps;
					this->m_lock.unlock();
				}
				return retval;
			}
			/// @brief Set the max number of steps for the task
			void setSteps(std::size_t _ms) {
				if (this->m_lock.try_lock_for(std::chrono::milliseconds(100))) {
					this->maxSteps = _ms;
					this->m_lock.unlock();
				}
				return;
			}
			/// @brief Get current advancement of the task
			std::size_t getAdvancement(void) {
				std::size_t retval = 0;
				if (this->m_lock.try_lock_for(std::chrono::milliseconds(100))) {
					retval = this->currentStep;
					this->m_lock.unlock();
				}
				return retval;
			}
			void setAdvancement(std::size_t newcurrentvalue) {
				if (this->m_lock.try_lock_for(std::chrono::milliseconds(100))) {
					this->currentStep = newcurrentvalue;
					this->m_lock.unlock();
				}
				return;
			}
			/// @brief Advances a step (thread-safe)
			void advance(void) {
				if (this->m_lock.try_lock_for(std::chrono::milliseconds(100))) {
					this->currentStep++;
					this->m_lock.unlock();
				}
				return;
			}
		protected:
			std::timed_mutex m_lock;				/// @brief The mutex resposible for thread-safety.
			std::atomic<std::size_t> currentStep;	/// @brief The current number of steps achieved
			std::size_t maxSteps;					/// @brief The maximum number of steps. If 0, task has not been initialized.
	};

	/// @ingroup discreteGrid
	/// @brief Very simple read cache which supports arbitrary indexes and data arrays.
	/// @warning This class has been superseeded by the Image::ReadCache class.
	/// @details This class allows to keep a few user-allocated data vectors, mostly used to keep slices of a grid in
	/// memory. Once the capacity (defined in maxCachedElements) is reached, the first element (chronologically) that
	/// was inserted is removed. This makes the class act as a FIFO data structure.
	template <typename cache_idx, typename cache_data>
	struct ReadCache {
		public:
			/// @brief Type alias to the internal index representation
			using index_t = cache_idx;

			/// @brief Type alias to the internal data representation
			using data_t_ptr = std::shared_ptr<cache_data>;
		protected:
			/// @brief The internal structuring of data in the cache vector
			using cached_data_t = std::pair<index_t, data_t_ptr>;

		public:
			/// @brief Default ctor. Allocates just enough memory for the empty struct.
			ReadCache(void) : m_data(0), lastInsertedElement(0) {}

			/// @brief Default dtor. Deallocates any elements
			~ReadCache(void) { this->clearCache(); }

			/// @brief Returns true if the cache has the data named referenced by Index 'x'
			bool hasData(const index_t searched) const {
				// For this, we don't need to conform to the lastInsertedElement index.
				// Just check we have the data requested :
				for (std::size_t i = 0; i < this->m_data.size(); ++i) {
					if (this->m_data[i].first == searched) { return true; }
				}
				return false;
			}

			/// @brief Returns a reference to the data at Index 'i'
			data_t_ptr getData(const index_t searched) const {
				// Check if we have the data, and if we do return it immediately :
				for (std::size_t i = 0; i < this->m_data.size(); ++i) {
					if (this->m_data[i].first == searched) { return this->m_data[i].second; }
				}
				// Otherwise, return a nullptr :
				return nullptr;
			}

			/// @brief Loads the data into the cache, cleearing up a space if necessary.
			void loadData(const index_t index, data_t_ptr& data) {
				// If we already have filled the vector, wrap around with the help of lastInsertedElement :
				if (this->m_data.size() == this->maxCachedElements) {
					// Might need to wrap around :
					if (this->lastInsertedElement == this->maxCachedElements-1) { this->lastInsertedElement = 0; }
					else { this->lastInsertedElement++; }
					// Remove the element in the place of lastInsertedElement, and replace it with the new data :
					this->m_data[this->lastInsertedElement].first = index;
					this->m_data[this->lastInsertedElement].second.swap(data);
				} else {
					// Otherwise, just call emplace_back() to add to the vector :
					this->lastInsertedElement = this->m_data.size();
					this->m_data.emplace_back(index, data);
				}
				return;
			}

			/// @brief Clears the cache manually.
			void clearCache(void) {
				// Reset the shared_ptrs so they can be deleted later (once they're all freed) :
				for (std::pair<index_t, data_t_ptr>& cached : this->m_data) { cached.second.reset(); }
				// Clear the vector :
				this->m_data.clear();
			}

		protected:
			///  @brief The maximum number of elements we can have stored at any time during the cache's lifetime
			constexpr static std::size_t maxCachedElements = 16;

			/// @brief The actual cached data.
			std::vector<cached_data_t> m_data;

			/// @brief The position of the last inserted element in the vector of data.
			std::size_t lastInsertedElement;
	};

	/// @brief Describes a downsampling level to apply when loading the image.
	enum DownsamplingLevel {
		Original = 0,	///< Does not downsample an image upon loading.
		Low = 1,		///< Downsamples the image using a 2x2x2 sub-region for one pixel.
		Lower = 2,		///< Downsamples the image using a 4x4x4 sub-region for one pixel.
		Lowest = 3		///< Downsamples the image using a 8x8x8 sub-region for one pixel.
	};

	/// @ingroup discreteGrid
	/// @brief Redirection for TIFF errors and warnings, which suppresses them.
	/// @note The function signature is made to be compatible with both TIFFErrorHandler and TIFFWarningHandler.
	void nullify_tiff_errors(const char* module, const char* fmt, va_list _va_);

	/// @ingroup discreteGrid
	/// @brief Checks if the file given in argument exists
	/// @param filename The name of the file to check
	bool FileExists(const char* filename);

	/// @ingroup discreteGrid
	/// @brief Returns the file base name (the name, without the extension at the end).
	/// @param filename The full name of the file, possibly with an extension or leading paths
	/// @warning If the file doesn't have an extension, returns nullptr.
	char* FileBaseName(const char* filename);

	/// @ingroup discreteGrid
	/// @brief Appends the required extension to the provided base name
	/// @param basename The base name of the file
	/// @param extension The extension to append to it.
	/// @return A new char array containing <basename>.<extension>, or nullptr if an error occured
	char* AppendExtension(char* basename, const char* extension);

	/// @ingroup discreteGrid
	/// @brief This class implements a basic reader to load data from disk directly into a voxel grid.
	/// @details This is the base class of many reader implementations. It defines a function interface to query data
	/// from disk. However, it can only work with one data type, and cannot be changed unless recompiled with another
	/// format in mind. Code is not automatically adapted to the new format, and should not be used anymore.
	/// @see Image::Grid Image::ImageReaderInterface
	/// @warning This class is kept here as legacy code is being migrated. Do not use it in any new code.
	class GenericGridReader {
		public:
			/// @brief Vector to store grid dimensions.
			typedef glm::vec<3, std::size_t, glm::defaultp> sizevec3;
			#ifdef VISUALISATION_USE_UINT8
			/// @brief Data type to be loaded from disk into memory.
			using data_t = unsigned char;
			#endif
			#ifdef VISUALISATION_USE_UINT16
			/// @brief Data type to be loaded from disk into memory.
			using data_t = uint16_t;
			#endif
			/// @brief Type of bounding box used in this class.
			typedef BoundingBox_General<float> bbox_t;

		protected:
			/// @brief Constructor for the class.
			/// @details Declared as protected in order not to have any instances of this class
			/// created, instead of instance based on derived classes.
			GenericGridReader(data_t threshold);

		public:
			/// @brief Destructor of the class. Closes files and frees up memory taken by the loaded data.
			/// @details Closes all filestreams, and clears all data associated with the loader.
			virtual ~GenericGridReader(void);

			/// @brief Set the threshold at which a voxel is considered data.
			virtual GenericGridReader& setDataThreshold(data_t _thresh);

			/// @brief Sets the filenames to load.
			virtual GenericGridReader& setFilenames(std::vector<std::string>& names);

			/// @brief Sets an interpolation structure to generate the image data upon loading.
			virtual GenericGridReader& setInterpolationMethod(std::shared_ptr<Interpolators::genericInterpolator<data_t>>& ptr);

			virtual GenericGridReader& setUserIntensityLimits(data_t min, data_t max);

			/// @brief Pre-compute some image data, such as size, voxel dimensions (...)
			virtual GenericGridReader& parseImageInfo(ThreadedTask::Ptr& task);

			/// @brief Starts the image loading process.
			virtual GenericGridReader& loadImage(ThreadedTask::Ptr& task);

			/// @brief Returns the whole data loaded, at once.
			virtual const std::vector<data_t>& getGrid(void) const;

			/// @brief Get the "raw" grid dimensions from file info.
			virtual sizevec3 getGridDimensions(void) const;

			/// @brief Get the estimated grid size, in bytes.
			virtual std::size_t getGridSizeBytes() const;

			/// @brief Get the grid's voxel dimensions, once loaded.
			virtual glm::vec3 getVoxelDimensions(void) const;

			/// @brief Get the grid's voxel dimensions, once loaded.
			virtual glm::vec3 getOriginalVoxelDimensions(void) const;

			/// @brief Get the associated transform of the grid, if any is provided.
			/// @note If none are provided, this will return an identity matrix.
			virtual glm::mat4 getTransform(void) const;

			/// @brief Get the pixel at the position (i,j,k)
			/// @note By default, this is a pure virtual function.
			virtual data_t getPixel(std::size_t i, std::size_t j, std::size_t k) = 0;

			/// @brief Gets the pixel at position (pos), in image space.
			/// @note By default, this is a pure virtual function.
			virtual data_t getPixel_ImageSpace(glm::vec4 pos) = 0;

			/// @brief Get the bounding box of the grid.
			virtual bbox_t getBoundingBox(void) const;

			/// @brief Get the grid's data bounding box.
			/// @note Computed according to the value set in `GenericGridReader::threshold`.
			virtual bbox_t getDataBoundingBox(void) const;

			/// @brief Returns the threshold from which data is considered information.
			virtual data_t getDataThreshold(void) const;

			/// @brief Returns the texture min/max values contained in the image.
			virtual glm::vec<2, data_t, glm::defaultp> getTextureLimits(void) const;

			/// @brief Swaps the contents from this grid's data to the target vector.
			virtual GenericGridReader& swapData(std::vector<data_t>& target);

			/// @brief Enables downsampling upon image loading at level '_level'.
			virtual GenericGridReader& enableDownsampling(DownsamplingLevel _level);

			/// @brief Get the currently enabled downsampling level.
			virtual DownsamplingLevel downsamplingLevel(void);

			/// @brief Return the filenames associated with this reader.
			virtual std::vector<std::string> getFilenames(void) const;

			/// @brief Sets the voxel size to the user-defined values
			virtual GenericGridReader& setUserVoxelSize(float _x, float _y, float _z);

		protected:
			/// @brief Open the file with the given name, and load its contents into memory.
			virtual GenericGridReader& openFile(const std::string& name);

			/// @brief Load an single full-res slice from the images, in all (possible) formats.
			/// @warning Might be very heavy in some binary or compressed file formats.
			virtual GenericGridReader& loadSlice(std::size_t idx, std::vector<data_t>& tgt);

		protected:
			/// @brief Checks if the files have been analysed
			bool isAnalyzed;
			/// @brief Signals the user has specified custom bounds
			bool hasUserBounds;
			/// @brief Filenames to open images from.
			std::vector<std::string> filenames;
			/// @brief Data loaded from images.
			std::vector<data_t> data;
			/// @brief 'raw' image dimensions, untouched by downsampling
			sizevec3 imageDimensions;
			/// @brief Grid dimensions (resolution of the grid).
			sizevec3 gridDimensions;
			/// @brief Dimensions of voxels, if provided. Otherwise, set to unit volume.
			glm::vec3 voxelDimensions;
			/// @brief Multiplier to apply to the voxel dimensions if the image set is downsampled upon loading.
			glm::vec3 voxelMultiplier;
			/// @brief Transform to apply to the grid, if any is provided. Identity matrix otherwise.
			glm::mat4 transform;
			/// @brief Bounding box of the grid, in the grid's space.
			bbox_t boundingBox;
			/// @brief Bounding box of the data in the grid, dictated by the grid threshold.
			bbox_t dataBoundingBox;
			/// @brief Minimum value from which data is considered valuable.
			data_t threshold;
			/// @brief Name of the grid, if provided. Otherwise, empty string.
			std::string name;
			/// @brief Tracks if the data needs to be downsampled upon loading.
			DownsamplingLevel downsampleLevel;
			/// @brief The minimum and maximum values of the texture.
			glm::vec<2, data_t, glm::defaultp> textureLimits;
			/// @brief The user-provided limits for the ROI.
			glm::vec<2, data_t, glm::defaultp> userLimits;
			/// @brief Structure to interpolate the data in the loaded images
			std::shared_ptr<Interpolators::genericInterpolator<data_t>> interpolator;
	};

	/// @ingroup discreteGrid
	/// @brief The specialization of GenericGridReader for DIM/IMA files.
	/// @warning This class is kept here as legacy code is being migrated. Do not use it in any new code.
	class DIMReader : public GenericGridReader {
		public:
			DIMReader(data_t _thresh);
			virtual ~DIMReader(void);

			virtual DIMReader& parseImageInfo(ThreadedTask::Ptr& task) override;

			/// @brief Loads the image from disk. If no filenames are provided, loads nothing.
			virtual DIMReader& loadImage(ThreadedTask::Ptr& task) override;
		protected:
			/// @brief Open the DIM and IMA files to read later.
			virtual DIMReader& openFile(const std::string& name) override;
			/// @brief Load a slice of the grid into memory.
			virtual DIMReader& loadSlice(std::size_t idx, std::vector<data_t>& tgt) override;
			virtual data_t getPixel(std::size_t i, std::size_t j, std::size_t k) override;
			virtual data_t getPixel_ImageSpace(glm::vec4 pos) override;

		protected:
			/// @brief The DIM file
			std::ifstream* dimFile;
			/// @brief The IMA file
			std::ifstream* imaFile;
	};

	/// @ingroup discreteGrid
	/// @brief The specialization of GenericGridReader for TIFF images, using the TinyTIFF library.
	/// @warning This class is kept here as legacy code is being migrated. Do not use it in any new code.
	class StackedTIFFReader : public GenericGridReader {
		public:
			StackedTIFFReader(data_t thresh);
			virtual ~StackedTIFFReader(void);

			virtual StackedTIFFReader& parseImageInfo(ThreadedTask::Ptr& task) override;

			/// @brief Loads the image from disk. If no filenames are provided, does nothing.
			virtual StackedTIFFReader& loadImage(ThreadedTask::Ptr& task) override;

		protected:
			/// @brief Preallocates the data vector and updates some data we can gather before loading the images.
			/// @details Loads the first image, takes its dimensions on X and Y and then opens each file to check
			/// the number of frames they have. Then, preallocates the whole grid to load the images faster.
			/// Also updates the grid's bounding box, as well as the grid dimensions & the voxel dimensions.
			virtual StackedTIFFReader& preAllocateStorage();

			/// @brief Opens the specified file to be able to read it later.
			virtual StackedTIFFReader& openFile(const std::string& filename) override;;

			/// @brief Loads the image at index 'idx' in the filenames in memory.
			virtual StackedTIFFReader& loadSlice(std::size_t idx, std::vector<data_t>& tgt) override;

			/// @brief Returns the point at the index (i,j,k).
			virtual data_t getPixel(std::size_t i, std::size_t j, std::size_t k) override;
			virtual data_t getPixel_ImageSpace(glm::vec4 pos) override;

		protected:
			TinyTIFFReaderFile* tiffFile;	///< Currently opened file, TinyTIFF's handle.
			std::size_t currentFile;	///< Currently opened file, indexed.
			/// @brief Index of grid slices to [TIFF filename, TIFF frame].
			/// @details Contains a pair of <i, j> where 'i' is the filename index in the available names,
			/// and 'j' is the index of the TIFF frame inside this file. This vector is indexed according
			/// to the grid slices. For example, if you want the slice 'N' in the full 3D image, you get
			/// the 'N'-th pair of indices, and get filenames[i].frame[j] to get the data.
			/// @note For the moment, this index is built but unused.
			std::vector<std::pair<std::size_t, std::size_t>> sliceToFilename;
	};

	/// @ingroup discreteGrid
	/// @brief The specialization of GenericGridReader for TIFF images, using libTIFF.
	/// @warning This class is kept here as legacy code is being migrated. Do not use it in any new code.
	class libTIFFReader : public GenericGridReader {
		private:
			struct TIFFFrame {
					friend struct TIFFStack;
				protected:
					/// @brief Default ctor. Initializes member variables to default values.
					TIFFFrame(void);
				public:
					TIFFFrame(std::string filename, tdir_t index) noexcept(false);
					~TIFFFrame(void);
				public: // Methods
					/// @brief Loads the file's information, once it has been loaded.
					/// @throws Can throw an exception if the file is in planar mode (PLANARCONFIG==2)
					void loadTIFFInfo(tdir_t index) noexcept(false);
					/// @brief Prefix for the printing of values for this frame
					void printInfo(std::string prefix);
				public:
					/// @brief The TIFF file to query for information
					std::string filename;
					/// @brief This frame's width
					uint32_t width;
					/// @brief This frame's height
					uint32_t height;
					/// @brief The number of rows per strip.
					uint32_t rowsPerStrip;
					/// @brief This frame's directory index
					tdir_t directoryOffset;
					/// @brief This frame's sample count
					uint16_t samplesPerPixel;
					/// @brief This frame's bits per sample
					std::vector<uint16_t> bitsPerSample;
					/// @brief The number of strips of this image
					uint64_t stripsPerImage;
			};
		public:
			libTIFFReader(data_t thresh);
			virtual ~libTIFFReader(void);

			/// @brief Pre-computes image data, and throws if the image is not valid.
			virtual libTIFFReader& parseImageInfo(ThreadedTask::Ptr& task) noexcept(false) override;

			/// @brief Loads the image from disk. If no filenames are provided, does nothing.
			virtual libTIFFReader& loadImage(ThreadedTask::Ptr& task) override;

		protected:
			/// @brief Opens all files and checks the data is valid
			virtual libTIFFReader& preAllocateStorage();

			/// @brief Opens the specified file to be able to read it later.
			virtual libTIFFReader& openFile(const std::string& filename) override;

			/// @brief Loads the image at index 'idx' in the filenames in memory.
			virtual libTIFFReader& loadSlice(std::size_t idx, std::vector<data_t>& tgt) override;

			/// @brief Returns the point at the index (i,j,k).
			virtual data_t getPixel(std::size_t i, std::size_t j, std::size_t k) override;
			virtual data_t getPixel_ImageSpace(glm::vec4 pos) override;

		protected:
			/// @brief The frames contained in this stack of images
			std::vector<TIFFFrame> frames;
			/// @brief The frame data type (once loaded, this is what it is loaded as)
			using frame_data_t = typename std::vector<GenericGridReader::data_t>;
			/// @brief A cache, retaining the last few frames loaded
			ReadCache<std::size_t, frame_data_t> cache;
	};

	/// @ingroup discreteGrid
	/// @brief Overload of the basic libTIFF reader, which implements a custom parsing function reading the header.
	/// @warning This class is kept here as legacy code is being migrated. Do not use it in any new code.
	class OMETIFFReader : public libTIFFReader {
		public:
			OMETIFFReader(data_t thresh);
			virtual ~OMETIFFReader(void) = default;
		public:
			virtual OMETIFFReader& parseImageInfo(ThreadedTask::Ptr& task) noexcept(false) override;
	};

	/// @ingroup discreteGrid
	/// @brief The specialization of GenericGridReader for NiftI images, using the nifti2 library.
	/// @warning This class is kept here as legacy code is being migrated. Do not use it in any new code.
	class NIFTIReader : public GenericGridReader {
		public:
			NIFTIReader(data_t thresh);
			virtual ~NIFTIReader(void);

			/// @brief Parses the image given by the user.
			virtual NIFTIReader& parseImageInfo(ThreadedTask::Ptr& task) noexcept(false) override;
			virtual NIFTIReader& openFile(const std::string& filename) override;
			virtual void preAllocateStorage();
		protected:
			///	@brief The image struct used to read NIFTI images.
			nifti_image* image;
	};

	namespace Reader {
		/// @brief Alias for the IO::DIMReader class.
		typedef ::IO::DIMReader DIM;
		/// @brief Alias for the IO::libTIFFReader class.
		typedef ::IO::libTIFFReader TIFF;
		/// @brief Alias for the IO::OMETIFFReader class.
		typedef ::IO::OMETIFFReader OME_TIFF;
	}
}

#endif // IMAGE_INCLUDE_READER_HPP_
