/**********************************************************************
 * FILE : image/include/image_loader.hpp
 * AUTH : Thibault de Villèle
 * DATE : 08/04/2020
 * DESC : An thread-safe image loader, which will load the filenames and
 *	  resolutions of images from a file (JSON, for example) and then
 *	  proceed to load them into memory, using some interpolation to
 *	  upscale all images to the same resolution (to determine later)
 **********************************************************************/

#ifndef IMAGE_INCLUDE_IMAGE_LOADER_HPP_
#define IMAGE_INCLUDE_IMAGE_LOADER_HPP_

#include "../../TinyTIFF/tinytiffreader.h"

#include <QString>

#include <atomic>
#include <string>
#include <vector>
#include <future>

// TODO : Look into TinyTIFF's loader to enable loading multiple IFDs (see http://paulbourke.net/dataformats/tiff/ for more info)
// TODO : look into why it is not possible to load images in multi-threaded mode

/**
 * @brief The ImageLoader class is a threaded image loader which loads info from a file (JSON file for example) and spits out an array of the loaded data.
 */
class ImageLoader {
	private:
		//! @brief Default constructor for the image loader.
		ImageLoader();

	public:
		//! @brief Allows for only having one instance of ImageLoader across the program
		static ImageLoader* getInstance();

		//! @brief Loads info from a JSON file provided in argument. Does NOT load the images.
		//! @param infoFilePath The path to the file containing the information needed for the images
		void loadInfoFrom(const std::string& infoFilePath);

		//! @brief Loads info from a JSON file provided in argument. Does NOT load the images.
		//! @param infoFilePath The path to the file containing the information needed for the images
		void loadInfoFrom(const char* infoFilePath);

		//! @brief Load images filenames from user input directly. Opens a file dialog to load as many pictures as the user needs.
		//! @param filenames Names (paths) of files to load
		void loadInfoDirect(std::vector<QString>& filenames);

		//! @brief Returns the images' width.
		inline std::size_t getImageWidth() const { return this->imageWidth; }

		//! @brief Returns the images' height.
		inline std::size_t getImageHeight() const { return this->imageHeight; }

		//! @brief Returns the stack depth.
		inline std::size_t getStackDepth() const { return this->stackDepth; }

		//! @brief Returns the number of images loaded (maximum, no matter the state of loading).
		inline std::size_t getNbImages() const { return this->nbImages; }

		//! @brief Loads images into a separate thread, allowing the main program to do whatever it wants.
		void loadImages();

		//! @brief Signals if the threads have finished loading the image data.
		bool hasFinishedLoading() const { return this->finishedLoading; }

		//! @brief Get the image data in the format desired.
		//! @note Upon retrieving the loaded data, it sets `finishedLoading`
		//! to false, in order to get the image data only once and wait for
		//! another loading afterwards.
		//! @warning Uses a reinterpret_cast<>()
		template <typename arrType> arrType* getImageDataAs() {
			if (this->finishedLoading == false) {
				return nullptr;
			}
			this->finishedLoading = false;
			return reinterpret_cast<arrType*>(this->dataLoaded);
		}

		//! @brief Resets the state of the image loader, deleting all data.
		//! @warning Does not take into account the fact that some threads
		//! might still be working in the background. Should only be used
		//! with extreme caution, might cause unexpected behavior !
		void resetState();

		//! @brief De-allocates the storage used up by the image loader.
		~ImageLoader();

	private:
		//! @brief Loading supervisor. A single thread responsible for launching, managing, and destroying the worker threads.
		void thread_Supervisor(ImageLoader* object);

		//! @brief Worker thread whose only job is to load images incrementally.
		//! @param identifier A simple identifier, only used to track the launch/delete sequence of threads
		void thread_Loader(std::size_t identifier, ImageLoader* object);

		//! @brief Opens up a TIF[F] image using TinyTIFF, and returns a valid handle or nullptr in case of any error
		//! @param pathname The path (relative or absolute) from which to load the image
		TinyTIFFReaderFile* openTiffImage(const char* pathname);

		//! @brief Updates the fields imageWidht and imageHeight if necessary, using TinyTIFF getter functions.
		//! @param tiffFile The TIFF handle from which to check the height and width of the image
		void updateImageSizes(TinyTIFFReaderFile* tiffImage);

		//! @brief Number of elements (here, pictures) loaded into memory. Used during thread loading, in order to track which file is the next one to load.
		std::atomic_size_t numberOfElementsLoaded;

		//! @brief Mutex for allowing to print to stdout asynchronously
		std::mutex iostreamMutex;

		//! @brief Width of the images in the stack
		std::size_t imageWidth;

		//! @brief Height of the images in the stack
		std::size_t imageHeight;

		//! @brief Depth of the image stack
		std::size_t stackDepth;

		//! @brief Since we might load two stacks (or more/less, mind you) this number can be different than stackDepth.
		std::size_t nbImages;

		//! @brief Size of all the elements loaded in memory. Computed once image{Width|Height} and stackDepth have been determined.
		std::size_t sizeOfElements;

		//! @brief The maximum number of threads we can spawn on the machine, determined once the instance of ImageLoader is created.
		const unsigned int p_cores;

		//! @brief Used to keep track of the threads currently executing.
		std::thread** threadPointers;

		// TODO : add two elements : image depth (8 bit or whatever) and maybe type used to load it (float, char ...) ?
		//! @brief Array of file paths to load
		char** filenamesToLoad;

		//! @brief Array of loaded data, stored in void* in order to allow for arbitrary data to be loaded (floats, uints, chars ...)
		unsigned char* dataLoaded;

		//! @brief Signals the end of the loading threads.
		bool finishedLoading;
};

#endif // IMAGE_INCLUDE_IMAGE_LOADER_HPP_
