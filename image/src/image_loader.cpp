#include "../include/image_loader.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <functional>
#include <string.h>

#define USE_SEQUENTIAL_READ_FOR_LOADING true

// ImageLoader : Constructor {{{
ImageLoader::ImageLoader() : p_cores(std::thread::hardware_concurrency()) {
	this->dataLoaded = nullptr;
	this->imageWidth = 0;
	this->imageHeight = 0;
	this->stackDepth = 0;
	this->sizeOfElements = 0;
	this->numberOfElementsLoaded = 0;

	this->filenamesToLoad = nullptr;
	this->finishedLoading = false;
	this->threadPointers = nullptr;
}
// }}}

// ImageLoader : Destructor {{{
ImageLoader::~ImageLoader() {
	this->resetState();
}
// }}}

// ImageLoader : GetInstance() : Allows to retrieve the signelton of ImageLoader {{{
ImageLoader* ImageLoader::getInstance() {
	static ImageLoader imgLd;
	return &imgLd;
}
// }}}

// ImageLoader : TODO : loadInfoFrom{JSON|XML|somethingElse} {{{
void ImageLoader::loadInfoFrom(const std::string& infoFilePath) {}
void ImageLoader::loadInfoFrom(const char* infoFilePath) {}
// }}}

// ImageLoader : loadInfoDirect() : Load information about the images directly. {{{
void ImageLoader::loadInfoDirect(std::vector<QString>& filenamesToParse) {
	#ifndef NDEBUG
	std::cerr << __PRETTY_FUNCTION__ << " : Loading image info started." << '\n';
	#endif

	if (filenamesToParse.size() == 0) { return; }

	// Get the number of files
	this->nbImages = filenamesToParse.size();

	// Allocate some memory :
	this->filenamesToLoad = static_cast<char**>(calloc(this->nbImages, sizeof(char*)));

	// Keeping track of the errors :
	std::size_t missedFiles = 0;
	std::vector<std::size_t> missedIdx;

	for (std::size_t i = 0; i < this->nbImages; ++i) {
		// Get filename as const char* :
		std::string temp = filenamesToParse[i].toStdString();
		const char* tempStr = temp.c_str();

		assert(strcmp(filenamesToParse[i].toStdString().c_str(), tempStr) == 0);

		// Open the TIFF file and check their dimensions :
		TinyTIFFReaderFile* currentTiff = this->openTiffImage(tempStr);
		if (currentTiff != nullptr) {
			this->updateImageSizes(currentTiff);
			TinyTIFFReader_close(currentTiff);
		} else {
			// Put the file into the 'missed' vector
			missedFiles++;
			missedIdx.push_back(i);
		}

		// Copy into newly allocated memory space :
		this->filenamesToLoad[i] = strdup(tempStr);
	}

	// Print info about all missed files :
	if (missedFiles > 0) {
		std::cout << "Couldn't open " << missedFiles << " files : " << '\n';
		for (std::vector<std::size_t>::size_type i = 0; i < missedIdx.size(); ++i) {
			std::cout << "File " << i << " : " << this->filenamesToLoad[missedIdx[i]] << '\n';
		}
	}

	#ifndef NDEBUG
	std::cerr << __PRETTY_FUNCTION__ << " : Loaded info of " << this->nbImages - missedFiles << " images out of " << this->nbImages << " requested." << '\n';
	#endif
}
// }}}

// ImageLoader : openTiffImage() : Returns a valid handle to a TIFF file, or nullptr if an error occured. {{{
TinyTIFFReaderFile* ImageLoader::openTiffImage(const char* pathname) {
	// Early exit for wrong argument :
	if (pathname == nullptr) { return nullptr; }

	std::cerr << __PRETTY_FUNCTION__ << " : pathname sent : \"" << pathname << "\"\n";

	// Try to open the file :
	TinyTIFFReaderFile* tiffFile = TinyTIFFReader_open(pathname);
	if (!tiffFile) {
		#ifndef NDEBUG
		std::cerr << __PRETTY_FUNCTION__ << " : Error opening filename " << pathname << ".\n" <<
			"Either the file is not accessible, doesn't exist, or " <<
			"is not a supported TIFF file." << '\n';
		#else
		std::cout << "Error opening filename " << pathname << ".\n" <<
			"Either the file is not accessible, doesn't exist, or " <<
			"is not a supported TIFF file." << '\n';
		#endif
		return nullptr;
	}

	// Check for errors :
	while (TinyTIFFReader_wasError(tiffFile)) {
		#ifndef NDEBUG
		std::cerr << __PRETTY_FUNCTION__ << " : TinyTIFF Error for filename " << pathname << " : \n" <<
			'\t' << TinyTIFFReader_getLastError(tiffFile) << '\n';
		#else
		// TODO : Redirect it to another stream (in a Qt window perhaps ?)
		std::cout << "TinyTIFF Error for filename " << pathname << " : \n" <<
			'\t' << TinyTIFFReader_getLastError(tiffFile) << '\n';
		#endif
	}

	// Return a valid handle :
	return tiffFile;
}
// }}}

// ImageLoader : updateImageSizes() : Checks the image's width & height aren't bigger than the currently stored width and height {{{
void ImageLoader::updateImageSizes(TinyTIFFReaderFile* tiffImage) {
	// Early exit for invalid tiff files :
	if (tiffImage == nullptr) { return; }

	std::size_t imgW = static_cast<std::size_t>(TinyTIFFReader_getWidth(tiffImage));
	std::size_t imgH = static_cast<std::size_t>(TinyTIFFReader_getHeight(tiffImage));

	// Check if the width is bigger :
	if (imgW > this->imageWidth) {
		#ifndef NDEBUG
		std::cerr << __PRETTY_FUNCTION__ << " : updating img width from " << this->imageWidth << " to " << imgW << '\n';
		#endif
		this->imageWidth = imgW;
	}

	// Check if the height is bigger :
	if (imgH > this->imageHeight) {
		#ifndef NDEBUG
		std::cerr << __PRETTY_FUNCTION__ << " : updating img height from " << this->imageHeight << " to " << imgH << '\n';
		#endif
		this->imageHeight = imgH;
	}

	return;
}
// }}}

// ImageLoader : [THREADED] loadImages() : Launches the master thread for image loading. {{{
void ImageLoader::loadImages() {
	// If there are no files to load, abort.
	if (this->filenamesToLoad == nullptr) { return; }

#ifndef USE_SEQUENTIAL_READ_FOR_LOADING
	// If the data is already loading, don't do it twice !
	if (this->threadPointers != nullptr) { return; }
#endif

	if (this->dataLoaded != nullptr ) {
		free(this->dataLoaded);
		this->dataLoaded = nullptr;
	}

	// Compute image size to be loaded, since it is has been
	// computed when loading the image's informations :
	this->sizeOfElements = this->imageWidth * this->imageHeight;

	// Allocate the storage to load the image data :
	this->dataLoaded = static_cast<unsigned char*>(calloc(this->sizeOfElements * this->nbImages, sizeof(unsigned char)));
	memset(this->dataLoaded, uchar(255), this->sizeOfElements * this->nbImages * sizeof(unsigned char));
	this->dataLoaded[(this->sizeOfElements * this->nbImages) - 1] = '\0';
	#ifndef NDEBUG
	{
		std::lock_guard<std::mutex> ioM(this->iostreamMutex);
		std::cerr << "Length of data loaded : " << strlen((char*)this->dataLoaded) << '\n';
	}
	#endif
	// TODO : Change the above to allow for any kind of data loading (floats, uints, chars ...)

#ifndef USE_SEQUENTIAL_READ_FOR_LOADING
	// Spawn a thread :
	std::thread masterThread(&ImageLoader::thread_Supervisor, this, this);

	#ifndef NDEBUG
	std::thread::id masterThreadId = masterThread.get_id();
	#endif

	// Detach it, allowing it to continue working in the background.
	masterThread.detach();

	#ifndef NDEBUG
	std::lock_guard<std::mutex> ioM(this->iostreamMutex);
	std::cerr << __PRETTY_FUNCTION__ << " : Launched master thread with thread ID [" << masterThreadId << "]" << '\n';
	#endif
#else
	this->thread_Loader(0, this);
	#ifndef NDEBUG
	std::cerr << __PRETTY_FUNCTION__ << " : Each image is " << this->sizeOfElements << " pixels" << '\n';
	std::cerr << __PRETTY_FUNCTION__ << " : There are " << this->nbImages << " images, so we should allocate" << this->sizeOfElements * this->nbImages << " bytes" << '\n';
	std::cerr << __PRETTY_FUNCTION__ << " : Should have loaded " << this->nbImages << " images." << '\n';
	#endif

	this->finishedLoading = true;
#endif
}
// }}}

// ImageLoader : [THREADED] thread_Supervisor() : Launches the worker threads to load images, and synchronises them. {{{
void ImageLoader::thread_Supervisor(ImageLoader* object) {

	assert(this==object);

	if (object->p_cores == 0) {
		std::lock_guard<std::mutex> ioM(object->iostreamMutex);
		std::cerr << "Could not detect the number of threads on the system.\n" <<
			     "Launching image loading on one core only." << '\n';
		object->thread_Loader((std::size_t)0, object);
		return;
	}

	{
		std::lock_guard<std::mutex> ioM(object->iostreamMutex);
		std::cout << __PRETTY_FUNCTION__ << " : Entered master thread" << '\n';
		std::cout << "Master thread reports an array of " << strlen((char*)object->dataLoaded) << " elements" << '\n';
	}

	// Get the number of threads to launch :
	std::size_t threadsToSpawn = std::min(object->nbImages, static_cast<std::size_t>(object->p_cores));
	// -2 because this thread and the parent thread also exist
	object->threadPointers = static_cast<std::thread**>(malloc(threadsToSpawn * sizeof(std::thread*)));

	// Create the threads :
	for (std::size_t i = 0; i < threadsToSpawn; ++i) {
		object->threadPointers[i] = new std::thread(&ImageLoader::thread_Loader, object, i, object);
	}

	// Join them :
	for (std::size_t i = 0; i < threadsToSpawn; ++i) {
		object->threadPointers[i]->join();
	}

	// Normally, they should all have exited.
	free(object->threadPointers);
	object->threadPointers = nullptr;

	std::lock_guard<std::mutex> ioM(object->iostreamMutex);
	std::cerr << __PRETTY_FUNCTION__<< " : Signaling true for rendering" << '\n';

	std::cerr.flush();

	// Signal the images have finished loading :
	object->finishedLoading = true;
}
// }}}

// ImageLoader : thread_Loader() : Iterates on the filenames given, and loads the images. {{{
void ImageLoader::thread_Loader(std::size_t identifier, ImageLoader* object) {
	{
		std::lock_guard<std::mutex> ioM(object->iostreamMutex);
		std::cout << __PRETTY_FUNCTION__ << " : Entered thread " << identifier << '\n';
		std::cout << "Thread " << identifier << " reports an array of " << strlen((char*)object->dataLoaded) << " elements" << '\n';
	}

	std::size_t currentItem = object->numberOfElementsLoaded++;

#if not defined NDEBUG and defined USE_SEQUENTIAL_READ_FOR_LOADING
	assert(this == object);
	std::cerr << "Filenames (each " << object->sizeOfElements << " bytes) : " << object->filenamesToLoad << '\n';
	for (std::size_t i = 0; i < this->nbImages; ++i) {
		std::cerr << '\t' << "From this   : (" << &this->filenamesToLoad[i] << ") " << this->filenamesToLoad[i] << '\n';
		std::cerr << '\t' << "From object : (" << &object->filenamesToLoad[i] << ") " << object->filenamesToLoad[i] << '\n';
	}
#endif

	// Iterate while the files are not loaded :
	while (currentItem < object->nbImages) {
		// Get the filename :
		//const char* currentFileName = strdup(object->filenamesToLoad[currentItem]);
		std::cerr << __PRETTY_FUNCTION__ << " : OPEN(" << currentItem << ") : (" << &this->filenamesToLoad[currentItem] << ") \"" << this->filenamesToLoad[currentItem] << "\"\n";
		// Open the TIFF file :
		TinyTIFFReaderFile* tiffFile = object->openTiffImage(object->filenamesToLoad[currentItem]);
		if (tiffFile == nullptr) {
			std::cerr << "Couldn't open " << object->filenamesToLoad[currentItem] << '\n';
		}

		// Get a local storage buffer :
		unsigned char* localImage = static_cast<unsigned char*>(calloc(object->sizeOfElements, sizeof(unsigned char)));
		// Read a TinyTIFF frame :
		TinyTIFFReader_readFrame<uchar>(tiffFile, localImage);

		// Copy into buffer :
		memcpy(&object->dataLoaded + currentItem * object->sizeOfElements * sizeof(unsigned char), localImage, object->sizeOfElements * sizeof(unsigned char));

		free(localImage);

		std::lock_guard<std::mutex> ioM(object->iostreamMutex);
		std::cout << __PRETTY_FUNCTION__ << " : Thread " << identifier << " has loaded image " << currentItem << '\n';

		// Update the currentItem field, using a std::atomic's mutex-like behaviour
		if (object->numberOfElementsLoaded > object->nbImages) { return; } // exit if already all loaded
		currentItem = object->numberOfElementsLoaded++;
	}

	return;
}
// }}}

// ImageLoader : resetState() : Resets the state of the ImageLoader, regardless of its state. {{{
void ImageLoader::resetState() {
	this->numberOfElementsLoaded = 0;
	this->imageWidth = 0;
	this->imageHeight = 0;
	this->stackDepth = 0;
	this->sizeOfElements = 0;
	this->finishedLoading = false;

	// Delete loaded data :
	free(this->dataLoaded);
	this->dataLoaded = nullptr;
	// Delete filenames :
	for (std::size_t i = 0; i < this->stackDepth; ++i) {
		free(this->filenamesToLoad[i]);
		this->filenamesToLoad[i] = nullptr;
	}
	free(this->filenamesToLoad);
	free(this->threadPointers);
}
// }}}

/* vim: set tabstop=8 foldmethod=marker foldmarker={{{,}}} */
